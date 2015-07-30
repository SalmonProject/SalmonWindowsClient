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
#include "resource.h"

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <algorithm>

#include "salmon_constants.h"
#include "salmon_globals.h"
#include "hwnds.h"

#include "localization.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "control_softether.h"
#include "VPNInfo.h"
#include "email.h"

#include "version.h"

#define HIDDEN_MAIN_WINDOW_HEIGHT 150
#define FULL_MAIN_WINDOW_HEIGHT 568

bool sendMobileconfigs();
void writeMobileconfigToFile(WCHAR* fname, WCHAR* ipAddr, WCHAR* psk, WCHAR* vpnName, WCHAR* vpnPW);

//for ssl version:
#include "../libvmime/src/openssl/crypto.h"
std::wstring getSalmonClientAndOpenSSLVersions()
{
	const WCHAR* salmonVer = SALMON_CLIENT_VERSION_STRING;
	const char* aSSLV = SSLeay_version(SSLEAY_VERSION);
	WCHAR* wSSLV = new WCHAR[strlen(aSSLV) + 1];
	mbstowcs(wSSLV, aSSLV, strlen(aSSLV));
	std::wstring wholeVerString(salmonVer);
	wholeVerString += L"\n";
	wholeVerString += wSSLV;
	delete wSSLV;
	return wholeVerString;
}


void createWindowsMain(LPCWSTR className, HINSTANCE thisInstance)
{

//main window
wndwMain = CreateWindowEx(
	WS_EX_CLIENTEDGE,                   // Extended possibilites for variation 
	className,         // Classname 
	localizeConst(SALMON_TITLE),       // Title Text 
	WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
	CW_USEDEFAULT,       // Windows decides the position 
	CW_USEDEFAULT,       // where the window ends up on the screen 
	230,                 // The programs width 
	HIDDEN_MAIN_WINDOW_HEIGHT,                 // and height in pixels HIDDEN_MAIN_WINDOW_HEIGHT<=>FULL_MAIN_WINDOW_HEIGHT
	HWND_DESKTOP,        // The window is a child-window to desktop 
	NULL,                // No menu 
	thisInstance,       // Program Instance handler 
	NULL                 // No Window Creation data 
	);

const int sttcConnectStatusY = 6;
sttcConnectStatus=CreateWindow(
	L"STATIC",   // predefined class
	gVPNConnected ? localizeConst(VPN_STATUS_CONNECTED) : localizeConst(VPN_STATUS_DISCONNECTED),
	WS_VISIBLE | WS_CHILD | SS_CENTER,  // styles
	3,         // starting x position
	sttcConnectStatusY,         // starting y position
	215,        // width
	16,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed


//NOTE i think you might not want WS_TABSTOP on hidden buttons, since presumably the tabbing would have to go through them.
//however, since there is nothing to tab through in the simple view, that doesn't matter, so to keep it simple i'm just giving WS_TABSTOP to all of them.
DWORD visibleButtonStyle = WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP;
DWORD hiddenButtonStyle = WS_CHILD | BS_DEFPUSHBUTTON | WS_TABSTOP;

const int connectButtonsY = sttcConnectStatusY + 24;
bttnConnect = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(CONNECT_BUTTON),       // text
	gVPNConnected ? hiddenButtonStyle : visibleButtonStyle,  // styles
	28,         // starting x position
	connectButtonsY,         // starting y position 
	156,        // width
	40,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnCancelConnect = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(CANCEL_CONNECTING_BUTTON),       // text
	hiddenButtonStyle,  // styles
	28,         // starting x position
	connectButtonsY,         // starting y position
	156,        // width
	40,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE)GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnDisconnect = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(DISCONNECT_BUTTON),       // text
	gVPNConnected ? visibleButtonStyle : hiddenButtonStyle,  // styles
	28,         // starting x position
	connectButtonsY,         // starting y position
	156,        // width
	40,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE)GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed




const int bttnShowHideOptionsY = connectButtonsY + 46;
bttnShowHideOptions = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(SHOW_ADVANCED_VIEW),       // text
	visibleButtonStyle,  // styles
	28,         // starting x position
	bttnShowHideOptionsY,         // starting y position
	156,        // width
	30,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int sttcTrustStatusY = bttnShowHideOptionsY + 40;
