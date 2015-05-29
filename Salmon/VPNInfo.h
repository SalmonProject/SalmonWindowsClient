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

	VPNInfo(const char* theAddr, int theBW, int theRTT, int theScore, int theFC, const char* thePSK)
	{
		if (strstr(theAddr, "VPNGATE"))
		{
			strncpy(addr, theAddr+strlen("VPNGATE"), 60); addr[59] = 0;
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

private:
	
	VPNInfo() {}
};

#endif //_SALMON_INCL_GUARD_VPNINFO_H_