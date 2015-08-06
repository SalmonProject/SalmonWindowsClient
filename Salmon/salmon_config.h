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

#ifndef _SALMON_INCL_GUARD_CONFIGMAIL_H_
#define _SALMON_INCL_GUARD_CONFIGMAIL_H_

#include <Windows.h>
#include "salmon_constants.h"

void wipeConfig();

void purgeServer(const char* purgeThisIP);
void executeAllPurges(const char* buf);

bool getSConfig();

//parameter is a RecvThreadStruct pointer
DWORD WINAPI recvThread(LPVOID lpParam);
DWORD WINAPI connectionAttemptThread(LPVOID lpParam);

void checkTrustCountdown();
void startTrustCountdown();
void ensureSalmonConfigDir();
void loadEmailIfStored();
bool generate_S_from_SalmonConfig();
void writeSConfigFromKnownServers();

//write into the config file's trust value
//(the second line of salmonconfig.txt is trust, so just overwrite that one)
void writeTrustToFile(int trust);
int readTrustFromFile();

#endif 