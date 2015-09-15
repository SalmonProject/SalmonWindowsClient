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

#ifndef _SALMON_INCLGUARG_MANUAL_OR_BLOCKING_EMAIL_H_
#define _SALMON_INCLGUARG_MANUAL_OR_BLOCKING_EMAIL_H_

#include <string>
#define NOMINMAX
#include <Windows.h>

//The manual send/recv process involves the user interacting with GUI elements in between
//the sending and receiving. This means that the thread that collects the response data
//is the main GUI thread, so to get the data back to whatever piece of logic initiated the
//send/recv, we need some global stuff. This class gathers it all together (and also covers
//the case where the requesting logic wanted to block until the response arrived).
class ManualEmailGlobalHelper
{
protected:
	//If requested (shouldSignalWhenReady = true), the manual-email window signals responseReadyEvent to let 
	//the synchronous send-and-receive know that responseText now contains the server's reply.
	//(The manual-email window also sets gManualPleaseSignal back to false).
	HANDLE responseReadyEvent;
	std::string responseText;
	bool shouldSignalWhenReady;

	//If the manual-email stuff is NOT using event signalling, it's instead using this callback.
	void(*manualEmailCallback)(RecvMailCodes, std::string);

public:

	//Are we currently actually using this stuff? Toggled by chekManualEmail.
	bool enabled;

	ManualEmailGlobalHelper();

	SendMailSuccess initiateAsyncManualEmail(std::wstring requestText, void(*theCallback)(RecvMailCodes, std::string));
	SendMailSuccess blockingManualSendRecv(const std::wstring& messageToSend, std::string* retString);
	void resolve(std::string receviedResponse);
};


//If you have a thread (hopefully not the main GUI one!!!), and would like to do a nice,
//simple, send-and-block-until-response-received with the directory server, then this is the
//function for you. The returned struct's .response field will have the text of the server's
//response, and if sending didn't even work, .sendStatus will let you know (SEND_MAIL_FAIL).
struct SyncMailReturn
{
	SendMailSuccess sendStatus;
	bool receivedAnything;
	std::string response;
};
SyncMailReturn sendAndRecvMailBlocking(const std::wstring& messageToSend);

#endif //_SALMON_INCLGUARG_MANUAL_OR_BLOCKING_EMAIL_H_
