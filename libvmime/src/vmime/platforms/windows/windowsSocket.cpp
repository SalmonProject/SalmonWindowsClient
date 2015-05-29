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


#if VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_MESSAGING_FEATURES


#pragma warning(disable: 4267)

#include "../vmime/platforms/windows/windowsSocket.hpp"

#include "../vmime/exception.hpp"

// FIX by Elmue: Added
#include "../vmime/platform.hpp"

#include <ws2tcpip.h>


namespace vmime {
namespace platforms {
namespace windows {


//
// windowsSocket
//

windowsSocket::windowsSocket(ref <vmime::net::timeoutHandler> th)
	: m_timeoutHandler(th), m_desc(INVALID_SOCKET), m_status(0)
{
	WSAData wsaData;
	WSAStartup(MAKEWORD(1, 1), &wsaData);
}


windowsSocket::~windowsSocket()
{
	if (m_desc != INVALID_SOCKET)
		::closesocket(m_desc);

	WSACleanup();
}

void windowsSocket::connect(const vmime::string& address, const vmime::port_t port)
{
	// Close current connection, if any
	if (m_desc != INVALID_SOCKET)
	{
		::closesocket(m_desc);
		m_desc = INVALID_SOCKET;
	}

    // FIX by Elmue: replaced 'memset' with '= {0}'
    ::sockaddr_in addr = {0};

	addr.sin_family = AF_INET;
	addr.sin_port = htons(static_cast <unsigned short>(port));
	addr.sin_addr.s_addr = ::inet_addr(address.c_str());

	if (addr.sin_addr.s_addr == static_cast <int>(-1))
	{
        // Resolve address
		::hostent* hostInfo = ::gethostbyname(address.c_str());
		if (hostInfo == NULL)
			throw vmime::exceptions::connection_error("Cannot resolve address.");

        // FIX by Elmue: memcpy takes any void* pointer and does not require a cast to char*
		memcpy(&addr.sin_addr, hostInfo->h_addr, hostInfo->h_length);
	}

	m_serverAddress = address;

	// Get a new socket
	m_desc = ::socket(AF_INET, SOCK_STREAM, 0);

    // FIX by Elmue: Why first throw an exception to then catch it? This can be done smarter.
	if (m_desc == INVALID_SOCKET)
        throwConnectError(WSAGetLastError());

    // FIX by Elmue: Change socket to non-blocking mode BEFORE connecting.
    // (A connect() may block for 30 seconds if the server does not respond on the given port!)
	unsigned long non_blocking = 1;
	::ioctlsocket(m_desc, FIONBIO, &non_blocking);

    // FIX by Elmue: Do not allow any network operations without timeout handler.
	if (!m_timeoutHandler)
        throw exception("Please specify a timeout handler. It is mandatory! "
                        "Some operations may block ETERNALLY due to the design of vmime code.");
    m_timeoutHandler->resetTimeOut();

	// Connect to the server's port in non-blocking mode
	if (::connect(m_desc, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0)
        return;
	
	int err = WSAGetLastError();
    if (err != WSAEWOULDBLOCK)
        throwConnectError(err);

    // FIX by Elmue: In non-blocking mode we must wait until waitForData tells us that the socket is writable
    while (true)
    {
		bool timedout;
		waitForData(WRITE, timedout);

        if (!timedout)
            break;

		if (m_timeoutHandler->isTimeOut() &&
           !m_timeoutHandler->handleTimeOut())
        {
            throwConnectError(ERROR_TIMEOUT);
        }
    }
}


bool windowsSocket::isConnected() const
{
	if (m_desc == INVALID_SOCKET)
		return false;

	char buff;

	return ::recv(m_desc, &buff, 1, MSG_PEEK) != 0;
}


void windowsSocket::disconnect()
{
	if (m_desc != INVALID_SOCKET)
	{
		::shutdown(m_desc, SD_BOTH);
		::closesocket(m_desc);

		m_desc = INVALID_SOCKET;
	}
}


static bool isNumericAddress(const char* address)
{
	struct addrinfo hint, *info = NULL;
	memset(&hint, 0, sizeof(hint));

	hint.ai_family = AF_UNSPEC;
	hint.ai_flags = AI_NUMERICHOST;

	if (getaddrinfo(address, 0, &hint, &info) == 0)
	{
		freeaddrinfo(info);
		return true;
	}
	else
	{
		return false;
	}
}


const string windowsSocket::getPeerAddress() const
{
	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	getpeername(m_desc, reinterpret_cast <sockaddr*>(&peer), &peerLen);

	// Convert to numerical presentation format
	char host[NI_MAXHOST + 1];
	char service[NI_MAXSERV + 1];

	if (getnameinfo(reinterpret_cast <sockaddr *>(&peer), peerLen,
			host, sizeof(host), service, sizeof(service),
			/* flags */ NI_NUMERICHOST) == 0)
	{
		return string(host);
	}

	return "";  // should not happen
}


const string windowsSocket::getPeerName() const
{
	// Get address of connected peer
	sockaddr peer;
	socklen_t peerLen = sizeof(peer);

	getpeername(m_desc, reinterpret_cast <sockaddr*>(&peer), &peerLen);

	// If server address as specified when connecting is a numeric
	// address, try to get a host name for it
	if (isNumericAddress(m_serverAddress.c_str()))
	{
		char host[NI_MAXHOST + 1];
		char service[NI_MAXSERV + 1];

		if (getnameinfo(reinterpret_cast <sockaddr *>(&peer), peerLen,
				host, sizeof(host), service, sizeof(service),
				/* flags */ NI_NAMEREQD) == 0)
		{
			return string(host);
		}
	}

	return m_serverAddress;
}


windowsSocket::size_type windowsSocket::getBlockSize() const
{
	return 16384;  // 16 KB
}


void windowsSocket::receive(vmime::string& buffer)
{
	const size_type size = receiveRaw(m_buffer, sizeof(m_buffer));
	buffer = vmime::string(m_buffer, size);
}


windowsSocket::size_type windowsSocket::receiveRaw(char* buffer, const size_type count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	// Check whether data is available
	bool timedout;
	waitForData(READ, timedout);

	if (timedout)
	{
		// No data available at this time
		// Check if we are timed out
		if (m_timeoutHandler &&
		    m_timeoutHandler->isTimeOut())
		{
			if (!m_timeoutHandler->handleTimeOut())
			{
				// Server did not react within timeout delay
                // FIX by Elmue: Windows gives a very long and stupid error message for WSAETIMEDOUT that is difficult to understand.
                throw exceptions::operation_timed_out();
			}
			else
			{
				// Reset timeout
				m_timeoutHandler->resetTimeOut();
			}
		}

		// Continue waiting for data
		return 0;
	}

	// Read available data
	int ret = ::recv(m_desc, buffer, count, 0);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err != WSAEWOULDBLOCK)
			throwSocketError(err);

		m_status |= STATUS_WOULDBLOCK;

		// Error or no data
		return (0);
	}
	else if (ret == 0)
	{
		// Host shutdown
		throwSocketError(WSAENOTCONN);
        return 0; // Fix by Elmue: Added to avoid compiler warning about missing return value
	}
	else
	{
		// Data received, reset timeout
		if (m_timeoutHandler)
			m_timeoutHandler->resetTimeOut();

		return ret;
	}
}


void windowsSocket::send(const vmime::string& buffer)
{
	sendRaw(buffer.data(), buffer.length());
}


void windowsSocket::sendRaw(const char* buffer, const size_type count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	size_type size = count;

	while (size > 0)
	{
		const int ret = ::send(m_desc, buffer, size, 0);

		if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (err != WSAEWOULDBLOCK)
				throwSocketError(err);

			bool timedout;
			waitForData(WRITE, timedout);
		}
		else
		{
			buffer += ret;
			size -= ret;
		}
	}

