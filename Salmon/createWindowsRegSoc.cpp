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

#include "salmon_globals.h"
#include "hwnds.h"

#include "localization.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "email.h"
#include "control_softether.h"

#include "emailReplyCallbacks.h"

void createWindowsSocial(LPCWSTR className, HINSTANCE thisInstance)
{
	//register with facebook/renren window
	wndwRegisterSocial = CreateWindowEx(
		0,                   // Extended possibilites for variation 
		className,         // Classname 
		localizeConst(SALMON_REG_TITLE),       // Title Text 
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
		CW_USEDEFAULT,       // Windows decides the position 
		CW_USEDEFAULT,       // where the window ends up on the screen 
		340,                 // The programs width 
		488,                 // and height in pixels 
		HWND_DESKTOP,        // The window is a child-window to desktop 
		NULL,                // No menu 
		thisInstance,       // Program Instance handler 
		NULL                 // No Window Creation data 
		);

	const int textEnterPostY = 10;
	textEnterPost = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		localizeConst(ENTER_POST_INSTRUCTIONS),
		WS_VISIBLE | WS_CHILD | ES_MULTILINE,  // styles
		10,         // starting x position
		textEnterPostY,         // starting y position
		310,        // width
		120,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int radioButtonsY = textEnterPostY + 124;
	/*COMMENTRENRENrdioFacebook = CreateWindow(
		L"BUTTON",   // predefined class
		L"Facebook",       // text
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // styles
		10,         // starting x position
		radioButtonsY,         // starting y position
		100,        // width
		20,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE) GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;*/


	/*COMMENTRENRENrdioRenren = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(RENREN_STR),       // text
		WS_VISIBLE | WS_CHILD | BS_AUTORADIOBUTTON,  // styles
		120,         // starting x position
		radioButtonsY,         // starting y position
		100,        // width
		20,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE) GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;*/

	const int sttcSocNetIDY = radioButtonsY + 26;
	sttcSocNetID = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(FACEBOOK_ID_INSTRUCTIONS),// text
		WS_VISIBLE | WS_CHILD | SS_EDITCONTROL,  // styles
		10,         // starting x position
		sttcSocNetIDY,         // starting y position
		300,        // width
		48,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed


	const int textSocNetIDY = sttcSocNetIDY + 50;
	textSocNetID = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | WS_TABSTOP,  // styles
		10,         // starting x position
		textSocNetIDY,         // starting y position
		300,        // width
		23,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;//NOTE should be ES_NUMBER if renren

	const int sttcSocEmailAddrY = textSocNetIDY + 26;
	sttcSocEmailAddr = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(PROMPT_EMAIL_ADDR_VERBOSE),// text
		WS_VISIBLE | WS_CHILD,  // styles
		10,         // starting x position
		sttcSocEmailAddrY,         // starting y position
		300,        // width
		80,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textSocEmailAddrY = sttcSocEmailAddrY + 84;
	textSocEmailAddr = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | WS_TABSTOP,  // styles
		10,         // starting x position
		textSocEmailAddrY,         // starting y position
		300,        // width
		23,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int sttcSocEmailPWY = textSocEmailAddrY + 26;
	sttcSocEmailPW = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(PROMPT_EMAIL_PW_VERBOSE),// text
		WS_VISIBLE | WS_CHILD,  // styles
		10,         // starting x position
		sttcSocEmailPWY,         // starting y position
		180,        // width
		16,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed

	bttnIsntThisUnsafeSocReg = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(ISNT_THIS_UNSAFE_BUTTON),// text
		WS_VISIBLE | WS_CHILD,  // styles
		186,         // starting x position
		sttcSocEmailPWY - 2,         // starting y position
		110,        // width
		21,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed

	const int textSocEmailPWY = sttcSocEmailPWY + 22;
	textSocEmailPW = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"EDIT",   // predefined class
		L"",       // text
		WS_VISIBLE | WS_CHILD | ES_PASSWORD | WS_TABSTOP,  // styles
		10,         // starting x position
		textSocEmailPWY,         // starting y position
		300,        // width
		26,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed;

	const int bttnSocRegSubmitY = textSocEmailPWY + 34;
	bttnSocRegSubmit = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(SUBMIT_REGISTRATION),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP,  // styles
		90,         // starting x position
		bttnSocRegSubmitY,         // starting y position
		146,        // width
		40,        // height
		wndwRegisterSocial,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed

	SendMessage(textEnterPost, WM_SETFONT, (WPARAM)gFontHandle, 0);
	//COMMENTRENRENSendMessage(rdioFacebook, WM_SETFONT, (WPARAM)gFontHandle, 0);
	//COMMENTRENRENSendMessage(rdioRenren, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcSocNetID, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textSocNetID, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcSocEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textSocEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcSocEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnIsntThisUnsafeSocReg, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(textSocEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(bttnSocRegSubmit, WM_SETFONT, (WPARAM)gFontHandle, 0);
}

