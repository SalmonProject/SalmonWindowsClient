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

/*NOTE
http://msdn.microsoft.com/en-us/library/windows/desktop/bb775951(v=vs.85).aspx
BS_ICON or BS_BITMAP set?	BM_SETIMAGE called?	Result
Yes							Yes					Show icon only.
No							Yes					Show icon and text.
Yes							No					Show text only.
No							No					Show text only
*/




void createWindowsLocale(LPCWSTR className, HINSTANCE thisInstance)
{

wndwLocaleSelect = CreateWindowEx(
	0,//WS_EX_CLIENTEDGE, // Extended possibilites for variation 
	className,         // Classname 
	L"Select language", //haha, wait, right... localizeConst(SELECT_LANGUAGE_TITLE),       // Title Text 
	WS_OVERLAPPEDWINDOW & ~WS_SIZEBOX, // default window 
	CW_USEDEFAULT,       // Windows decides the position 
	CW_USEDEFAULT,       // where the window ends up on the screen 
	250,                 // The programs width 
	190,                 // and height in pixels 
	HWND_DESKTOP,        // The window is a child-window to desktop 
	NULL,                // No menu 
	thisInstance,       // Program Instance handler 
	NULL                 // No Window Creation data 
	);

const int bttnEnglishY = 20;
//initial language selection window
bttnEnglish = CreateWindow(
	L"BUTTON",   // predefined class
	L"English",       // text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
	68,         // starting x position
	bttnEnglishY,         // starting y position
	100,        // width
	30,        // height
	wndwLocaleSelect,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwLocaleSelect, GWL_HINSTANCE),
	NULL);      // pointer not needed;

const int bttnChineseY = bttnEnglishY + 40;
bttnChinese = CreateWindow(
	L"BUTTON",   // predefined class
	L"中文",       // text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
	68,         // starting x position
	bttnChineseY,         // starting y position
	100,        // width
	30,        // height
	wndwLocaleSelect,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwLocaleSelect, GWL_HINSTANCE),
	NULL);      // pointer not needed;

const int bttnPersianY = bttnChineseY + 40;
bttnPersian = CreateWindow(
	L"BUTTON",   // predefined class
	L"فارسی",       // text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_EX_RTLREADING,  // styles
	68,         // starting x position
	bttnPersianY,         // starting y position
	100,        // width
	30,        // height
	wndwLocaleSelect,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(wndwLocaleSelect, GWL_HINSTANCE),
	NULL);      // pointer not needed;

SendMessage(bttnEnglish, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnChinese, WM_SETFONT, (WPARAM)gFontHandle, 0);
SendMessage(bttnPersian, WM_SETFONT, (WPARAM)gFontHandle, 0);

}


void winProcLocale(HWND theHwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if ((HWND)lParam == bttnEnglish)
	{
		//NOTE doesn't work, neither does setting BST_CHECKED... so use BM_SETCHECK
		//COMMENTRENRENButton_SetState(rdioFacebook, BST_PUSHED);
		gCurSocialNetwork = SOCNET_FACEBOOK;
		//COMMENTRENRENSendMessage(rdioFacebook, BM_SETCHECK, 1, 0);
		gChosenLanguage = SALMON_LANGUAGE_EN;
		saveLanguageToConfig(gChosenLanguage);
		ShowWindow(wndwLocaleSelect, SW_HIDE);
		ShowWindow(wndwLogin, SW_SHOW);
		setAllText();
		MessageBox(NULL, localizeConst(LEGAL_WARNING), L"Warning", MB_OK);
	}
	else if ((HWND)lParam == bttnChinese)
	{
		//COMMENTRENRENSendMessage(rdioRenren, BM_SETCHECK, 1, 0);
		gCurSocialNetwork = SOCNET_FACEBOOK;//COMMENTRENRENgCurSocialNetwork = SOCNET_RENREN;
		//COMMENTRENRENStatic_SetText(sttcSocNetID, localizeConst(RENREN_ID_INSTRUCTIONS));
		gChosenLanguage = SALMON_LANGUAGE_ZH;
		saveLanguageToConfig(gChosenLanguage);
		ShowWindow(wndwLocaleSelect, SW_HIDE);
		ShowWindow(wndwLogin, SW_SHOW);
		setAllText();
		MessageBox(NULL, localizeConst(LEGAL_WARNING), L"法律声明", MB_OK);
	}
	else if ((HWND)lParam == bttnPersian)
	{
		//COMMENTRENRENSendMessage(rdioFacebook, BM_SETCHECK, 1, 0);
		gCurSocialNetwork = SOCNET_FACEBOOK;
		gChosenLanguage = SALMON_LANGUAGE_IR;
		saveLanguageToConfig(gChosenLanguage);
		ShowWindow(wndwLocaleSelect, SW_HIDE);
		ShowWindow(wndwLogin, SW_SHOW);
		setAllText();
		MessageBox(NULL, localizeConst(LEGAL_WARNING), L"هشدار", MB_OK);
	}
	
}