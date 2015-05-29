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

#ifndef VMIME_NET_TLS_TLSSESSION_HPP_INCLUDED
#define VMIME_NET_TLS_TLSSESSION_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT


#include "../vmime/types.hpp"

#include "../vmime/net/tls/TLSSocket.hpp"
#include "../vmime/net/tls/TLSProperties.hpp"

#include "../vmime/security/cert/certificateVerifier.hpp"


namespace vmime {
namespace net {
namespace tls {


/** Describe a TLS connection between a client and a server.
  */
class VMIME_EXPORT TLSSession : public object
{
public:

	/** Create and initialize a new TLS session.
	  *
	  * @param cv object responsible for verifying certificates
	  * sent by the server
	  * @param props TLS properties for this session
	  * @return a new TLS session
	  */
	static ref <TLSSession> create(ref <security::cert::certificateVerifier> cv, ref <TLSProperties> props);

	/** Create a new socket that adds a TLS security layer around
	  * an existing socket. You should create only one socket
	  * per session.
	  *
	  * @param sok socket to wrap
	  * @return TLS socket wrapper
	  */
	virtual ref <TLSSocket> getSocket(ref <socket> sok) = 0;

	/** Get the object responsible for verifying certificates when
	  * using secured connections (TLS/SSL).
	  */
	virtual ref <security::cert::certificateVerifier> getCertificateVerifier() = 0;

    // FIX by Elmue:
    // Gets the version of OpenSSL or GnuTLS including the name of the library.
    // e.g. "OpenSSL 1.0.1e" or "GnuTLS 3.2.7"
    virtual string getLibraryVersion() = 0;

protected:

	TLSSession();

private:

	TLSSession(const TLSSession&);
};


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT

#endif // VMIME_NET_TLS_TLSSESSION_HPP_INCLUDED
