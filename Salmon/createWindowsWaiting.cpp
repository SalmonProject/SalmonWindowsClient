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

void createWindowsWaiting(LPCWSTR className, HINSTANCE thisInstance)
{
	wndwWaiting = CreateWindowEx(
		WS_EX_TOPMOST,    // Extended possibilites for variation 
		className,         // Classname 
		localizeConst(SALMON_PLEASE_WAIT_TITLE),       // Title Text 
		WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
		CW_USEDEFAULT,       // Windows decides the position 
		CW_USEDEFAULT,       // where the window ends up on the screen 
		336,                 // The programs width 
		220,                 // and height in pixels 
		HWND_DESKTOP,        // The window is a child-window to desktop 
		NULL,                // No menu 
		thisInstance,       // Program Instance handler 
		NULL                 // No Window Creation data 
		);

	//waiting for directory server to respond
	sttcWaiting = CreateWindow(
		L"STATIC",   // predefined class
		localizeConst(WAITING_FOR_RESPONSE),// text
		WS_VISIBLE | WS_CHILD | SS_CENTER,  // styles
		2,         // starting x position
		4,         // starting y position
		320,        // width
		120,        // height
		wndwWaiting,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwWaiting, GWL_HINSTANCE),
		NULL);      // pointer not needed

	SetWindowPos(sttcWaiting, HWND_BOTTOM, 1, 2, 3, 4, SWP_NOMOVE | SWP_NOSIZE);

	bttnCancelWaiting = CreateWindow(
		L"BUTTON",   // predefined class
		localizeConst(CANCEL_BUTTON),       // text
		WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
		110,         // starting x position
		130,         // starting y position
		100,        // width
		40,        // height
		wndwWaiting,       // parent window
		NULL,       // No menu
		(HINSTANCE)GetWindowLong(wndwRegisterSocial, GWL_HINSTANCE),
		NULL);      // pointer not needed

	SendMessage(bttnCancelWaiting, WM_SETFONT, (WPARAM)gFontHandle, 0);
	SendMessage(sttcWaiting, WM_SETFONT, (WPARAM)gFontHandle, 0);
}

extern void enableAllButtonsMain();
extern void enableAllButtonsLogin();
extern void enableAllButtonsRecReg();
extern void enableAllButtonsSocReg();

extern bool cancelRecvFlag;
void winProcWaiting(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND && (HWND)lParam == bttnCancelWaiting || message == WM_CLOSE)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		cancelRecvFlag = true;
		enableAllButtonsMain();
		enableAllButtonsLogin();
		enableAllButtonsRecReg();
		enableAllButtonsSocReg();
	}
}
