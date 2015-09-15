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
#include <Windows.h>

#include <vector>
#include <string>
using std::vector;
using std::string;

#include "hwnds.h"
#include "email.h"
#include "localization.h"
#include "VPNInfo.h"

#include "control_softether.h"

#include "salmon_utility.h"
#include "salmon_config.h"

#include "emailReplyCallbacks.h"

void noOpMailCallback(RecvMailCodes dummy, string dummy2){ return; }

namespace{

//Parse the text of the dir server's response to an existingLogin message. Returns false
//if the response was malformed in any way (in which case some/all out-params will be unfilled).
//lastCodeRequestSSE is toString of a seconds since Unix epoch int.
//The response comes with the .pem certs' \n's replaced with *'s; this function converts them back to \n's.
bool parseExistingLoginResponse(const string& responseText, char* receivedVPN_basePassword, char* trustLvlStr, char* lastCodeRequestSSE,
								vector<string>* serverIPs, vector<string>* serverPSKs, vector<string>* serverBandwidths, vector<string>* serverCerts)
{
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

	//first line should be vpn base password
	char* forTokening = strdup(responseText.c_str());
	char* cur = strtok(forTokening, "\n\r");
	if (cur == NULL)
		goto MalformedError;
	memcpy(receivedVPN_basePassword, cur, VPN_BASE_PASSWORD_LENGTH);
	receivedVPN_basePassword[VPN_BASE_PASSWORD_LENGTH] = 0;

	//second line should be trust level
	cur = strtok(NULL, "\n\r");
	if (cur == NULL || strlen(cur) > 9)
		goto MalformedError;
	strcpy(trustLvlStr, cur);

	//third line should be last time this user asked for a new rec code
	cur = strtok(NULL, "\n\r");
	if (cur == NULL || strlen(cur) > 59)
		goto MalformedError;
	strcpy(lastCodeRequestSSE, cur);

	//now zero or more server info lines (ipaddr offeredBW cert)
	for (cur = strtok(NULL, "\n\r"); cur != NULL; cur = strtok(NULL, "\n\r"))
	{
		//skip any empty lines...
		while (cur != NULL && lineIsJustWhitespace(cur))
			cur = strtok(NULL, "\n\r");
		//...and if we have arrived at the end of the text, we're done.
		if (cur == NULL)
			break;

		//each of these lines should be "ipaddr psk bw cert"
		if (strchr(cur, ' ') == NULL || strchr(strchr(cur, ' ') + 1, ' ') == NULL || strchr(strchr(strchr(cur, ' ') + 1, ' ') + 1, ' ') == NULL)
			continue;

		char newServerIP[250];
		strncpy(newServerIP, cur, 249);	newServerIP[249] = 0;
		if (strchr(newServerIP, ' '))
			*strchr(newServerIP, ' ') = 0;
		else continue;

		char newServerPSK[270];
		strncpy(newServerPSK, strchr(cur, ' ') + 1, 269);	newServerPSK[269] = 0;
		if (strchr(newServerPSK, ' '))
			*strchr(newServerPSK, ' ') = 0;
		else continue;

		char newServerBandwidth[280];
		strncpy(newServerBandwidth, strchr(strchr(cur, ' ') + 1, ' ') + 1, 279);	newServerBandwidth[279] = 0;
		if (strchr(newServerBandwidth, ' '))
			*strchr(newServerBandwidth, ' ') = 0;
		else continue;

		char newServerCert[3000];
		strncpy(newServerCert, strchr(strchr(strchr(cur, ' ') + 1, ' ') + 1, ' ') + 1, 2999); newServerCert[2999] = 0;
		for (char* astersToNewlines = newServerCert; *astersToNewlines; astersToNewlines++)
			if (*astersToNewlines == '*')
				*astersToNewlines = '\n';

		serverIPs->push_back(string(newServerIP));
		serverPSKs->push_back(string(newServerPSK));
		serverBandwidths->push_back(string(newServerBandwidth));
		serverCerts->push_back(string(newServerCert));
	}
	free(forTokening);
	return true;

MalformedError:
	free(forTokening);
	return false;
}


void writeSalmonConfigFromExistingLoginResponse(const char* receivedVPN_basePassword, const char* trustLvlStr, const char* lastCodeRequestSSE, 
												const vector<string>& serverIPs, const vector<string>& serverPSKs, const vector<string>& serverBandwidths)
{
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
}

} //anonymous namespace

namespace{
class ManualCallbackCallerThreadArgs
{
public:
	void(*manualEmailCallback)(RecvMailCodes, string);
	string callbackStringArg;

	ManualCallbackCallerThreadArgs(void(*theCallback)(RecvMailCodes, string), string stringArg)
	{
		callbackStringArg = stringArg;
		manualEmailCallback = theCallback;
	}
private:
	ManualCallbackCallerThreadArgs(){}
};
DWORD WINAPI manualCallbackCallerThread(LPVOID lpParam)
{
	ManualCallbackCallerThreadArgs* pointerArg = (ManualCallbackCallerThreadArgs*)lpParam;
	ManualCallbackCallerThreadArgs argsCopy = *pointerArg;
	delete pointerArg;

	argsCopy.manualEmailCallback(RECV_MAIL_SUCCESS, argsCopy.callbackStringArg);

	return 0;
}
} //anonymous namespace
//Helper function for calling one of these callbacks in its own thread
void asyncCallCallback(void(*theCallback)(RecvMailCodes, string), string stringArg)
{
	//(delete'd in thread)
	ManualCallbackCallerThreadArgs* passThreadArgs = new ManualCallbackCallerThreadArgs(theCallback, stringArg);
	CreateThread(NULL, 0, manualCallbackCallerThread, passThreadArgs, 0, NULL);
}

void loginMailCallback(RecvMailCodes successful, string responseText)
{
	if (successful != RECV_MAIL_FAIL && successful != RECV_MAIL_CANCEL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwLogin);
		
		//NOTE YES we can rely on $ here
		if (strchr(responseText.c_str(), '$'))//$ means there's some error; display it to the user
		{
			localizeDirServMsgBox(responseText.c_str(), localizeConst(ERROR_STR));
			enableAllButtonsAll();
			return;
		}

		ShowWindow(wndwLogin, SW_HIDE);
		ShowWindow(wndwMain, SW_SHOW);
		enableAllButtonsAll();

		char receivedVPN_basePassword[VPN_BASE_PASSWORD_LENGTH + 1];
		char trustLvlStr[20];
		char lastCodeRequestSSE[60]; //toString of a seconds since Unix epoch int.
		vector<string> serverIPs;
		vector<string> serverPSKs;
		vector<string> serverBandwidths;
		vector<string> serverCerts; //NOTE: *'s will have been converted to \n's
		if (!parseExistingLoginResponse(responseText, receivedVPN_basePassword, trustLvlStr, lastCodeRequestSSE, &serverIPs, &serverPSKs, &serverBandwidths, &serverCerts))
		{
			MessageBox(NULL, localizeConst(DIRSERV_GAVE_MALFORMED_RESPONSE_TO_LOGIN), localizeConst(FATAL_ERROR), MB_OK);
			enableAllButtonsAll();
			return;
		}

		writeSalmonConfigFromExistingLoginResponse(receivedVPN_basePassword, trustLvlStr, lastCodeRequestSSE, serverIPs, serverPSKs, serverBandwidths);

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
			gKnownServers.clear();
			strcpy(gBaseVPNPassword, receivedVPN_basePassword);
			for (int i = 0; serverIPs.size(); i++)
			{
				VPNInfo newVPN(serverIPs[i].c_str(), atoi(serverBandwidths[i].c_str()), 50, atoi(serverBandwidths[i].c_str()) - 50, 0, serverPSKs[i].c_str());
				newVPN.lastAttempt = 0;
				newVPN.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);
				gKnownServers.push_back(newVPN);
			}

			for (int i = 0; i < gKnownServers.size(); i++)
				createConnectionSetting(gKnownServers[i].addr);
		}
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwLogin);
		MessageBox(NULL, localizeConst(ABORT_LOGIN_NO_RESPONSE), localizeConst(ERROR_STR), MB_OK);
	}
	enableAllButtonsAll();
}


void getRecMailCallback(RecvMailCodes successful, string textResponse)
{
	if (successful != RECV_MAIL_FAIL && successful != RECV_MAIL_CANCEL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);

		localizeDirServMsgBox(textResponse.c_str(), localizeConst(DIR_SERVER_SAYS));
		enableAllButtonsAll();

		if (!strncmp(textResponse.c_str(), "$0", 2))
			return;

		if (!strncmp(textResponse.c_str(), "$12", 3))
			trustLevelDisplayAndWriteFile(3);
		else if (!strncmp(textResponse.c_str(), "$13", 3) || !strncmp(textResponse.c_str(), "$14", 3))
			trustLevelDisplayAndWriteFile(6);
		else if (!strncmp(textResponse.c_str(), "$15", 3))
			trustLevelDisplayAndWriteFile(7);
		else if (readTrustFromFile() <= 6)
			trustLevelDisplayAndWriteFile(6);
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
		MessageBox(NULL, localizeConst(NO_RESPONSE_FROM_DIR), localizeConst(ERROR_STR), MB_OK);
		enableAllButtonsAll();
	}
}

void redeemCodeMailCallback(RecvMailCodes successful, string responseText)
{
	if (successful != RECV_MAIL_FAIL && successful != RECV_MAIL_CANCEL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
		localizeDirServMsgBox(responseText.c_str(), localizeConst(DIR_SERVER_SAYS));

		if (!strncmp(responseText.c_str(), "$17", 3) || !strncmp(responseText.c_str(), "$16", 3) || !strncmp(responseText.c_str(), "$10", 3))
			trustLevelDisplayAndWriteFile(5);
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwMain);
		MessageBox(NULL, localizeConst(NO_RESPONSE_FROM_DIR), localizeConst(ERROR_STR), MB_OK);
	}
	enableAllButtonsAll();
}



