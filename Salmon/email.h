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

#ifndef _SALMON_EMAIL_INCLGUARD_H_
#define _SALMON_EMAIL_INCLGUARD_H_

#include <vector>

#include "salmon_constants.h"
#include "connect_attempt.h"

extern char gUserEmailAccount[EMAIL_ADDR_BUFSIZE];
extern char gUserEmailPassword[EMAIL_PASSWORD_BUFSIZE];

bool sendMail(const WCHAR* send_buf, char* rndStr);
bool sendSelfMail(const WCHAR* send_buf, const WCHAR* mailSubject);
bool sendMobileconfigs();

enum RecvMailCodes
{
	RECV_MAIL_FAIL = -1,
	RECV_MAIL_CANCEL = 0,
	RECV_MAIL_SUCCESS = 1
};

class RecvThreadStruct
{
public:
	char* randomString;
	char* buffer;
	int charsRecvd;
	void(*callback)(RecvMailCodes);

	RecvThreadStruct(char* rStr, void(*theCB)(RecvMailCodes))
	{
		buffer = 0;
		charsRecvd = 0;
		callback = theCB;
		randomString = strdup(rStr);
	}
	~RecvThreadStruct()
	{
		free(buffer);
		free(randomString);
	}
	void receiveData(const char* data, int length)
	{
		buffer = (char*)malloc(length + 1);
		memcpy(buffer, data, length);
		buffer[length] = 0;
		charsRecvd = length;
	}

private:
	RecvThreadStruct(){}
};



void nullMailCallback(RecvMailCodes dummy);

extern HANDLE recvThreadMutex;


//parameter is a RecvThreadStruct pointer
DWORD WINAPI recvThread(LPVOID lpParam);

enum NeedServerSuccess
{
	NEED_SERVER_GOT_NONE = 0,
	NEED_SERVER_GOT_SALMON = 1,
	NEED_SERVER_GOT_VPNGATE = 2
};
NeedServerSuccess needServer(const ConnectAnyVPNAttemptResult& res, std::vector<VPNInfo>* VPNGateServers);

#endif 