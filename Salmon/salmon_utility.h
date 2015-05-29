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

#ifndef _SALMON_INCL_GUARD_UTILITY_H_
#define _SALMON_INCL_GUARD_UTILITY_H_

#include "salmon_constants.h"

FILE* openConfigFile(const char* filename, const char* mode);
void unlinkConfigFile(const char* filename);
void trustLevelDisplayAndWriteFile(int level);
void showConnectionStatus(bool isConnected);
bool lineIsJustWhitespace(char* theLine);
bool fileExists(LPCTSTR szPath);
__int64 getHNsecsSince1600();
void systemNiceW(wchar_t* execMe);
void systemNice(char* execMe);
void loadCurrentVPN_Addr();

void derivePassword(char* pwToUse, char* gBaseVPNPassword, char* serverIP_Addr);
void deriveUsername(char* pwToUse, char* gBaseVPNPassword, char* serverIP_Addr);

void writeMobileconfigToFile(WCHAR* fname, WCHAR* ipAddr, WCHAR* psk, WCHAR* vpnName, WCHAR* vpnPW);

#endif
