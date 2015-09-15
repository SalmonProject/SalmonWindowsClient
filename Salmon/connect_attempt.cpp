//Copyright 2015 The Salmon Censorship Circumvention Project
//
//This file is part of the Salmon Client (Windows).
//
//The Salmon Client (Windows) is free software; you can redistribute it and / or
//modify it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//
//The full text of the license can be found at:
//http://www.gnu.org/licenses/gpl.html

#include "stdafx.h"
#include <Windows.h>
#include <WindowsX.h>

#include <time.h>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <set>
using std::set;

#include "salmon_constants.h"
#include "salmon_globals.h"
#include "hwnds.h"

#include "salmon_utility.h"
#include "salmon_config.h"
#include "localization.h"
#include "email.h"
#include "control_softether.h"

#include "connect_attempt.h"

bool checkGoogleHTTPS();
bool checkMicrosoftUpdateHTTPS();
bool connectedToInternet()
{
	if (gChosenLanguage == SALMON_LANGUAGE_ZH)
		return checkMicrosoftUpdateHTTPS();
	else
		return checkGoogleHTTPS(); 
}

//Shows, on the connection status STATIC of the main window, how many servers
//the current connection attempt has left to go before it gives up.
void showServersLeft(int numLeft)
{
	wchar_t connCountStr[40];
	wchar_t connStatusWithCount[300];
	_itow(numLeft, connCountStr, 10);
	wcscpy(connStatusWithCount, localizeConst(VPN_STATUS_CONNECTING));
	wcscat(connStatusWithCount, L" (");
	wcscat(connStatusWithCount, connCountStr);
	wcscat(connStatusWithCount, L" servers left)");
	Static_SetText(sttcConnectStatus, connStatusWithCount);
}

//Try to connect to any server in the knownServers list. If we can't connect
//to any, record (in the returned object) which addresses we tried, and more
//specifically which had some sort of error, and which just seem offline. 
ConnectAnyVPNAttemptResult tryConnectAnyServer()
{
	ConnectAnyVPNAttemptResult ret;

	if (gKnownServers.empty())
		return ret; //false

	time_t now;

	bool nowTryingSkippedOnes = false;
	bool okToNeedServer = true;

	vector<int> tryLater;

TrySomeServers:
	for (int ind = 0; ind< (nowTryingSkippedOnes ? tryLater.size() : gKnownServers.size()); ind++)
	{
		int i = (nowTryingSkippedOnes ? tryLater[ind] : ind);

		showServersLeft(nowTryingSkippedOnes ? tryLater.size() - ind : (gKnownServers.size() - i) + tryLater.size());

		//First, just give up if the user cancelled the connection attempt.
		if (gCancelConnectionAttempt)
			return ret;

		//A failed connection attempt takes several seconds, so we really need to skip ones that are likely to be down. So, have 
		//a random interval of [2,7] days after a failed attempt before that server is tried again. We want to skip likely-down 
		//ones most of the time so as not to make the user sit through dozens of seconds of connection attempts, but also, in 
		//case a server has better performance than some always-up server, we should give it occasional chances to come back.
		//And of course, all of this is complemented by the purging system.
		//(And also of course, if none of the first ones we try can connect, we go through and try the rest anyways, so it's safe.)
		time(&now);
		if (!nowTryingSkippedOnes && now - gKnownServers[i].lastAttempt <= gKnownServers[i].secondsTilNextAttempt)
		{
			tryLater.push_back(i);
			continue;
		}

		ConnectServerStatus conStatus = connectToVPNServer(gKnownServers[i]);
		if (conStatus == CONNECT_SERVER_SUCCESS)
		{
			gKnownServers[i].failureCount = 0;
			ret.connectedSuccessfully = true;
			return ret;
		}
		//Although the server appears offline, it might be that we have spotty internet. Before reporting that the server is 
		//blocked, check+double-check internet connectivity, and double check the server. If for any server, both the check
		//and double-check of internet connectivity fail, then we WILL NOT send a needServer, and will instead tell the user
		//that their internet appears to be temporarily down.
		else if (conStatus == CONNECT_SERVER_OFFLINE)
		{
			if(connectedToInternet())
			{
				ConnectServerStatus conTry2Status = connectToVPNServer(gKnownServers[i]);
				if (conTry2Status == CONNECT_SERVER_SUCCESS)
				{
					gKnownServers[i].failureCount = 0;
					ret.connectedSuccessfully = true;
					return ret;
				}
				else if(conTry2Status == CONNECT_SERVER_OFFLINE && connectedToInternet())
				{	
					ret.triedAddrs.insert(gKnownServers[i].addr);
					gKnownServers[i].lastAttempt = now;
					gKnownServers[i].failureCount++;
				}
				else if (conTry2Status == CONNECT_SERVER_OFFLINE)
				{
					//If we've detected internet connectivity problems, it is NOT ok to make a (probably erroneous) needServer request.
					okToNeedServer = false;
				}
				else if (conStatus == CONNECT_SERVER_ERROR)
				{
					ret.serverErrorAddrs.insert(gKnownServers[i].addr);
					gKnownServers[i].lastAttempt = now;
					gKnownServers[i].failureCount++;
				}
				//else if(conStatus == CONNECT_SERVER_CLIENT_ERROR)
					//just pretend we didn't try; this lets needServer fix our bad entry
			}
			else //If we've detected internet connectivity problems, it is NOT ok to make a (probably erroneous) needServer request.
				okToNeedServer = false;
		}
		else if (conStatus == CONNECT_SERVER_ERROR)
		{
			ret.serverErrorAddrs.insert(gKnownServers[i].addr);
			gKnownServers[i].lastAttempt = now;
			gKnownServers[i].failureCount++;
		}
		//else if(conStatus == CONNECT_SERVER_CLIENT_ERROR)
			//just pretend we didn't try; this lets needServer fix our bad entry
	}

	if (!ret.connectedSuccessfully && !nowTryingSkippedOnes)
	{
		nowTryingSkippedOnes = true;
		goto TrySomeServers;
	}

	//If we've detected internet connectivity problems, it is NOT ok to make a (probably erroneous) needServer request.
	//We can get the correct behavior (all servers tried, but no needServer generated) by pretending the user cancelled the 
	//request right after all servers were tried, but before tryConnectAnyServer() returned.
	if (!okToNeedServer)
	{
		MessageBox(NULL, localizeConst(GAVE_UP_DUE_TO_CONNECTIVITY_PROBLEMS), L"", MB_OK);
		ret.connectedSuccessfully = false; //(should not be necessary, but just to be explicit)
		gCancelConnectionAttempt = true;
		return ret;
	}

	//NOTE: this function call is not reached if the user cancels the attempt, and that's ok.
	writeSConfigFromKnownServers();

	return ret;
}

