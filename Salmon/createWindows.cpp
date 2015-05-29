#define WIN32_LEAN_AND_MEAN

#include "stdafx.h"

#include <windows.h>

//initial language selection window
extern HWND hwndLocaleSelect;
extern HWND bttnEnglish;
extern HWND bttnChinese;
extern HWND bttnPersian;


//login or register window
extern HWND hwndLogin;
extern HWND framFirstTime;
extern HWND bttnRegisterSocial;
extern HWND bttnRegisterRecd;
extern HWND framExistingUser;
extern HWND sttcLoginEmailAddr;
extern HWND sttcLoginEmailPW;
extern HWND textLoginEmailAddr;
extern HWND textLoginEmailPW;//NOTE should be ES_PASSWORD
extern HWND chckRemember;
extern HWND bttnLogin;


//register with facebook/renren window
extern HWND hwndRegisterSocial;
extern HWND textEnterPost;
extern HWND rdioFacebook;//NOTE doesn't need separate static text; gets a text label
extern HWND rdioRenren;//TODO BS_AUTORADIOBUTTON?
extern HWND sttcSocNetID;
extern HWND textSocNetID;//NOTE should be ES_NUMBER if renren
extern HWND sttcSocEmailAddr;
extern HWND textSocEmailAddr;
extern HWND sttcSocEmailPW;
extern HWND textSocEmailPW;//NOTE should be ES_PASSWORD
extern HWND bttnSocRegSubmit;


//register with rec code
extern HWND hwndRegisterRecd;
extern HWND sttcRegRecCode;
extern HWND textRegRecCode;
extern HWND sttcRecEmailAddr;
extern HWND textRecEmailAddr;
extern HWND sttcRecEmailPW;
extern HWND textRecEmailPW;//NOTE should be ES_PASSWORD
extern HWND bttnRecRegSubmit;


//main window
extern HWND hwndMain;
extern HWND sttcConnectStatus;
extern HWND bttnDisConnect;
extern HWND bttnShowHideOptions;
extern HWND sttcTrustStatus;
extern HWND bttnGetRec;
extern HWND framRedeemCode;//SS_ETCHEDFRAME
extern HWND textRedeemCode;
extern HWND bttnRedeemCode;
extern HWND bttnWipeConfig;


//waiting for directory server to respond
extern HWND hwndWaiting;
extern HWND sttcWaiting;


/*NOTE
http://msdn.microsoft.com/en-us/library/windows/desktop/bb775951(v=vs.85).aspx
BS_ICON or BS_BITMAP set?	BM_SETIMAGE called?	Result
Yes							Yes					Show icon only.
No							Yes					Show icon and text.
Yes							No					Show text only.
No							No					Show text only
*/










void createWindows()
{


//initial language selection window
bttnEnglish = CreateWindow(
	L"BUTTON",   // predefined class
	L"English",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
	68,         // starting x position
	20,         // starting y position
	100,        // button width
	30,        // button height
	hwndLocaleSelect,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLocaleSelect, GWL_HINSTANCE),
	NULL);      // pointer not needed;

bttnChinese = CreateWindow(
	L"BUTTON",   // predefined class
	L"中文",       // button text TODO TODO chinese
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
	68,         // starting x position
	60,         // starting y position
	100,        // button width
	30,        // button height
	hwndLocaleSelect,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLocaleSelect, GWL_HINSTANCE),
	NULL);      // pointer not needed;
bttnPersian = CreateWindow(
	L"BUTTON",   // predefined class
	L"Farsi",       // button text TODO TODO creative solution :) L"فارسی"
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | WS_EX_RTLREADING,  // styles
	68,         // starting x position
	100,         // starting y position
	100,        // button width
	30,        // button height
	hwndLocaleSelect,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLocaleSelect, GWL_HINSTANCE),
	NULL);      // pointer not needed;











