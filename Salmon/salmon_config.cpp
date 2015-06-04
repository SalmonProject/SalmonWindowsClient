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

//#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <windowsx.h>
#include <process.h>
#include <set>
using std::set;
using std::string;
using std::vector;

#include "../Wrapper/cSmtp.hpp"
#include "../Wrapper/cPop3.hpp"
#include "../Wrapper/cImap.hpp"

#include "salmon_constants.h"
#include "salmon_globals.h"
#include "hwnds.h"

#include "localization.h"
#include "control_softether.h"
#include "salmon_utility.h"
#include "email.h"
#include "salmon_config.h"

bool operator<(const VPNInfo& a, const VPNInfo& b)
{
	return a.score < b.score;
}

//the normal sorting doesn't take consecutive failures into account. this is fine, because
//the connection attempt logic has other mechanisms for not constantly retrying the likely-failed
//ones. however, for sending the email to an android user, we want all of the topmost entries to
//be ones that were online last time we checked.
bool compWithFailure(const VPNInfo& a, const VPNInfo& b)
{
	return a.score - 10000 * a.failureCount < b.score - b.failureCount * 10000;
}


void writeSConfigFromKnownServers()
{
	WaitForSingleObject(gServerInfoMutex, INFINITE);
	sort(knownServers.begin(), knownServers.end());
	FILE* SConfig = openConfigFile("SConfig.txt", "wt");

	for (int i = 0; i < knownServers.size(); i++)
	{
		knownServers[i].score = knownServers[i].bandwidth - knownServers[i].rtt;
		VPNInfo curServer = knownServers[i];
		//write the IP addr
		if (curServer.vpnGate)
			fwrite("VPNGATE", 1, strlen("VPNGATE"), SConfig);
		fwrite(curServer.addr, 1, strlen(curServer.addr), SConfig);
		fwrite("\n", 1, 1, SConfig);

		//write the score
		string s = to_string(curServer.score);
		s += "\n";
		fwrite(s.c_str(), 1, s.length(), SConfig);
		s.clear();

		//write the rtt
		s = to_string(curServer.rtt);
		s += "\n";
		fwrite(s.c_str(), 1, s.length(), SConfig);
		s.clear();

		//write the bandwidth
		s = to_string(curServer.bandwidth);
		s += "\n";
		fwrite(s.c_str(), 1, s.length(), SConfig);
		s.clear();

		//write the lastAttempt
		s = to_string(curServer.lastAttempt);
		s += "\n";
		fwrite(s.c_str(), 1, s.length(), SConfig);
		s.clear();

		//write the failure
		s = to_string(curServer.failureCount);
		s += "\n";
		fwrite(s.c_str(), 1, s.length(), SConfig);
		s.clear();

		//write the psk
		fwrite(curServer.psk, 1, strlen(curServer.psk), SConfig);
		fputc('\n', SConfig);
	}
	fclose(SConfig);
	ReleaseMutex(gServerInfoMutex);
}

void wipeConfig()
{
	unlinkConfigFile("SConfig.txt");
	unlinkConfigFile("SalmonConfig.txt");
	unlinkConfigFile("CurrentEmail.txt");
	unlinkConfigFile("TrustDaysEstimate.txt");
	unlinkConfigFile("locale.txt");
	unlinkConfigFile("previousaddr.txt");



	//ah... now with the VPN Gate fallback, there can be .pem files that we don't remember writing.
	//so, instead of going by knownServers, we have to actually search the directory.
	WCHAR WwholeThing[300];
	char wholeThing[300];

	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\*.pem");
	mbstowcs(WwholeThing, wholeThing, 300);

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	hFind = FindFirstFile(WwholeThing, &ffd);

	char tempASCII[MAX_PATH];
	char toUnlink[350];
	do
	{
		wcstombs(tempASCII, ffd.cFileName, MAX_PATH);
		strcpy(toUnlink, getenv("APPDATA"));
		strcat(toUnlink, "\\salmon\\");
		strcat(toUnlink, tempASCII);
		unlink(toUnlink);
	} while (FindNextFile(hFind, &ffd) != 0);


	//RemoveDirectory will fail if the directory isn't empty... which is correct behavior:
	//if the user put some alien files in there, we should leave them (and therefore the directory) alone.
	strcpy(wholeThing, getenv("APPDATA"));
	WCHAR wholeThingW[300];
	mbstowcs(wholeThingW, wholeThing, 300);
	wcscat(wholeThingW, L"\\salmon");
	RemoveDirectory(wholeThingW);
}


