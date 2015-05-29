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

#ifndef VMIME_SECURITY_SASL_SASLSESSION_HPP_INCLUDED
#define VMIME_SECURITY_SASL_SASLSESSION_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT


#include "../vmime/types.hpp"

#include "../vmime/security/sasl/SASLAuthenticator.hpp"
#include "../vmime/security/sasl/SASLMechanism.hpp"
#include "../vmime/security/sasl/SASLSocket.hpp"


namespace vmime {
namespace security {
namespace sasl {


class SASLContext; // forward declaration


/** An SASL client session.
  */
class VMIME_EXPORT SASLSession : public object
{
	friend class builtinSASLMechanism;
	friend class SASLSocket;

public:

	~SASLSession();

	/** Construct a new SASL session.
	  *
	  * @param serviceName name of the service using this session
	  * @param ctx SASL context
	  * @param auth authenticator to use for this session
	  * @param mech SASL mechanism
	  */
	SASLSession(const string& serviceName, ref <SASLContext> ctx,
	        ref <authenticator> auth, ref <SASLMechanism> mech);

	/** Initialize this SASL session. This must be called before
	  * calling any other method on this object (except accessors).
	  */
	void init();

	/** Return the authenticator used for this session. This is the
	  * authenticator which has been previously set with a call to
	  * setAuthenticator().
	  *
	  * @return authenticator object
	  */
	ref <authenticator> getAuthenticator();

	/** Return the mechanism used for this session.
	  *
	  * @return SASL mechanism
	  */
	ref <SASLMechanism> getMechanism();

	/** Return the SASL context.
	  *
	  * @return SASL context
	  */
	ref <SASLContext> getContext();

	/** Perform one step of SASL authentication. Accept data from the
	  * server (challenge), process it and return data to be returned
	  * in response to the server.
	  *
	  * @param challenge challenge sent from the server
	  * @param challengeLen length of challenge
	  * @param response response to send to the server (allocated by
	  * this function, free with delete[])
	  * @param responseLen length of response buffer
	  * @return true if authentication terminated successfully, or
	  * false if the authentication process should continue
	  * @throw exceptions::sasl_exception if an error occured during
	  * authentication (in this case, the values in 'response' and
	  * 'responseLen' are undetermined)
	  */
	bool evaluateChallenge
		(const byte_t* challenge, const long challengeLen,
		 byte_t** response, long* responseLen);

	/** Return a socket in which transmitted data is integrity
	  * and/or privacy protected, depending on the QOP (Quality of
	  * Protection) negotiated during the SASL authentication.
	  *
	  * @param sok socket to wrap
	  * @return secured socket
	  */
	ref <net::socket> getSecuredSocket(ref <net::socket> sok);

	/** Return the name of the service which is using this
	  * SASL session (eg. "imap"). This value should be returned
	  * by the authenticator when INFO_SERVICE is requested.
	  *
	  * @return service name
	  */
	const string getServiceName() const;

private:

	const string m_serviceName;

	ref <SASLContext> m_context;
	ref <authenticator> m_auth;
	ref <SASLMechanism> m_mech;

#ifdef GSASL_VERSION
	Gsasl* m_gsaslContext;
	Gsasl_session* m_gsaslSession;

	static int gsaslCallback(Gsasl* ctx, Gsasl_session* sctx, Gsasl_property prop);
#else
	void* m_gsaslContext;
	void* m_gsaslSession;

	static int gsaslCallback(void* ctx, void* sctx, int prop);
#endif // GSASL_VERSION

}; // SASLSession



/************************************************

 FIX by Elmue:
 Class declararion of SASLContext moved to SASLSession.hpp to avoid compiler error on VS
 due to mutual inclusion of header files.

************************************************/


/** An SASL client context.
  */
class VMIME_EXPORT SASLContext : public object
{
	friend class SASLSession;
	friend class builtinSASLMechanism;

public:

	~SASLContext();

	/** Construct and initialize a new SASL context.
	  */
	SASLContext();

	/** Create and initialize a new SASL session.
	  *
	  * @param serviceName name of the service which will use the session
	  * @param auth authenticator object to use during the session
	  * @param mech SASL mechanism
	  * @return a new SASL session
	  */
	ref <SASLSession> createSession
		(const string& serviceName,
		 ref <authenticator> auth, ref <SASLMechanism> mech);

	/** Create an instance of an SASL mechanism.
	  *
	  * @param name mechanism name
	  * @return a new instance of the specified SASL mechanism
	  * @throw exceptions::no_such_mechanism if no mechanism is
	  * registered for the specified name
	  */
	ref <SASLMechanism> createMechanism(const string& name);

	/** Suggests an SASL mechanism among a set of mechanisms
	  * supported by the server.
	  *
	  * @param mechs list of mechanisms
	  * @return suggested mechanism (usually the safest mechanism
	  * supported by both the client and the server)
	  */
	ref <SASLMechanism> suggestMechanism
		(const std::vector <ref <SASLMechanism> >& mechs);

	/** Helper function for decoding Base64-encoded challenge.
	  *
	  * @param input input buffer
	  * @param output output buffer
	  * @param outputLen length of output buffer
	  */
	void decodeB64(const string& input, byte_t** output, long* outputLen);

	/** Helper function for encoding challenge in Base64.
	  *
	  * @param input input buffer
	  * @param inputLen length of input buffer
	  * @return Base64-encoded challenge
	  */
	const string encodeB64(const byte_t* input, const long inputLen);

private:

	static const string getErrorMessage(const string& fname, const int code);


#ifdef GSASL_VERSION
	Gsasl* m_gsaslContext;
#else
	void* m_gsaslContext;
#endif // GSASL_VERSION

};  // SASLContext





} // sasl
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_SASL_SUPPORT

#endif // VMIME_SECURITY_SASL_SASLSESSION_HPP_INCLUDED

