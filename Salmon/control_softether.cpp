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
#include "Resource.h"

//#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <process.h>

using std::string;

#include <winsock2.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>
#include <icmpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")

#if defined(_MSC_VER) && !defined(EFIX64) && !defined(EFI32)
#include <basetsd.h>
typedef UINT32 uint32_t;
#else
#include <inttypes.h>
#endif

#include "salmon_globals.h"
#include "hwnds.h"

#include "localization.h"
#include "control_softether.h"
#include "salmon_utility.h"
#include "pipefile.h"
#include "salmon_config.h"

extern "C" BOOL popenCheckExe(const char* cmdToExec);

//NOTE: yes, this is always the Program Files (NOT x86) directory. softether installs to Program Files (NOT x86) on a 64 bit machine.
//microsoft doesn't want a 32 bit program seeing the "real" Program Files directory on a 64-bit machine, since it would mess with
//the emulation layer that the 32-on-64 thing works on. that doesn't apply to us, though; we're just trying to find vpncmd.exe to system("") it.
void load_vpncmdexe_Path()
{
	WCHAR vpncmdPathW[VPNCMD_PATH_BUFSIZE];

	GetSystemWindowsDirectory(vpncmdPathW, 150);
	wcstombs(g_vpncmdPath, vpncmdPathW, 150);
	for (int i = 0; i < strlen(g_vpncmdPath); i++)
		g_vpncmdPath[i] = (char)toupper(g_vpncmdPath[i]);
	char* lastWindows = 0;
	char* lastTemp = strstr(g_vpncmdPath, "WINDOWS");
	while (lastTemp)
	{
		lastWindows = lastTemp;
		lastTemp = strstr(lastTemp+1, "WINDOWS");
	}
	if (lastWindows)
		*lastWindows = '\0';

	char vpnc[VPNCMD_PATH_BUFSIZE];
	char vpnc64[VPNCMD_PATH_BUFSIZE];

	strcpy(vpnc, g_vpncmdPath);
	strcpy(vpnc64, g_vpncmdPath);
	strcat(vpnc, "Program Files\\SoftEther VPN Client\\vpncmd.exe");
	strcat(vpnc64, "Program Files\\SoftEther VPN Client\\vpncmd_x64.exe");

	mbstowcs(vpncmdPathW, vpnc64, VPNCMD_PATH_BUFSIZE);
	DWORD vpncmdAttributes = GetFileAttributes(vpncmdPathW);
	if (vpncmdAttributes != INVALID_FILE_ATTRIBUTES && !(vpncmdAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//ok, the 64-bit version is present... BUT. In a 32-bit Windows installation on a 64-bit machine,
		//SoftEther will put vpncmd64 in here, but then it won't work. So, ensure the vpncmd64 that is here actually works.
		char toExec[EXEC_VPNCMD_BUFSIZE];
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd accountget dummycheck", vpnc64);
		if (popenCheckExe(toExec))
		{
			strcpy(g_vpncmdPath, vpnc64);
			return;
		}
	}

	mbstowcs(vpncmdPathW, vpnc, VPNCMD_PATH_BUFSIZE);
	vpncmdAttributes = GetFileAttributes(vpncmdPathW);
	if (vpncmdAttributes != INVALID_FILE_ATTRIBUTES && !(vpncmdAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		//might as well also do a sanity check on the 32-bit one
		char toExec[EXEC_VPNCMD_BUFSIZE];
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd accountget dummycheck", vpnc);
		if (popenCheckExe(toExec))
		{
			strcpy(g_vpncmdPath, vpnc);
			return;
		}
		else
		{
			MessageBox(NULL, L"64-bit vpncmd was either not present in Program Files\\SoftEther VPN Client or did not work, and 32-bit vpncmd WAS present but did NOT work. This is bizarre...", L"Fatal Error", MB_OK);
			ExitProcess(0);
		}
	}

	strcpy(g_vpncmdPath, vpnc);
	WCHAR vpncmdErrMsg[300];
	wcscpy(vpncmdErrMsg, localizeConst(SOFTETHER_EXE_DOESNT_EXIST));
	wcscat(vpncmdErrMsg, vpncmdPathW);
	MessageBox(NULL, vpncmdErrMsg, localizeConst(FATAL_ERROR), MB_OK);
	ExitProcess(0);
}

//returns true if we have a good connection to a VPN server, where good is defined as "they have
//given us a default gateway (and so presumably also an IP address)." If the server has no access
//to the rest of the internet (say because they chose to use SecureNAT, and SecureNAT is asking
//Comcast to give it a second IP address), this would still return true.
//
//oh also: so that we don't have to worry about whether the NIC exists: if a "VPN Client" adapter 
//isn't present, it calls NICcreate. calling this at program start ensures the NIC will be there.
bool checkConnection()
{
	bool foundVPN = false;
	bool connected = false;

	//example taken from http://msdn.microsoft.com/en-us/library/windows/desktop/aa365917(v=vs.85).aspx

	// It is possible for an adapter to have multiple
	// IPv4 addresses, gateways, and secondary WINS servers
	// assigned to the adapter. 
	//
	// Note that this sample code only prints out the 
	// first entry for the IP address/mask, and gateway, and
	// the primary and secondary WINS server for each adapter. 

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) 
	{
		MessageBoxA(NULL, "Error allocating memory needed to call GetAdaptersinfo", "Fatal Error", MB_OK);
		return false;
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			MessageBoxA(NULL, "Error allocating memory needed to call GetAdaptersinfo", "Fatal Error", MB_OK);
			return false;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) 
	{
		pAdapter = pAdapterInfo;
		while (pAdapter) 
		{
			if (strstr(pAdapter->Description, "VPN Client"))
			{
				//MessageBoxA(NULL, pAdapter->GatewayList.IpAddress.String, "sdfasd", MB_OK);
				foundVPN = true;
				if (!strstr(pAdapter->GatewayList.IpAddress.String, "0.0.0.0"))//0.0.0.0 being the default for unconnected
					connected = true;
				break;
			}

			pAdapter = pAdapter->Next;
		}
	}
	else 
	{
		char errorStr[100];
		sprintf(errorStr, "GetAdaptersInfo failed with error: %d\n", dwRetVal);
		MessageBoxA(NULL, errorStr, "Fatal Error", MB_OK);
	}
	if (pAdapterInfo)
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);

	if (!foundVPN)
	{
		char toExec[EXEC_VPNCMD_BUFSIZE];
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:niccreate VPN", g_vpncmdPath);
		systemNice(toExec);
	}

	return connected;

}