//login or register window
framFirstTime=CreateWindow(
	L"BUTTON",   // predefined class
	L"First time user?",// text
	WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
	10,         // starting x position
	10,         // starting y position
	274,        // width
	88,        // height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnRegisterSocial = CreateWindow(
	L"BUTTON",   // predefined class
	L"Register with Facebook or Renren",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,  // styles
	20,         // starting x position
	32,         // starting y position
	120,        // button width
	60,        // button height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

bttnRegisterRecd = CreateWindow(
	L"BUTTON",   // predefined class
	L"Register with recommendation code",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,  // styles
	154,         // starting x position
	32,         // starting y position
	120,        // button width
	60,        // button height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

framExistingUser=CreateWindow(
	L"BUTTON",   // predefined class
	L"Existing user?",// button text
	WS_VISIBLE | WS_CHILD | BS_GROUPBOX,  // styles
	10,         // starting x position
	108,         // starting y position
	274,        // width
	200,        // height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed

sttcLoginEmailAddr=CreateWindow(
	L"STATIC",   // predefined class
	L"Hotmail address (inbox will be wiped):",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	20,         // starting x position
	130,         // starting y position
	250,        // width
	16,        // height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed

sttcLoginEmailPW=CreateWindow(
	L"STATIC",   // predefined class
	L"Hotmail password:",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	20,         // starting x position
	177,         // starting y position
	130,        // width
	16,        // height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed

textLoginEmailAddr = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD | CBS_AUTOHSCROLL ,  // styles
	20,         // starting x position
	150,         // starting y position
	250,        // width
	23,        // height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

textLoginEmailPW = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD | ES_PASSWORD,  // styles
	20,         // starting x position
	198,         // starting y position
	250,        // width
	26,        // height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

chckRemember = CreateWindow(
	L"BUTTON",   // predefined class
	L"Remember password",       // button text
	WS_VISIBLE | WS_CHILD | BS_AUTOCHECKBOX,  // styles
	20,         // starting x position
	232,         // starting y position
	165,        // button width
	18,        // button height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE)GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;

bttnLogin = CreateWindow(
	L"BUTTON",   // predefined class
	L"Login",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles
	110,         // starting x position
	258,         // starting y position
	80,        // button width
	40,        // button height
	hwndLogin,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndLogin, GWL_HINSTANCE),
	NULL);      // pointer not needed;










//register with facebook/renren window
textEnterPost = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"Enter a post that you have not yet posted on your wall. After clicking \"Submit Registration\", you will be instructed to post it to your wall.",
	WS_VISIBLE | WS_CHILD | ES_MULTILINE ,  // styles
	10,         // starting x position
	10,         // starting y position
	310,        // width
	120,        // height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed;

rdioFacebook = CreateWindow(
	L"BUTTON",   // predefined class
	L"Facebook",       // button text
	WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,  // styles
	10,         // starting x position
	134,         // starting y position
	100,        // button width
	20,        // button height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed;

rdioRenren = CreateWindow(
	L"BUTTON",   // predefined class
	L"Renren",       // button text
	WS_VISIBLE | WS_CHILD | BS_RADIOBUTTON,  // styles
	120,         // starting x position
	134,         // starting y position
	100,        // button width
	20,        // button height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed;

sttcSocNetID=CreateWindow(
	L"STATIC",   // predefined class
	L"Facebook ID: if your profile URL is www.facebook.com/john.doe.123, \nenter john.doe.123",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	10,         // starting x position
	160,         // starting y position
	300,        // width
	48,        // height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed

textSocNetID = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD ,  // styles
	10,         // starting x position
	210,         // starting y position
	300,        // button width
	23,        // button height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed;//NOTE should be ES_NUMBER if renren

sttcSocEmailAddr=CreateWindow(
	L"STATIC",   // predefined class
	L"Enter a hotmail.com address you own. Create a new one for this purpose. DO NOT use your personal account: your inbox will be deleted.",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	10,         // starting x position
	236,         // starting y position
	300,        // width
	32,        // height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed

textSocEmailAddr = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD ,  // styles
	10,         // starting x position
	272,         // starting y position
	300,        // width
	23,        // height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed;

sttcSocEmailPW=CreateWindow(
	L"STATIC",   // predefined class
	L"The email account's password:",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	10,         // starting x position
	298,         // starting y position
	220,        // width
	16,        // height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed

textSocEmailPW = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD | ES_PASSWORD,  // styles
	10,         // starting x position
	320,         // starting y position
	300,        // width
	26,        // height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed;

bttnSocRegSubmit = CreateWindow(
	L"BUTTON",   // predefined class
	L"Submit Registration",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	90,         // starting x position
	354,         // starting y position
	146,        // button width
	40,        // button height
	hwndRegisterSocial,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterSocial, GWL_HINSTANCE),
	NULL);      // pointer not needed

















//register with rec code
sttcRegRecCode=CreateWindow(
	L"STATIC",   // predefined class
	L"Recommendation code gotten from your friend:",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	10,         // starting x position
	4,         // starting y position
	310,        // width
	16,        // height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed

textRegRecCode = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD ,  // styles
	10,         // starting x position
	26,         // starting y position
	96,        // button width
	23,        // button height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed;

sttcRecEmailAddr=CreateWindow(
	L"STATIC",   // predefined class
	L"Enter a hotmail.com address you own. Create a new one for this purpose. DO NOT use your personal account: your inbox will be deleted.",// button text
	WS_VISIBLE | WS_CHILD ,  // styles
	10,         // starting x position
	54,         // starting y position
	310,        // width
	48,        // height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed

textRecEmailAddr = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD ,  // styles
	10,         // starting x position
	108,         // starting y position
	240,        // width
	23,        // height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed;

sttcRecEmailPW=CreateWindow(
	L"STATIC",   // predefined class
	L"The email account's password:",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	10,         // starting x position
	136,         // starting y position
	216,        // width
	16,        // height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed

textRecEmailPW = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	WS_VISIBLE | WS_CHILD | ES_PASSWORD,  // styles
	10,         // starting x position
	156,         // starting y position
	240,        // button width
	26,        // button height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed;

bttnRecRegSubmit = CreateWindow(
	L"BUTTON",   // predefined class
	L"Submit Registration",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	96,         // starting x position
	186,         // starting y position
	140,        // button width
	32,        // button height
	hwndRegisterRecd,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndRegisterRecd, GWL_HINSTANCE),
	NULL);      // pointer not needed











//main window
sttcConnectStatus=CreateWindow(
	L"STATIC",   // predefined class
	L"VPN status: DISCONNECTED",// button text TODO TODO make red/green
	WS_VISIBLE | WS_CHILD | SS_CENTER,  // styles
	9,         // starting x position
	6,         // starting y position
	196,        // width
	16,        // height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnDisConnect = CreateWindow(
	L"BUTTON",   // predefined class
	L"Connect",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	28,         // starting x position
	30,         // starting y position
	156,        // button width
	40,        // button height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnShowHideOptions = CreateWindow(
	L"BUTTON",   // predefined class
	L"Show advanced view",       // button text
	WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	28,         // starting x position
	76,         // starting y position
	156,        // button width
	30,        // button height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

sttcTrustStatus=CreateWindow(
	L"STATIC",   // predefined class
	L"Trust level: ordinary",// button text TODO TODO colors
	 WS_CHILD | SS_CENTER,  // styles
	9,         // starting x position
	116,         // starting y position
	196,        // width
	16,        // height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnGetRec = CreateWindow(
	L"BUTTON",   // predefined class
	L"Get recommendation code for friend",       // button text
	 WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	28,         // starting x position
	140,         // starting y position
	156,        // button width
	46,        // button height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

framRedeemCode=CreateWindow(
	L"BUTTON",   // predefined class
	L"Boost trust w/ rec. code",// button text
	 WS_CHILD | BS_GROUPBOX,  // styles
	18,         // starting x position
	190,         // starting y position
	176,        // width
	113,        // height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

textRedeemCode = CreateWindowEx(
	WS_EX_CLIENTEDGE,
	L"EDIT",   // predefined class
	L"",       // button text
	 WS_CHILD ,  // styles
	28,         // starting x position
	212,         // starting y position
	100,        // width
	23,        // height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed;

bttnRedeemCode = CreateWindow(
	L"BUTTON",   // predefined class
	L"Redeem recommendation code",       // button text
	 WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	28,         // starting x position
	240,         // starting y position
	156,        // button width
	56,        // button height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed

bttnWipeConfig = CreateWindow(
	L"BUTTON",   // predefined class
	L"Wipe Salmon\nconfig files",       // button text
	 WS_CHILD | BS_DEFPUSHBUTTON | BS_MULTILINE,  // styles

	// Size and position values are given explicitly, because
	// the CW_USEDEFAULT constant gives zero values for buttons.
	28,         // starting x position
	308,         // starting y position
	156,        // button width
	46,        // button height
	hwndMain,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndMain, GWL_HINSTANCE),
	NULL);      // pointer not needed














//waiting for directory server to respond
sttcWaiting=CreateWindow(
	L"STATIC",   // predefined class
	L"Waiting for directory server's response email. This should take 10-20 seconds.",// button text
	WS_VISIBLE | WS_CHILD,  // styles
	100,         // starting x position
	0,         // starting y position
	300,        // width
	100,        // height
	hwndWaiting,       // parent window
	NULL,       // No menu
	(HINSTANCE) GetWindowLong(hwndWaiting, GWL_HINSTANCE),
	NULL);      // pointer not needed

}





