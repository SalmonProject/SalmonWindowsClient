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

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <Commdlg.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "salmon_globals.h"
#include "salmon_utility.h"
#include "hwnds.h"
#include "localization.h"

void createWindowsManualEmail(LPCWSTR className, HINSTANCE thisInstance)
{
	const int standardX = 20;
	const int standardWidth = 466;

	wndwManualEmail = CreateWindowEx(
		WS_EX_CLIENTEDGE, // Extended possibilites for variation 
		className,         // Classname 
		L"Salmon - Manual Email",       // Title Text 
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
		CW_USEDEFAULT,       // Windows decides the position 
		CW_USEDEFAULT,       // where the window ends up on the screen 
		520,                 // The programs width 
		496,                 // and height in pixels 
		HWND_DESKTOP,        // The window is a child-window to desktop 
		NULL,                // No menu 
		thisInstance,       // Program Instance handler 
		NULL                 // No Window Creation data 
		);

	const int sttcManualSendToY = 16;
	sttcManualEmailTo = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(COPY_THIS_ADDRESS_TO_SEND),// text
		WS_VISIBLE | WS_CHILD,  // styles
		standardX,         // starting x position
		sttcManualSendToY,         // starting y position
		standardWidth,        // width
		16,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textManualSendToY = sttcManualSendToY + 20;
	textManualEmailTo = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"cs-backend-salmon@mx.uillinois.edu",       // text
		WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | WS_TABSTOP | ES_READONLY,  // styles
		standardX,         // starting x position
		textManualSendToY,         // starting y position
		standardWidth,        // width
		23,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcManualSubjectY = textManualSendToY + 27;
	sttcManualEmailSubject = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(COPY_THIS_SUBJECT),// text
		WS_VISIBLE | WS_CHILD,  // styles
		standardX,         // starting x position
		sttcManualSubjectY,         // starting y position
		standardWidth,        // width
		16,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textManualSubjectY = sttcManualSubjectY + 20;
	textManualEmailSubject = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"USER SHOULD NOT SEE THIS",       // text
		WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | WS_TABSTOP | ES_READONLY,  // styles
		standardX,         // starting x position
		textManualSubjectY,         // starting y position
		standardWidth,        // width
		23,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcManualBodyY = textManualSubjectY + 27;
	sttcManualEmailBody = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(COPY_THIS_BODY),// text
		WS_VISIBLE | WS_CHILD,  // styles
		standardX,         // starting x position
		sttcManualBodyY,         // starting y position
		standardWidth,        // width
		16,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textManualBodyY = sttcManualBodyY + 20;
	textManualEmailBody = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"USER SHOULD NOT SEE THIS",       // text
		WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | WS_TABSTOP | ES_READONLY,  // styles
		standardX,         // starting x position
		textManualBodyY,         // starting y position
		standardWidth,        // width
		200,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcManualFindFileDescriptionY = textManualBodyY + 204;
	sttcManualFindFileDescription = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(USE_THIS_BUTTON_TO_SELECT_ATTACHMENT),// text
		WS_VISIBLE | WS_CHILD,  // styles
		standardX,         // starting x position
		sttcManualFindFileDescriptionY,         // starting y position
		standardWidth,        // width
		60,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int bttnManualEmailFindRecvdY = sttcManualFindFileDescriptionY + 60;
	bttnManualEmailFindRecvd = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(OPEN_RECEIVED_ATTACHMENT),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP | BS_MULTILINE,  // styles
		196,         // starting x position
		bttnManualEmailFindRecvdY, // starting y position
		100,        // width
		60,        // height
		wndwManualEmail,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	// set the new font
	SendMessage(sttcManualEmailTo, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textManualEmailTo, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcManualEmailSubject, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textManualEmailSubject, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcManualEmailBody, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textManualEmailBody, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcManualFindFileDescription, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnManualEmailFindRecvd, WM_SETFONT, (WPARAM)gFontHandle, 0);
}


//Alright... sometimes the file opening/reading is taking tens of seconds; not sure why, 
//but anyways we should be able to deal with it. (What if they're opening the file from 
//some crazy networked drive across the world?) So, here is a thread to do the stuff that
//would go at the end of the bttnManualEmailFindRecvd action. (Additionally, though, we're
//going to disable bttnManualEmailFindRecvd until the processing is done, to let the user
//know that something is happening and prevent them from starting this thread twice.)
DWORD WINAPI processManualResponse(LPVOID lpParam)
{
	std::wstring* passedString = (std::wstring*)lpParam;
	std::wstring downloadedFileName(passedString->c_str());
	delete passedString;

	FILE* openedAttachment = _wfopen(downloadedFileName.c_str(), L"rt");
	if (!openedAttachment)
	{
		MessageBox(NULL, localizeConst(FILE_DOES_NOT_EXIST), localizeConst(ERROR_STR), MB_OK);
		Button_Enable(bttnManualEmailFindRecvd, TRUE);
		UpdateWindow(wndwManualEmail);
		return 0;
	}

	string dirResponseFromAttachment;

	//read the specified file into dirResponseFromAttachment, and then delete the file
	fseek(openedAttachment, 0, SEEK_END);
	int attachFileLen = ftell(openedAttachment);
	rewind(openedAttachment);
	char* attachBuffer = new char[attachFileLen + 1];
	fread(attachBuffer, 1, attachFileLen, openedAttachment);
	fclose(openedAttachment);
	attachBuffer[attachFileLen] = 0;
	dirResponseFromAttachment = string(attachBuffer);
	delete attachBuffer;
	_wunlink(downloadedFileName.c_str());

	//We're now done with the manual email window - hide it, and reenable other windows.
	enableAllButtonsAll();
	ShowWindow(wndwManualEmail, SW_HIDE);
	Button_Enable(bttnManualEmailFindRecvd, TRUE);
	UpdateWindow(wndwManualEmail);

	gManualEmail.resolve(dirResponseFromAttachment);

	return 0;
}

void winProcManualEmail(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND && (HWND)lParam == bttnManualEmailFindRecvd)
	{
		OPENFILENAME ofn;       // common dialog box structure
		WCHAR szFile[260];       // buffer for file name
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
		// use the contents of szFile to initialize itself.
		szFile[0] = 0;

		// Initialize OPENFILENAME
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = L"Salmon Emails\0*.SALMONATTACH\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_DONTADDTORECENT;

		if (GetOpenFileName(&ofn))
		{
			//Disable the button that started all of this until the processing thread is done,
			//so the user can't start a second instance of this at the same time.
			Button_Enable(bttnManualEmailFindRecvd, FALSE);
			UpdateWindow(wndwManualEmail);
			std::wstring* passFilename = new std::wstring(ofn.lpstrFile); //deleted in thread
			CreateThread(NULL, 0, processManualResponse, passFilename, 0, NULL);
		}
	}
}