//Once we have connected to a VPN server, we can measure our RTT to it. The iptables masquerade
//tap interface thing appears willing to reply to pings, so ping that. 
//This function returns RTT in ms, and returns 99999999 if you actually aren't connected after all.
int measureCurServerRTT()
{
	bool foundVPN = false;
	int rttValue = 99999999;

	//example taken from http://msdn.microsoft.com/en-us/library/windows/desktop/aa365917(v=vs.85).aspx
	//			and from http://msdn.microsoft.com/en-us/library/aa366050%28VS.85%29.aspx

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
	{
		MessageBoxA(NULL, "Error allocating memory needed to call GetAdaptersinfo", "Fatal Error", MB_OK);
		return false;
	}
	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *)HeapAlloc(GetProcessHeap(), 0, ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			MessageBoxA(NULL, "Error allocating memory needed to call GetAdaptersinfo", "Fatal Error", MB_OK);
			return false;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			if (strstr(pAdapter->Description, "VPN Client"))
			{
				foundVPN = true;
				if (!strstr(pAdapter->GatewayList.IpAddress.String, "0.0.0.0"))//TODO check if valid IP addr
				{

					uint32_t ipaddr = inet_addr(pAdapter->GatewayList.IpAddress.String);
					DWORD numPingResults = 0;
					char SendData[32] = "Data Buffer";
					LPVOID ReplyBuffer = NULL;
					DWORD ReplySize = 0;

					HANDLE hIcmpFile = IcmpCreateFile();
					if (hIcmpFile == INVALID_HANDLE_VALUE) 
					{
						MessageBoxA(NULL, "Unable to IcmpCreatefile.", "Fatal Error", MB_OK);
						break;
					}

					ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
					ReplyBuffer = (VOID*)malloc(ReplySize);
					if (ReplyBuffer == NULL) 
					{
						MessageBoxA(NULL, "Error allocating memory needed to call IcmpSendEcho", "Fatal Error", MB_OK);
						IcmpCloseHandle(hIcmpFile);
						break;
					}


					numPingResults = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 1000);
					if (numPingResults > 0)
					{
						PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
						if (pEchoReply->Status == IP_SUCCESS)
							rttValue = pEchoReply->RoundTripTime;
					}
					free(ReplyBuffer);
					IcmpCloseHandle(hIcmpFile);
				}
				break;
			}
			pAdapter = pAdapter->Next;
		}
	}
	else
	{
		char errorStr[100];
		sprintf(errorStr, "GetAdaptersInfo failed with error: %d\n", dwRetVal);
		MessageBoxA(NULL, errorStr, "Fatal Error", MB_OK);
	}
	if (pAdapterInfo)
		HeapFree(GetProcessHeap(), 0, pAdapterInfo);

	return rttValue;
}
DWORD WINAPI measureCurRTT_Thread(LPVOID dummyArg)
{
	int theI = 0;
	for (theI = 0; theI < knownServers.size(); theI++)
		if (!strcmp(gCurrentConnection.addr, knownServers[theI].addr))
			break;
	if (theI == knownServers.size())
		return 0;

	FILE* writeCurAddr = openConfigFile("previousaddr.txt", "wt");
	fwrite(gCurrentConnection.addr, 1, strlen(gCurrentConnection.addr), writeCurAddr);
	fclose(writeCurAddr);

	int theRTT = measureCurServerRTT();
	WaitForSingleObject(gServerInfoMutex, INFINITE);
	gCurrentConnection.rtt = knownServers[theI].rtt = theRTT;
	gCurrentConnection.score = knownServers[theI].score = knownServers[theI].bandwidth - knownServers[theI].rtt;
	ReleaseMutex(gServerInfoMutex);
	writeSConfigFromKnownServers();
	return 0;
}

