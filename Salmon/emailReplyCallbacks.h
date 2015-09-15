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

#ifndef _SALMON_INCLGUARD_EMAIL_REPLY_CALLBACKS_H_
#define _SALMON_INCLGUARD_EMAIL_REPLY_CALLBACKS_H_

#include <string>

#include "email.h"

//The on-response-received callbacks for the various parts of the program logic that communicate with the dir server
void loginMailCallback(RecvMailCodes successful, std::string responseText);
void getRecMailCallback(RecvMailCodes successful, std::string textResponse);
void redeemCodeMailCallback(RecvMailCodes successful, std::string responseText);
void recRegMailCallback(RecvMailCodes successful, std::string responseText);
void finishRegMailCallback(RecvMailCodes successful, std::string responseText);
void startRegMailCallback(RecvMailCodes successful, std::string responseText);

void noOpMailCallback(RecvMailCodes dummy, std::string dummy2);

//Helper function for calling one of these callbacks in its own thread
void asyncCallCallback(void(*theCallback)(RecvMailCodes, std::string), std::string stringArg);

#endif //_SALMON_INCLGUARD_EMAIL_REPLY_CALLBACKS_H_
