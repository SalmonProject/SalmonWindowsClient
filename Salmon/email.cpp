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
#include "Resource.h"

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <WinCrypt.h>
#include <process.h>
#include <set>
using std::set;
using std::string;

#include "../Wrapper/cSmtp.hpp"
#include "../Wrapper/cPop3.hpp"
#include "../Wrapper/cImap.hpp"

#include "../libvmime/src/vmime/vmime.hpp"

#include "localization.h"
#include "email.h"
#include "salmon_constants.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "connect_attempt.h"
#include "control_softether.h"

//populate a VPNInfo struct with a new server's info, and add it to the knownServers list
void addVPNInfo(char* ipAddrBuf, int serverBW, char* serverPSK)
{
	//HACK initially, just set the rtt to 50, because pings only (are guaranteed to) get through after we have connected to the server.
	VPNInfo new_ip(ipAddrBuf, serverBW, 50, serverBW - 50, 0, serverPSK);
	new_ip.lastAttempt = 0;
	new_ip.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);

	//add (or overwrite) the new struct to the knownServers list, update SConfig.txt, and create a "connection setting" in SoftEther for the new server
	bool newServerOverwrites = false;
	for (int i = 0; i < knownServers.size(); i++)
		if (!strcmp(new_ip.addr, knownServers[i].addr))
		{
		knownServers[i] = new_ip;
		newServerOverwrites = true;
		}
	if (!newServerOverwrites)
		knownServers.push_back(new_ip);
	writeSConfigFromKnownServers();

	if(strstr(ipAddrBuf, "VPNGATE"))
		createVPNGateConnectionSetting(new_ip);
	else
		createConnectionSetting(new_ip.addr);
}

void executeAllPurges(char* buf)
{
	//We are interested in any "purge this ip addr" lines.
	char* purgeLine = strstr(buf, "PURGE");
	while (purgeLine)
	{
		char* purgeEnd = strchr(purgeLine, '\n');
		if (!purgeEnd)
			break;//malformed

		//no strndup in visual studio...
		char* purgeThis = (char*)malloc(1 + purgeEnd - purgeLine);
		memcpy(purgeThis, purgeLine, purgeEnd - purgeLine);
		purgeThis[purgeEnd - purgeLine] = 0;

		purgeServer(purgeThis);

		free(purgeThis);

		purgeLine = strstr(purgeLine + 1, "PURGE");
	}
}