extern "C" size_t popenOneShotR(const char* cmdToExec, char* buf, size_t bufSize);

//returns a malloc()'d pointer to a buffer with the output of accountget consetting
char* accountGet(const char* conSetting)
{
	char toExec[EXEC_VPNCMD_BUFSIZE];
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd accountget %s", g_vpncmdPath, conSetting);
	char* retBuf = (char*)malloc(4096);
	popenOneShotR(toExec, retBuf, 4096);
	return retBuf;
}

//creates the SoftEther "connection setting" for the specified IP address. Requires the gBaseVPNPassword global 
//variable to contain the right password, and requires that the right certificate file is at %APPDATA%\salmon\serverip.pem
void createConnectionSetting(const char* serverIP_Addr)
{
	char toExec[EXEC_VPNCMD_BUFSIZE];

	char certPath[300];
	strcpy(certPath, getenv("APPDATA"));
	strcat(certPath, "\\salmon\\");
	strcat(certPath, serverIP_Addr);
	strcat(certPath, ".pem");

	//using the same password on all servers can actually lead to some kind of bad mischief, so the actual per-server password will be derived from sha1(base_password, server_ip)
	char pwToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
	char userNameToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
	derivePassword(pwToUse, gBaseVPNPassword, serverIP_Addr);
	deriveUsername(userNameToUse, gBaseVPNPassword, serverIP_Addr);

	//First, create the account (this does nothing if it already exists; doesn't even set the username to what was requested.)
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountcreate %s /server:%s:443 /hub:salmon /username:%s /nicname:vpn", g_vpncmdPath, serverIP_Addr, serverIP_Addr, userNameToUse);
	systemNice(toExec);
	
	//Handle SoftEther asynchronicity; don't proceed until we know the account exists, with certs enabled.
	char* accountDetails = 0;
	for 
	(
		accountDetails = accountGet(serverIP_Addr);
		//Keep trying as long as 'Verify Server Certificate' is not there, or is set to "Disable"
		!strstr(accountDetails, "Verify Server Certificate") ||
		*(strchr(strstr(accountDetails, "Verify Server Certificate"), '|') + 1) == 'D'
		//Try again (ok since we free() at end of loop and after loop)
		; accountDetails = accountGet(serverIP_Addr)
	)
	{
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountservercertenable %s", g_vpncmdPath, serverIP_Addr);
		systemNice(toExec);
		Sleep(50);
		free(accountDetails);
	}
	free(accountDetails);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountstatushide %s", g_vpncmdPath, serverIP_Addr);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountretryset %s /num:1 /interval:7", g_vpncmdPath, serverIP_Addr);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountservercertset %s /loadcert:%s", g_vpncmdPath, serverIP_Addr, certPath);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountpasswordset %s /password:%s /type:standard", g_vpncmdPath, serverIP_Addr, pwToUse);
	systemNice(toExec);

	//NOTE: these are necessary even though they're provided in accountcreate! if the setting already existed but with a wrong name or IP address,
	//		the accountcreate command will just be like "oh ok it exists" and not bother to set the things that you are telling it to set.
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountusernameset %s /username:%s", g_vpncmdPath, serverIP_Addr, userNameToUse);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountset %s /server:%s:443 /hub:salmon", g_vpncmdPath, serverIP_Addr, serverIP_Addr);
	systemNice(toExec);

	//Handle SoftEther asynchronicity; don't proceed until we know the account has had the right username (and hopefully therefore password) set
	for
	(
		accountDetails = accountGet(serverIP_Addr)
		//Keep trying as long as the right user name isn't in there
		; !strstr(accountDetails, userNameToUse)
		//Try again (ok since we free() at end of loop and after loop)
		; accountDetails = accountGet(serverIP_Addr)
	)
	{
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountpasswordset %s /password:%s /type:standard", g_vpncmdPath, serverIP_Addr, pwToUse);
		systemNice(toExec);
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountusernameset %s /username:%s", g_vpncmdPath, serverIP_Addr, userNameToUse);
		systemNice(toExec);
		Sleep(50);
		free(accountDetails);
	}
	free(accountDetails);
}




