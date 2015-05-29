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

#ifndef _SALMON_CLIENT_INCLGUARD_HWNDS_
#define _SALMON_CLIENT_INCLGUARD_HWNDS_

//initial language selection window
extern HWND wndwLocaleSelect;
extern HWND bttnEnglish;
extern HWND bttnChinese;
extern HWND bttnPersian;


//login or register window
extern HWND wndwLogin;
extern HWND framFirstTime;
extern HWND bttnRegisterSocial;
extern HWND bttnRegisterRecd;
extern HWND framExistingUser;
extern HWND sttcLoginEmailAddr;
extern HWND sttcLoginEmailPW;
extern HWND textLoginEmailAddr;
extern HWND textLoginEmailPW;//NOTE should be ES_PASSWORD
//extern HWND chckRemember;
extern HWND bttnLogin;


//register with facebook/renren window
extern HWND wndwRegisterSocial;
extern HWND textEnterPost;
//extern HWND rdioFacebook;//NOTE doesn't need separate static text; gets a text label
//extern HWND rdioRenren;//TODO BS_AUTORADIOBUTTON?
extern HWND sttcSocNetID;
extern HWND textSocNetID;//NOTE should be ES_NUMBER if renren
extern HWND sttcSocEmailAddr;
extern HWND textSocEmailAddr;
extern HWND sttcSocEmailPW;
extern HWND textSocEmailPW;//NOTE should be ES_PASSWORD
extern HWND bttnSocRegSubmit;


//register with rec code
extern HWND wndwRegisterRecd;
extern HWND sttcRegRecCode;
extern HWND textRegRecCode;
extern HWND sttcRecEmailAddr;
extern HWND textRecEmailAddr;
extern HWND sttcRecEmailPW;
extern HWND textRecEmailPW;//NOTE should be ES_PASSWORD
extern HWND bttnRecRegSubmit;


//main window
extern HWND wndwMain;
extern HWND sttcConnectStatus;
extern HWND bttnCancelConnect;
extern HWND bttnDisconnect;
extern HWND bttnConnect;
extern HWND bttnShowHideOptions;
extern HWND sttcTrustStatus;
extern HWND bttnGetRec;
extern HWND framRedeemCode;//SS_ETCHEDFRAME
extern HWND textRedeemCode;
extern HWND bttnRedeemCode;
extern HWND bttnAndroidGetter;
extern HWND bttniOSGetter;
extern HWND bttnWipeConfig;
extern HWND sttcVersion;


//waiting for directory server to respond
extern HWND wndwWaiting;
extern HWND sttcWaiting;
extern HWND bttnCancelWaiting;

extern HFONT gFontHandle;


#endif