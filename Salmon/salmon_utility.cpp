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

//#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <WinCrypt.h>
#include <fstream>

#include "hwnds.h"
#include "salmon_globals.h"

#include "localization.h"
#include "salmon_config.h"
#include "sha1.h"

#include "salmon_utility.h"

//exists and is not directory
bool fileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

FILE* openConfigFile(const char* filename, const char* mode)
{
	char wholeThing[300];
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\");
	strcat(wholeThing, filename);

	return fopen(wholeThing, mode);
}

void unlinkConfigFile(const char* filename)
{
	char wholeThing[300];
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\");
	strcat(wholeThing, filename);

	_unlink(wholeThing);
}

void trustLevelDisplayAndWriteFile(int level)
{
	writeTrustToFile(level);

	if (level > 6)
		Static_SetText(sttcTrustStatus, localizeConst(TRUST_LVL_MAX));
	else if (level >= 6)
		Static_SetText(sttcTrustStatus, localizeConst(TRUST_LVL_HIGHEST));
	else if (level >= 5)
		Static_SetText(sttcTrustStatus, localizeConst(TRUST_LVL_HIGH));
	else if (level >= 0)
		Static_SetText(sttcTrustStatus, localizeConst(TRUST_LVL_ORDINARY));
	else
		Static_SetText(sttcTrustStatus, localizeConst(TRUST_LVL_LOW));
}


void loadCurrentVPN_Addr()
{
	FILE* readCurAddr = openConfigFile("previousaddr.txt", "rt");
	if (!readCurAddr)
	{
		strcpy(gCurrentConnection.addr, "NONE");
		return;
	}
	memset(gCurrentConnection.addr, 0, 60);
	fread(gCurrentConnection.addr, 1, 59, readCurAddr);
	fclose(readCurAddr);
}

void showConnectionStatus(bool isConnected)
{
	if (isConnected)
	{
		ShowWindow(bttnConnect, SW_HIDE);
		ShowWindow(bttnCancelConnect, SW_HIDE);
		ShowWindow(bttnDisconnect, SW_SHOW);

		Static_SetText(sttcConnectStatus, localizeConst(VPN_STATUS_CONNECTED));
	}
	else
	{
		ShowWindow(bttnConnect, SW_SHOW);
		ShowWindow(bttnCancelConnect, SW_HIDE);
		ShowWindow(bttnDisconnect, SW_HIDE);

		Static_SetText(sttcConnectStatus, localizeConst(VPN_STATUS_DISCONNECTED));
	}
}

bool lineIsJustWhitespace(char* theLine)
{
	if (theLine[0] == 0)
		return true;
	char* cur = theLine;
	while (*cur)
	{
		if (!isspace(*cur))
			return false;
		cur++;
	}
	return true;
}

__int64 getHNsecsSince1600()
{
	SYSTEMTIME curTime;
	FILETIME asFileTime;
	GetSystemTime(&curTime);
	SystemTimeToFileTime(&curTime, &asFileTime);
	ULARGE_INTEGER anotherStep;
	anotherStep.u.HighPart = asFileTime.dwHighDateTime;
	anotherStep.u.LowPart = asFileTime.dwLowDateTime;
	return anotherStep.QuadPart;
}

//using the same password on all servers can actually lead to some kind of bad mischief, so the actual per-server password will be derived (as below) from sha1(base_password, server_ip)
//NOTE: pwToUse must be size VPN_DERIVED_PASSWORD_LENGTH + 1
void derivePassword(char* pwToUse, char* gBaseVPNPassword, char* serverIP_Addr)
{
	char shaInBuf[300];
	unsigned char shaOut[20];
	strcpy(shaInBuf, gBaseVPNPassword);
	strcat(shaInBuf, serverIP_Addr);

	sha1((unsigned char*)shaInBuf, strlen(shaInBuf), shaOut);
	
	pwToUse[VPN_DERIVED_PASSWORD_LENGTH] = 0;
	for (int j = 0; j < VPN_DERIVED_PASSWORD_LENGTH; j++)
		pwToUse[j] = (char)('a' + shaOut[j] % 26);
}

//NOTE: pwToUse must be size VPN_DERIVED_PASSWORD_LENGTH + 1
void deriveUsername(char* unToUse, char* gBaseVPNPassword, char* serverIP_Addr)
{
	char modifiedIP[60];
	strcpy(modifiedIP, serverIP_Addr);
	for (int i = 0; modifiedIP[i]; i++)
		modifiedIP[i]++;

	char shaInBuf[300];
	unsigned char shaOut[20];
	strcpy(shaInBuf, gBaseVPNPassword);
	strcat(shaInBuf, modifiedIP);

	sha1((unsigned char*)shaInBuf, strlen(shaInBuf), shaOut);

	unToUse[VPN_DERIVED_PASSWORD_LENGTH] = 0;
	for (int j = 0; j < VPN_DERIVED_PASSWORD_LENGTH; j++)
		unToUse[j] = (char)('a' + shaOut[j] % 26);
}

void reportSoftEtherError()
{
	DWORD retSize;
	LPTSTR pTemp = NULL;

	retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
		NULL, GetLastError(), LANG_NEUTRAL, (LPTSTR)&pTemp, 0, NULL);

	MessageBoxW(NULL, pTemp, L"Error running SoftEther; is it installed correctly?", MB_OK);
	LocalFree(pTemp);
}

