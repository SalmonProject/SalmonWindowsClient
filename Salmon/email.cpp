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
#include <string>
using std::string;

#include "../Wrapper/cSmtp.hpp"
#include "../Wrapper/cPop3.hpp"
#include "../Wrapper/cImap.hpp"

#include "../libvmime/src/vmime/vmime.hpp"

#include "salmon_constants.h"
#include "salmon_utility.h"
#include "salmon_config.h"
#include "hwnds.h"
#include "localization.h"
#include "email.h"
#include "manual_or_blocking_email.h"
#include "connect_attempt.h"
#include "control_softether.h"

bool validateEmailAddress(const char* theAddress)
{
	//If they're doing manual email sending, just make sure it's an email-address-looking thing.
	return gManualEmail.enabled && theAddress[0] != 0 && theAddress[0] != '@' && strchr(theAddress, '@')
	//If the email sending is not manual, then make sure the address is with one of the services we support.
		|| strstr(theAddress, "@gmail.com") || strstr(theAddress, "@hotmail.com") || strstr(theAddress, "@outlook.com") || strstr(theAddress, "@yahoo.com");
}

string generateEmailRandomID()
{
	char rawRandBuf[EMAIL_RANDOM_ID_STRLEN+1];

	HCRYPTPROV cryptProvider = 0;
	CryptAcquireContext(&cryptProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(cryptProvider, EMAIL_RANDOM_ID_STRLEN, (BYTE*)rawRandBuf);
	CryptReleaseContext(cryptProvider, 0);
	for (int i = 0; i < EMAIL_RANDOM_ID_STRLEN; i++)
		rawRandBuf[i] = (char)(97 + ((unsigned char)rawRandBuf[i]) % 26);
	rawRandBuf[EMAIL_RANDOM_ID_STRLEN] = '\0';

	return string(rawRandBuf);
}

SendMailSuccess sendEmailToSalmonBackend(const std::wstring& messageToSend, const string& rndStr)
{
	using namespace vmime::wrapper;
	//By default, SoftEther servers don't let clients do SMTP, which is reasonable.
	//So, if the user is connected and wants to do something that would send an email 
	//(e.g. have VPN info mailed to phone), tell them to disconnect first.
	if (gVPNConnected)
	{
		MessageBox(NULL, localizeConst(CANT_SEND_EMAIL_WHILE_CONNECTED), localizeConst(ERROR_STR), MB_OK);
		return SEND_MAIL_FAIL;
	}

	//build the attachment
	WCHAR salmonAttachTempPath[300];
	GetTempPath(300, salmonAttachTempPath);
	wcscat(salmonAttachTempPath, L"salmonattach.txt");
	FILE* outFile = _wfopen(salmonAttachTempPath, L"wt, ccs=UTF-8");
	fwrite(messageToSend.c_str(), sizeof(WCHAR), wcslen(messageToSend.c_str()), outFile);
	fclose(outFile);


	//build the email
	WCHAR userLoginW[EMAIL_ADDR_BUFSIZE];
	WCHAR passwordW[EMAIL_PASSWORD_BUFSIZE];
	mbstowcs(userLoginW, gUserEmailAccount, EMAIL_ADDR_BUFSIZE);
	mbstowcs(passwordW, gUserEmailPassword, EMAIL_PASSWORD_BUFSIZE);

	const WCHAR* mailSubject = L"salmon backend - does not need to be seen by humans";
	cEmailBuilder emailObjectToSend(userLoginW, mailSubject, IDR_MIME_TYPES);

	emailObjectToSend.AddTo(salmonDirServAddr);

	WCHAR emailBody[200];
	wcscpy(emailBody, L"Hello, do not reply to this email. No person reads mail that this address receives.\nUnique identifier ");
	WCHAR rndStrW[51];
	mbstowcs(rndStrW, rndStr.c_str(), 51);
	rndStrW[50] = L'\0';
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

	return success ? SEND_MAIL_SUCCESS : SEND_MAIL_FAIL;
}


//NOTE because we're relying on a null terminator, this can only send unicode strings
//Sends a message to the directory server's email account, in the standard format.
//Handles the manual / automatic distinction (returns SEND_MAIL_MANUAL if it's going to be done manually).
SendMailSuccess sendMessageToDirServ(const std::wstring& messageToSend, void(*theCallback)(RecvMailCodes, string))
{
	//If they have chosen to relay the emails manually through their normal way
	//of sending email, go do that instead of the automated SMTP stuff in here.
	if (gManualEmail.enabled)
		return gManualEmail.initiateAsyncManualEmail(messageToSend, theCallback);
	else
	{
		disableAllButtonsAll();

		string randomEmailID = generateEmailRandomID();
		SendMailSuccess mailSucceeded = sendEmailToSalmonBackend(messageToSend, randomEmailID);

		if (mailSucceeded == SEND_MAIL_FAIL)
		{
			MessageBox(NULL, localizeConst(FAILED_TO_SEND_EMAIL), localizeConst(ERROR_STR), MB_OK);
			enableAllButtonsAll();
		}

		RecvThreadArguments* passArgs = new RecvThreadArguments(randomEmailID, theCallback, false);
		//new'd arguments are delete'd in thread
		CreateThread(NULL, 0, recvThread, passArgs, 0, NULL);

		return SEND_MAIL_SUCCESS;
	}
}


//It's already a little unfortunate that we're asking them for an email password...
//so rather than having a general "send mail" function, which would be creepy, 
//the program is only capable of sending to the salmon address, or to the user's own address.
bool sendSelfMail(const WCHAR* send_buf, const WCHAR* mailSubject)
{
	using namespace vmime::wrapper;
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

//Attempts to receive an email via IMAP. Discards any "salmon backend" subject mails that don't match randomString.
//Returns text of the received message - that is, the contents of the attached file attach.txt.
string emailAccountTryRecvMessage(const string& randomString, vmime::wrapper::cImap IMAPinstance, int numPastEmailsToExamine)
{
	using namespace vmime::wrapper;

	string toReturn = "";
	try
	{
		bool allDoneFlag = false;
		bool anyDeletions = false;
		int charsRead = 0;

		WCHAR randStrW[51];
		mbstowcs(randStrW, randomString.c_str(), 51);


		
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

				recvdEmail->GetAttachmentAt(0, dummyName_ref, dummyMimeType_ref, attachData_ref);
				toReturn = string(attachData);

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
			return toReturn;
	}
	catch (const std::exception& e)
	{
		WCHAR* exceptionW = new WCHAR[strlen(e.what()) + 1];
		mbstowcs(exceptionW, e.what(), strlen(e.what()) + 1);
		MessageBox(NULL, exceptionW, localizeConst(FAILED_TO_READ_EMAIL), MB_OK);
		delete exceptionW;
	}
	return toReturn;
}


HANDLE recvThreadMutex;
//Tries several times over the course of ~a minute to retrieve a specific directory server response.
DWORD waitForEmailMessage(RecvThreadArguments* ourStruct, string* optionalOutString)
{
	using namespace vmime::wrapper;

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
		string receivedMessage = emailAccountTryRecvMessage(ourStruct->randomString, theIMAP, numPastEmailsToExamine);
		numPastEmailsToExamine++;

		if (receivedMessage.length() > 0)
		{
			//NOTE: release mutex before callback, because maybe the callback will also want to do a recvThread, 
			//		and we're defintiely out of the dangerous part by this point.
			theIMAP.Close();
			ReleaseMutex(recvThreadMutex);

			if (!cancelRecvFlag)
			{
				if (optionalOutString) 
					*optionalOutString = receivedMessage;
				ourStruct->callback(RECV_MAIL_SUCCESS, receivedMessage);
			}
			else
			{
				if (optionalOutString)
					*optionalOutString = "";
				ourStruct->callback(RECV_MAIL_CANCEL, "");
			}
			enableAllButtonsAll();

			cancelRecvFlag = false;

			return 0;
		}

		if (cancelRecvFlag)
		{
			cancelRecvFlag = false;
			theIMAP.Close();
			ReleaseMutex(recvThreadMutex);
			if (optionalOutString)
				*optionalOutString = "";
			ourStruct->callback(RECV_MAIL_CANCEL, "");
			enableAllButtonsAll();
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
	ReleaseMutex(recvThreadMutex);
	if (optionalOutString)
		*optionalOutString = "";
	ourStruct->callback(RECV_MAIL_FAIL, "");
	enableAllButtonsAll();


	return 0;
}

//If optionalOutString is not null, we should store the server's response text in
//it before calling the callback. This is only used by the blocking version of
//the email communication, which is why the recvThread() function below always
//passes 0 for this parameter.
DWORD recvThreadFunction(RecvThreadArguments* theArgs, string* optionalOutString)
{
	Static_SetText(sttcWaiting, localizeConst(WAITING_FOR_RESPONSE));
	ShowWindow(bttnCancelWaiting, SW_SHOW);

	RecvThreadArguments ourStruct = *theArgs;
	delete theArgs;

	if (ourStruct.sendingIsManual == false)
		return waitForEmailMessage(&ourStruct, optionalOutString);
	else if (ourStruct.sendingIsManual == true)
	{
		//nothing to be done! callback is called by the manual send/recv window's completion button.
		return 0;
	}
	return 0;
}

//parameter is a RecvThreadStruct pointer
DWORD WINAPI recvThread(LPVOID lpParam)
{
	return recvThreadFunction((RecvThreadArguments*)lpParam, 0);
}


std::wstring constructVPNListEmailForAndroid()
{
	std::wstring credBody(localizeConst(ANDROID_VPNLIST_HEADER));
	//credBody should look like:
	//=====================================
	//[Localized, explanatory header blurb]
	//Server: 1.2.3.4
	//Pre-shared key (PSK): abcdef
	//Username: abcdefghikjlsdfj
	//Password: abcdfeghtifdjkds
	//
	//Server: 111.211.113.114
	//Pre-shared key (PSK): abcdef
	//Username: bbbbbbbbbbbbbbbbbb
	//Password: ccccccccccccccccc
	//
	//Server: 5.42.23.24
	//Pre-shared key (PSK): abcdef
	//Username: eeeeeeeeeeeeeeee
	//Password: ffffffffffffffff
	//
	//[etc.]
	//=====================================
	for (int i = 0; i < gKnownServers.size(); i++)
	{
		WCHAR wAddr[60];
		mbstowcs(wAddr, gKnownServers[i].addr, 60); wAddr[59] = 0;
		credBody += L"Server: ";
		credBody += wAddr;

		WCHAR wPSK[10];
		mbstowcs(wPSK, gKnownServers[i].psk, 10); wPSK[9] = 0;
		credBody += L"\nPre-shared key (PSK): ";
		credBody += wPSK;

		//derive username and pw
		char pwToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
		char userNameToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
		derivePassword(pwToUse, gBaseVPNPassword, gKnownServers[i].addr);
		deriveUsername(userNameToUse, gBaseVPNPassword, gKnownServers[i].addr);
		WCHAR wPW[VPN_DERIVED_PASSWORD_LENGTH + 1];
		WCHAR wName[VPN_DERIVED_PASSWORD_LENGTH + 1];
		mbstowcs(wPW, pwToUse, VPN_DERIVED_PASSWORD_LENGTH + 1);
		mbstowcs(wName, userNameToUse, VPN_DERIVED_PASSWORD_LENGTH + 1);

		credBody += L"\nUsername: ";
		credBody += wName;
		credBody += L"\nPassword: ";
		credBody += wPW;
		credBody += L"\n\n";
	}
	return credBody;
}

bool sendMobileconfigs()
{
	using namespace vmime::wrapper;
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
	vector<wstring> attachFiles;
	for (int i = 0; i < gKnownServers.size(); i++)
	{
		//retrieve IP addr, PSK, and derive vpn username+pw
		WCHAR wAddr[60];
		mbstowcs(wAddr, gKnownServers[i].addr, 60); wAddr[59] = 0;

		WCHAR wPSK[10];
		mbstowcs(wPSK, gKnownServers[i].psk, 10); wPSK[9] = 0;

		//derive vpn username and pw from base password
		char pwToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
		char userNameToUse[VPN_DERIVED_PASSWORD_LENGTH + 1];
		derivePassword(pwToUse, gBaseVPNPassword, gKnownServers[i].addr);
		deriveUsername(userNameToUse, gBaseVPNPassword, gKnownServers[i].addr);
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