	// Reset timeout
	if (m_timeoutHandler)
		m_timeoutHandler->resetTimeOut();
}


windowsSocket::size_type windowsSocket::sendRawNonBlocking(const char* buffer, const size_type count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	const int ret = ::send(m_desc, buffer, count, 0);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();

		if (err == WSAEWOULDBLOCK)
		{
			m_status |= STATUS_WOULDBLOCK;

			// No data can be written at this time
			return 0;
		}
		else
		{
			throwSocketError(err);
		}
	}

	return ret;
}


unsigned int windowsSocket::getStatus() const
{
	return m_status;
}

// FIX by Elmue: Modified
void windowsSocket::throwSocketError(const int err)
{
    throw exceptions::socket_exception(explainWindowsError(err));
}

// FIX by Elmue: Added
void windowsSocket::throwConnectError(const int err)
{
    ::closesocket(m_desc);
    m_desc = INVALID_SOCKET;

    throw exceptions::connection_error("Error connecting to server.\n" + explainWindowsError(err));
}

// FIX by Elmue: Moved into a separate function
string windowsSocket::explainWindowsError(const int err)
{
    // Fix by Elmue: Bugfix and using Widechar API
	vmime::wstring desc;

	WCHAR* lpMsgBuf;
	if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			            NULL, err, 0, (LPWSTR)&lpMsgBuf, 0, NULL))
	{
        // It will not happen here that Windows cannot explain the error code.
        // There are only a few COM errors with values like 0x80XXXXXX that have no explanation.
        // But the entire Windows API uses error codes that have a textual explanation.
        WCHAR buf[100];
        swprintf_s(buf, L"Windows error %d.", err);
        desc = buf;
	}
	else
	{
        desc = lpMsgBuf;
		LocalFree(lpMsgBuf);
	}

	return charset::WstringToUtf8(desc);
}


void windowsSocket::waitForData(const WaitOpType t, bool& timedOut)
{
    // FIX by Elmue: The user must be able to abort
    platform::handler::checkCanceled();

	// Check whether data is available
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_desc, &fds);

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	int ret;

	if (t & READ)
		ret = ::select((int)m_desc + 1, &fds, NULL, NULL, &tv);
	else if (t & WRITE)
		ret = ::select((int)m_desc + 1, NULL, &fds, NULL, &tv);
	else
		ret = ::select((int)m_desc + 1, &fds, &fds, NULL, &tv);

	timedOut = (ret == 0);

	if (ret == SOCKET_ERROR)
	{
		int err = WSAGetLastError();
		throwSocketError(err);
	}
}



//
// posixSocketFactory
//

ref <vmime::net::socket> windowsSocketFactory::create()
{
	ref <vmime::net::timeoutHandler> th = NULL;
	return vmime::create <windowsSocket>(th);
}

ref <vmime::net::socket> windowsSocketFactory::create(ref <vmime::net::timeoutHandler> th)
{
    return vmime::create <windowsSocket>(th);
}

} // posix
} // platforms
} // vmime


#endif // VMIME_PLATFORM_IS_WINDOWS && VMIME_HAVE_MESSAGING_FEATURES