//creates the SoftEther "connection setting" for the specified VPN Gate IP address. 
//This version makes the appropriate changes for VPN Gate: port not fixed to 443, uses VPNGATE hub, uses anonymous login (user vpn, no password)
//Requires that the right certificate file is at %APPDATA%\salmon\serverip.pem
void createVPNGateConnectionSetting(const VPNInfo& theServerInfo)
{
	char toExec[EXEC_VPNCMD_BUFSIZE];

	char certPath[300];
	strcpy(certPath, getenv("APPDATA"));
	strcat(certPath, "\\salmon\\");
	strcat(certPath, theServerInfo.addr);
	strcat(certPath, ".pem");

	//this is an anonymous logic; username=vpn, no password.
	const char* userNameToUse = "vpn";

	//First, create the account (this does nothing if it already exists; doesn't even set the username to what was requested.)
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountcreate %s /server:%s:%d /hub:VPNGATE /username:%s /nicname:vpn", g_vpncmdPath, theServerInfo.addr, theServerInfo.addr, theServerInfo.port, userNameToUse);
	systemNice(toExec);

	//Handle SoftEther asynchronicity; don't proceed until we know the account exists, with certs enabled.
	char* accountDetails = 0;
	for
		(
		accountDetails = accountGet(theServerInfo.addr);
	//Keep trying as long as 'Verify Server Certificate' is not there, or is set to "Disable"
	!strstr(accountDetails, "Verify Server Certificate") ||
		*(strchr(strstr(accountDetails, "Verify Server Certificate"), '|') + 1) == 'D'
		//Try again (ok since we free() at end of loop and after loop)
		; accountDetails = accountGet(theServerInfo.addr)
		)
	{
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountservercertenable %s", g_vpncmdPath, theServerInfo.addr);
		systemNice(toExec);
		Sleep(50);
		free(accountDetails);
	}
	free(accountDetails);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountstatushide %s", g_vpncmdPath, theServerInfo.addr);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountretryset %s /num:1 /interval:7", g_vpncmdPath, theServerInfo.addr);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountservercertset %s /loadcert:%s", g_vpncmdPath, theServerInfo.addr, certPath);
	systemNice(toExec);

	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountanonymousset %s", g_vpncmdPath, theServerInfo.addr);
	systemNice(toExec);

	//NOTE: these are necessary even though they're provided in accountcreate! if the setting already existed but with a wrong name or IP address (or port, in this case),
	//		the accountcreate command will just be like "oh ok it exists" and not bother to set the things that you are telling it to set.
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountset %s /server:%s:%d /hub:VPNGATE", g_vpncmdPath, theServerInfo.addr, theServerInfo.addr, theServerInfo.port);
	systemNice(toExec);
}






