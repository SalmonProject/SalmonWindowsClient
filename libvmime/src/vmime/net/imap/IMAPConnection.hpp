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

#ifndef VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED
#define VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP


#include "../vmime/net/socket.hpp"
#include "../vmime/net/timeoutHandler.hpp"
#include "../vmime/net/session.hpp"
#include "../vmime/net/connectionInfos.hpp"

#include "../vmime/net/imap/IMAPParser.hpp"

#include "../vmime/security/authenticator.hpp"


namespace vmime {
namespace net {
namespace imap {


class IMAPTag;
class IMAPStore;


class VMIME_EXPORT IMAPConnection : public object
{
public:

	IMAPConnection(ref <IMAPStore> store, ref <security::authenticator> auth);
	~IMAPConnection();


	void connect();
	bool isConnected() const;
	void disconnect();


	enum ProtocolStates
	{
		STATE_NONE,
		STATE_NON_AUTHENTICATED,
		STATE_AUTHENTICATED,
		STATE_SELECTED,
		STATE_LOGOUT
	};

	ProtocolStates state() const;
	void setState(const ProtocolStates state);


	char hierarchySeparator() const;


    // FIX by Elmue: Added user defined Trace output to hide passwords in Trace
	void send(bool tag, const string& what, bool end, const char* s8_Trace=NULL);
	void sendRaw(const char* buffer, const int count);

	IMAPParser::response* readResponse(IMAPParser::literalHandler* lh = NULL);


	ref <const IMAPStore> getStore() const;
	ref <IMAPStore> getStore();

	ref <session> getSession();

	void fetchCapabilities();
	void invalidateCapabilities();
	const std::vector <string> getCapabilities();
	bool hasCapability(const string& capa);

	ref <security::authenticator> getAuthenticator();

	bool isSecuredConnection() const;
	ref <connectionInfos> getConnectionInfos() const;

	ref <const socket> getSocket() const;

	bool isMODSEQDisabled() const;
	void disableMODSEQ();

private:

	void authenticate();
#if VMIME_HAVE_SASL_SUPPORT
	void authenticateSASL();
#endif // VMIME_HAVE_SASL_SUPPORT

#if VMIME_HAVE_TLS_SUPPORT
	void startTLS();
#endif // VMIME_HAVE_TLS_SUPPORT

	bool processCapabilityResponseData(const IMAPParser::response* resp);
	void processCapabilityResponseData(const IMAPParser::capability_data* capaData);


	weak_ref <IMAPStore> m_store;

	ref <security::authenticator> m_auth;

	ref <socket> m_socket;

	ref <IMAPParser> m_parser;

	ref <IMAPTag> m_tag;

	char m_hierarchySeparator;

	ProtocolStates m_state;

	ref <timeoutHandler> m_timeoutHandler;

	bool m_secured;
	ref <connectionInfos> m_cntInfos;

	bool m_firstTag;

	std::vector <string> m_capabilities;
	bool m_capabilitiesFetched;

	bool m_noModSeq;


	void internalDisconnect();

	void initHierarchySeparator();

    // FIX by Elmue: Added static instance counter for Trace output
    static int g_instanceID;
           int m_instanceID;
};


} // imap
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

#endif // VMIME_NET_IMAP_IMAPCONNECTION_HPP_INCLUDED
