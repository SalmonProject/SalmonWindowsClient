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

#ifndef _SALMON_CLIENT_INCLGUARD_LOCALIZATION_
#define _SALMON_CLIENT_INCLGUARD_LOCALIZATION_

#include "salmon_globals.h"

enum LocalizedString
{
	//pop-up message boxes
	COULDNT_READ_CONFIG_FILE = 0,
	COULDNT_CONNECT_WILL_needServer = 1,
	SORRY_NO_SERVERS_AVAILABLE = 2,
	FAILED_TO_SEND_EMAIL = 3,
	FAILED_TO_READ_EMAIL = 4,
	SOFTETHER_EXE_DOESNT_EXIST = 5,
	COULDNT_POPEN_PING = 6,
	DIRSERV_GAVE_MALFORMED_RESPONSE_TO_LOGIN = 7,
	ABORT_REG_NO_RESPONSE = 8,
	ABORT_LOGIN_NO_RESPONSE = 9,
	NO_RESPONSE_FROM_DIR = 10,
	MUST_SELECT_FB_OR_RENREN = 11,
	HAVE_YOU_POSTED_YET = 12,
	PLEASE_POST_NOW = 13,


	//static text boxes
	VPN_STATUS_CONNECTED = 14,
	VPN_STATUS_DISCONNECTED = 15,
	VPN_STATUS_CONNECTING = 16,
	FIRST_TIME_USER_Q = 17,
	EXISTING_USER_Q = 18,
	PROMPT_EMAIL_ADDR = 19,
	PROMPT_EMAIL_ADDR_VERBOSE = 20,
	PROMPT_EMAIL_PASSWORD = 21,
	PROMPT_EMAIL_PW_VERBOSE = 22,
	TRUST_LVL_ORDINARY = 23,
	TRUST_LVL_LOW = 24,
	TRUST_LVL_HIGH = 25,
	TRUST_LVL_HIGHEST = 26,
	TRUST_LVL_MAX = 27,
	BOOST_TRUST_W_CODE = 28,
	REC_CODE_FROM_FRIEND = 29,
	ENTER_POST_INSTRUCTIONS = 30,
	RENREN_STR = 31,
	FACEBOOK_ID_INSTRUCTIONS = 32,
	RENREN_ID_INSTRUCTIONS = 33,
	WAITING_FOR_RESPONSE = 34,


	//buttons
	REG_FB_OR_RENREN = 35,
	REG_RECOMMENDED = 36,
	LOGIN_BUTTON = 37,
	CONNECT_BUTTON = 38,
	CANCEL_CONNECTING_BUTTON = 39,
	DISCONNECT_BUTTON = 40,
	CANCEL_BUTTON = 41,
	SHOW_ADVANCED_VIEW = 42,
	HIDE_ADVANCED_VIEW = 43,
	GET_REC_CODE_FOR_FRIEND = 44,
	REDEEM_REC_CODE = 45,
	WIPE_CONFIG = 46,
	SUBMIT_REGISTRATION = 47,


	//window titles
	SELECT_LANGUAGE_TITLE = 48,
	LOGIN_OR_REG_TITLE = 49,
	SALMON_TITLE = 50,
	SALMON_REG_TITLE = 51,
	SALMON_PLEASE_WAIT_TITLE = 52,


	//misc / added later
	FACEBOOK_STR = 53,
	ERROR_STR = 54,
	FATAL_ERROR = 55,
	DIR_SERVER_SAYS = 56,
	LEGAL_WARNING = 57,
	WAITING_LONGER_FOR_RESPONSE = 58,
	GET_CREDENTIALS_FOR_ANDROID = 59,
	IS_IT_OK_TO_MAIL_VPNS = 60,
	MAILING_TO_ANDROID = 61,
	DONE_MAILING_VPNS = 62,
	INVALID_EMAIL_ADDR = 63,
	WIPE_WARNING = 64,
	WIPE_COMPLETE = 65,
	WIPE_TITLE = 66,
	WRONG_RECCODE_LENGTH = 67,
	GET_CREDENTIALS_FOR_IOS = 68,
	IS_IT_OK_TO_MAIL_VPNS_IOS = 69,
	MAILING_TO_IOS = 70,
	MOBILECONFIG_HOWTO = 71,
	ANDROID_VPNLIST_HEADER = 72,
	CANT_SEND_EMAIL_WHILE_CONNECTED = 73,
	GAVE_UP_DUE_TO_CONNECTIVITY_PROBLEMS = 74,
	WHY_EMAIL_PASSWORD = 75,
	ISNT_THIS_UNSAFE_BUTTON = 76
};


void localizeDirServMsgBox(const char* asciiStr, LPTSTR boxTitle);
const LPTSTR localizeConst(LocalizedString theStrEnumMember);
bool checkIfSuccessfulStartReg(const char* asciiStr);
void setAllText();

void saveLanguageToConfig(SalmonLanguage theLang);
SalmonLanguage loadLanguageFromConfig();
const WCHAR* countryFromLanguage(SalmonLanguage theLang);


#endif