enum HTTPSResult { HTTPSResult_Failed = 0, HTTPSResult_Succeeded = 1, HTTPSResult_Unfinished = 2 };
bool checkSoftEtherHTTPS(const std::string& serverIP_Addr);
class HTTPSArgPasser
{
public:
	string theServerAddr;
	HTTPSResult theResult;
	//okToDelete is a mutex that starts out owned by the thread that creates the object.
	//once it's available, httpsCheckerThread knows it's ok to delete this object.
	//the thread that created httpsCheckerThread should release it as soon as it is
	//no longer interested in the contents of this object.
	HANDLE okToDelete;
	HTTPSArgPasser(const char* sAddr)
	{
		theServerAddr = string(sAddr);
		theResult = HTTPSResult_Unfinished;
		okToDelete = CreateMutex(NULL, TRUE, NULL);
	}
private:
	HTTPSArgPasser(){}
};
DWORD WINAPI httpsCheckerThread(LPVOID lpParam)
{
	HTTPSArgPasser* storeResult = (HTTPSArgPasser*)lpParam;
	bool httpsSuccess = checkSoftEtherHTTPS(storeResult->theServerAddr);
	if (httpsSuccess)
		storeResult->theResult = HTTPSResult_Succeeded;
	else
		storeResult->theResult = HTTPSResult_Failed;

	//caller thread is only supposed to be around for 5 seconds + 50 checkConnection()s, so no need to wait forever.
	WaitForSingleObject(storeResult->okToDelete, 30 * 1000);
	//I'm not sure that I want to trust that it's safe to have a thread ready to delete an object the instant
	//a mutex inside the object is released (documentation for ReleaseMutex() doesn't promise that the mutex doesn't 
	//become available before the function returns), and this Sleep() isn't going to slow anything down.
	Sleep(1000);
	delete storeResult;

	return 0;
}


//attempt to connect to a VPN server; there should already be a 'connection setting' set up inside softether.
//return value:	CONNECT_SERVER_SUCCESS obvious, CONNECT_SERVER_OFFLINE connection attempt and https get failed, 
//				CONNECT_SERVER_CLIENT_ERROR we determined our connection setting wasn't correct, CONNECT_SERVER_ERROR https get succeeded, connect failed
ConnectServerStatus connectToVPNServer(const VPNInfo& theServer)
{
	char toExec[EXEC_VPNCMD_BUFSIZE];

	//the connection setting "ipaddr" should be pre-loaded with that ipaddr's cert, and the correct username and password
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountconnect %s", g_vpncmdPath, theServer.addr);
	char bufAccConn[4096];
	popenOneShotR(toExec, bufAccConn, 4096);
	
	//if (con setting didnt exist), then disconnect and return CONNECT_SERVER_CLIENT_ERROR;
	//unfortunately, i don't think it's possible to doublecheck the username or actual target ip address; not printed from this command.
	if (strstr(bufAccConn, "VPN Connection Setting does not exist.") || strstr(bufAccConn, "Error code: 36"))
	{
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountdisconnect %s", g_vpncmdPath, theServer.addr);
		systemNice(toExec);
		Sleep(10);
		return CONNECT_SERVER_CLIENT_ERROR;
	}

	//all of this HTTPS checking is for Salmon block checking purposes, so don't bother for VPN Gate servers.
	HTTPSArgPasser* storeHTTPSResult = 0;
	if (!theServer.vpnGate)
	{
		//while softether is trying to establish the connection, also make our own TLS connection, and test if a server
		//1) is there, 2) has the cert we expect, 3) responds with a 404 to an HTTP GET like softether does.
		//(as a thread since this is a piece of logic that becomes important if the softether connection failed... in which
		// case we've ALREADY waited 8 seconds.)
		storeHTTPSResult = new HTTPSArgPasser(theServer.addr);
		CreateThread(NULL, 0, httpsCheckerThread, storeHTTPSResult, 0, NULL);
	}

	//sometimes if it's the first connection to the server in a while, we're reporting it as failed even though it succeeded.
	//i'm assuming that vpncmd returns, having set the stuff in motion, but without guaranteeing that the connection is set up.
	//it's probably necessary to have a decent Sleep() here, since i'm currently testing on a local network with tiny RTT.
	//hopefully 8s is enough here... i don't want to add huge delays to failed connections for no reason.
	for (int connChecks = 0; (storeHTTPSResult == 0 || storeHTTPSResult->theResult != HTTPSResult_Failed) && connChecks < 80; connChecks++)
	{
		Sleep(100);
		if (checkConnection())
		{
			if (storeHTTPSResult)
				ReleaseMutex(storeHTTPSResult->okToDelete);
			gVPNConnected = true;
			strcpy(gCurrentConnection.addr, theServer.addr);
			gCurrentConnection.bandwidth = theServer.bandwidth;
			//rtt and score updated in thread
			gCurrentConnection.failureCount = 0;
			gCurrentConnection.lastAttempt = 0;

			//measureCurServerRTT() takes ~60ms even for a ~0ms RTT, so let's not slow things down more than necessary here.
			//we spin off a thread to take the measurement and save it to knownServers. since that new thread needs to know
			//which knownServers item to modify, we let the thread do the sorting+writing that would have happened at the 
			//end of this function (needToWriteSConfigHere = false). (the thread also writes to the previousaddr.txt file.)
			CreateThread(NULL, 0, measureCurRTT_Thread, 0, 0, NULL);
			return CONNECT_SERVER_SUCCESS;
		}
	}
	//if you get past this loop, the connection has failed.

	//we'll probably be trying other connection settings, so don't let this ongoing attempt get in the way.
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountdisconnect %s", g_vpncmdPath, theServer.addr);
	systemNice(toExec);
	Sleep(10);

	if (storeHTTPSResult && storeHTTPSResult->theResult == HTTPSResult_Succeeded)
	{
		ReleaseMutex(storeHTTPSResult->okToDelete);
		return CONNECT_SERVER_ERROR;
	}
	//failed OR unfinished can be treated as "server offline".
	else
	{
		if(storeHTTPSResult)
			ReleaseMutex(storeHTTPSResult->okToDelete);
		return CONNECT_SERVER_OFFLINE;
	}
}