bool generate_S_from_SalmonConfig();
bool readExistingSConfig();
//return if SConfig.txt was either successfully generated, or already existed and was successfully read.
//base on the salmon_config file from the dir_server
//first, sanity check: open salmon_config see if that exist
//second, extract the info out of the file: gBaseVPNPassword, trustlevel, last_rec_time, server IP list along with Bandwidth score
//third, insert them into the priority queue along with computed RTT and the overall score, set Last Attemp to epoch time 0, consercutive failure 0
//fourth, write to the SConfig file
bool getSConfig()
{
	knownServers.clear();

	//NOTE don't use openConfigFile here
	char wholeThing[300];
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\SConfig.txt");
	FILE* SConfig = fopen(wholeThing, "rt");

	unsigned int scFlen = 0;
	if (SConfig)
	{
		fseek(SConfig, 0, SEEK_END);
		scFlen = ftell(SConfig);
		rewind(SConfig);
		fclose(SConfig);
	}

	if (scFlen == 0)
		return generate_S_from_SalmonConfig();
	else
		return readExistingSConfig();
}

//quick little format documentation for SalmonConfig.txt:
//gBaseVPNPassword
//trust
//seconds since epoch: last asked for a rec code
//[ipaddr bw]
//[ipaddr bw]
//...
bool generate_S_from_SalmonConfig()
{
	char wholeThing[300];
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\SalmonConfig.txt");
	WCHAR wholeThingW[300];
	mbstowcs(wholeThingW, wholeThing, 300);
	if(!fileExists(wholeThingW))
		return false;

	ifstream lineStream(wholeThing);

	char Salmon_config_buffer[CONFIG_BUF_SIZE];
	memset(Salmon_config_buffer, 0, CONFIG_BUF_SIZE);
	lineStream.getline(Salmon_config_buffer, CONFIG_BUF_SIZE);
	if (lineIsJustWhitespace(Salmon_config_buffer) || strlen(Salmon_config_buffer) < 10)
	{
		lineStream.close();
		unlink(wholeThing);
		return false;
	}
	memcpy(gBaseVPNPassword, Salmon_config_buffer, VPN_BASE_PASSWORD_LENGTH);
	gBaseVPNPassword[VPN_BASE_PASSWORD_LENGTH] = 0;

	memset(Salmon_config_buffer, 0, CONFIG_BUF_SIZE);
	lineStream.getline(Salmon_config_buffer, CONFIG_BUF_SIZE);
	if (lineIsJustWhitespace(Salmon_config_buffer))
	{
		lineStream.close();
		unlink(wholeThing);
		return false;
	}
	//trust level: atoi(Salmon_config_buffer)). However, since we always just get it from the file,
	//we can just leave it there! Don't need to do anything with it here.

	memset(Salmon_config_buffer, 0, CONFIG_BUF_SIZE);
	lineStream.getline(Salmon_config_buffer, CONFIG_BUF_SIZE);
	if (lineIsJustWhitespace(Salmon_config_buffer))
	{
		lineStream.close();
		unlink(wholeThing);
		return false;
	}

	//at this point Salmon_config_buffer has recTime, but we don't currently need it from here

	//now read all the server lines. format: ipaddr offeredbw psk
	memset(Salmon_config_buffer, 0, CONFIG_BUF_SIZE);
	while (lineStream.getline(Salmon_config_buffer, CONFIG_BUF_SIZE))
	{
		if (lineIsJustWhitespace(Salmon_config_buffer))
			break;

		if (strchr(Salmon_config_buffer, ' ') == NULL || strchr(strchr(Salmon_config_buffer, ' ') + 1, ' ') == NULL)
			return false;

		char bwBuf[CONFIG_BUF_SIZE];
		strcpy(bwBuf, strchr(Salmon_config_buffer, ' ') + 1);
		*strchr(bwBuf, ' ') = 0;
		int new_ip_bandwidth = atoi(bwBuf);

		char new_ip_psk[10];
		strncpy(new_ip_psk, strchr(strchr(Salmon_config_buffer, ' ') + 1, ' ') + 1, 10); new_ip_psk[9] = 0;

		*strchr(Salmon_config_buffer, ' ') = 0;
		char new_ip_addr[60];
		strncpy(new_ip_addr, Salmon_config_buffer, 60); new_ip_addr[59] = 0;

		//HACK just set the rtt to 50, because pings only (reliably) get through after we have connected to the server.
		VPNInfo new_ip(new_ip_addr, new_ip_bandwidth, 50, new_ip_bandwidth - 50, 0, new_ip_psk);
		new_ip.lastAttempt = 0;
		new_ip.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);
		knownServers.push_back(new_ip);
	}
	lineStream.close();

	writeSConfigFromKnownServers();

	for (int i = 0; i < knownServers.size(); i++)
		createConnectionSetting(knownServers[i].addr);

	return true;
}