void systemNice(char* execMe)
{
	wchar_t toExecW[EXEC_VPNCMD_BUFSIZE];
	mbstowcs(toExecW, execMe, EXEC_VPNCMD_BUFSIZE);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));


	if (!CreateProcess(NULL, toExecW, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		reportSoftEtherError();
	WaitForSingleObject(pi.hProcess, INFINITE);
}

void uuidBinToString(WCHAR* dest, BYTE* src)
{
	wsprintf(dest, L"%x-%x-%x-%x-%x%x", 
		*(DWORD*)(src),
		*(WORD*)(src + sizeof(DWORD)),
		*(WORD*)(src + sizeof(DWORD) + sizeof(WORD)),
		*(WORD*)(src + sizeof(DWORD) + 2 * sizeof(WORD)),
		*(WORD*)(src + sizeof(DWORD) + 3 * sizeof(WORD)),
		*(DWORD*)(src + sizeof(DWORD) + 4 * sizeof(WORD)));
	dest[14] = L'4';//version 4
	dest[19] = L'a';//¯\_(ツ)_/¯
}

void writeMobileconfigToFile(WCHAR* fname, WCHAR* ipAddr, WCHAR* psk, WCHAR* vpnName, WCHAR* vpnPW)
{
	//NOTE minor detail: need to encode the PSK as base64
	char pskA[20];
	wcstombs(pskA, psk, 20);
	WCHAR psk64[40];
	DWORD outputBufSize = 40;
	CryptBinaryToString((const BYTE*)pskA, strlen(pskA), CRYPT_STRING_BASE64, psk64, &outputBufSize);
	if (wcschr(psk64, L'\r'))
		*wcschr(psk64, L'\r') = 0;
	if (wcschr(psk64, L'\n'))
		*wcschr(psk64, L'\n') = 0;

	//Random UUIDs appear to work fine, for whatever Apple is using them for...
	HCRYPTPROV cryptProvider = 0;
	BYTE randData[4 * 10 + 6];
	CryptAcquireContext(&cryptProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(cryptProvider, 4 * 10 + 6, randData);
	CryptReleaseContext(cryptProvider, 0);

	BYTE uuid1Data[16];
	BYTE uuid2Data[16];
	BYTE uuid3Data[16];
	BYTE uuidHostData[16];
	WCHAR uuid1[50];
	WCHAR uuid2[50];
	WCHAR uuid3[50];
	WCHAR uuidHost[50];

	memcpy(uuid1Data + 10, randData + 40, 6);
	memcpy(uuid2Data + 10, randData + 40, 6);
	memcpy(uuid3Data + 10, randData + 40, 6);
	memcpy(uuidHostData + 10, randData + 40, 6);

	memcpy(uuid1Data, randData, 10);
	memcpy(uuid2Data, randData + 10, 10);
	memcpy(uuid3Data, randData + 20, 10);
	memcpy(uuidHostData, randData + 30, 10);

	uuidBinToString(uuid1, uuid1Data);
	uuidBinToString(uuid2, uuid2Data);
	uuidBinToString(uuid3, uuid3Data);
	uuidBinToString(uuidHost, uuidHostData);

	std::wofstream writeFile(fname);
	writeFile <<
L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n\
<plist version = \"1.0\">\n\
<dict>\n\
<key>PayloadContent</key>\n\
<array>\n\
<dict>\n\
<key>IPSec</key>\n\
<dict>\n\
<key>AuthenticationMethod</key>\n\
<string>SharedSecret</string>\n\
<key>LocalIdentifierType</key>\n\
<string>KeyID</string>\n\
<key>SharedSecret</key>\n\
<data>" << psk64 << L"</data>\n\
</dict>\n\
<key>IPv4</key>\n\
<dict>\n\
<key>OverridePrimary</key>\n\
<integer>1</integer>\n\
</dict>\n\
<key>PPP</key>\n\
<dict>\n\
<key>AuthName</key>\n\
<string>" << vpnName << L"</string>\n\
<key>AuthPassword</key>\n\
<string>" << vpnPW << L"</string>\n\
<key>CommRemoteAddress</key>\n\
<string>" << ipAddr << L"</string>\n\
</dict>\n\
<key>PayloadDescription</key>\n\
<string>Configures VPN settings</string>\n\
<key>PayloadDisplayName</key>\n\
<string>VPN: " << ipAddr << L"</string>\n\
<key>PayloadIdentifier</key>\n\
<string>dummyhost." << uuidHost << L".com.apple.vpn.managed." << uuid3 << L"</string>\n\
<key>PayloadType</key>\n\
<string>com.apple.vpn.managed</string>\n\
<key>PayloadUUID</key>\n\
<string>" << uuid1 << L"</string>\n\
<key>PayloadVersion</key>\n\
<real>1</real>\n\
<key>Proxies</key>\n\
<dict/>\n\
<key>UserDefinedName</key>\n\
<string>" << ipAddr << L"</string>\n\
<key>VPNType</key>\n\
<string>L2TP</string>\n\
</dict>\n\
</array>\n\
<key>PayloadDisplayName</key>\n\
<string>" << ipAddr << L"</string>\n\
<key>PayloadIdentifier</key>\n\
<string>dummyhost." << uuidHost << L"</string>\n\
<key>PayloadOrganization</key>\n\
<string>Salmon</string>\n\
<key>PayloadRemovalDisallowed</key>\n\
<false/>\n\
<key>PayloadType</key>\n\
<string>Configuration</string>\n\
<key>PayloadUUID</key>\n\
<string>" << uuid2 << L"</string>\n\
<key>PayloadVersion</key>\n\
<integer>1</integer>\n\
</dict>\n\
</plist>" << std::endl;

	writeFile.close();
}
