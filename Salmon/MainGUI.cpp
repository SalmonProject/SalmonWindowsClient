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

//#include <stdio.h>

#define _WIN32_WINNT    0x0501  // minimum version Windows XP
#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <CommCtrl.h>

#include "../Wrapper/cSmtp.hpp"
#include "../Wrapper/cPop3.hpp"
#include "../Wrapper/cImap.hpp"

#include "salmon_constants.h"
#include "salmon_globals.h"
#include "hwnds.h"

#include "localization.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "control_softether.h"
#include "email.h"

LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);
const LPCWSTR _win32_WindowClass = L"SalmonClient";

void createWindowsSocial(LPCWSTR className, HINSTANCE thisInstance);
void createWindowsMain(LPCWSTR className, HINSTANCE thisInstance);
void createWindowsLogin(LPCWSTR className, HINSTANCE thisInstance);
void createWindowsRec(LPCWSTR className, HINSTANCE thisInstance);
void createWindowsLocale(LPCWSTR className, HINSTANCE thisInstance);
void createWindowsWaiting(LPCWSTR className, HINSTANCE thisInstance);

int WINAPI WinMain(HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int windowStyle)

{
	gServerInfoMutex = CreateMutex(NULL, FALSE, NULL);

	//first, in case we're going to write any config files, we want to be sure the directory exists.
	ensureSalmonConfigDir();

	//get the full path we need to use to call vpncmd.exe. also, complain if it isn't there. (will exit program on failure.)
	load_vpncmdexe_Path();

	//See if softether already has a connection, so that we can show a disconnect button rather than connect.
	//ALSO, this will create the "VPN Client" interface if it doesn't exist. So, it should definitely come
	//before anything else that touches SoftEther.
	//Also, if there was already a connection, we assume gUserWantsConnection, until they explicitly click Disconnect.
	gUserWantsConnection = gVPNConnected = checkConnection();
	//if we start off connected, the Disconnect button must be available. To call softether's accountdisconnect,
	//we need to know which connection setting we're trying to disconnect (there is no disconnect all option).
	//so, every time you connect, save the IP addr to a config file, so that we can read it here in case we start up connected.
	if (gVPNConnected)
		loadCurrentVPN_Addr();

	gConnectionStateMutex = CreateMutex(NULL, FALSE, NULL);
	//begin the connection monitor thread. does nothing while we aren't connected, monitors connection when we are connected,
	//and tries to reconnect (as if they had clicked on the connect button again) if it detects the connection is down.
	CreateThread(NULL, 0, monitorConnection, NULL, 0, NULL);
	
	//get language setting, if one was already selected from a previous run
	gChosenLanguage = loadLanguageFromConfig();

	//if possible, load email address into gUserEmailAccount from %APPDATA%\salmon\CurrentEmail.txt, and set the relevant edit box to it
	//(NOTE: actually, the edit box doesn't exist at this point in the program, so we can't set it right now. Do it when created.)
	loadEmailIfStored();

	recvThreadMutex = CreateMutex(
		NULL,              // default security attributes
		FALSE,             // initially not owned
		NULL);             // unnamed mutex

	WNDCLASSEX wincl;

	INITCOMMONCONTROLSEX icc;

	// Initialise common controls.
	icc.dwSize = sizeof(icc);
	icc.dwICC = ICC_STANDARD_CLASSES | ICC_WIN95_CLASSES | ICC_NATIVEFNTCTL_CLASS;
	InitCommonControlsEx(&icc);

	// The Window structure 
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = _win32_WindowClass;
	wincl.lpfnWndProc = WindowProcedure;      // This function is called by windows 
	wincl.style = CS_DBLCLKS;                 // Catch double-clicks 
	wincl.cbSize = sizeof (WNDCLASSEX);

	// Use default icon and mouse-pointer 
	wincl.hIcon = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hIconSm = LoadIcon (NULL, IDI_APPLICATION);
	wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
	wincl.lpszMenuName = NULL;                 // No menu 
	wincl.cbClsExtra = 0;                      // No extra bytes after the window class 
	wincl.cbWndExtra = 0;                      // structure or the window instance 
	// Use Windows's default color as the background of the window 
	//wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;
	//wincl.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	//wincl.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	wincl.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);

	// Register the window class, and if it fails quit the program 
	if (!RegisterClassEx (&wincl))
		return 0;

	NONCLIENTMETRICS ncm;
	ncm.cbSize = sizeof(ncm);
	SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
	gFontHandle = CreateFontIndirect(&ncm.lfMessageFont);

	//CreateWindow() calls for buttons and stuff put elsewhere to reduce clutter
	createWindowsSocial(_win32_WindowClass, hThisInstance);
	createWindowsLogin(_win32_WindowClass, hThisInstance);
	createWindowsMain(_win32_WindowClass, hThisInstance);
	createWindowsRec(_win32_WindowClass, hThisInstance);
	createWindowsLocale(_win32_WindowClass, hThisInstance);
	createWindowsWaiting(_win32_WindowClass, hThisInstance);

	trustLevelDisplayAndWriteFile(readTrustFromFile());

	if (gChosenLanguage != SALMON_LANGUAGE_INVALID)
	{
		setAllText();
		ShowWindow(wndwLogin, windowStyle);
		ShowWindow(wndwLocaleSelect, SW_HIDE);
		checkTrustCountdown();
	}
	else
	{
		ShowWindow(wndwLogin, SW_HIDE);
		ShowWindow(wndwLocaleSelect, windowStyle);
		startTrustCountdown();
	}

	MSG messages;
	while (GetMessage(&messages, NULL, 0, 0) > 0)
	{
		HWND focusedControl = GetFocus();

		if (messages.message == WM_KEYDOWN && messages.wParam == VK_RETURN)
		{
			if (focusedControl == textLoginEmailAddr || focusedControl == textLoginEmailPW)
			{
				SendMessage(wndwLogin, WM_COMMAND, 0, (LPARAM)bttnLogin);
				continue;
			}
			else if (focusedControl == textRegRecCode || focusedControl == textRecEmailAddr || focusedControl == textRecEmailPW)
			{
				SendMessage(wndwRegisterRecd, WM_COMMAND, 0, (LPARAM)bttnRecRegSubmit);
				continue;
			}
			else if (textSocEmailAddr || focusedControl == textSocEmailPW || focusedControl == textSocNetID)
			{
				SendMessage(wndwRegisterSocial, WM_COMMAND, 0, (LPARAM)bttnSocRegSubmit);
				continue;
			}
		}
		
		if 
		(
			//don't let IsDialogMessage eat a tab that ought to become a real tab character in textEnterPost.
			//no actually do let it tab. no jk it's impossible to let it tab without WAY more work than this is worth.
			focusedControl == textEnterPost ||
			//let the "dialog" version of winproc handle VK_RETURN on email/pw text boxes, so that we can have it respond the same way as clicking the submit button
			!IsDialogMessage(GetActiveWindow(), &messages)
		)
		{
			TranslateMessage(&messages);
			DispatchMessage(&messages);
		}
	}
	
	//getting out of the loop should mean a WM_QUIT message was received; 
	//this would then be its wParam (which is supposed to be used as the program's exit code)
	return messages.wParam;
}

