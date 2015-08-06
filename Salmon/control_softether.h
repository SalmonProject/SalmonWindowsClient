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

#ifndef _SALMON_INCLGUARD_CONTROL_SOFTETHER_H_
#define _SALMON_INCLGUARD_CONTROL_SOFTETHER_H_

#include "salmon_constants.h"

extern char g_vpncmdPath[VPNCMD_PATH_BUFSIZE];
void load_vpncmdexe_Path();

void disconnectVPN();
void createConnectionSetting(const char* serverIP_Addr);
void createVPNGateConnectionSetting(const VPNInfo& theServerInfo);
void deleteConnectionSettings();
enum ConnectServerStatus { CONNECT_SERVER_SUCCESS, CONNECT_SERVER_OFFLINE,
							CONNECT_SERVER_CLIENT_ERROR, CONNECT_SERVER_ERROR};
ConnectServerStatus connectToVPNServer(const VPNInfo& theServer);
bool checkConnection();
int measureCurServerRTT();

DWORD WINAPI monitorConnection(LPVOID lpParam);

extern HANDLE gConnectionStateMutex;
extern bool gUserWantsConnection;

#endif