//Makes the needServer request text: we need to tell the directory about all of the servers we failed to connect to.
std::wstring buildNeedServerRequestString(const ConnectAnyVPNAttemptResult& res)
{
	string needServerString = "needServer";
	if (!res.triedAddrs.empty())
	{
		needServerString += "\n^*tried:^*";
		for (set<string>::const_iterator itty = res.triedAddrs.begin(); itty != res.triedAddrs.end(); itty++)
			needServerString += ("\n" + *itty);
	}

	if (!res.serverErrorAddrs.empty())
	{
		needServerString += "\n^*error:^*";
		for (set<string>::const_iterator itty = res.serverErrorAddrs.begin(); itty != res.serverErrorAddrs.end(); itty++)
			needServerString += ("\n" + *itty);
	}

	WCHAR* convertNeedServer = new WCHAR[needServerString.length() + 1];
	mbstowcs(convertNeedServer, needServerString.c_str(), needServerString.length() + 1);
	std::wstring passNeedServerString(convertNeedServer);
	delete convertNeedServer;

	return passNeedServerString;
}

//Tries to get a new server from the directory server. Returns a status code (enum NeedServerSuccess).
//writes the gotten IP address into ipAddrBuf if successful. This function blocks until the dir server responds.
NeedServerSuccess needServer(const ConnectAnyVPNAttemptResult& res, vector<VPNInfo>* VPNGateServers)
{
	std::wstring needServerRequest = buildNeedServerRequestString(res);

	SyncMailReturn sendMailReply = sendAndRecvMailBlocking(needServerRequest);
	if (sendMailReply.sendStatus == SEND_MAIL_FAIL)
	{
		MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
		return NEED_SERVER_GOT_NONE;
	}

	if (sendMailReply.receivedAnything && strchr(sendMailReply.response.c_str(), '$')) //dir server indicating an error. (but might be including VPN Gate servers.)
	{
		localizeDirServMsgBox(sendMailReply.response.c_str(), localizeConst(ERROR_STR));
		ShowWindow(wndwWaiting, SW_HIDE);

		bool gotAnyVPNGates = false;

		//if the directory server couldn't give us any salmon servers, it might instead give some VPN gate servers. 
		const char* curVPNgate = strstr(sendMailReply.response.c_str(), "VPNGATE");
		while (curVPNgate)
		{
			parseVPNGateItem(curVPNgate, VPNGateServers);
			gotAnyVPNGates = true;

			if (gCancelConnectionAttempt)
				break;

			//advance to next item, if there is another
			curVPNgate = strstr(curVPNgate + 1, "VPNGATE");
		}

		//forget any servers the directory server wants us to forget
		executeAllPurges(sendMailReply.response.c_str());

		if (gotAnyVPNGates)
			return NEED_SERVER_GOT_VPNGATE;
		else
			return NEED_SERVER_GOT_NONE;
	}
	else if (sendMailReply.receivedAnything) //received some sort of non-$error response; try to parse it.
	{
		if (!parseNewSalmonServer(sendMailReply.response))
			return NEED_SERVER_GOT_NONE;

		//forget any servers the directory server wants us to forget
		executeAllPurges(sendMailReply.response.c_str());

		return NEED_SERVER_GOT_SALMON;
	}
	else
		return NEED_SERVER_GOT_NONE;
}

