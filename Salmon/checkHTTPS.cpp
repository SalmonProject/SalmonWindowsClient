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

#include "../libvmime/src/vmime/platforms/windows/windowsHandler.hpp"
#include "../libvmime/src/vmime/types.hpp"
#include "../libvmime/src/vmime/net/tls/openssl/TLSSession_OpenSSL.hpp"
#include "../libvmime/src/vmime/security/cert/X509Certificate.hpp"
#include "../libvmime/src/vmime/security/cert/defaultCertificateVerifier.hpp"

#include "salmon_utility.h"

class Salmon_timeoutHandler : public vmime::net::timeoutHandler
{
	__int64 startTime; //in hnsecs since jan 1, 1601
	int curInterval;

	__int64 getHNSecsNow()
	{
		SYSTEMTIME temp;
		GetLocalTime(&temp);
		FILETIME ftemp;
		SystemTimeToFileTime(&temp, &ftemp);
		ULARGE_INTEGER ti;
		ti.LowPart = ftemp.dwLowDateTime;
		ti.HighPart = ftemp.dwHighDateTime;
		return ti.QuadPart;
	}

public:
	Salmon_timeoutHandler()
	{
		startTime = getHNSecsNow();
		curInterval = 2;
	}
	~Salmon_timeoutHandler() {}

	bool isTimeOut()
	{
		__int64 curTime = getHNSecsNow();
		__int64 diff = curInterval * 10000000;

		return startTime + diff < curTime;
	}

	void resetTimeOut()
	{
		startTime = getHNSecsNow();
	}

	bool handleTimeOut()
	{
		return false; //cancels operation
	}

	void ModifyInterval(int newto)
	{
		curInterval = newto;
	}
};

//hacky verifier for when you know exactly what cert you should be getting (but with a CN that probably
//won't correspond to their domain name or ip address)
class singleCertVerifier : public vmime::security::cert::defaultCertificateVerifier
{
	std::vector <vmime::ref <vmime::security::cert::X509Certificate> > myX509TrustedCerts;
public:

	void setX509TrustedCerts(const std::vector <vmime::ref <vmime::security::cert::X509Certificate> >& trustedCerts)
	{
		myX509TrustedCerts = trustedCerts;
	}

	void verify(vmime::ref <vmime::security::cert::certificateChain> chain, const vmime::string& hostname)
	{
		const vmime::datetime now = vmime::datetime::now();

		vmime::ref <vmime::security::cert::X509Certificate> onlyCert =
			chain->getAt(0).dynamicCast <vmime::security::cert::X509Certificate>();

		const vmime::datetime begin = onlyCert->getActivationDate();
		const vmime::datetime end = onlyCert->getExpirationDate();

		if (now < begin || now > end)
		{
			MessageBoxA(NULL, "Server presented an expired or not yet valid certificate.", "Certificate error", MB_OK);
			throw std::exception("Server presented an expired or not yet valid certificate.");
		}

		for (unsigned int i = 0; i < myX509TrustedCerts.size(); i++)
		{
			vmime::ref < vmime::security::cert::X509Certificate> cert = myX509TrustedCerts[i];

			if (onlyCert->equals(cert))
				return;
		}

		throw std::exception("Server presented a certificate we don't trust.");
	}
};

bool checkHTTPS(const char* serverIP_Addr)
{
	//i wish vmime would just use polarssl... but i'm not going to compile two separate libraries in.
	//and now i have spent the time to figure out how to use vmime's tls wrapper thing, anyways.

	char* certData;
	size_t certLen;

	char certName[300];
	strcpy(certName, serverIP_Addr);
	strcat(certName, ".pem");
	FILE* readCert = openConfigFile(certName, "rb");
	if (!readCert)
		return false;
	fseek(readCert, 0, SEEK_END);
	certLen = ftell(readCert);
	rewind(readCert);
	certData = (char*)malloc(certLen + 1);
	fread(certData, 1, certLen, readCert);
	certData[certLen] = 0;
	fclose(readCert);

	vmime::ref <vmime::security::cert::X509Certificate> curServCert = vmime::security::cert::X509Certificate::import((const vmime::byte_t*)certData, certLen);
	std::vector < vmime::ref<vmime::security::cert::X509Certificate> > certSingleton;
	certSingleton.push_back(curServCert);
	vmime::ref < singleCertVerifier > theVerifier = vmime::create <  singleCertVerifier >();
	theVerifier->setX509TrustedCerts(certSingleton);

	vmime::ref < vmime::net::tls::TLSProperties > tlsProps = vmime::create <vmime::net::tls::TLSProperties>();
	tlsProps->setCipherSuite(vmime::net::tls::TLSProperties::CIPHERSUITE_DEFAULT);

	vmime::ref< vmime::net::tls::TLSSession > tls = vmime::net::tls::TLSSession_OpenSSL::create(theVerifier, tlsProps);

	vmime::ref < vmime::net::timeoutHandler > timeouter = vmime::create < Salmon_timeoutHandler >();
	timeouter->ModifyInterval(2);

	vmime::ref < vmime::net::socket > theSocket = vmime::platform::getHandler()->getSocketFactory()->create(timeouter);
	vmime::ref < vmime::net::tls::TLSSocket > tlsSocket = tls->getSocket(theSocket);

	vmime::string vmimeAddr(serverIP_Addr);

	try{
		tlsSocket->connect(vmimeAddr, (vmime::port_t)443);
	}
	catch (std::exception e){}// MessageBoxA(NULL, e.what(), "Exception", MB_OK);

	if (!tlsSocket->isConnected())
	{
		free(certData);
		return false;
	}

	std::stringstream ss;
	ss << "GET /index.html HTTP/1.1\r\nHost: " << serverIP_Addr <<
"\r\nUser-Agent: Mozilla/5.0 (Windows NT 6.3; rv:36.0) Gecko/20100101 Firefox/36.0\r\n\
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n\
Accept-Language: en-US,en;q=0.5\r\n\
Accept-Encoding: gzip, deflate\r\n\r\n";

	vmime::string theRequest(ss.str());

	try
	{
		tlsSocket->send(theRequest);

		char recvBuf[1000];
		memset(recvBuf, 0, 1000);
		tlsSocket->receiveRaw(recvBuf, 999);
		recvBuf[999] = 0;
		tlsSocket->disconnect();

		free(certData);

		if (strstr(recvBuf, "404"))
			return true;
		else
			return false;
	}
	catch (std::exception e)
	{
		return false;
	}
}
