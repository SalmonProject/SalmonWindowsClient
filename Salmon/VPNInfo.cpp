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

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>

#include <vector>
using std::vector;

#include "salmon_constants.h"
#include "salmon_globals.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "localization.h"
#include "control_softether.h"

#include "VPNInfo.h"


VPNInfo::VPNInfo(const char* theAddr, int theBW, int theRTT, int theScore, int theFC, const char* thePSK)
{
	if (strstr(theAddr, "VPNGATE"))
	{
		strncpy(addr, theAddr + strlen("VPNGATE"), 60); addr[59] = 0;
		vpnGate = true;
	}
	else
	{
		strncpy(addr, theAddr, 60); addr[59] = 0;
		vpnGate = false;
	}

	if (strchr(addr, ':'))
	{
		port = (unsigned short int)atoi(strchr(addr, ':') + 1);
		*strchr(addr, ':') = 0;
	}
	else
		port = 443;

	bandwidth = theBW;
	rtt = theRTT;
	score = theScore;
	failureCount = theFC;
	strncpy(psk, thePSK, 10); psk[9] = 0;
}


//populate a VPNInfo struct with a new server's info, and add it to the knownServers list
//returns true if this IP address was already in knownServers
bool addVPNInfo(char* ipAddrBuf, int serverBW, char* serverPSK)
{
	//HACK initially, just set the rtt to 50, because pings only (are guaranteed to) get through after we have connected to the server.
	VPNInfo newInfo(ipAddrBuf, serverBW, 50, serverBW - 50, 0, serverPSK);
	return addVPNInfo(newInfo);
}

//same as above, but bring-your-own-VPNInfo
//returns true if this IP address was already in knownServers
bool addVPNInfo(VPNInfo toAdd)
{
	//HACK initially, just set the rtt to 50, because pings only (are guaranteed to) get through after we have connected to the server.
	toAdd.lastAttempt = 0;
	toAdd.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);

	//add (or overwrite) the new struct to the knownServers list, update SConfig.txt, and create a "connection setting" in SoftEther for the new server
	bool newServerOverwrites = false;
	for (int i = 0; i < knownServers.size(); i++)
		if (!strcmp(toAdd.addr, knownServers[i].addr))
		{
			knownServers[i] = toAdd;
			newServerOverwrites = true;
		}
	if (!newServerOverwrites)
		knownServers.push_back(toAdd);

	if (toAdd.vpnGate)
		createVPNGateConnectionSetting(toAdd);
	else
		createConnectionSetting(toAdd.addr);

	return newServerOverwrites;
}

bool parseNewSalmonServer(char* recvBuffer)
{
	int wholeReplyLength = strlen(recvBuffer);
	for (int i = 0; i < wholeReplyLength; i++)
		if (recvBuffer[i] == '*')
			recvBuffer[i] = '\n';
	if (!strchr(recvBuffer, ' ') || !strchr(strchr(recvBuffer, ' ') + 1, ' ') || !strstr(recvBuffer, "-----END CERTIFICATE-----"))
	{
		MessageBox(NULL, localizeConst(DIRSERV_GAVE_MALFORMED_RESPONSE_TO_LOGIN), localizeConst(ERROR_STR), MB_OK);
		return false;
	}
	int ipAddrLen = (int)(strchr(recvBuffer, ' ') - recvBuffer);
	char ipAddrBuf[260];
	memcpy(ipAddrBuf, recvBuffer, ipAddrLen);
	ipAddrBuf[ipAddrLen] = 0;

	char* pskStart = strchr(recvBuffer, ' ') + 1;
	int pskLen = (int)(strchr(pskStart, ' ') - pskStart);
	char serverPSK[10];
	memcpy(serverPSK, pskStart, pskLen);
	serverPSK[pskLen] = 0;

	//==============================
	//ok... i ended up writing this because of a silly mistake (was printing serverBW when i meant to print *serverBW; thought
	//the text was coming through as garbage), BUT, it's more robust in case there is any weirdness, so might as well keep it
	char bwTextBuf[100];
	char* startBW = 1 + strchr(recvBuffer, ' ');
	int curDigit = 0;
	for (; curDigit < 99 && startBW[curDigit] >= '0' && startBW[curDigit] <= '9'; curDigit++)
		bwTextBuf[curDigit] = startBW[curDigit];
	bwTextBuf[curDigit] = 0;
	//==============================
	int serverBW = atoi(bwTextBuf);

	//ok, at this point, ip address and bw have been safely stored. now do cert and purges.

	//NOTE: we already converted the *s in the cert back to \ns a few lines earlier.
	char* certStart = strstr(recvBuffer, "-----BEGIN CERTIFICATE-----");
	char* certEnd = strchr(strstr(recvBuffer, "-----END CERTIFICATE-----"), '\n');
	//SoftEther's default .pem certificate appears to be ~1200 bytes, so 2500 should be reasonably future-proof
	char serverCertBuf[2500];
	memcpy(serverCertBuf, certStart, (int)(certEnd - certStart));
	serverCertBuf[(int)(certEnd - certStart)] = 0;
	//ok... just immediately write the server certificate we got into a file in the config dir. 
	//you can always access the cert whenever you need it, because it's saved under the server's ip.
	char certFileName[200];
	strcpy(certFileName, ipAddrBuf);
	strcat(certFileName, ".pem");
	FILE* writeCert = openConfigFile(certFileName, "wt");
	fwrite(serverCertBuf, 1, strlen(serverCertBuf), writeCert);
	fclose(writeCert);

	//add this server to knownServers and create a connection setting for it.
	addVPNInfo(ipAddrBuf, serverBW, serverPSK);
	writeSConfigFromKnownServers();

	return true;
}