//Tries to get a new server from the directory server. Returns true if successful.
//writes the gotten IP address into ipAddrBuf if successful. This function blocks until the dir server responds.
bool needServer(ConnectAnyVPNAttemptResult res)
{
	int serverBW;
	char serverPSK[10];
	char ipAddrBuf[260];

	//SoftEther's default .pem certificate appears to be ~1200 bytes, so 2500 should be reasonably future-proof
	char serverCertBuf[2500];

	//make the needServer request. we need to tell the directory about all of the servers we failed to connect to.
	string needServerString = "needServer";
	if (!res.triedAddrs.empty())
	{
		needServerString += "\n^*tried:^*";
		for (set<string>::iterator itty = res.triedAddrs.begin(); itty != res.triedAddrs.end(); itty++)
			needServerString += ("\n" + *itty);
	}

	if (!res.serverErrorAddrs.empty())
	{
		needServerString += "\n^*error:^*";
		for (set<string>::iterator itty = res.serverErrorAddrs.begin(); itty != res.serverErrorAddrs.end(); itty++)
			needServerString += ("\n" + *itty);
	}


	WCHAR* passNeedServerString = new WCHAR[needServerString.length() + 1];
	mbstowcs(passNeedServerString, needServerString.c_str(), needServerString.length() + 1);

	char ourRandStr[51];
	if (!sendMail(passNeedServerString, ourRandStr))
	{
		MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
		delete passNeedServerString;
		return false;
	}
	delete passNeedServerString;

	RecvThreadStruct tempRecvStruct(DEFAULT_MAX_EMAIL_REPLY_SIZE, ourRandStr, nullMailCallback);
	recvThread(&tempRecvStruct);

	if (tempRecvStruct.charsRecvd > 0 && strchr(tempRecvStruct.buffer, '$')) //dir server indicating an error. (but might be including VPN Gate servers.)
	{
		localizeMsgBox(tempRecvStruct.buffer, localizeConst(ERROR_STR));

		//if the directory server couldn't give us any salmon servers, it might instead give some VPN gate servers. 
		char* curVPNgate = strstr(tempRecvStruct.buffer, "VPNGATE");
		//for each VPN Gate server entry....
		while (curVPNgate)
		{
			//parse the line. format: VPNGATE1.2.3.4:443 psk(always=vpn) offeredbw the*cert*star*delims
			char ipPortVPNGate[80];
			char pskVPNGate[10];
			int offeredBW_VPNGate;
			char certVPNGate[2500];

			//get ipPortVPNGate (including the VPNGATE string, so VPNInfo ctor will know it's VPNGATE
			memcpy(ipPortVPNGate, curVPNgate, strchr(curVPNgate, ' ') - curVPNgate);
			ipPortVPNGate[strchr(curVPNgate, ' ') - curVPNgate] = 0;

			//get pskVPNGate
			char* tempPSK = strchr(curVPNgate, ' ') + 1;
			strncpy(pskVPNGate, tempPSK, 9);
			*strchr(pskVPNGate, ' ') = 0;

			//get offeredBW_VPNGate
			char* tempBW = strchr(tempPSK, ' ') + 1;
			char tempConvertBW[20];
			strncpy(tempConvertBW, tempBW, 19);
			*strchr(tempConvertBW, ' ') = 0;
			offeredBW_VPNGate = atoi(tempConvertBW);

			//get certVPNGate from the buffer...
			char* certStart = strchr(tempBW, ' ') + 1;
			char* certEnd = strchr(certStart, '\n');
			memcpy(certVPNGate, certStart, certEnd - certStart);
			certVPNGate[certEnd - certStart] = 0;
			//...convert *s to \ns...
			for (int i = 0; i < certEnd - certStart; i++)
				if (certVPNGate[i] == '*')
					certVPNGate[i] = '\n';
			//...and write to the ipaddr.pem cert file.
			char certFileName[200];
			strcpy(certFileName, ipPortVPNGate+strlen("VPNGATE"));
			if (strchr(certFileName, ':'))
				*strchr(certFileName, ':') = 0;
			strcat(certFileName, ".pem");
			FILE* writeCert = openConfigFile(certFileName, "wt");
			fwrite(certVPNGate, 1, strlen(certVPNGate), writeCert);
			fclose(writeCert);



			//add this VPNGATE item to knownServers and create a connection setting for it.
			addVPNInfo(ipPortVPNGate, offeredBW_VPNGate, pskVPNGate);


			//advance to next item, if there is another
			curVPNgate = strstr(curVPNgate+1, "VPNGATE");
		}

		//forget any servers the directory server wants us to forget
		executeAllPurges(tempRecvStruct.buffer);

		return false;
	}
	else if (tempRecvStruct.charsRecvd > 0) //received some sort of non-$error response; try to parse it.
	{
		int wholeReplyLength = strlen(tempRecvStruct.buffer);
		for (int i = 0; i < wholeReplyLength; i++)
			if (tempRecvStruct.buffer[i] == '*')
				tempRecvStruct.buffer[i] = '\n';
		if (!strchr(tempRecvStruct.buffer, ' ') || !strchr(strchr(tempRecvStruct.buffer, ' ')+1, ' ') || !strstr(tempRecvStruct.buffer, "-----END CERTIFICATE-----"))
		{
			MessageBox(NULL, localizeConst(DIRSERV_GAVE_MALFORMED_RESPONSE_TO_LOGIN), localizeConst(ERROR_STR), MB_OK);
			return false;
		}
		int ipAddrLen = (int)(strchr(tempRecvStruct.buffer, ' ') - tempRecvStruct.buffer);
		memcpy(ipAddrBuf, tempRecvStruct.buffer, ipAddrLen);
		ipAddrBuf[ipAddrLen] = 0;

		char* pskStart = strchr(tempRecvStruct.buffer, ' ') + 1;
		int pskLen = (int)(strchr(pskStart, ' ') - pskStart);
		memcpy(serverPSK, pskStart, pskLen);
		serverPSK[pskLen] = 0;
	
		//==============================
		//ok... i ended up writing this because of a silly mistake (was printing serverBW when i meant to print *serverBW; thought
		//the text was coming through as garbage), BUT, it's more robust in case there is any weirdness, so might as well keep it
		char bwTextBuf[100];
		char* startBW = 1 + strchr(tempRecvStruct.buffer, ' ');
		int curDigit = 0;
		for (; curDigit < 99 && startBW[curDigit] >= '0' && startBW[curDigit] <= '9'; curDigit++)
			bwTextBuf[curDigit] = startBW[curDigit];
		bwTextBuf[curDigit] = 0;
		//==============================
		serverBW = atoi(bwTextBuf);

		//ok, at this point, ip address and bw have been safely stored. now do cert and purges.
		
		//NOTE: we already converted the *s in the cert back to \ns a few lines earlier.
		char* certStart = strstr(tempRecvStruct.buffer, "-----BEGIN CERTIFICATE-----");
		char* certEnd = strchr(strstr(tempRecvStruct.buffer, "-----END CERTIFICATE-----"),'\n');
		memcpy(serverCertBuf, certStart, (int)(certEnd - certStart));
		serverCertBuf[(int)(certEnd - certStart)] = 0;
		//ok... just immediately write the server certificate we got into a file in the config dir. 
		//you can always access the cert whenever you need it, because it's saved under the server's ip.
		char certFileName[200];
		strcpy(certFileName, ipAddrBuf);
		strcat(certFileName, ".pem");
		FILE* writeCert = openConfigFile(certFileName, "wt");
		fwrite(serverCertBuf, 1, strlen(serverCertBuf), writeCert);
		fclose(writeCert);

		//add this server to knownServers and create a connection setting for it.
		addVPNInfo(ipAddrBuf, serverBW, serverPSK);

		//forget any servers the directory server wants us to forget
		executeAllPurges(tempRecvStruct.buffer);

		return true;
	}
	else
	{
		ipAddrBuf[0] = 0;
		return false;
	}
}


