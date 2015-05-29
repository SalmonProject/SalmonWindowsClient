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
#include <Windows.h>

//=========================================================
//Handles for windows controls:
//initial language selection window
HWND wndwLocaleSelect;
HWND bttnEnglish;
HWND bttnChinese;
HWND bttnPersian;

//login or register window
HWND wndwLogin;
HWND framFirstTime;
HWND bttnRegisterSocial;
HWND bttnRegisterRecd;
HWND framExistingUser;
HWND sttcLoginEmailAddr;
HWND sttcLoginEmailPW;
HWND textLoginEmailAddr;
HWND textLoginEmailPW;//NOTE should be asterisks
HWND bttnLogin;

//register with facebook/renren window
HWND wndwRegisterSocial;
HWND textEnterPost;
HWND sttcFacebook;
//HWND sttcRenren;
//HWND rdioFacebook;
//HWND rdioRenren;
HWND sttcSocNetID;
HWND textSocNetID;
HWND sttcSocEmailAddr;
HWND textSocEmailAddr;
HWND sttcSocEmailPW;
HWND textSocEmailPW;//NOTE should be asterisks
HWND bttnSocRegSubmit;

//register with rec code
HWND wndwRegisterRecd;
HWND sttcRegRecCode;
HWND textRegRecCode;
HWND sttcRecEmailAddr;
HWND textRecEmailAddr;
HWND sttcRecEmailPW;
HWND textRecEmailPW;//NOTE should be asterisks
HWND bttnRecRegSubmit;

//main window
HWND wndwMain;
HWND sttcConnectStatus;
HWND bttnCancelConnect;
HWND bttnDisconnect;
HWND bttnConnect;
HWND bttnShowHideOptions;
HWND sttcTrustStatus;
HWND bttnGetRec;
HWND framRedeemCode;
HWND textRedeemCode;
HWND bttnRedeemCode;
HWND bttnAndroidGetter;
HWND bttniOSGetter;
HWND bttnWipeConfig;
HWND sttcVersion;

//waiting for directory server to respond
HWND wndwWaiting;
HWND sttcWaiting;
HWND bttnCancelWaiting;

//Some background win32 stuff; window controls need to get connected with this, or else they look like 1995.
HFONT gFontHandle;
//=========================================================
