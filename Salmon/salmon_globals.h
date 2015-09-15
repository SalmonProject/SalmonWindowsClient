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

#ifndef _SALMON_INCL_GUARD_GLOBALS_H_
#define _SALMON_INCL_GUARD_GLOBALS_H_

#include <string>

#include "salmon_constants.h"
#include "VPNInfo.h"
#include "emailReplyCallbacks.h"
#include "manual_or_blocking_email.h"



//All our VPN servers are told to accept a username+pw that is a hash of this, salted with their IP address.
//Loaded by getSConfig(), or when logging in requires us to get our account info from the directory server.
extern char gBaseVPNPassword[VPN_BASE_PASSWORD_LENGTH + 1];


//Do we currently have a working connection with a VPN server? If so, its info is in gCurrentConnection.
extern bool gVPNConnected;// = false;
extern VPNInfo gCurrentConnection;

//If the monitor thread sees that we aren't currently connected, should it initiate a connection attempt?
extern bool gUserWantsConnection;// = false;

//Mutex to keep the connection monitor thread from stepping on the toes of an ongoing connection attempt
extern HANDLE gConnectionStateMutex;

//Mutex for modifying knownServers
extern HANDLE gServerInfoMutex;
//Any server read from a config file, or returned by needServer, gets put in here.
extern std::vector<VPNInfo> gKnownServers;

//The manual send/recv process involves the user interacting with GUI elements in between
//the sending and receiving. This means that the thread that collects the response data
//is the main GUI thread, so to get the data back to whatever piece of logic initiated the
//send/recv, we need some global stuff. This class gathers it all together (and also covers
//the case where the requesting logic wanted to block until the response arrived).
extern ManualEmailGlobalHelper gManualEmail;

//Full path for vpncmd.exe. load_vpncmdexe_Path() loads it.
extern char g_vpncmdPath[VPNCMD_PATH_BUFSIZE];

//User's login credentials. Should be login credentials for gmail, microsoft, or yahoo mail. (Including e.g. @gmail.com).
extern char gUserEmailAccount[EMAIL_ADDR_BUFSIZE];
extern char gUserEmailPassword[EMAIL_PASSWORD_BUFSIZE];

//Which social network is currently chosen? Set at locale select, and can be changed by the user in the socnet-reg window.
enum SocialNetwork { SOCNET_NONE, SOCNET_FACEBOOK, SOCNET_RENREN };
extern SocialNetwork gCurSocialNetwork;// = SOCNET_NONE;

//When the user clicks "Connect", it becomes "Cancel connecting" until the connection attempt succeeds or fails. When
//the user clicks "Cancel connecting", we simply set this flag to true. The connectionAttemptThread, and every iteration
//of the tryConnectAnyServer() function it calls, checks this flag before attempting the next major step in the process
//(making a specific connection attempt, or doing needServer()).
extern bool gCancelConnectionAttempt;// = false;

//Language the user chose in the wndwLocaleSelect window.
enum SalmonLanguage {SALMON_LANGUAGE_INVALID = -1,	SALMON_LANGUAGE_EN = 0,	SALMON_LANGUAGE_ZH = 1,	SALMON_LANGUAGE_IR = 2};
extern SalmonLanguage gChosenLanguage;// = SALMON_LANGUAGE_INVALID;

extern const WCHAR* salmonDirServAddr;
extern const WCHAR* microsoftSMTP;
extern const WCHAR* microsoftIMAP;
extern const WCHAR* gmailSMTP;
extern const WCHAR* gmailIMAP;
extern const WCHAR* yahooIMAP;
extern const WCHAR* yahooSMTP;

#endif //_SALMON_INCL_GUARD_GLOBALS_H_