sttcTrustStatus=CreateWindow(
	L"STATIC",   // predefined class
	localizeConst(TRUST_LVL_ORDINARY),
	 WS_CHILD | SS_CENTER,  // styles
	9,         // starting x position
	sttcTrustStatusY,         // starting y position
	196,        // width
	16,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int bttnGetRecY = sttcTrustStatusY + 24;
bttnGetRec = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(GET_REC_CODE_FOR_FRIEND),       // text
	 WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
	28,         // starting x position
	bttnGetRecY,         // starting y position
	156,        // width
	46,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int framRedeemCodeY = bttnGetRecY + 50;
framRedeemCode=CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(BOOST_TRUST_W_CODE),// text
	 WS_CHILD | BS_GROUPBOX,  // styles
	18,         // starting x position
	framRedeemCodeY,         // starting y position
	176,        // width
	113,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int textRedeemCodeY = framRedeemCodeY + 22;
textRedeemCode = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // text
	 WS_CHILD | WS_TABSTOP ,  // styles
	28,         // starting x position
	textRedeemCodeY,         // starting y position
	100,        // width
	23,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed;

const int bttnRedeemCodeY = textRedeemCodeY + 28;
bttnRedeemCode = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(REDEEM_REC_CODE),       // text
	WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
	28,         // starting x position
	bttnRedeemCodeY,         // starting y position
	156,        // width
	56,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int bttnAndroidGetterY = bttnRedeemCodeY + 68;
bttnAndroidGetter = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(GET_CREDENTIALS_FOR_ANDROID),       // text
	WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
	28,         // starting x position
	bttnAndroidGetterY,         // starting y position
	156,        // width
	56,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE)GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int bttniOSGetterY = bttnAndroidGetterY + 68;
bttniOSGetter = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(GET_CREDENTIALS_FOR_IOS),       // text
	WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
	28,         // starting x position
	bttniOSGetterY,         // starting y position
	156,        // width
	56,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE)GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int bttnWipeConfigY = bttniOSGetterY + 68;
bttnWipeConfig = CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(WIPE_CONFIG),       // text
	WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE | WS_TABSTOP,  // styles
	28,         // starting x position
	bttnWipeConfigY,         // starting y position
	156,        // width
	46,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

