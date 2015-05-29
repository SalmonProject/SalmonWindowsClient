//
// VMime library (http://www.vmime.org)
// Copyright (C) 2002-2013 Vincent Richard <vincent@vmime.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 3 of
// the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// Linking this library statically or dynamically with other modules is making
// a combined work based on this library.  Thus, the terms and conditions of
// the GNU General Public License cover the whole combination.
//

#include "../vmime/config.hpp"


#if VMIME_PLATFORM_IS_WINDOWS


#include "../vmime/platforms/windows/windowsHandler.hpp"

#include "../vmime/platforms/windows/windowsCriticalSection.hpp"

#include "../vmime/utility/stringUtils.hpp"

#define NOMINMAX
#include <time.h>
#include <locale.h>
#include <process.h>
#include <winsock2.h> // for WSAStartup()
#include <windows.h>  // for winnls.h
#include <ws2tcpip.h>
#include <wincrypt.h>

#if VMIME_HAVE_MLANG
#   include <mlang.h>
#endif


namespace vmime {
namespace platforms {
namespace windows {


windowsHandler::windowsHandler()
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);

#if VMIME_HAVE_MESSAGING_FEATURES
	m_socketFactory = vmime::create <windowsSocketFactory>();
#endif
#if VMIME_HAVE_FILESYSTEM_FEATURES
	m_fileSysFactory = vmime::create <windowsFileSystemFactory>();
#endif
}


windowsHandler::~windowsHandler()
{
	WSACleanup();
}


unsigned long windowsHandler::getUnixTime() const
{
	return static_cast <unsigned long>(::time(NULL));
}


const vmime::datetime windowsHandler::getCurrentLocalTime() const
{
	const time_t t(::time(NULL));

	// Get the local time
#if VMIME_HAVE_LOCALTIME_S
	tm local;
	::localtime_s(&local, &t);
#elif VMIME_HAVE_LOCALTIME_R
	tm local;
	::localtime_r(&t, &local);
#else
	tm local = *::localtime(&t);  // WARNING: this is not thread-safe!
#endif

	// Get the UTC time
#if VMIME_HAVE_GMTIME_S
	tm gmt;
	::gmtime_s(&gmt, &t);
#elif VMIME_HAVE_GMTIME_R
	tm gmt;
	::gmtime_r(&t, &gmt);
#else
	tm gmt = *::gmtime(&t);  // WARNING: this is not thread-safe!
#endif

	// "A negative value for tm_isdst causes mktime() to attempt
	//  to determine whether Daylight Saving Time is in effect
	//  for the specified time."
	local.tm_isdst = -1;
	gmt.tm_isdst = -1;

	// Calculate the difference (in seconds)
	const int diff = (const int)(::mktime(&local) - ::mktime(&gmt));

	// Return the date
	return vmime::datetime(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday,
					local.tm_hour, local.tm_min, local.tm_sec, diff / 60);  // minutes needed
}

// FIX by Elmue: Modified
// All strings passed to the vmime library must be UTF-8 strings!
const vmime::charset windowsHandler::getLocalCharset() const
{
    return vmime::charsets::UTF_8;
}


static inline bool isFQDN(const vmime::string& str)
{
	if (utility::stringUtils::isStringEqualNoCase(str, "localhost", 9))
		return false;

	const vmime::string::size_type p = str.find_first_of(".");
	return p != vmime::string::npos && p > 0 && p != str.length() - 1;
}


const vmime::string windowsHandler::getHostName() const
{
	char hostname[256];

	// Try with 'gethostname'
	::gethostname(hostname, sizeof(hostname));
	hostname[sizeof(hostname) - 1] = '\0';

	// If this is a Fully-Qualified Domain Name (FQDN), return immediately
	if (isFQDN(hostname))
		return hostname;

	if (::strlen(hostname) == 0)
	{
#if VMIME_HAVE_STRCPY_S
		::strcpy_s(hostname, "localhost");
#else
		::strcpy(hostname, "localhost");
#endif // VMIME_HAVE_STRCPY_S
	}

	// Try to get canonical name for the hostname
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;  // either IPV4 or IPV6
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_CANONNAME;

	struct addrinfo* info;

	if (getaddrinfo(hostname, "http", &hints, &info) == 0)
	{
		for (struct addrinfo* p = info ; p != NULL ; p = p->ai_next)
		{
			if (p->ai_canonname && isFQDN(p->ai_canonname))
			{
				const string ret(p->ai_canonname);
				freeaddrinfo(info);
				return ret;
			}
		}

		freeaddrinfo(info);
	}

	return hostname;
}


unsigned int windowsHandler::getProcessId() const
{
	return (static_cast <unsigned int>(::GetCurrentProcessId()));
}


unsigned int windowsHandler::getThreadId() const
{
	return static_cast <unsigned int>(::GetCurrentThreadId());
}


#if VMIME_HAVE_MESSAGING_FEATURES

ref <vmime::net::socketFactory> windowsHandler::getSocketFactory()
{
	return m_socketFactory;
}

#endif


#if VMIME_HAVE_FILESYSTEM_FEATURES

ref <vmime::utility::fileSystemFactory> windowsHandler::getFileSystemFactory()
{
	return m_fileSysFactory;
}


ref <vmime::utility::childProcessFactory> windowsHandler::getChildProcessFactory()
{
	// TODO: Not implemented
	return (NULL);
}

#endif


void windowsHandler::wait() const
{
	// FIX by Elmue: The user must be able to cancel lengthy operations!
	checkCanceled();
	::Sleep(100);
}


void windowsHandler::generateRandomBytes(unsigned char* buffer, const unsigned int count)
{
	HCRYPTPROV cryptProvider = 0;
	CryptAcquireContext(&cryptProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT);
	CryptGenRandom(cryptProvider, static_cast <unsigned long>(count), static_cast <unsigned char*>(buffer));
	CryptReleaseContext(cryptProvider, 0);
}


ref <utility::sync::criticalSection> windowsHandler::createCriticalSection()
{
	return vmime::create <windowsCriticalSection>();
}

} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS

