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
#include <vector>
using std::vector;

#include "salmon_constants.h"
#include "salmon_globals.h"


//All our VPN servers are told to accept a username+pw that is a hash of this, salted with their IP address.
//Loaded by getSConfig(), or when logging in requires us to get our account info from the directory server.
char gBaseVPNPassword[VPN_BASE_PASSWORD_LENGTH + 1];


//Do we currently have a working connection with a VPN server? If so, its info is in gCurrentConnection.
bool gVPNConnected = false;
VPNInfo gCurrentConnection("", 0, 10000, -10000, 0, "");

//If the monitor thread sees that we aren't currently connected, should it initiate a connection attempt?
bool gUserWantsConnection = false;

//Mutex to keep the connection monitor thread from stepping on the toes of an ongoing connection attempt
HANDLE gConnectionStateMutex;

//Mutex for modifying knownServers
HANDLE gServerInfoMutex;
//Any server read from a config file, or returned by needServer, gets put in here.
vector<VPNInfo> knownServers;

//Full path for vpncmd.exe. load_vpncmdexe_Path() loads it.
char g_vpncmdPath[VPNCMD_PATH_BUFSIZE];

//User's login credentials. Should be login credentials for gmail, microsoft, or yahoo mail. (Including e.g. @gmail.com).
char gUserEmailAccount[EMAIL_ADDR_BUFSIZE];
char gUserEmailPassword[EMAIL_PASSWORD_BUFSIZE];

//Which social network is currently chosen? Set at locale select, and can be changed by the user in the socnet-reg window.
SocialNetwork gCurSocialNetwork = SOCNET_NONE;

//When the user clicks "Connect", it becomes "Cancel connecting" until the connection attempt succeeds or fails. When
//the user clicks "Cancel connecting", we simply set this flag to true. The connectionAttemptThread, and every iteration
//of the tryConnectAnyServer() function it calls, checks this flag before attempting the next major step in the process
//(making a specific connection attempt, or doing needServer()).
bool cancelConnectionAttempt = false;

//US, CN, IR: country TLD, in uppercase
WCHAR globalLocale[3];

//Language the user chose in the wndwLocaleSelect window.
SalmonLanguage gChosenLanguage = SALMON_LANGUAGE_EN;

const WCHAR* salmonDirServAddr = L"cs-backend-salmon@mx.uillinois.edu";
const WCHAR* microsoftSMTP = L"smtp.live.com";
const WCHAR* microsoftIMAP = L"imap-mail.outlook.com";
const WCHAR* gmailSMTP = L"smtp.gmail.com";
const WCHAR* gmailIMAP = L"imap.gmail.com";
const WCHAR* yahooIMAP = L"imap.mail.yahoo.com";
const WCHAR* yahooSMTP = L"smtp.mail.yahoo.com";