std::wstring displayVersionString = getSalmonClientAndOpenSSLVersions();
const int sttcVersionY = bttnWipeConfigY + 54;
sttcVersion = CreateWindow(
	L"STATIC",   // predefined class
	displayVersionString.c_str(),       // text
	WS_CHILD | SS_CENTER,  // styles
	9,         // starting x position
	sttcVersionY,         // starting y position
	196,        // width
	32,        // height
	wndwMain,       // parent window
	NULL,       // No menu
	(HINSTANCE)GetWindowLong(wndwMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

SendMessage(sttcConnectStatus, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnCancelConnect, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnDisconnect, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnConnect, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnShowHideOptions, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(sttcTrustStatus, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnGetRec, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(framRedeemCode, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(textRedeemCode, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnRedeemCode, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnAndroidGetter, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttniOSGetter, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnWipeConfig, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(sttcVersion, WM_SETFONT, (WPARAM)gFontHandle, 0);

}

void disableAllButtonsMain()
{
	Button_Enable(bttnCancelConnect, FALSE);
	Button_Enable(bttnDisconnect, FALSE);
	Button_Enable(bttnConnect, FALSE);
	Button_Enable(bttnGetRec, FALSE);
	Button_Enable(bttnRedeemCode, FALSE);
	Button_Enable(bttnAndroidGetter, FALSE);
	Button_Enable(bttniOSGetter, FALSE);
	Button_Enable(bttnWipeConfig, FALSE);
}

void enableAllButtonsMain()
{
	Button_Enable(bttnCancelConnect, TRUE);
	Button_Enable(bttnDisconnect, TRUE);
	Button_Enable(bttnConnect, TRUE);
	Button_Enable(bttnGetRec, TRUE);
	Button_Enable(bttnRedeemCode, TRUE);
	Button_Enable(bttnAndroidGetter, TRUE);
	Button_Enable(bttniOSGetter, TRUE);
	Button_Enable(bttnWipeConfig, TRUE);
}


RecvThreadStruct* mainRecvStruct;
void getRecMailCallback(RecvMailCodes successful)
{
	if (successful == RECV_MAIL_SUCCESS)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);

		localizeMsgBox(mainRecvStruct->buffer, localizeConst(DIR_SERVER_SAYS));
		enableAllButtonsMain();

		if (!strncmp(mainRecvStruct->buffer, "$0", 2))
		{
			delete mainRecvStruct;
			return;
		}

		if (!strncmp(mainRecvStruct->buffer, "$12", 3))
			trustLevelDisplayAndWriteFile(3);
		else if (!strncmp(mainRecvStruct->buffer, "$13", 3) || !strncmp(mainRecvStruct->buffer, "$14", 3))
			trustLevelDisplayAndWriteFile(6);
		else if (!strncmp(mainRecvStruct->buffer, "$15", 3))
			trustLevelDisplayAndWriteFile(7);
		else if (readTrustFromFile() <= 6)
			trustLevelDisplayAndWriteFile(6);
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
		MessageBox(NULL, localizeConst(NO_RESPONSE_FROM_DIR), localizeConst(ERROR_STR), MB_OK);
		enableAllButtonsMain();
	}
	delete mainRecvStruct;
}

void redeemCodeMailCallback(RecvMailCodes successful)
{
	if (successful == RECV_MAIL_SUCCESS)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
		localizeMsgBox(mainRecvStruct->buffer, localizeConst(DIR_SERVER_SAYS));

		if (!strncmp(mainRecvStruct->buffer, "$17", 3) || !strncmp(mainRecvStruct->buffer, "$16", 3) || !strncmp(mainRecvStruct->buffer, "$10", 3))
			trustLevelDisplayAndWriteFile(5);

		enableAllButtonsMain();
		delete mainRecvStruct;
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
		MessageBox(NULL, localizeConst(NO_RESPONSE_FROM_DIR), localizeConst(ERROR_STR), MB_OK);
		enableAllButtonsMain();
		delete mainRecvStruct;
	}
}


bool compWithFailure(const VPNInfo& a, const VPNInfo& b);
bool mainWindowExpanded = false;
void winProcMain(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND)
	{
		if ((HWND)lParam == bttnShowHideOptions)
		{
			if (mainWindowExpanded)
			{
				SetWindowPos(wndwMain, HWND_DESKTOP, 1, 2, 230, HIDDEN_MAIN_WINDOW_HEIGHT, SWP_NOMOVE);
				ShowWindow(sttcTrustStatus, SW_HIDE);
				ShowWindow(bttnGetRec, SW_HIDE);
				ShowWindow(framRedeemCode, SW_HIDE);
				ShowWindow(textRedeemCode, SW_HIDE);
				ShowWindow(bttnRedeemCode, SW_HIDE);
				ShowWindow(bttnAndroidGetter, SW_HIDE);
				ShowWindow(bttniOSGetter, SW_HIDE);
				ShowWindow(bttnWipeConfig, SW_HIDE);
				ShowWindow(sttcVersion, SW_HIDE);
				Button_SetText(bttnShowHideOptions, localizeConst(SHOW_ADVANCED_VIEW));
			}
			else
			{
				SetWindowPos(wndwMain, HWND_DESKTOP, 1, 2, 230, FULL_MAIN_WINDOW_HEIGHT, SWP_NOMOVE);
				ShowWindow(sttcTrustStatus, SW_SHOW);
				ShowWindow(bttnGetRec, SW_SHOW);
				ShowWindow(framRedeemCode, SW_SHOW);
				ShowWindow(textRedeemCode, SW_SHOW);
				ShowWindow(bttnRedeemCode, SW_SHOW);
				ShowWindow(bttnAndroidGetter, SW_SHOW);
				ShowWindow(bttniOSGetter, SW_SHOW);
				ShowWindow(bttnWipeConfig, SW_SHOW);
				ShowWindow(sttcVersion, SW_SHOW);
				Button_SetText(bttnShowHideOptions, localizeConst(HIDE_ADVANCED_VIEW));
			}
			mainWindowExpanded = !mainWindowExpanded;
		}
		else if ((HWND)lParam == bttnConnect)
		{
			ShowWindow(bttnConnect, SW_HIDE);
			ShowWindow(bttnCancelConnect, SW_SHOW);
			ShowWindow(bttnDisconnect, SW_HIDE);

			Static_SetText(sttcConnectStatus, localizeConst(VPN_STATUS_CONNECTING));

			cancelConnectionAttempt = false;
			gUserWantsConnection = true;
			CreateThread(NULL, 0, connectionAttemptThread, NULL, 0, NULL);
		}
		else if ((HWND)lParam == bttnCancelConnect)
		{
			showConnectionStatus(false);
			cancelConnectionAttempt = true;
			gUserWantsConnection = false;
		}
		else if ((HWND)lParam == bttnDisconnect)
		{
			showConnectionStatus(false);
			gUserWantsConnection = false;
			disconnectVPN();
		}
		else if ((HWND)lParam == bttnGetRec)
		{
			char ourRandStr[51];
			if (sendMail(L"requestRecommendation", ourRandStr) == false)
			{
				MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
				return;
			}

			disableAllButtonsMain();
			UpdateWindow(wndwMain);

			mainRecvStruct = new RecvThreadStruct(ourRandStr, getRecMailCallback);
			CreateThread(NULL, 0, recvThread, mainRecvStruct, 0, NULL);
		}
		else if ((HWND)lParam == bttnRedeemCode)
		{
			WCHAR theRecCode[20];
			WCHAR sendRedeem[40];

			Edit_GetText(textRedeemCode, theRecCode, 20);
			if (wcslen(theRecCode) != 7)
			{
				MessageBox(NULL, localizeConst(WRONG_RECCODE_LENGTH), localizeConst(ERROR_STR), MB_OK);
				return;
			}
			wcscpy(sendRedeem, L"redeemRecommendation\n");
			wcscat(sendRedeem, theRecCode);

			char ourRandStr[51];
			if (sendMail(sendRedeem, ourRandStr) == false)
			{
				MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
				return;
			}

			disableAllButtonsMain();
			UpdateWindow(wndwMain);

			mainRecvStruct = new RecvThreadStruct(ourRandStr, redeemCodeMailCallback);
			CreateThread(NULL, 0, recvThread, mainRecvStruct, 0, NULL);
		}
		else if ((HWND)lParam == bttnAndroidGetter)
		{
			WCHAR userLoginW[EMAIL_ADDR_BUFSIZE];
			mbstowcs(userLoginW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
			WCHAR tempMsgBoxBody[1000 + EMAIL_ADDR_BUFSIZE]; //also used to receive DONE_MAILING_VPNS
			wcscpy(tempMsgBoxBody, localizeConst(IS_IT_OK_TO_MAIL_VPNS));
			wcscat(tempMsgBoxBody, userLoginW);
			if (MessageBox(NULL, tempMsgBoxBody, localizeConst(MAILING_TO_ANDROID), MB_OKCANCEL) != IDCANCEL)
			{
				//first, be sure they're sorted last-connection-attempt-succeeded-first, then best-score-first.
				std::sort(knownServers.begin(), knownServers.end(), compWithFailure);
				//also, if we're currently connected to one, that should be the very first.
				if (gVPNConnected)
					for (int i = 0; i < knownServers.size(); i++)
						if (!strcmp(gCurrentConnection.addr, knownServers[i].addr))
						{
							VPNInfo tempHolder = knownServers[0];
							knownServers[0] = knownServers[i];
							knownServers[i] = tempHolder;
						}

				WCHAR* credBody = new WCHAR[knownServers.size() * 150 + 200];

				wcscpy(credBody, localizeConst(ANDROID_VPNLIST_HEADER));
				//credBody should look like
				//Server: 1.2.3.4
				//Pre-shared key (PSK): abcdef
				//Username: abcdefghikjlsdfj
				//Password: abcdfeghtifdjkds
				//
				//Server: 111.211.113.114
				//Pre-shared key (PSK): abcdef
				//Username: bbbbbbbbbbbbbbbbbb
				//Password: ccccccccccccccccc
				//
				//Server: 5.42.23.24
				//Pre-shared key (PSK): abcdef
				//Username: eeeeeeeeeeeeeeee
				//Password: ffffffffffffffff
				for (int i = 0; i < knownServers.size(); i++)
				{
					WCHAR wAddr[60];
					mbstowcs(wAddr, knownServers[i].addr, 60); wAddr[59] = 0;
					wcscat(credBody, L"Server: ");
					wcscat(credBody, wAddr);

					WCHAR wPSK[10];
					mbstowcs(wPSK, knownServers[i].psk, 10); wPSK[9] = 0;
					wcscat(credBody, L"\nPre-shared key (PSK): ");
					wcscat(credBody, wPSK);
					
					//derive username and pw
					char pwToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
					char userNameToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
					derivePassword(pwToUse, gBaseVPNPassword, knownServers[i].addr);
					deriveUsername(userNameToUse, gBaseVPNPassword, knownServers[i].addr);
					WCHAR wPW[VPN_DERIVED_PASSWORD_LENGTH + 1];
					WCHAR wName[VPN_DERIVED_PASSWORD_LENGTH + 1];
					mbstowcs(wPW, pwToUse, VPN_DERIVED_PASSWORD_LENGTH + 1);
					mbstowcs(wName, userNameToUse, VPN_DERIVED_PASSWORD_LENGTH + 1);

					wcscat(credBody, L"\nUsername: ");
					wcscat(credBody, wName);
					wcscat(credBody, L"\nPassword: ");
					wcscat(credBody, wPW);
					wcscat(credBody, L"\n\n");
				}

				if (sendSelfMail(credBody, L"credentials"))
				{
					wcscpy(tempMsgBoxBody, localizeConst(DONE_MAILING_VPNS));
					wcscat(tempMsgBoxBody, userLoginW);
					MessageBox(NULL, tempMsgBoxBody, localizeConst(MAILING_TO_ANDROID), MB_OK);
				}
				else
					MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);

				delete credBody;
			}
		}
		else if ((HWND)lParam == bttniOSGetter)
		{
			WCHAR userLoginW[EMAIL_ADDR_BUFSIZE];
			mbstowcs(userLoginW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
			WCHAR tempMsgBoxBody[1000 + EMAIL_ADDR_BUFSIZE];
			wcscpy(tempMsgBoxBody, localizeConst(IS_IT_OK_TO_MAIL_VPNS_IOS));
			wcscat(tempMsgBoxBody, userLoginW);
			if (MessageBox(NULL, tempMsgBoxBody, localizeConst(MAILING_TO_IOS), MB_OKCANCEL) != IDCANCEL)
			{
				//first, be sure they're sorted last-connection-attempt-succeeded-first, then best-score-first.
				std::sort(knownServers.begin(), knownServers.end(), compWithFailure);
				//also, if we're currently connected to one, that should be the very first.
				if (gVPNConnected)
					for (int i = 0; i < knownServers.size(); i++)
						if (!strcmp(gCurrentConnection.addr, knownServers[i].addr))
						{
							VPNInfo tempHolder = knownServers[0];
							knownServers[0] = knownServers[i];
							knownServers[i] = tempHolder;
						}

				//now generate .mobileconfig files (one per knownServers[i]), and email them to the user as attachments.
				if (sendMobileconfigs())
				{
					wcscpy(tempMsgBoxBody, localizeConst(DONE_MAILING_VPNS));
					wcscat(tempMsgBoxBody, userLoginW);
					MessageBox(NULL, tempMsgBoxBody, localizeConst(MAILING_TO_IOS), MB_OK);
				}
				else
					MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
			}
		}
		else if ((HWND)lParam == bttnWipeConfig)
		{
			
			if (IDCANCEL != MessageBox(NULL, localizeConst(WIPE_WARNING), localizeConst(WIPE_TITLE), MB_OKCANCEL))
			{
				showConnectionStatus(false);
				gUserWantsConnection = false;
				disconnectVPN();

				getSConfig();//ensure knownServers is up to date, so that we wipe everything described in the file (needed if they wipe without ever trying to connect)
				deleteConnectionSettings(); //deletes all connection settings named after the IP addresses in knownServers
				wipeConfig();

				MessageBox(NULL, localizeConst(WIPE_COMPLETE), localizeConst(WIPE_TITLE), MB_OK);
				PostQuitMessage(0);
			}
		}
	}
}