bool readExistingSConfig()
{
	char wholeThing[300];
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\SConfig.txt");
	ifstream Sconfig_lineStream(wholeThing);

	char SConfig_buffer[200];
	memset(SConfig_buffer, 0, 200);

	set<string> addrsSeen;
	while (Sconfig_lineStream.getline(SConfig_buffer, 200))
	{
		if (lineIsJustWhitespace(SConfig_buffer))
			break;

		//(IP addr was read to verify whether this loop iteration should happen)
		char holder_addr[60];
		strncpy(holder_addr, SConfig_buffer, 60); holder_addr[59] = 0;
		
		//read the score
		memset(SConfig_buffer, 0, 200);
		Sconfig_lineStream.getline(SConfig_buffer, 200);
		if (lineIsJustWhitespace(SConfig_buffer))
			break;
		int holder_score = atoi(SConfig_buffer);

		//read the rtt
		memset(SConfig_buffer, 0, 200);
		Sconfig_lineStream.getline(SConfig_buffer, 200);
		if (lineIsJustWhitespace(SConfig_buffer))
			break;
		int holder_rtt = atoi(SConfig_buffer);

		//read the bandwidth
		memset(SConfig_buffer, 0, 200);
		Sconfig_lineStream.getline(SConfig_buffer, 200);
		if (lineIsJustWhitespace(SConfig_buffer))
			break;
		int holder_bandwidth = atoi(SConfig_buffer);

		//read the lastAttempt
		memset(SConfig_buffer, 0, 200);
		Sconfig_lineStream.getline(SConfig_buffer, 200);
		if (lineIsJustWhitespace(SConfig_buffer))
			break;
		time_t holder_lastAttempt = atoi(SConfig_buffer);

		//read the failureCount
		memset(SConfig_buffer, 0, 200);
		Sconfig_lineStream.getline(SConfig_buffer, 200);
		if (lineIsJustWhitespace(SConfig_buffer))
			break;
		int holder_failureCount = atoi(SConfig_buffer);

		//read the psk
		memset(SConfig_buffer, 0, 200);
		Sconfig_lineStream.getline(SConfig_buffer, 200);
		if (lineIsJustWhitespace(SConfig_buffer))
			break;
		char holder_psk[10];
		strncpy(holder_psk, SConfig_buffer, 10); holder_psk[9] = 0;

		VPNInfo holder(holder_addr, holder_bandwidth, holder_rtt, holder_score, holder_failureCount, holder_psk);
		holder.lastAttempt = holder_lastAttempt;
		holder.secondsTilNextAttempt = 2 * 24 * 3600 + rand() % (5 * 24 * 3600);

		//prevent duplicates
		string curIP = string(holder.addr);
		if (addrsSeen.count(curIP) == 0)
		{
			addrsSeen.insert(curIP);
			knownServers.push_back(holder);
		}
	}

	Sconfig_lineStream.close();

	//and finally, read gBaseVPNPassword, because that was originally being done in the half of the code that generated an SConfig when there was none.
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon\\SalmonConfig.txt");
	WCHAR wholeThingW[300];
	mbstowcs(wholeThingW, wholeThing, 300);
	if (!fileExists(wholeThingW))
		return false;

	ifstream lineStream(wholeThing);

	char readLineBuf[200];
	lineStream.getline(readLineBuf, 200);
	if (lineIsJustWhitespace(readLineBuf))
	{
		lineStream.close();
		return false;
	}
	memcpy(gBaseVPNPassword, readLineBuf, VPN_BASE_PASSWORD_LENGTH);
	gBaseVPNPassword[VPN_BASE_PASSWORD_LENGTH] = 0;
	lineStream.close();

	if (!knownServers.empty())
		return true;
	else
		return false;
}

