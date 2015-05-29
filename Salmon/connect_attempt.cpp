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

DWORD WINAPI measureCurRTT_Thread(LPVOID pCurServerIndex)
{
	int i = *((int*)pCurServerIndex);
	free(pCurServerIndex);

	FILE* writeCurAddr = openConfigFile("previousaddr.txt", "wt");
	fwrite(gCurrentConnection.addr, 1, strlen(gCurrentConnection.addr), writeCurAddr);
	fclose(writeCurAddr);

	int theRTT = measureCurServerRTT();
	WaitForSingleObject(gServerInfoMutex, INFINITE);
	gCurrentConnection.rtt = knownServers[i].rtt = theRTT;
	gCurrentConnection.score = knownServers[i].score = knownServers[i].bandwidth - knownServers[i].rtt;
	ReleaseMutex(gServerInfoMutex);
	writeSConfigFromKnownServers();
	return 0;
}

//try to connect the servers
//first, ping the server to make sure it is online (NO LONGER DOING THIS; CANT BE SURE PINGS GET THROUGH)
//second, try to connect, check the accountlist after 3 seconds, if not connected then accountdisconnect && go back and ping the next one on line
//		you might wanna update the RTT along with the process
//if connected then return true
//third, see if it reaches the end of the queue, if so, return false && ask dir_server for new server
ConnectAnyVPNAttemptResult tryConnectAnyServer()
{
	ConnectAnyVPNAttemptResult ret;

	if (knownServers.empty())
		return ret; //false

	time_t now;

	bool nowTryingSkippedOnes = false;

	vector<int> tryLater;

TrySomeServers:
	for (int ind = 0; ind< (nowTryingSkippedOnes ? tryLater.size() : knownServers.size()); ind++)
	{
		//First, just give up if the user cancelled the connection attempt.
		if (cancelConnectionAttempt)
			return ret;

		int i = (nowTryingSkippedOnes ? tryLater[ind] : ind);
		wchar_t connCountStr[40];
		wchar_t connStatusWithCount[300];
		_itow(nowTryingSkippedOnes ? tryLater.size() - ind : (knownServers.size() - i) + tryLater.size(), connCountStr, 10);
		wcscpy(connStatusWithCount, localizeConst(VPN_STATUS_CONNECTING));
		wcscat(connStatusWithCount, L" (");
		wcscat(connStatusWithCount, connCountStr);
		wcscat(connStatusWithCount, L")");

		Static_SetText(sttcConnectStatus, connStatusWithCount);

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

		ConnectServerStatus conStatus = connectToVPNServer(knownServers[i].addr);

		if (conStatus == CONNECT_SERVER_SUCCESS)
		{
			gVPNConnected = true;
			strcpy(gCurrentConnection.addr, knownServers[i].addr);
			gCurrentConnection.bandwidth = knownServers[i].bandwidth;
			//rtt and score updated in thread
			gCurrentConnection.failureCount = knownServers[i].failureCount = 0;
			gCurrentConnection.lastAttempt = 0;
			//measureCurServerRTT() takes ~60ms even for a ~0ms RTT, so let's not slow things down more than necessary here.
			//we spin off a thread to take the measurement and save it to knownServers. since that new thread needs to know
			//which knownServers item to modify, we let the thread do the sorting+writing that would have happened at the 
			//end of this function (needToWriteSConfigHere = false). (the thread also writes to the previousaddr.txt file.)
			int* passIt = (int*)malloc(sizeof(int));
			*passIt = i;
			CreateThread(NULL, 0, measureCurRTT_Thread, passIt, 0, NULL);
			ret.connectedSuccessfully = true;
			return ret;
		}
		else if (conStatus == CONNECT_SERVER_OFFLINE)
		{
			ret.triedAddrs.insert(knownServers[i].addr);
			knownServers[i].lastAttempt = now;
			knownServers[i].failureCount++;
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
	{
		cancelConnectionAttempt = false;
		ReleaseMutex(gConnectionStateMutex);
		return 0;
	}
	else if (!res.connectedSuccessfully)
	{
		//... then send the "needServer" message to the directory server, and wait for a response.
		MessageBox(NULL, localizeConst(COULDNT_CONNECT_WILL_needServer), L"", MB_OK);

		if (cancelConnectionAttempt)
		{
			cancelConnectionAttempt = false;
			ReleaseMutex(gConnectionStateMutex);
			return 0;
		}
		if (!needServer(res))
		{
			MessageBox(NULL, localizeConst(SORRY_NO_SERVERS_AVAILABLE), localizeConst(ERROR_STR), MB_OK);
			ShowWindow(wndwWaiting, SW_HIDE);
			showConnectionStatus(false);

			SetFocus(wndwMain);
			ReleaseMutex(gConnectionStateMutex);
			return 0;
		}


		

		//Now try connecting again. The "last attempt" logic should mean that only the new server gets tried.
		ConnectAnyVPNAttemptResult res2 = tryConnectAnyServer();
		if (cancelConnectionAttempt && !res2.connectedSuccessfully)
		{
			cancelConnectionAttempt = false;
			ReleaseMutex(gConnectionStateMutex);
			return 0;
		}
		else if (!res2.connectedSuccessfully)
			MessageBox(NULL, localizeConst(SORRY_NO_SERVERS_AVAILABLE), localizeConst(ERROR_STR), MB_OK);

		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
	}
	if (gVPNConnected)
		showConnectionStatus(true);

	ReleaseMutex(gConnectionStateMutex);
	return 0;
}