using namespace vmime::wrapper;

//NOTE because we're relying on a null terminator, this can only send unicode strings
//NOTE rndStr must be an allocated wchar_t array of at least 51 elements
//Sends a message to the directory server's email account, in the standard format.
bool sendMail(const WCHAR* send_buf, char* rndStr)
{
	//By default, SoftEther servers don't let clients do SMTP, which is reasonable.
	//So, if the user is connected and wants to do something that would send an email 
	//(e.g. have VPN info mailed to phone), tell them to disconnect first.
	if (gVPNConnected)
	{
		MessageBox(NULL, localizeConst(CANT_SEND_EMAIL_WHILE_CONNECTED), localizeConst(ERROR_STR), MB_OK);
		return false;
	}

	HCRYPTPROV cryptProvider = 0;
	CryptAcquireContext(&cryptProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(cryptProvider, 50, (BYTE*)rndStr);
	CryptReleaseContext(cryptProvider, 0);

	for (int i = 0; i < 50; i++)
		rndStr[i] = (char)(97 + ((unsigned char)rndStr[i]) % 26);
	rndStr[50] = '\0';
	WCHAR rndStrW[51];
	mbstowcs(rndStrW, rndStr, 51);
	rndStrW[50] = L'\0';

	const WCHAR* mailSubject = L"salmon backend - does not need to be seen by humans";

	WCHAR salmonAttachTempPath[300];
	GetTempPath(300, salmonAttachTempPath);
	wcscat(salmonAttachTempPath, L"salmonattach.txt");

	FILE* outFile = _wfopen(salmonAttachTempPath, L"wt, ccs=UTF-8");
	fwrite(send_buf, sizeof(WCHAR), wcslen(send_buf), outFile);
	fclose(outFile);

	//build the email

	WCHAR userLoginW[EMAIL_ADDR_BUFSIZE];
	WCHAR passwordW[EMAIL_PASSWORD_BUFSIZE];
	mbstowcs(userLoginW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
	mbstowcs(passwordW, gUserEmailPassword, EMAIL_PASSWORD_BUFSIZE);


	cEmailBuilder emailObjectToSend(userLoginW, mailSubject, IDR_MIME_TYPES);

	emailObjectToSend.AddTo(salmonDirServAddr);

	WCHAR emailBody[200];
	wcscpy(emailBody, L"Hello, do not reply to this email. No person reads mail that this address receives.\nUnique identifier ");
	wcscat(emailBody, rndStrW);
	emailObjectToSend.SetPlainText(emailBody);

	emailObjectToSend.AddAttachment(salmonAttachTempPath, L"", L"");
	emailObjectToSend.Generate();

	vmime_uint16 usePort = 0; // 0 -> use default port
	cCommon::eSecurity securityType;
	const bool allowInvalidCertificate = false;
	const WCHAR* SMTPserverName;
	if (strstr(gUserEmailAccount, "@gmail.com"))
	{
		SMTPserverName = gmailSMTP;
		securityType = cCommon::Secur_SSL;
	}
	else if (strstr(gUserEmailAccount, "@yahoo.com"))
	{
		SMTPserverName = yahooSMTP;
		//usePort = 587;
		securityType = cCommon::Secur_TLS_force;
	}
	else
	{
		SMTPserverName = microsoftSMTP;
		securityType = cCommon::Secur_SSL;
	}
	cSmtp theSMTP(SMTPserverName, usePort, securityType, allowInvalidCertificate, IDR_ROOT_CA);

	theSMTP.SetAuthData(userLoginW, passwordW);
	bool success = theSMTP.Send(&emailObjectToSend);

	_wunlink(salmonAttachTempPath);

	return success;
}

//It's already a little unfortunate that we're asking them for an email password...
//so rather than having a general "send mail" function, which would be creepy, 
//the program is only capable of sending to the salmon address, or to the user's own address.
bool sendSelfMail(const WCHAR* send_buf, WCHAR* mailSubject)
{
	//By default, SoftEther servers don't let clients do SMTP, which is reasonable.
	//So, if the user is connected and wants to do something that would send an email 
	//(e.g. have VPN info mailed to phone), tell them to disconnect first.
	if (gVPNConnected)
	{
		MessageBox(NULL, localizeConst(CANT_SEND_EMAIL_WHILE_CONNECTED), localizeConst(ERROR_STR), MB_OK);
		return false;
	}

	WCHAR userLoginW[EMAIL_ADDR_BUFSIZE];
	WCHAR passwordW[EMAIL_PASSWORD_BUFSIZE];
	mbstowcs(userLoginW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
	mbstowcs(passwordW, gUserEmailPassword, EMAIL_PASSWORD_BUFSIZE);

	cEmailBuilder emailObjectToSend(userLoginW, mailSubject, IDR_MIME_TYPES);

	emailObjectToSend.AddTo(userLoginW);
	emailObjectToSend.SetPlainText(send_buf);
	emailObjectToSend.Generate();

	vmime_uint16 usePort = 0; // 0 -> use default port
	cCommon::eSecurity securityType;
	const bool allowInvalidCertificate = false;
	const WCHAR* SMTPserverName;
	if (strstr(gUserEmailAccount, "@gmail.com"))
	{
		SMTPserverName = gmailSMTP;
		securityType = cCommon::Secur_SSL;
	}
	else if (strstr(gUserEmailAccount, "@yahoo.com"))
	{
		SMTPserverName = yahooSMTP;
		//usePort = 587;
		securityType = cCommon::Secur_TLS_force;
	}
	else
	{
		SMTPserverName = microsoftSMTP;
		securityType = cCommon::Secur_SSL;
	}
	cSmtp theSMTP(SMTPserverName, usePort, securityType, allowInvalidCertificate, IDR_ROOT_CA);

	theSMTP.SetAuthData(userLoginW, passwordW);
	return theSMTP.Send(&emailObjectToSend);
}

bool cancelRecvFlag = false;

//attempt to receive an email into buffer. discard any "salmon backend" subject mails that don't match randomStr. returns bytes written into buffer.
int recvMail(char* buffer, unsigned int buffer_size, char* randomStr, cImap IMAPinstance, int numPastEmailsToExamine)
{
	try
	{
		bool allDoneFlag = false;
		bool anyDeletions = false;
		int charsRead = 0;

		WCHAR randStrW[51];
		mbstowcs(randStrW, randomStr, 51);

		
		try{ IMAPinstance.selectDefaultFolder(); }
		catch (const std::exception& e){MessageBoxA(NULL, e.what(), "selectdefault", MB_OK);}




		//IMAPinstance.SelectFolder(L"INBOX"); // Sends the SELECT command
		int numEmailsInInbox;
		try{ numEmailsInInbox = IMAPinstance.GetEmailCount(); }
		catch (const std::exception& e){ MessageBoxA(NULL, e.what(), "getemailcount", MB_OK); }



		vector< int > emailsToDelete;

		//NOTE: if you start with M = 0 and count up, then you're going oldest -> newest.

		int emailsExamined = 0;

		for (int M = numEmailsInInbox - 1; M >= 0 && emailsExamined < numPastEmailsToExamine; M--, emailsExamined++)
		{
			GuardPtr<cEmailParser> recvdEmail = IMAPinstance.FetchEmailAt(M);
			
			//wstring bigOleBlobOfEmail = recvdEmail->GetEmail();
			//bool hasCorrectRandomString = (NULL != wcsstr(bigOleBlobOfEmail.c_str(), randStrW));

			wstring justSubject = recvdEmail->GetSubject();
			bool hasCorrectRandomString = (NULL != wcsstr(justSubject.c_str(), randStrW));
			
			//only examine this email if its subject contains the random string we expect
			if (hasCorrectRandomString)
			{
				vmime::wstring dummyName;
				vmime::wstring dummyMimeType;
				vmime::string attachData;

				vmime::wstring& dummyName_ref = dummyName;
				vmime::wstring& dummyMimeType_ref = dummyMimeType;
				vmime::string& attachData_ref = attachData;

				memset(buffer, 0, buffer_size);
				recvdEmail->GetAttachmentAt(0, dummyName_ref, dummyMimeType_ref, attachData_ref);

				if (attachData.length() < buffer_size)
				{
					attachData.copy(buffer, attachData.length(), 0);
					allDoneFlag = true;
					charsRead = attachData.length();
				}
				else
				{
					strncpy(buffer, attachData.c_str(), buffer_size - 1);
					allDoneFlag = true;
					charsRead = buffer_size - 1;
				}
				emailsToDelete.push_back(M+1);
			}
			//so... yes, the cancelRecvFlag thing is hacky, but this plus the mutex guarantees correctness - in this case, that
			//we who have been cancelled won't delete an email here that was actually for a recvMail that will be called later.
			else if (!cancelRecvFlag && wcsstr(justSubject.c_str(), L"salmon backend - does not need to be seen by humans"))
				emailsToDelete.push_back(M + 1);

			if (allDoneFlag)
				break;
		}

		if (emailsToDelete.size() > 0)
			IMAPinstance.deleteMessages(emailsToDelete);

		if (allDoneFlag)
			return charsRead;
	}
	catch (const std::exception& e)
	{
		WCHAR* exceptionW = new WCHAR[strlen(e.what()) + 1];
		mbstowcs(exceptionW, e.what(), strlen(e.what()) + 1);
		MessageBox(NULL, exceptionW, localizeConst(FAILED_TO_READ_EMAIL), MB_OK);
		delete exceptionW;
	}
	return 0;
}



HANDLE recvThreadMutex;
void nullMailCallback(RecvMailCodes dummy){ return; }
//parameter is a RecvThreadStruct pointer

extern HWND wndwWaiting;
extern HWND sttcWaiting;
extern HWND bttnCancelWaiting;
DWORD WINAPI recvThread(LPVOID lpParam)
{
	Static_SetText(sttcWaiting, localizeConst(WAITING_FOR_RESPONSE));
	ShowWindow(bttnCancelWaiting, SW_SHOW);

	RecvThreadStruct* ourStruct = (RecvThreadStruct*)lpParam;

	//NOTE: don't need to care about a WAIT_ABANDONED result, because we're just trying to limit this thread to one invocation at a time, not share some resource.	
	WaitForSingleObject(recvThreadMutex, INFINITE);

	ShowWindow(wndwWaiting, SW_SHOW);

	WCHAR userLoginW[300];
	WCHAR passwordW[100];
	mbstowcs(userLoginW, gUserEmailAccount, 300);
	mbstowcs(passwordW, gUserEmailPassword, 100);

	//it will never be this fast, so might as well save some hits to the server
	Sleep(5000);

	vmime_uint16 usePort = 0;  // 0 -> use default port
	const bool allowInvalidCertificate = false;
	const WCHAR* IMAPserverName;
	cCommon::eSecurity securityType = cCommon::Secur_SSL;
	if (strstr(gUserEmailAccount, "@gmail.com"))
		IMAPserverName = (WCHAR*)gmailIMAP;
	else if (strstr(gUserEmailAccount, "@yahoo.com"))
		IMAPserverName = (WCHAR*)yahooIMAP;
	else
		IMAPserverName = (WCHAR*)microsoftIMAP;
	cImap theIMAP(IMAPserverName, usePort, securityType, allowInvalidCertificate, IDR_ROOT_CA);
	theIMAP.SetAuthData(userLoginW, passwordW);

	//NOTE: we know we want "INBOX", so no need to LIST
	//i_Imap.EnumFolders(i_FolderList); // Sends the LIST command 

	__int64 hnsecs1600Start;
	bool firstWaitDone = false;
	__int64 secondsToWait = 70;
	//just in case our response somehow got buried under a (very recent) huge pile, keep letting recvMail check one more back into the future.
	int numPastEmailsToExamine = 2;
	
	WaitForMail:
	hnsecs1600Start = getHNsecsSince1600();
	while (getHNsecsSince1600() - hnsecs1600Start < secondsToWait * 1000 * 1000 * 10)
	{
		//try reading an email into buffer twice per second. only read an email
		//that matches the random string we're told to be on the lookout for, thus avoiding
		//spurious replies, e.g. to operations the user previously cancelled out of.
		//if we are told to cancel, simply exit (don't need to do a MAIL_FAILED notify; see below).
		ourStruct->charsRecvd = recvMail(ourStruct->buffer, ourStruct->bufferSize, ourStruct->randomString, theIMAP, numPastEmailsToExamine);
		numPastEmailsToExamine++;

		if (ourStruct->charsRecvd > 0)
		{
			//NOTE: release mutex before callback, because maybe the callback will also want to do a recvThread, 
			//		and we're defintiely out of the dangerous part by this point.
			theIMAP.Close();
			ReleaseMutex(recvThreadMutex);
			
			if (!cancelRecvFlag)
				ourStruct->callback(RECV_MAIL_SUCCESS);
			else
				ourStruct->callback(RECV_MAIL_CANCEL);

			cancelRecvFlag = false;

			return 0;
		}

		if (cancelRecvFlag)
		{
			cancelRecvFlag = false;
			theIMAP.Close();
			ourStruct->charsRecvd = 0;
			ReleaseMutex(recvThreadMutex);
			ourStruct->callback(RECV_MAIL_CANCEL);
			return 0;
		}
		Sleep(1000);
	}
	if (!firstWaitDone)
	{
		firstWaitDone = true;
		secondsToWait = 100;
		Static_SetText(sttcWaiting, localizeConst(WAITING_LONGER_FOR_RESPONSE));
		goto WaitForMail;
	}

	//[timeout has happened since we have reached here, so RECV_MAIL_FAIL]
	theIMAP.Close();
	ourStruct->charsRecvd = 0;
	ReleaseMutex(recvThreadMutex);
	ourStruct->callback(RECV_MAIL_FAIL);


	return 0;
}



bool sendMobileconfigs()
{
	//By default, SoftEther servers don't let clients do SMTP, which is reasonable.
	//So, if the user is connected and wants to do something that would send an email 
	//(e.g. have VPN info mailed to phone), tell them to disconnect first.
	if (gVPNConnected)
	{
		MessageBox(NULL, localizeConst(CANT_SEND_EMAIL_WHILE_CONNECTED), localizeConst(ERROR_STR), MB_OK);
		return false;
	}

	const WCHAR* credSubject = L"mobileconfig";
	const WCHAR* credBody = localizeConst(MOBILECONFIG_HOWTO);
	const WCHAR* mimeType = L"application/x-apple-aspen-config";

	WCHAR userLoginW[EMAIL_ADDR_BUFSIZE];
	WCHAR passwordW[EMAIL_PASSWORD_BUFSIZE];
	mbstowcs(userLoginW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
	mbstowcs(passwordW, gUserEmailPassword, EMAIL_PASSWORD_BUFSIZE);

	cEmailBuilder emailObjectToSend(userLoginW, credSubject, IDR_MIME_TYPES);
	emailObjectToSend.AddTo(userLoginW);
	emailObjectToSend.SetPlainText(credBody);

	//generate each mobileconfig file (one per VPN server we know of), and attach it.
	std::vector<std::wstring> attachFiles;
	for (int i = 0; i < knownServers.size(); i++)
	{
		//retrieve IP addr, PSK, and derive vpn username+pw
		WCHAR wAddr[60];
		mbstowcs(wAddr, knownServers[i].addr, 60); wAddr[59] = 0;

		WCHAR wPSK[10];
		mbstowcs(wPSK, knownServers[i].psk, 10); wPSK[9] = 0;

		//derive vpn username and pw from base password
		char pwToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
		char userNameToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
		derivePassword(pwToUse, gBaseVPNPassword, knownServers[i].addr);
		deriveUsername(userNameToUse, gBaseVPNPassword, knownServers[i].addr);
		WCHAR wPW[VPN_DERIVED_PASSWORD_LENGTH + 1];
		WCHAR wName[VPN_DERIVED_PASSWORD_LENGTH + 1];
		mbstowcs(wPW, pwToUse, VPN_DERIVED_PASSWORD_LENGTH + 1);
		mbstowcs(wName, userNameToUse, VPN_DERIVED_PASSWORD_LENGTH + 1);


		//generate the mobileconfig file and write it to a temp file

		WCHAR attachFullPath[300];
		GetTempPath(300, attachFullPath);
		wcscat(attachFullPath, wAddr);
		wcscat(attachFullPath, L".mobileconfig");

		writeMobileconfigToFile(attachFullPath, wAddr, wPSK, wName, wPW);

		emailObjectToSend.AddAttachment(attachFullPath, mimeType, L"");
		attachFiles.push_back(wstring(attachFullPath));
	}

	emailObjectToSend.Generate();

	vmime_uint16 usePort = 0; // 0 -> use default port
	cCommon::eSecurity securityType;
	const bool allowInvalidCertificate = false;
	const WCHAR* SMTPserverName;
	if (strstr(gUserEmailAccount, "@gmail.com"))
	{
		SMTPserverName = gmailSMTP;
		securityType = cCommon::Secur_SSL;
	}
	else if (strstr(gUserEmailAccount, "@yahoo.com"))
	{
		SMTPserverName = yahooSMTP;
		//usePort = 587;
		securityType = cCommon::Secur_TLS_force;
	}
	else
	{
		SMTPserverName = microsoftSMTP;
		securityType = cCommon::Secur_SSL;
	}
	cSmtp theSMTP(SMTPserverName, usePort, securityType, allowInvalidCertificate, IDR_ROOT_CA);

	theSMTP.SetAuthData(userLoginW, passwordW);
	bool success = theSMTP.Send(&emailObjectToSend);

	for (int i = 0; i<attachFiles.size(); i++)
		_wunlink(attachFiles[i].c_str());

	return success;
}
