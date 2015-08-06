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

//try to connect to any server in the knownServers list
ConnectAnyVPNAttemptResult tryConnectAnyServer()
{
	ConnectAnyVPNAttemptResult ret;

	if (knownServers.empty())
		return ret; //false

	time_t now;

	bool nowTryingSkippedOnes = false;
	bool okToNeedServer = true;

	vector<int> tryLater;

TrySomeServers:
	for (int ind = 0; ind< (nowTryingSkippedOnes ? tryLater.size() : knownServers.size()); ind++)
	{
		int i = (nowTryingSkippedOnes ? tryLater[ind] : ind);

		showServersLeft(nowTryingSkippedOnes ? tryLater.size() - ind : (knownServers.size() - i) + tryLater.size());

		//First, just give up if the user cancelled the connection attempt.
		if (cancelConnectionAttempt)
			return ret;

		//alright, the "skip if failed to connect recently" logic used to be 1hr, but now that we can't ping to check if servers are up
		//(meaning we have to try connecting to all of them, and it appears that even waiting for 3s sometimes isn't enough?), we really
		//need to skip ones that are likely to be down. so now the interval will be random between [2,7] days. the idea is that we want 
		//to skip likely-down ones most of the time so as not to make the user sit through like dozens of seconds of connection attempts,
		//but also, in case it's a better bw / RTT than some always-up server, we should give it occasional chances to come back.
		//and of course, all of this is complemented by the purging system.
		//(and of course, if none of them can connect, we go through and try all anyways, so it's safe.)
		time(&now);
		if (!nowTryingSkippedOnes && now - knownServers[i].lastAttempt <= knownServers[i].secondsTilNextAttempt)
		{
			tryLater.push_back(i);
			continue;
		}



		ConnectServerStatus conStatus = connectToVPNServer(knownServers[i]);

		if (conStatus == CONNECT_SERVER_SUCCESS)
		{
			knownServers[i].failureCount = 0;
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
				ConnectServerStatus conTry2Status = connectToVPNServer(knownServers[i]);
				if (conTry2Status == CONNECT_SERVER_SUCCESS)
				{
					knownServers[i].failureCount = 0;
					ret.connectedSuccessfully = true;
					return ret;
				}
				else if(conTry2Status == CONNECT_SERVER_OFFLINE && connectedToInternet())
				{	
					ret.triedAddrs.insert(knownServers[i].addr);
					knownServers[i].lastAttempt = now;
					knownServers[i].failureCount++;
				}
				else if (conTry2Status == CONNECT_SERVER_OFFLINE)
				{
					//If we've detected internet connectivity problems, it is NOT ok to make a (probably erroneous) needServer request.
					okToNeedServer = false;
				}
				else if (conStatus == CONNECT_SERVER_ERROR)
				{
					ret.serverErrorAddrs.insert(knownServers[i].addr);
					knownServers[i].lastAttempt = now;
					knownServers[i].failureCount++;
				}
				else //if(conStatus == CONNECT_SERVER_CLIENT_ERROR)
				{
					//just pretend we didn't try; this lets needServer fix our bad entry
				}
			}
			else
			{
				//If we've detected internet connectivity problems, it is NOT ok to make a (probably erroneous) needServer request.
				okToNeedServer = false;
			}
		}
		else if (conStatus == CONNECT_SERVER_ERROR)
		{
			ret.serverErrorAddrs.insert(knownServers[i].addr);
			knownServers[i].lastAttempt = now;
			knownServers[i].failureCount++;
		}
		else //if(conStatus == CONNECT_SERVER_CLIENT_ERROR)
		{
			//just pretend we didn't try; this lets needServer fix our bad entry
		}
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
		cancelConnectionAttempt = true;
		return ret;
	}

	//NOTE: this function doesn't get called if the user cancels the attempt, and that's ok.
	writeSConfigFromKnownServers();

	return ret;
}

DWORD WINAPI connectionAttemptThread(LPVOID lpParam)
{
	WaitForSingleObject(gConnectionStateMutex, INFINITE);

	//try to read SConfig.txt - or generate it if necessary.
	if (!getSConfig())
		MessageBox(NULL, localizeConst(COULDNT_READ_CONFIG_FILE), localizeConst(ERROR_STR), MB_OK);

	//try all of our servers. if we can't connect to any of them...
	ConnectAnyVPNAttemptResult res = tryConnectAnyServer();
	if (cancelConnectionAttempt && !res.connectedSuccessfully)
		goto EndConnAttemptThread;
	else if (!res.connectedSuccessfully)
	{
		//... then send the "needServer" message to the directory server, and wait for a response.
		MessageBox(NULL, localizeConst(COULDNT_CONNECT_WILL_needServer), L"", MB_OK);

		if (cancelConnectionAttempt)
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
			if (cancelConnectionAttempt && !res2.connectedSuccessfully)
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
				for (; indexKnownServers < knownServers.size(); indexKnownServers++)
					if (!strcmp(knownServers[indexKnownServers].addr, VPNGateServers[i].addr))
						break;
				if (indexKnownServers >= knownServers.size())
					goto EndConnAttemptThread;

				if (connectToVPNServer(knownServers[indexKnownServers]) == CONNECT_SERVER_SUCCESS)
				{
					knownServers[indexKnownServers].failureCount = 0;
					writeSConfigFromKnownServers();
					break;
				}
				else
					knownServers.erase(knownServers.begin() + indexKnownServers);

				if (cancelConnectionAttempt)
					goto EndConnAttemptThread;
			}
		}
	}

EndConnAttemptThread:
	cancelConnectionAttempt = false;
	showConnectionStatus(gVPNConnected);
	ShowWindow(wndwWaiting, SW_HIDE);
	SetFocus(wndwMain);
	ReleaseMutex(gConnectionStateMutex);
	return 0;
}






