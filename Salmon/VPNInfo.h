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

#ifndef _SALMON_INCL_GUARD_VPNINFO_H_
#define _SALMON_INCL_GUARD_VPNINFO_H_

#include <vector>

class VPNInfo
{
public:
	char addr[60];
	int bandwidth;
	int rtt;
	int score;
	int failureCount;
	char psk[10];
	time_t lastAttempt;
	time_t secondsTilNextAttempt;
	bool vpnGate;
	unsigned short int port;

	VPNInfo(const char* theAddr, int theBW, int theRTT, int theScore, int theFC, const char* thePSK);

private:
	
	VPNInfo() {}
};

//populate a VPNInfo struct with a new server's info, and add it to the knownServers list
//returns true if this IP address was already in knownServers
bool addVPNInfo(char* ipAddrBuf, int serverBW, char* serverPSK);
//same as above, but bring-your-own-VPNInfo
//returns true if this IP address was already in knownServers
bool addVPNInfo(VPNInfo toAdd);

bool parseNewSalmonServer(char* recvBuffer);
bool parseVPNGateItem(const char* curVPNgate, std::vector<VPNInfo>* VPNGateServers);


#endif //_SALMON_INCL_GUARD_VPNINFO_H_