void winProcWaiting(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam);
void winProcLogin(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam);
void winProcLocale(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam);
void winProcSocial(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam);
void winProcMain(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam);
void winProcRecd(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam);
//This function is called by the Windows function DispatchMessage()
LRESULT CALLBACK WindowProcedure (HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(message==WM_COMMAND || message==WM_NOTIFY || message==WM_KEYDOWN&&wParam==VK_RETURN || message==WM_CLOSE)
	{
		HWND activeWindow = GetActiveWindow();
		if (message == WM_CLOSE)
		{
			if (activeWindow == wndwWaiting)
				winProcWaiting(theHwnd, message, wParam, lParam);
			else
				return DefWindowProc(theHwnd, message, wParam, lParam);
		}

		if (activeWindow == wndwLocaleSelect)
			winProcLocale(theHwnd, message, wParam, lParam);
		else if (activeWindow == wndwLogin)
			winProcLogin(theHwnd, message, wParam, lParam);
		else if (activeWindow == wndwMain)
			winProcMain(theHwnd, message, wParam, lParam);
		else if (activeWindow == wndwRegisterSocial)
			winProcSocial(theHwnd, message, wParam, lParam);
		else if (activeWindow == wndwRegisterRecd)
			winProcRecd(theHwnd, message, wParam, lParam);
		else if (activeWindow == wndwWaiting)
			winProcWaiting(theHwnd, message, wParam, lParam);
	}
	else if(message==WM_DESTROY)
		PostQuitMessage (0);       //send a WM_QUIT to the message queue
	else
		return DefWindowProc(theHwnd, message, wParam, lParam);
	return 0;
}
