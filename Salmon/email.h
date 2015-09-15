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
#include <string>

#include "salmon_constants.h"
#include "connect_attempt.h"
#include "VPNInfo.h"

extern char gUserEmailAccount[EMAIL_ADDR_BUFSIZE];
extern char gUserEmailPassword[EMAIL_PASSWORD_BUFSIZE];

enum SendMailSuccess
{
	SEND_MAIL_FAIL = 0,
	SEND_MAIL_SUCCESS = 1,
	SEND_MAIL_MANUAL = 2
};

enum RecvMailCodes
{
	RECV_MAIL_FAIL = -1,
	RECV_MAIL_CANCEL = 0,
	RECV_MAIL_SUCCESS = 1
};

bool validateEmailAddress(const char* theAddress);

SendMailSuccess sendMessageToDirServ(const std::wstring& messageToSend, void(*theCallback)(RecvMailCodes, std::string));
bool sendSelfMail(const WCHAR* send_buf, const WCHAR* mailSubject);
bool sendMobileconfigs();

class RecvThreadArguments
{
public:
	std::string randomString;
	void(*callback)(RecvMailCodes, std::string);
	bool sendingIsManual;

	RecvThreadArguments(const std::string& rStr, void(*theCB)(RecvMailCodes, std::string), bool isManual)
	{
		callback = theCB;
		randomString = rStr;
		sendingIsManual = isManual;
	}

private:
	RecvThreadArguments(){}
};


extern HANDLE recvThreadMutex;


//parameter is a RecvThreadStruct pointer
DWORD WINAPI recvThread(LPVOID lpParam);
DWORD recvThreadFunction(RecvThreadArguments* theArgs, std::string* optionalOutString);

std::wstring constructVPNListEmailForAndroid();

#endif //_SALMON_EMAIL_INCLGUARD_H_