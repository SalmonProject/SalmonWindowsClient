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

#ifndef _SALMON_INCL_GUARD_CONNECTATTEMPT_H_
#define _SALMON_INCL_GUARD_CONNECTATTEMPT_H_

#include <string>
#include <set>

#include "VPNInfo.h"

class ConnectAnyVPNAttemptResult
{
public:
	bool connectedSuccessfully;
	std::set<std::string> triedAddrs;
	std::set<std::string> serverErrorAddrs;

	ConnectAnyVPNAttemptResult()
	{
		connectedSuccessfully = false;
	}
};

enum NeedServerSuccess
{
	NEED_SERVER_GOT_NONE = 0,
	NEED_SERVER_GOT_SALMON = 1,
	NEED_SERVER_GOT_VPNGATE = 2
};
NeedServerSuccess needServer(const ConnectAnyVPNAttemptResult& res, std::vector<VPNInfo>* VPNGateServers);

#endif //_SALMON_INCL_GUARD_CONNECTATTEMPT_H_