//remove any server entry from sconfig and salmonconfig that has IP address purgeThisIP
void purgeServer(char* purgeThisIP)
{
	//first, check that this is actually a purge line.
	if (strstr(purgeThisIP, "PURGE") != purgeThisIP)
		return;

	//purgeThisIP can come in with a trailing \n, which will of course mess up strstr
	char strippedIP[300];
	strcpy(strippedIP, purgeThisIP+strlen("PURGE"));
	
	//NOTE NOTE HACK: ok i don't get it. the following two lines are NOT removing the newline...
	//and strtok shouldn't be leaving newlines on these strings anyways! the really bizarre thing
	//is that i can definitely be like "oh if [the whole string] has a '\n' in it, proceed to the
	//purgeServer logic" which would seem to be affected by the unix/windows newline issue, if that
	//were really the problem here. (strchr vs strstr makes no difference).
	//if (strstr(strippedIP, "\n"))
	//	*strstr(strippedIP, "\n") = 0;
	char* curChar = strippedIP;
	do
	{
		if (!(*curChar == '.' || *curChar == '-' || *curChar >= '0' && *curChar <= '9' || *curChar >= 'a' && *curChar <= 'z' || *curChar >= 'A' && *curChar <= 'Z'))
		{
			*curChar = 0;
			break;
		}
		curChar++;
	} while (*curChar);

	//SConfig.txt comes in blocks of 7 lines. so, if you see the ip address, skip that line and the next 6 lines.
	FILE* theFile = openConfigFile("SConfig.txt", "rt");

	if (!theFile)
		return;

	fseek(theFile, 0, SEEK_END);
	long theFileLen = ftell(theFile);
	rewind(theFile);

	if (theFileLen == 0)
	{
		fclose(theFile);
		return;
	}

	char* wholeFileBuf = (char*)malloc(theFileLen + 1);
	memset(wholeFileBuf, 0, theFileLen + 1);
	fread(wholeFileBuf, 1, theFileLen, theFile);
	fclose(theFile);
	wholeFileBuf[theFileLen] = 0;

	theFile = openConfigFile("SConfig.txt", "wt");
	char* curLine = strtok(wholeFileBuf, "\n");

	do
	{
		if (!strstr(curLine, strippedIP))
		{
			fwrite(curLine, 1, strlen(curLine), theFile);
			fputc('\n', theFile);
		}
		else //SConfig.txt comes in blocks of 7 lines. so, if you see the ip address, skip that line and the next 6 lines.
			for (int skipLines = 0; skipLines < 6 && curLine; skipLines++)
				curLine = strtok(0, "\n");

	} while ((curLine = strtok(0, "\n")));

	fclose(theFile);
	free(wholeFileBuf);

	//==========================================================================

	//SalmonConfig.txt can be done by just doing "skip lines that contain the ip address".
	theFile = openConfigFile("SalmonConfig.txt", "rt");

	if (!theFile)
		return;

	fseek(theFile, 0, SEEK_END);
	theFileLen = ftell(theFile);
	rewind(theFile);

	if (theFileLen == 0)
	{
		fclose(theFile);
		return;
	}

	wholeFileBuf = (char*)malloc(theFileLen + 1);
	memset(wholeFileBuf, 0, theFileLen + 1);
	fread(wholeFileBuf, 1, theFileLen, theFile);
	fclose(theFile);
	wholeFileBuf[theFileLen] = 0;

	theFile = openConfigFile("SalmonConfig.txt", "wt");
	curLine = strtok(wholeFileBuf, "\n");

	do
	{
		if (!strstr(curLine, strippedIP))
		{
			fwrite(curLine, 1, strlen(curLine), theFile);
			fwrite("\n", 1, 1, theFile);
		}
	} while ((curLine = strtok(0, "\n")) && *curLine);

	fclose(theFile);
	free(wholeFileBuf);
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

void loadEmailIfStored()
{
	memset(gUserEmailAccount, 0, 300);

	FILE* readEmailSetting = openConfigFile("CurrentEmail.txt", "rt");
	if (!readEmailSetting)
	{
		Edit_SetText(textLoginEmailAddr, L"");
		return;
	}
	fgets(gUserEmailAccount, 300, readEmailSetting);
	if (strchr(gUserEmailAccount, '\n'))
		*strchr(gUserEmailAccount, '\n') = 0;

	WCHAR emailW[300];
	mbstowcs(emailW, gUserEmailAccount, 300);
	Edit_SetText(textLoginEmailAddr, emailW);
	fclose(readEmailSetting);
}

void ensureSalmonConfigDir()
{
	char wholeThing[300];
	strcpy(wholeThing, getenv("APPDATA"));
	strcat(wholeThing, "\\salmon");
	WCHAR asWstr[300];
	mbstowcs(asWstr, wholeThing, 300);
	CreateDirectory(asWstr, NULL);
}

void startTrustCountdown()
{
	FILE* trustFile = openConfigFile("TrustDaysEstimate.txt", "wt");
	char sseTime[100];
	time_t timeVal = time(0);
	sprintf(sseTime, "%u", timeVal);
	fwrite(sseTime, 1, strlen(sseTime), trustFile);
	fclose(trustFile);
}

void checkTrustCountdown()
{
	FILE* trustFile = openConfigFile("TrustDaysEstimate.txt", "rt");
	if (!trustFile)
		return;

	char sseTime[40];
	int numRead = fread(sseTime, 1, 40, trustFile);
	sseTime[numRead] = 0;
	fclose(trustFile);

	time_t oldTimeVal;
	sscanf(sseTime, "%u", &oldTimeVal);

	time_t curTimeVal = time(0);

	//at least 128 days have passed
	if (curTimeVal - oldTimeVal > 128 * 24 * 60 * 60)
	{
		trustLevelDisplayAndWriteFile(6);

		//all done with this!
		unlinkConfigFile("TrustDaysEstimate.txt");
	}
	//at least 14 days have passed
	else if (curTimeVal - oldTimeVal > 14 * 24 * 60 * 60)
		trustLevelDisplayAndWriteFile(5);
}

void writeTrustToBlankFile(int trust)
{
	FILE* writeSalmonConfig = openConfigFile("SalmonConfig.txt", "wt");
	char toWrite[40];
	sprintf(toWrite, "BLANK\n%d\n0\n", trust);
	fwrite(toWrite, 1, strlen(toWrite), writeSalmonConfig);
	fclose(writeSalmonConfig);
}

void writeTrustAndVPNToBlankFile(int trust, char* theVPNpw)
{
	FILE* writeSalmonConfig = openConfigFile("SalmonConfig.txt", "wt");
	char toWrite[40];
	sprintf(toWrite, "%s\n%d\n0\n", theVPNpw, trust);
	fwrite(toWrite, 1, strlen(toWrite), writeSalmonConfig);
	fclose(writeSalmonConfig);
}

//write into the config file's trust value
//(the second line of salmonconfig.txt is trust, so just overwrite that one)
void writeTrustToFile(int trust)
{
	FILE* readSalmonConfig = openConfigFile("SalmonConfig.txt", "rt");
	if (!readSalmonConfig)
	{
		writeTrustToBlankFile(trust);
		return;
	}

	fseek(readSalmonConfig, 0, SEEK_END);
	long fileLen = ftell(readSalmonConfig);
	rewind(readSalmonConfig);

	if (fileLen < VPN_BASE_PASSWORD_LENGTH)
	{
		fclose(readSalmonConfig);
		writeTrustToBlankFile(trust);
		return;
	}

	char* wholeFileBuf = (char*)malloc(fileLen + 1);
	int charsRead = fread(wholeFileBuf, 1, fileLen, readSalmonConfig);
	fclose(readSalmonConfig);
	wholeFileBuf[charsRead] = 0;
	
	//advance past the first two lines (which should be pw and then trust).
	//if two+ lines aren't found, just write to the file as if it was from scratch. (with the pw that was read, though.)
	char* afterTrust = strstr(wholeFileBuf, "\n");
	if (afterTrust == NULL || NULL == (afterTrust = strstr(afterTrust + 1, "\n")))
	{
		char vpnPW[VPN_BASE_PASSWORD_LENGTH + 1];
		memcpy(vpnPW, wholeFileBuf, VPN_BASE_PASSWORD_LENGTH);
		vpnPW[VPN_BASE_PASSWORD_LENGTH] = 0;
		writeTrustAndVPNToBlankFile(trust, vpnPW);
		free(wholeFileBuf);
		return;
	}
	afterTrust++;

	FILE* writeSalmonConfig = openConfigFile("SalmonConfig.txt", "wt");

	fwrite(wholeFileBuf, 1, VPN_BASE_PASSWORD_LENGTH, writeSalmonConfig);
	fputc('\n', writeSalmonConfig);
	char trustStr[30];
	sprintf(trustStr, "%d\n", trust);
	fwrite(trustStr, 1, strlen(trustStr), writeSalmonConfig);
	fwrite(afterTrust, 1, strlen(afterTrust), writeSalmonConfig);


	fclose(writeSalmonConfig);
	free(wholeFileBuf);
}

int readTrustFromFile()
{
	FILE* readSalmonConfig = openConfigFile("SalmonConfig.txt", "rt");
	if (!readSalmonConfig)
		return 1;

	fseek(readSalmonConfig, 0, SEEK_END);
	long fileLen = ftell(readSalmonConfig);
	rewind(readSalmonConfig);

	if (fileLen < VPN_BASE_PASSWORD_LENGTH)
	{
		fclose(readSalmonConfig);
		return 1;
	}

	char* wholeFileBuf = (char*)malloc(fileLen + 1);
	int charsRead = fread(wholeFileBuf, 1, fileLen, readSalmonConfig);
	fclose(readSalmonConfig);
	wholeFileBuf[charsRead] = 0;

	char* curLine = strtok(wholeFileBuf, "\n");
	curLine = strtok(0, "\n");
	if (!curLine)
	{
		free(wholeFileBuf);
		return 1;
	}
	int toReturn = atoi(curLine);
	free(wholeFileBuf);
	return toReturn;
}

