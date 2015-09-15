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
#include "Resource.h"

#define NOMINMAX
#include <Windows.h>
#include <WindowsX.h>

#include <string>
using std::string;

#include "salmon_constants.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "hwnds.h"
#include "localization.h"
#include "email.h"
#include "connect_attempt.h"
#include "control_softether.h"

#include "manual_or_blocking_email.h"

string generateEmailRandomID();
SendMailSuccess sendEmailToSalmonBackend(const std::wstring& messageToSend, const string& rndStr);

//Shows the manual email window, after pre-filling its text boxes with what we want the user to send.
//When the user finally provides the dir server's response, theCallback(optionalResultStorage) will be
//called. (optionalResultStorage can be null, of course; it's just for the hacky synchronous version).
void openManualMailWindow(const std::wstring& send_buf)
{
	string emailRandomID = generateEmailRandomID();

	Edit_SetText(textManualEmailTo, L"cs-backend-salmon@mx.uillinois.edu");
	WCHAR setEmailSubject[500];
	mbstowcs(setEmailSubject, (string("salmon msgInBody ID=" + emailRandomID)).c_str(), 500);

	Edit_SetText(textManualEmailSubject, setEmailSubject);
	Edit_SetText(textManualEmailBody, send_buf.c_str());

	disableAllButtonsAll();
	ShowWindow(wndwManualEmail, SW_SHOW);
}

//If you have a thread (hopefully not the main GUI one!!!), and would like to do a nice,
//simple, send-and-block-until-response-received with the directory server, then this is the
//function for you. The returned struct's .response field will have the text of the server's
//response, and if sending didn't even work, .sendStatus will let you know (SEND_MAIL_FAIL).
SyncMailReturn sendAndRecvMailBlocking(const std::wstring& messageToSend)
{
	SyncMailReturn toReturn;

	if (gManualEmail.enabled)
		toReturn.sendStatus = gManualEmail.blockingManualSendRecv(messageToSend, &toReturn.response);
	else
	{
		string myEmailID = generateEmailRandomID();

		SendMailSuccess retSuccess = sendEmailToSalmonBackend(messageToSend, myEmailID);
		if (retSuccess == SEND_MAIL_FAIL)
		{
			toReturn.response = "";
			toReturn.sendStatus = retSuccess;
		}

		//To get our synchronous behavior: rather than creating the recv thread, call the recvThreadFunction()
		//that recvThread() wraps, but with a non-0 second argument (unlike what recvThread() does).
		//This causes the directory's response to be stored into that second argument. To complete it all,
		//the required callback function is a no-op.
		RecvThreadArguments* threadStruct = new RecvThreadArguments(myEmailID, noOpMailCallback, false);
		//(new'd argument struct deleted in recvThread 'thread')
		recvThreadFunction(threadStruct, &toReturn.response);
		toReturn.sendStatus = retSuccess;
	}
	toReturn.receivedAnything = (toReturn.response.length() > 0);
	return toReturn;
}



//The manual send/recv process involves the user interacting with GUI elements in between
//the sending and receiving. This means that the thread that collects the response data
//is the main GUI thread, so to get the data back to whatever piece of logic initiated the
//send/recv, we need some global stuff. This class gathers it all together (and also covers
//the case where the requesting logic wanted to block until the response arrived).

ManualEmailGlobalHelper::ManualEmailGlobalHelper()
{
	shouldSignalWhenReady = false;
	responseReadyEvent = CreateEvent(
		NULL,               // default security attributes
		FALSE,               // auto-reset event
		FALSE,              // initial state is nonsignaled
		TEXT("ManualEmailReadyResponseEvent"));  // object name
	responseText = "";
	manualEmailCallback = noOpMailCallback;
	enabled = false;
}

SendMailSuccess ManualEmailGlobalHelper::initiateAsyncManualEmail(std::wstring requestText, void(*theCallback)(RecvMailCodes, string))
{
	manualEmailCallback = theCallback;
	shouldSignalWhenReady = false;
	openManualMailWindow(requestText);
	return SEND_MAIL_MANUAL;
}

SendMailSuccess ManualEmailGlobalHelper::blockingManualSendRecv(const std::wstring& messageToSend, string* retString)
{
	shouldSignalWhenReady = true;
	openManualMailWindow(messageToSend);
	WaitForSingleObject(responseReadyEvent, INFINITE);
	*retString = responseText;
	return SEND_MAIL_MANUAL;
}

void ManualEmailGlobalHelper::resolve(string receviedResponse)
{
	if (shouldSignalWhenReady) //we are resolving a blocking query; just signal that responseText is ready
	{
		responseText = receviedResponse;
		shouldSignalWhenReady = false;
		SetEvent(responseReadyEvent);
	}
	else //we are resolving an async query; call the callback (in a new thread, since we are probably coming from the GUI thread)
		asyncCallCallback(manualEmailCallback, receviedResponse);
}