DWORD WINAPI connectionAttemptThread(LPVOID lpParam)
{
	WaitForSingleObject(gConnectionStateMutex, INFINITE);

	//try to read SConfig.txt - or generate it if necessary.
	if (!getSConfig())
		MessageBox(NULL, localizeConst(COULDNT_READ_CONFIG_FILE), localizeConst(ERROR_STR), MB_OK);

	//try all of our servers. if we can't connect to any of them...
	ConnectAnyVPNAttemptResult res = tryConnectAnyServer();
	if (gCancelConnectionAttempt && !res.connectedSuccessfully)
		goto EndConnAttemptThread;
	else if (!res.connectedSuccessfully)
	{
		//... then send the "needServer" message to the directory server, and wait for a response.
		MessageBox(NULL, localizeConst(COULDNT_CONNECT_WILL_needServer), L"", MB_OK);

		if (gCancelConnectionAttempt)
			goto EndConnAttemptThread;

		vector<VPNInfo> VPNGateServers; //if needServer gets VPNGate servers for us to process, this is how it tells us about them
		NeedServerSuccess gotAnyServers = needServer(res, &VPNGateServers);
		if (gotAnyServers == NEED_SERVER_GOT_NONE)
		{
			MessageBox(NULL, localizeConst(SORRY_NO_SERVERS_AVAILABLE), localizeConst(ERROR_STR), MB_OK);
			goto EndConnAttemptThread;
		}
		else if (gotAnyServers == NEED_SERVER_GOT_SALMON)
		{
			//Now try connecting again. The "last attempt" logic should mean that only the new server gets tried.
			ConnectAnyVPNAttemptResult res2 = tryConnectAnyServer();
			if (gCancelConnectionAttempt && !res2.connectedSuccessfully)
				goto EndConnAttemptThread;
			else if (!res2.connectedSuccessfully)
				MessageBox(NULL, localizeConst(SORRY_NO_SERVERS_AVAILABLE), localizeConst(ERROR_STR), MB_OK);
		}
		else if(gotAnyServers == NEED_SERVER_GOT_VPNGATE)
		{
			//Try each of the received VPN Gate servers. If one works, it gets saved to knownServers+SConfig.
			for (int i = 0; i < VPNGateServers.size(); i++)
			{
				showServersLeft(VPNGateServers.size() - i);
				addVPNInfo(VPNGateServers[i]);
				int indexKnownServers = 0;
				for (; indexKnownServers < gKnownServers.size(); indexKnownServers++)
					if (!strcmp(gKnownServers[indexKnownServers].addr, VPNGateServers[i].addr))
						break;
				if (indexKnownServers >= gKnownServers.size())
					goto EndConnAttemptThread;

				if (connectToVPNServer(gKnownServers[indexKnownServers]) == CONNECT_SERVER_SUCCESS)
				{
					gKnownServers[indexKnownServers].failureCount = 0;
					writeSConfigFromKnownServers();
					break;
				}
				else
					gKnownServers.erase(gKnownServers.begin() + indexKnownServers);

				if (gCancelConnectionAttempt)
					goto EndConnAttemptThread;
			}
		}
	}

EndConnAttemptThread:
	gCancelConnectionAttempt = false;
	showConnectionStatus(gVPNConnected);
	ShowWindow(wndwWaiting, SW_HIDE);
	SetFocus(wndwMain);
	ReleaseMutex(gConnectionStateMutex);
	return 0;
}






