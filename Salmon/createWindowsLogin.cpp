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
	320,                 // and height in pixels 
	HWND_DESKTOP,        // The window is a child-window to desktop 
	NULL,                // No menu 
	thisInstance,       // Program Instance handler 
	NULL                 // No Window Creation data 
	);

const int framFirstTimeY = 10;
//login or register window
framFirstTime=CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(FIRST_TIME_USER_Q),// text
	WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
	10,         // starting x position
	framFirstTimeY, // starting y position
	282,        // width
	88,        // height
	wndwLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
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
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
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
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

const int framExistingUserY = framFirstTimeY + 98;
framExistingUser=CreateWindow(
	L"BUTTON",   // predefined class
	localizeConst(EXISTING_USER_Q),// text
	WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
	10,         // starting x position
	framExistingUserY,         // starting y position
	282,        // width
	168,        // height
	wndwLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed

const int sttcLoginEmailAddrY = framExistingUserY + 22;
sttcLoginEmailAddr=CreateWindow(
	L"STATIC",   // predefined class
	localizeConst(PROMPT_EMAIL_ADDR),// text
	WS_VISIBLE | WS_CHILD,  // styles
	20,         // starting x position
	sttcLoginEmailAddrY,         // starting y position
	262,        // width
	16,        // height
	wndwLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
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
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

const int sttcLoginEmailPWY = textLoginEmailAddrY + 27;
sttcLoginEmailPW = CreateWindow(
	L"STATIC",   // predefined class
	localizeConst(PROMPT_EMAIL_PASSWORD),// text
	WS_VISIBLE | WS_CHILD,  // styles
	20,         // starting x position
	sttcLoginEmailPWY,         // starting y position
	130,        // width
	16,        // height
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
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
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
	(HINSTANCE) GetWindowLong(wndwLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;



// set the new font
SendMessage(framFirstTime, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnRegisterSocial, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnRegisterRecd, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(framExistingUser, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(sttcLoginEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(sttcLoginEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(textLoginEmailAddr, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(textLoginEmailPW, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnLogin, WM_SETFONT, (WPARAM)gFontHandle, 0);


/*gFontHandle = CreateFont(-48, 0, 0, 0, FW_BOLD, FALSE,
	FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH,
	L"Calibri");
if (gFontHandle == NULL)
MessageBox(NULL, L"aww error", L"errr", MB_OK);
SendMessage(bttnLogin, WM_SETFONT, (WPARAM)gFontHandle, (LPARAM)MAKELONG(TRUE, 0));
SendMessage(sttcLoginEmailPW, WM_SETFONT, (WPARAM)gFontHandle, (LPARAM)MAKELONG(TRUE, 0));*/

}

void enableAllButtonsLogin()
{
	Button_Enable(bttnRegisterSocial, TRUE);
	Button_Enable(bttnRegisterRecd, TRUE);
	Button_Enable(bttnLogin, TRUE);
}

void disableAllButtonsLogin()
{
	Button_Enable(bttnRegisterSocial, FALSE);
	Button_Enable(bttnRegisterRecd, FALSE);
	Button_Enable(bttnLogin, FALSE);
}

RecvThreadStruct* loginRecvStruct;

void loginMailCallback(RecvMailCodes successful)
{
	if (successful == RECV_MAIL_SUCCESS)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwLogin);
		/*
		reply format should be:
		vpn base password (these are going to be VPN_BASE_PASSWORD_LENGTH ASCII characters, from ASCII values 97-122)
		trust level
		last time YOU successfully asked the dir to generate a rec code for you. format: toString(seconds since unix epoch)
		VPN_IP_addr psk offered_bw ----BEGIN CERT----*jfsdoifjsdoifj*oijfosidfsd*-----END CERT----\n
		VPN_IP_addr psk offered_bw ----BEGIN CERT----*jfsdoifjsdoifj*oijfosidfsd*-----END CERT----\n
		VPN_IP_addr psk offered_bw ----BEGIN CERT----*jfsdoifjsdoifj*oijfosidfsd*-----END CERT----\n
		...
		*/
		char receivedVPN_basePassword[VPN_BASE_PASSWORD_LENGTH+1];
		char trustLvlStr[20];
		char lastCodeRequestSSE[20]; //toString of an int representing seconds since midnight jan 1 1970
		vector < string > serverIPs;
		vector < string > serverPSKs;
		vector < string > serverBandwidths;
		vector < string > serverCerts;

		//NOTE YES we can rely on $ here
		if (strchr(loginRecvStruct->buffer, '$'))//$ means there's some error; display it to the user
		{
			localizeMsgBox(loginRecvStruct->buffer, localizeConst(ERROR_STR));
			enableAllButtonsLogin();
			delete loginRecvStruct;
			return;
		}
		
		ShowWindow(wndwLogin, SW_HIDE);
		ShowWindow(wndwMain, SW_SHOW);

		//first line should be vpn base password
		char* cur = strtok(loginRecvStruct->buffer, "\n");
		if (cur == NULL)
			goto MalformedError;
		memcpy(receivedVPN_basePassword, cur, VPN_BASE_PASSWORD_LENGTH);
		receivedVPN_basePassword[VPN_BASE_PASSWORD_LENGTH] = 0;

		//second line should be trust level
		cur = strtok(NULL, "\n");
		if (cur == NULL || strlen(cur) > 9)
			goto MalformedError;
		strcpy(trustLvlStr, cur);

		//third line should be last time this user asked for a new rec code
		cur = strtok(NULL, "\n");
		if (cur == NULL || strlen(cur) > 19)
			goto MalformedError;
		strcpy(lastCodeRequestSSE, cur);

		//now zero or more server info lines (ipaddr offeredBW cert)
		cur = strtok(NULL, "\n");
		while (cur != NULL)
		{
			//skip any empty lines...
			while (cur != NULL && lineIsJustWhitespace(cur))
				cur = strtok(NULL, "\n");
			//...and if we have arrived at the end of the text, we're done.
			if(cur == NULL)
				break;

			//each of these lines should be "ipaddr psk bw cert"
			if (strchr(cur, ' ') == NULL || strchr(strchr(cur, ' ') + 1, ' ') == NULL || strchr(strchr(strchr(cur, ' ') + 1, ' ')+1, ' ') == NULL)
				goto MalformedError;

			char newServerIP[250];
			strncpy(newServerIP, cur, 249);	newServerIP[249] = 0;
			if (strchr(newServerIP, ' '))
				*strchr(newServerIP, ' ') = 0;
			else
				goto MalformedError;


			char newServerPSK[270];
			strncpy(newServerPSK, strchr(cur, ' ') + 1, 269);	newServerPSK[269] = 0;
			if (strchr(newServerPSK, ' '))
				*strchr(newServerPSK, ' ') = 0;
			else
				goto MalformedError;


			char newServerBandwidth[280];
			strncpy(newServerBandwidth, strchr(strchr(cur, ' ') + 1, ' ') + 1, 279);	newServerBandwidth[279] = 0;
			if (strchr(newServerBandwidth, ' '))
				*strchr(newServerBandwidth, ' ') = 0;
			else
				goto MalformedError;

			char newServerCert[3000];
			strncpy(newServerCert, strchr(strchr(strchr(cur, ' ') + 1, ' ') + 1, ' ') + 1, 2999); newServerCert[2999] = 0;
			for (char* astersToNewlines = newServerCert; *astersToNewlines; astersToNewlines++)
				if (*astersToNewlines == '*')
					*astersToNewlines = '\n';

			serverIPs.push_back(string(newServerIP));
			serverPSKs.push_back(string(newServerPSK));
			serverBandwidths.push_back(string(newServerBandwidth));
			serverCerts.push_back(string(newServerCert));

			cur = strtok(NULL, "\n");
		}

		/*
		AT THIS POINT WE HAVE PARSED THE DATA INTO THESE VALUES:
		char receivedVPN_basePassword[VPN_BASE_PASSWORD_LENGTH+1];
		char trustLvlStr[10];
		char lastCodeRequestSSE[20]; //toString of an int representing seconds since midnight jan 1 1970
		vector < string > serverIPs;
		vector < string > serverPSKs;
		vector < string > serverBandwidths;
		vector < string > serverCerts; //note: *s have been converted to \ns
		*/

		//the "SalmonConfig" file was supposed to just be a direct dump of the data, but that appears to add in some extra newlines.
		//So, we can use the parsed values. I didn't write all that parsing for nothing after all!
		FILE* writeSalmonConfig = openConfigFile("SalmonConfig.txt", "wb");//NOTE apparently wt in Windows appends \n after each fwrite?
		fwrite(receivedVPN_basePassword, 1, strlen(receivedVPN_basePassword), writeSalmonConfig);
		fputc('\n', writeSalmonConfig);
		fwrite(trustLvlStr, 1, strlen(trustLvlStr), writeSalmonConfig);
		fputc('\n', writeSalmonConfig);
		fwrite(lastCodeRequestSSE, 1, strlen(lastCodeRequestSSE), writeSalmonConfig);
		fputc('\n', writeSalmonConfig);
		for (int i = 0; i < serverIPs.size(); i++)
		{
			if (i >= serverBandwidths.size())
				break;
			fwrite(serverIPs[i].c_str(), 1, serverIPs[i].length(), writeSalmonConfig);
			fputc(' ', writeSalmonConfig);
			fwrite(serverBandwidths[i].c_str(), 1, serverBandwidths[i].length(), writeSalmonConfig);
			fputc(' ', writeSalmonConfig);
			fwrite(serverPSKs[i].c_str(), 1, serverPSKs[i].length(), writeSalmonConfig);
			fputc('\n', writeSalmonConfig);
		}
		fclose(writeSalmonConfig);
		//NOTE trustLevelDisplayAndWriteFile writes to SalmonConfig.txt, so it must be called here after the fclose().
		trustLevelDisplayAndWriteFile(atoi(trustLvlStr));

		//the certs go into their own files, rather than SalmonConfig.txt.
		for (int i = 0; i < serverIPs.size(); i++)
		{
			string certFilename = serverIPs[i] + ".pem";
			FILE* writeCert = openConfigFile(certFilename.c_str(), "wt");
			fwrite(serverCerts[i].c_str(), 1, serverCerts[i].length(), writeCert);
			fclose(writeCert);
		}

		//generate_S_from_SalmonConfig will load in gBaseVPNPassword and knownServers from the (should be) nicely
		//formatted SalmonConfig.txt we just wrote, and create the appropriate connection settings in softether.
		//but, if it reports that anything went wrong...
		if (!generate_S_from_SalmonConfig())
		{
			//...try to make do with the password, bandwidth, & server IPs we have here, which should be correct.
			knownServers.clear();
			strcpy(gBaseVPNPassword, receivedVPN_basePassword);
			for (int i = 0; serverIPs.size(); i++)
			{
				VPNInfo newVPN(serverIPs[i].c_str(), atoi(serverBandwidths[i].c_str()), 50, atoi(serverBandwidths[i].c_str())- 50, 0, serverPSKs[i].c_str());
				newVPN.lastAttempt = 0;
				newVPN.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);
				knownServers.push_back(newVPN);
			}

			for (int i = 0; i < knownServers.size(); i++)
				createConnectionSetting(knownServers[i].addr);
		}

		enableAllButtonsLogin();
		delete loginRecvStruct;

		return;

		MalformedError:
		MessageBox(NULL, localizeConst(DIRSERV_GAVE_MALFORMED_RESPONSE_TO_LOGIN), localizeConst(FATAL_ERROR), MB_OK);
		enableAllButtonsLogin();
		delete loginRecvStruct;
		return;
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwLogin);
		MessageBox(NULL, localizeConst(ABORT_LOGIN_NO_RESPONSE), localizeConst(ERROR_STR), MB_OK);
		enableAllButtonsLogin();
		delete loginRecvStruct;
	}
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
		if (!(strstr(gUserEmailAccount, "@gmail.com") || strstr(gUserEmailAccount, "@hotmail.com") || strstr(gUserEmailAccount, "@outlook.com") || strstr(gUserEmailAccount, "@yahoo.com")))
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
		{
			disableAllButtonsLogin();
			UpdateWindow(wndwLogin);

			char ourRandStr[51];
			if (sendMail(L"existingLogin", ourRandStr) == false)
			{
				MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
				enableAllButtonsLogin();
				return;
			}

			loginRecvStruct = new RecvThreadStruct(ourRandStr, loginMailCallback);
			CreateThread(NULL, 0, recvThread, loginRecvStruct, 0, NULL);
		}
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
}