//disconnect the VPN server
void disconnectVPN()
{
	//Just in case gCurrentConnection.addr is empty, which would make vpncmd ask for input and hang.
	if (!strchr(gCurrentConnection.addr, '.'))
		return;

	char toExec[EXEC_VPNCMD_BUFSIZE];
	//NOTE: if we're connected, gCurrentConnection.addr is supposed to have the current server's IP address.
	//If salmon started with softether already being connected, then we should have read it from previousaddr.txt.
	sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountdisconnect %s", g_vpncmdPath, gCurrentConnection.addr);
	systemNice(toExec);

	gVPNConnected = false;
}

//use vpncmd to remove every 'connection setting' whose name is an IP address currently in knownServers
void deleteConnectionSettings()
{
	char toExec[EXEC_VPNCMD_BUFSIZE];

	for (int i = 0; i < knownServers.size(); i++)
	{
		sprintf(toExec, "\"%s\" localhost /client /hub:vpn /cmd:accountdelete %s", g_vpncmdPath, knownServers[i].addr);
		systemNice(toExec);
	}
}


extern bool cancelConnectionAttempt;
//as long as we think we should be connected (gVPNConnected == true), check softether once per 5 seconds to be sure we are.
DWORD WINAPI monitorConnection(LPVOID lpParam)
{
	while (1)
	{
		//sadly, it sounds like SleepConditionVariableCS etc aren't available on XP.
		if (gVPNConnected)
		{
			WaitForSingleObject(gConnectionStateMutex, INFINITE);

			gVPNConnected = checkConnection();
			if (!gVPNConnected && gUserWantsConnection)
			{
				showConnectionStatus(false);
				ShowWindow(bttnConnect, SW_HIDE);
				ShowWindow(bttnCancelConnect, SW_SHOW);
				ShowWindow(bttnDisconnect, SW_HIDE);

				Static_SetText(sttcConnectStatus, localizeConst(VPN_STATUS_CONNECTING));

				cancelConnectionAttempt = false;
				ReleaseMutex(gConnectionStateMutex); //because the thread is going to need it
				CreateThread(NULL, 0, connectionAttemptThread, NULL, 0, NULL);
			}
			else
				ReleaseMutex(gConnectionStateMutex);
		}

		Sleep(5000);
	}
}