void recRegMailCallback(RecvMailCodes successful, string responseText)
{
	if (successful != RECV_MAIL_FAIL && successful != RECV_MAIL_CANCEL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwRegisterRecd);

		//if the registration succeeded, the dir. gives us a VPN password, which is VPN_PASSWORD_LENGTH letters a-z. (and that's it).
		//so, presence of a $ indicates an error that should be MessageBox'd.
		if (!strchr(responseText.c_str(), '$'))
		{
			ShowWindow(wndwRegisterRecd, SW_HIDE);
			ShowWindow(wndwMain, SW_SHOW);
			SetFocus(wndwMain);

			//write an initial config file. we have the VPN password, and other than that it's just defaults.
			FILE* writeConfig = openConfigFile("SalmonConfig.txt", "wt");
			fwrite(responseText.c_str(), 1, VPN_BASE_PASSWORD_LENGTH, writeConfig);
			fputc('\n', writeConfig); fputc('0', writeConfig); fputc('\n', writeConfig); fputc('0', writeConfig); fputc('\n', writeConfig);
			fclose(writeConfig);

			trustLevelDisplayAndWriteFile(5);

			//this is an implicit $7 from the directory server.
			localizeDirServMsgBox("$7", L"");
		}
		else
			localizeDirServMsgBox(responseText.c_str(), localizeConst(ERROR_STR));
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwRegisterRecd);
		MessageBox(NULL, localizeConst(ABORT_REG_NO_RESPONSE), localizeConst(ERROR_STR), MB_OK);
	}
	enableAllButtonsAll();
}

namespace{
void waitUntilUserPostedPost()
{
	int havePosted = IDNO;
	while (havePosted == IDNO)
	{
		havePosted = MessageBox(NULL, localizeConst(HAVE_YOU_POSTED_YET), localizeConst(DIR_SERVER_SAYS), MB_YESNO);
		if (havePosted == IDNO)
			MessageBox(NULL, localizeConst(PLEASE_POST_NOW), localizeConst(DIR_SERVER_SAYS), MB_OK);
	}
}
} //anonymous namespace

void finishRegMailCallback(RecvMailCodes successful, string responseText)
{
	if (successful != RECV_MAIL_FAIL && successful != RECV_MAIL_CANCEL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwRegisterSocial);

		//if the registration succeeded, the dir. gives us a VPN password, which is VPN_PASSWORD_LENGTH letters a-z. (and that's it).
		//so, presence of a $ indicates an error that should be MessageBox'd.
		if (!strchr(responseText.c_str(), '$'))
		{
			//the buffer will only have the password, but since we've gotten here we know it's a success. i've set dirsrvmsg $7 to be something you can print here.
			localizeDirServMsgBox("$7", L"");

			ShowWindow(wndwRegisterSocial, SW_HIDE);
			ShowWindow(wndwMain, SW_SHOW);

			//write an initial config file. we have the VPN password, and other than that it's just defaults.
			FILE* writeConfig = openConfigFile("SalmonConfig.txt", "wt");
			fwrite(responseText.c_str(), 1, VPN_BASE_PASSWORD_LENGTH, writeConfig);
			fputc('\n', writeConfig); fputc('0', writeConfig); fputc('\n', writeConfig); fputc('0', writeConfig); fputc('\n', writeConfig);
			fclose(writeConfig);
		}
		//if the failure was just because "we didn't see your post", give the user a second chance, rather than restarting the process.
		else if (!strncmp(responseText.c_str(), "$8", 2))
		{
			localizeDirServMsgBox(responseText.c_str(), localizeConst(ERROR_STR)); //something like "we didn't see the post"
			waitUntilUserPostedPost();
			sendMessageToDirServ(L"finishRegistration", finishRegMailCallback);
			return;
		}
		else
			localizeDirServMsgBox(responseText.c_str(), localizeConst(ERROR_STR));
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwRegisterSocial);
		MessageBox(NULL, localizeConst(ABORT_REG_NO_RESPONSE), localizeConst(ERROR_STR), MB_OK);
	}
	enableAllButtonsAll();
}

void startRegMailCallback(RecvMailCodes successful, string responseText)
{
	if (successful != RECV_MAIL_FAIL && successful != RECV_MAIL_CANCEL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwRegisterSocial);

		localizeDirServMsgBox(responseText.c_str(), localizeConst(DIR_SERVER_SAYS));

		if (!checkIfSuccessfulStartReg(responseText.c_str()))
		{
			enableAllButtonsAll();
			return;
		}

		waitUntilUserPostedPost();
		sendMessageToDirServ(L"finishRegistration", finishRegMailCallback);
	}
	else if (successful == RECV_MAIL_FAIL)
	{
		ShowWindow(wndwWaiting, SW_HIDE);
		SetFocus(wndwRegisterSocial);
		MessageBox(NULL, localizeConst(ABORT_REG_NO_RESPONSE), localizeConst(ERROR_STR), MB_OK);
		enableAllButtonsAll();
	}
}
