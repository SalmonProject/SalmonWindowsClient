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

#ifndef VMIME_NET_TLS_TLSSOCKET_GNUTLS_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSOCKET_GNUTLS_HPP_INCLUDED


#ifndef VMIME_BUILDING_DOC


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include "../vmime/net/tls/TLSSocket.hpp"


namespace vmime {
namespace net {
namespace tls {


class TLSSession;
class TLSSession_GnuTLS;


class TLSSocket_GnuTLS : public TLSSocket
{
	friend class vmime::creator;

public:

	TLSSocket_GnuTLS(ref <TLSSession_GnuTLS> session, ref <socket> sok);
	~TLSSocket_GnuTLS();


	void handshake(ref <timeoutHandler> toHandler = NULL);

	ref <security::cert::certificateChain> getPeerCertificates() const;

	// Implementation of 'socket'
	void connect(const string& address, const port_t port);
	void disconnect();
	bool isConnected() const;

	void receive(string& buffer);
	size_type receiveRaw(char* buffer, const size_type count);

	void send(const string& buffer);
	void sendRaw(const char* buffer, const size_type count);
	size_type sendRawNonBlocking(const char* buffer, const size_type count);

	size_type getBlockSize() const;

	unsigned int getStatus() const;

	const string getPeerName() const;
	const string getPeerAddress() const;

private:

	void internalThrow();

#ifdef LIBGNUTLS_VERSION
	static ssize_t gnutlsPushFunc(gnutls_transport_ptr trspt, const void* data, size_t len);
	static ssize_t gnutlsPullFunc(gnutls_transport_ptr trspt, void* data, size_t len);
#else
	static int gnutlsPushFunc(void* trspt, const void* data, size_t len);
	static int gnutlsPullFunc(void* trspt, void* data, size_t len);
#endif // LIBGNUTLS_VERSION


	ref <TLSSession_GnuTLS> m_session;
	ref <socket> m_wrapped;

	bool m_connected;

	char m_buffer[65536];

	bool m_handshaking;
	ref <timeoutHandler> m_toHandler;

	exception* m_ex;

	unsigned int m_status;
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS

#endif // VMIME_BUILDING_DOC

#endif // VMIME_NET_TLS_TLSSOCKET_GNUTLS_HPP_INCLUDED