bool parseVPNGateItem(const char* curVPNgate, vector<VPNInfo>* VPNGateServers)
{
	//parse the line. format: VPNGATE1.2.3.4:443 psk(always=vpn) offeredbw the*cert*star*delims
	char ipPortVPNGate[80];
	char pskVPNGate[10];
	int offeredBW_VPNGate;
	char certVPNGate[2500];

	//get ipPortVPNGate (including the VPNGATE string, so VPNInfo ctor will know it's VPNGATE
	if (strchr(curVPNgate, ' ') - curVPNgate >= 80)
	{
		MessageBox(NULL, L"VPN Gate server address too long.", L"Error", MB_OK);
		return false;
	}
	else if (strchr(curVPNgate, ' ') == 0)
	{
		MessageBox(NULL, L"Malformed VPN Gate server entry.", L"Error", MB_OK);
		return false;
	}
	memcpy(ipPortVPNGate, curVPNgate, strchr(curVPNgate, ' ') - curVPNgate);
	ipPortVPNGate[strchr(curVPNgate, ' ') - curVPNgate] = 0;

	//get pskVPNGate
	const char* tempPSK = strchr(curVPNgate, ' ') + 1;
	strncpy(pskVPNGate, tempPSK, 9);
	if (strchr(pskVPNGate, ' '))
		*strchr(pskVPNGate, ' ') = 0;

	//get offeredBW_VPNGate
	const char* tempBW = strchr(tempPSK, ' ') + 1;
	char tempConvertBW[20];
	strncpy(tempConvertBW, tempBW, 19);
	if (strchr(tempConvertBW, ' '))
		*strchr(tempConvertBW, ' ') = 0;
	offeredBW_VPNGate = atoi(tempConvertBW);

	//get certVPNGate from the buffer...
	const char* certStart = strchr(tempBW, ' ') + 1;
	const char* certEnd = strchr(certStart, '\n');
	if (certEnd - certStart >= 2500)
	{
		MessageBox(NULL, L"Certificate for VPN Gate server is too large.", L"Error", MB_OK);
		return false;
	}
	else if (certStart == (char*)1 || certEnd == 0)
	{
		MessageBox(NULL, L"Malformed VPN Gate server certificate.", L"Fatal Error", MB_OK);
		return false;
	}
	memcpy(certVPNGate, certStart, certEnd - certStart);
	certVPNGate[certEnd - certStart] = 0;
	//...convert *s to \ns...
	for (int i = 0; i < certEnd - certStart; i++)
		if (certVPNGate[i] == '*')
			certVPNGate[i] = '\n';
	//...and write to the ipaddr.pem cert file.
	char certFileName[200];
	strcpy(certFileName, ipPortVPNGate + strlen("VPNGATE"));
	if (strchr(certFileName, ':'))
		*strchr(certFileName, ':') = 0;
	strcat(certFileName, ".pem");
	FILE* writeCert = openConfigFile(certFileName, "wt");
	fwrite(certVPNGate, 1, strlen(certVPNGate), writeCert);
	fclose(writeCert);


	VPNInfo new_ip(ipPortVPNGate, offeredBW_VPNGate, 50, offeredBW_VPNGate - 50, 0, pskVPNGate);
	new_ip.lastAttempt = 0;
	new_ip.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);
	VPNGateServers->push_back(new_ip);

	return true;
}