void disableAllButtonsSocReg()
{
	Button_Enable(bttnSocRegSubmit, FALSE);
}

void enableAllButtonsSocReg()
{
	Button_Enable(bttnSocRegSubmit, TRUE);
}



void winProcSocial(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool clearPostOnlyOnce = true;
	if (message == WM_COMMAND && (HWND)lParam == textEnterPost && clearPostOnlyOnce)
	{
		Edit_SetText(textEnterPost, L"");
		clearPostOnlyOnce = false;
	}
	//COMMENTRENRENelse if (message == WM_COMMAND && (HWND)lParam == rdioFacebook)
	//COMMENTRENREN{
	//COMMENTRENREN	Static_SetText(sttcSocNetID, localizeConst(FACEBOOK_ID_INSTRUCTIONS));
	//COMMENTRENREN	gCurSocialNetwork = SOCNET_FACEBOOK;
	//COMMENTRENREN}
	//COMMENTRENRENelse if (message == WM_COMMAND && (HWND)lParam == rdioRenren)
	//COMMENTRENREN{
	//COMMENTRENREN	Static_SetText(sttcSocNetID, localizeConst(RENREN_ID_INSTRUCTIONS));
	//COMMENTRENREN	gCurSocialNetwork = SOCNET_RENREN;
	//COMMENTRENREN}
	else if (message == WM_COMMAND && (HWND)lParam == bttnSocRegSubmit)
	{
		/*
		beginRegistration
		somefbaccount123
		facebook
		US
		hey this is a test facebook post
		it's for salmon
		the post is multiple lines and that's fine
		*/
		std::wstring begRegMsg = L"beginRegistration\n";

		WCHAR theSocID[200];
		Edit_GetText(textSocNetID, theSocID, 200);
		begRegMsg += (std::wstring(theSocID) + L"\n");

		if (gCurSocialNetwork==SOCNET_RENREN)
			begRegMsg += L"renren\n";
		else if (gCurSocialNetwork == SOCNET_FACEBOOK)
			begRegMsg += L"facebook\n";
		else
		{
			MessageBox(NULL, localizeConst(MUST_SELECT_FB_OR_RENREN), localizeConst(ERROR_STR), MB_OK);
			return;
		}
		begRegMsg += countryFromLanguage(gChosenLanguage);
		begRegMsg += L"\n";
		WCHAR thePost[1500];
		Edit_GetText(textEnterPost, thePost, 1500);
		begRegMsg += thePost;

		WCHAR getLogin[EMAIL_ADDR_BUFSIZE];
		WCHAR getPassword[EMAIL_PASSWORD_BUFSIZE];
		Edit_GetText(textSocEmailAddr, getLogin, 300);
		Edit_GetText(textSocEmailPW, getPassword, 100);
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

		sendMessageToDirServ(begRegMsg, startRegMailCallback);
	}
	else if (message == WM_COMMAND && (HWND)lParam == bttnIsntThisUnsafeSocReg)
		MessageBox(NULL, localizeConst(WHY_EMAIL_PASSWORD), L"", MB_OK);
}
