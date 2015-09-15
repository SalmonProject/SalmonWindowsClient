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

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "salmon_globals.h"
#include "hwnds.h"

#include "localization.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "email.h"
#include "control_softether.h"
#include "emailReplyCallbacks.h"

void createWindowsLogin(LPCWSTR className, HINSTANCE thisInstance)
{


	wndwLogin = CreateWindowEx(
		WS_EX_CLIENTEDGE, // Extended possibilites for variation 
		className,         // Classname 
		localizeConst(LOGIN_OR_REG_TITLE),       // Title Text 
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
		CW_USEDEFAULT,       // Windows decides the position 
		CW_USEDEFAULT,       // where the window ends up on the screen 
		320,                 // The programs width 
		420,                 // and height in pixels 
		HWND_DESKTOP,        // The window is a child-window to desktop 
		NULL,                // No menu 
		thisInstance,       // Program Instance handler 
		NULL                 // No Window Creation data 
		);

	const int framFirstTimeY = 10;
	//login or register window
	framFirstTime = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(FIRST_TIME_USER_Q),// text
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
		10,         // starting x position
		framFirstTimeY, // starting y position
		282,        // width
		88,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int bttnRegisterSocialY = framFirstTimeY + 22;
	bttnRegisterSocial = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(REG_FB_OR_RENREN),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
		20,         // starting x position
		bttnRegisterSocialY,// starting y position
		120,        // width
		60,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int bttnRegisterRecdY = framFirstTimeY + 22;
	bttnRegisterRecd = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(REG_RECOMMENDED),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
		154,         // starting x position
		bttnRegisterRecdY, // starting y position
		128,        // width
		60,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int framExistingUserY = framFirstTimeY + 98;
	framExistingUser = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(EXISTING_USER_Q),// text
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
		10,         // starting x position
		framExistingUserY,         // starting y position
		282,        // width
		168,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int sttcLoginEmailAddrY = framExistingUserY + 22;
	sttcLoginEmailAddr = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(PROMPT_EMAIL_ADDR),// text
		WS_VISIBLE | WS_CHILD,  // styles
		20,         // starting x position
		sttcLoginEmailAddrY,         // starting y position
		262,        // width
		16,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	WCHAR emailW[EMAIL_ADDR_BUFSIZE];
	mbstowcs(emailW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
	const int textLoginEmailAddrY = sttcLoginEmailAddrY + 20;
	textLoginEmailAddr = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		emailW,       // text
		WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL | WS_TABSTOP,  // styles
		20,         // starting x position
		textLoginEmailAddrY,         // starting y position
		262,        // width
		23,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcLoginEmailPWY = textLoginEmailAddrY + 27;
	sttcLoginEmailPW = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(PROMPT_EMAIL_PASSWORD),// text
		WS_VISIBLE | WS_CHILD,  // styles
		20,         // starting x position
		sttcLoginEmailPWY,         // starting y position
		140,        // width
		16,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	bttnIsntThisUnsafeMain = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(ISNT_THIS_UNSAFE_BUTTON),// text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
		173,         // starting x position
		sttcLoginEmailPWY - 2,         // starting y position
		110,        // width
		21,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textLoginEmailPWY = sttcLoginEmailPWY + 21;
	textLoginEmailPW = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | ES_PASSWORD | WS_TABSTOP,  // styles
		20,         // starting x position
		textLoginEmailPWY, // starting y position
		262,        // width
		26,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	bttnLogin = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(LOGIN_BUTTON),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,  // styles
		115,         // starting x position
		textLoginEmailPWY + 32, // starting y position
		80,        // width
		40,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int framCantEmailY = framExistingUserY + 178;
	framCantEmailFrame = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(SALMON_CANT_EMAIL_Q),// text
		WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
		10,         // starting x position
		framCantEmailY,         // starting y position
		282,        // width
		92,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwLogin, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int chekManualEmailY = framCantEmailY + 16;
	chekManualEmail = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(MANUAL_EMAIL_TOGGLE_CAPTION),       // text
		WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX | BS_MULTILINE,  // styles
		20,         // starting x position
		chekManualEmailY,         // starting y position
		260,        // width
		72,        // height
		wndwLogin,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	/*gFontHandle = CreateFont(-48, 0, 0, 0, FW_BOLD, FALSE,
	FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
	L"Calibri");
	if (gFontHandle == NULL)
	MessageBox(NULL, L"aww error", L"errr", MB_OK);
	SendMessage(bttnLogin, WM_SETFONT, (WPARAM)gFontHandle, (LPARAM)MAKELONG(TRUE, 0));
	SendMessage(sttcLoginEmailPW, WM_SETFONT, (WPARAM)gFontHandle, (LPARAM)MAKELONG(TRUE, 0));*/

	// set the new font
	SendMessage(framFirstTime, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnRegisterSocial, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnRegisterRecd, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(framExistingUser, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcLoginEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcLoginEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnIsntThisUnsafeMain, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textLoginEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textLoginEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnLogin, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(framCantEmailFrame, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(chekManualEmail, WM_SETFONT, (WPARAM)gFontHandle, 0);
}

void enableAllButtonsLogin()
{
	Button_Enable(bttnRegisterSocial, TRUE);
	Button_Enable(bttnRegisterRecd, TRUE);
	Button_Enable(bttnLogin, TRUE);
	Button_Enable(chekManualEmail, TRUE);
}

void disableAllButtonsLogin()
{
	Button_Enable(bttnRegisterSocial, FALSE);
	Button_Enable(bttnRegisterRecd, FALSE);
	Button_Enable(bttnLogin, FALSE);
	Button_Enable(chekManualEmail, FALSE);
}

void updateGUIWithManualEmailStatus(bool theStatus)
{
	//hide / show email+password boxes and warning buttons: they're not applicable when doing manual email
	ShowWindow(sttcLoginEmailPW, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(bttnIsntThisUnsafeMain, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(textLoginEmailPW, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(sttcSocEmailPW, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(bttnIsntThisUnsafeSocReg, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(textSocEmailPW, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(sttcRecEmailPW, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(bttnIsntThisUnsafeRecReg, theStatus ? SW_HIDE : SW_SHOW);
	ShowWindow(textRecEmailPW, theStatus ? SW_HIDE : SW_SHOW);
	//Actually... do prompt for an email, even though we don't need it: it will help the user
	//realize that their Salmon usage is supposed to be associated with a specific email account.
	//ShowWindow(sttcRecEmailAddr, theStatus ? SW_HIDE : SW_SHOW);
	//ShowWindow(textRecEmailAddr, theStatus ? SW_HIDE : SW_SHOW);

	//change the email box labels to show the appropriate string: restrict to gmail etc if not manual
	SetWindowText(sttcRecEmailAddr, localizeConst(theStatus ? ENTER_ANY_EMAIL_ADDRESS_VERBOSE : PROMPT_EMAIL_ADDR_VERBOSE));
	SetWindowText(sttcSocEmailAddr, localizeConst(theStatus ? ENTER_ANY_EMAIL_ADDRESS_VERBOSE : PROMPT_EMAIL_ADDR_VERBOSE));
	SetWindowText(sttcLoginEmailAddr, localizeConst(theStatus ? ENTER_ANY_EMAIL_ADDRESS : PROMPT_EMAIL_ADDR));

	Button_SetCheck(chekManualEmail, theStatus ? BST_CHECKED : BST_UNCHECKED);
}

void winProcLogin(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND && (HWND)lParam == bttnLogin)
	{
		char oldUserLogin[EMAIL_ADDR_BUFSIZE];//for determining emailAddrChanged
		strcpy(oldUserLogin, gUserEmailAccount);

		WCHAR getUserLoginW[EMAIL_ADDR_BUFSIZE];
		WCHAR getPassW[EMAIL_PASSWORD_BUFSIZE];
		Edit_GetText(textLoginEmailAddr, getUserLoginW, EMAIL_ADDR_BUFSIZE);
		Edit_GetText(textLoginEmailPW, getPassW, EMAIL_PASSWORD_BUFSIZE);
		wcstombs(gUserEmailAccount, getUserLoginW, EMAIL_ADDR_BUFSIZE);
		wcstombs(gUserEmailPassword, getPassW, EMAIL_PASSWORD_BUFSIZE);
		//ensure address ends in @ something we can handle - if not, TELL THEM!
		if(!validateEmailAddress(gUserEmailAccount))
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

		if (getSConfig()) //true if there was enough config info present to be "valid"
		{
			ShowWindow(wndwMain, SW_SHOW);
			ShowWindow(wndwLogin, SW_HIDE);
		}
		else//they've logged in without config info loaded, so we need to do the login "tell me my stuff" message to directory server
			sendMessageToDirServ(L"existingLogin", loginMailCallback);
	}
	else if (message == WM_COMMAND && (HWND)lParam == bttnRegisterSocial)
	{
		ShowWindow(wndwLogin, SW_HIDE);
		ShowWindow(wndwRegisterSocial, SW_SHOW);
		// doesn't appear to do anything when called here... SetFocus(textRegRecCode);
	}
	else if (message == WM_COMMAND && (HWND)lParam == bttnRegisterRecd)
	{
		ShowWindow(wndwLogin, SW_HIDE);
		ShowWindow(wndwRegisterRecd, SW_SHOW);
	}
	else if (message == WM_COMMAND && (HWND)lParam == bttnIsntThisUnsafeMain)
	{
		MessageBox(NULL, localizeConst(WHY_EMAIL_PASSWORD), L"", MB_OK);
	}
	else if (message == WM_COMMAND && (HWND)lParam == chekManualEmail)
	{
		gManualEmail.enabled = (Button_GetCheck(chekManualEmail) == BST_CHECKED);
		updateGUIWithManualEmailStatus(gManualEmail.enabled);
		saveManualEmailStatus(gManualEmail.enabled);
	}
}
