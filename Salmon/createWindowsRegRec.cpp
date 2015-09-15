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

#include "hwnds.h"
#include "localization.h"
#include "salmon_utility.h"
#include "email.h"
#include "control_softether.h"
#include "emailReplyCallbacks.h"

void createWindowsRec(LPCWSTR className, HINSTANCE thisInstance)
{
	//register with rec code
	wndwRegisterRecd = CreateWindowEx(
		WS_EX_CLIENTEDGE,                   // Extended possibilites for variation 
		className,         // Classname 
		localizeConst(SALMON_REG_TITLE),       // Title Text 
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
		CW_USEDEFAULT,       // Windows decides the position 
		CW_USEDEFAULT,       // where the window ends up on the screen 
		350,                 // The programs width 
		296,                 // and height in pixels 
		HWND_DESKTOP,        // The window is a child-window to desktop 
		NULL,                // No menu 
		thisInstance,       // Program Instance handler 
		NULL                 // No Window Creation data 
		);

	const int sttcRegRecCodeY = 4;
	sttcRegRecCode = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(REC_CODE_FROM_FRIEND),// text
		WS_VISIBLE | WS_CHILD,  // styles
		10,         // starting x position
		sttcRegRecCodeY,         // starting y position
		310,        // width
		16,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textRegRecCodeY = sttcRegRecCodeY + 22;
	textRegRecCode = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | WS_TABSTOP,  // styles
		10,         // starting x position
		textRegRecCodeY,         // starting y position
		96,        // width
		23,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcRecEmailAddrY = textRegRecCodeY + 28;
	sttcRecEmailAddr = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(PROMPT_EMAIL_ADDR_VERBOSE),// text
		WS_VISIBLE | WS_CHILD,  // styles
		10,         // starting x position
		sttcRecEmailAddrY,         // starting y position
		310,        // width
		80,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textRecEmailAddrY = sttcRecEmailAddrY + 86;
	textRecEmailAddr = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | WS_TABSTOP,  // styles
		10,         // starting x position
		textRecEmailAddrY,         // starting y position
		310,        // width
		23,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcRecEmailPWY = textRecEmailAddrY + 28;
	sttcRecEmailPW = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(PROMPT_EMAIL_PW_VERBOSE),// text
		WS_VISIBLE | WS_CHILD,  // styles
		10,         // starting x position
		sttcRecEmailPWY,         // starting y position
		240,        // width
		16,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed

	bttnIsntThisUnsafeRecReg = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(ISNT_THIS_UNSAFE_BUTTON),// text
		WS_VISIBLE | WS_CHILD,  // styles
		190,         // starting x position
		sttcRecEmailPWY - 2,         // starting y position
		110,        // width
		19,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textRecEmailPWY = sttcRecEmailPWY + 20;
	textRecEmailPW = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | ES_PASSWORD | WS_TABSTOP,  // styles
		10,         // starting x position
		textRecEmailPWY,         // starting y position
		310,        // width
		26,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int bttnRecRegSubmitY = textRecEmailPWY + 34;
	bttnRecRegSubmit = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(SUBMIT_REGISTRATION),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,  // styles
		96,         // starting x position
		bttnRecRegSubmitY,         // starting y position
		140,        // width
		32,        // height
		wndwRegisterRecd,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterRecd, GWL_HINSTANCE),
		NULL);      // pointer not needed


	SendMessage(sttcRegRecCode, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textRegRecCode, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcRecEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textRecEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcRecEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnIsntThisUnsafeRecReg, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textRecEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnRecRegSubmit, WM_SETFONT, (WPARAM)gFontHandle, 0);
}


void disableAllButtonsRecReg()
{
	Button_Enable(bttnRecRegSubmit, FALSE);
}

void enableAllButtonsRecReg()
{
	Button_Enable(bttnRecRegSubmit, TRUE);
}


void winProcRecd(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND && (HWND)lParam == bttnRecRegSubmit)
	{
		//recdRegistration
		//nationality
		//recommendation code
		std::wstring toSend = L"recdRegistration\n" + std::wstring(countryFromLanguage(gChosenLanguage)) + L"\n";

		WCHAR theRecCode[20];
		Edit_GetText(textRegRecCode, theRecCode, 20);
		if (wcslen(theRecCode) != 7)
		{
			MessageBox(NULL, localizeConst(WRONG_RECCODE_LENGTH), localizeConst(ERROR_STR), MB_OK);
			return;
		}
		toSend += theRecCode;

		WCHAR getLogin[EMAIL_ADDR_BUFSIZE];
		WCHAR getPassword[EMAIL_PASSWORD_BUFSIZE];
		Edit_GetText(textRecEmailAddr, getLogin, EMAIL_ADDR_BUFSIZE);
		Edit_GetText(textRecEmailPW, getPassword, EMAIL_PASSWORD_BUFSIZE);
		wcstombs(gUserEmailAccount, getLogin, EMAIL_ADDR_BUFSIZE);
		wcstombs(gUserEmailPassword, getPassword, EMAIL_PASSWORD_BUFSIZE);

		//ensure address ends in @ something we can handle - if not, TELL THEM!
		if (!validateEmailAddress(gUserEmailAccount))
		{
			MessageBox(NULL, localizeConst(INVALID_EMAIL_ADDR), localizeConst(ERROR_STR), MB_OK);
			return;
		}
		//save their email account name (but not password) to file, so it can be pre-filled next time
		FILE* writeEmailSetting = openConfigFile("CurrentEmail.txt", "wt");
		if (writeEmailSetting)
		{
			fwrite(gUserEmailAccount, 1, strlen(gUserEmailAccount), writeEmailSetting);
			fclose(writeEmailSetting);
		}

		sendMessageToDirServ(toSend, recRegMailCallback);
	}
	else if (message == WM_COMMAND && (HWND)lParam == bttnIsntThisUnsafeRecReg)
		MessageBox(NULL, localizeConst(WHY_EMAIL_PASSWORD), L"", MB_OK);
}
