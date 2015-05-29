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


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL


#include "../openssl/ssl.h"
#include "../openssl/err.h"

#include "../vmime/net/tls/openssl/TLSSocket_OpenSSL.hpp"
#include "../vmime/net/tls/openssl/TLSSession_OpenSSL.hpp"
#include "../vmime/net/tls/openssl/OpenSSLInitializer.hpp"

#include "../vmime/platform.hpp"

#include "../vmime/security/cert/openssl/X509Certificate_OpenSSL.hpp"

#include <vector>


namespace vmime {
namespace net {
namespace tls {


static OpenSSLInitializer::autoInitializer openSSLInitializer;


// static
BIO_METHOD TLSSocket_OpenSSL::sm_customBIOMethod =
{
	100 | BIO_TYPE_SOURCE_SINK,
	"vmime::socket glue",
	TLSSocket_OpenSSL::bio_write,
	TLSSocket_OpenSSL::bio_read,
	TLSSocket_OpenSSL::bio_puts,
	NULL,  // gets
	TLSSocket_OpenSSL::bio_ctrl,
	TLSSocket_OpenSSL::bio_create,
	TLSSocket_OpenSSL::bio_destroy,
	0
};


// static
ref <TLSSocket> TLSSocket::wrap(ref <TLSSession> session, ref <socket> sok)
{
	return vmime::create <TLSSocket_OpenSSL>
		(session.dynamicCast <TLSSession_OpenSSL>(), sok);
}


TLSSocket_OpenSSL::TLSSocket_OpenSSL(ref <TLSSession_OpenSSL> session, ref <socket> sok)
	: m_session(session), m_wrapped(sok), m_connected(false), m_ssl(0), m_status(0), m_ex(NULL)
{
}


TLSSocket_OpenSSL::~TLSSocket_OpenSSL()
{
	try
	{
		disconnect();

		if (m_ssl)
		{
			SSL_free(m_ssl);
			m_ssl = 0;
		}
	}
	catch (...)
	{
		// Don't throw in destructor
	}
}


void TLSSocket_OpenSSL::createSSLHandle()
{
	if (m_wrapped->isConnected())
	{
		BIO* sockBio = BIO_new(&sm_customBIOMethod);
		sockBio->ptr = this;
		sockBio->init = 1;

		m_ssl = SSL_new(m_session->getContext());

		if (!m_ssl)
		{
			BIO_free(sockBio);
			throw exceptions::tls_exception("Cannot create SSL object");
		}

		SSL_set_bio(m_ssl, sockBio, sockBio);
		SSL_set_connect_state(m_ssl);

        SSL_set_mode(m_ssl, SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
	}
	else
	{
		throw exceptions::tls_exception("Unconnected socket error");
	}
}

void TLSSocket_OpenSSL::connect(const string& address, const port_t port)
{
	m_wrapped->connect(address, port);
	createSSLHandle();
	handshake(NULL);
	m_connected = true;
}


void TLSSocket_OpenSSL::disconnect()
{
	if (m_connected)
	{
		if (m_ssl)
		{
			// Don't shut down the socket more than once.
			int shutdownState = SSL_get_shutdown(m_ssl);
			bool shutdownSent = (shutdownState & SSL_SENT_SHUTDOWN) == SSL_SENT_SHUTDOWN;

			if (!shutdownSent)
				SSL_shutdown(m_ssl);
		}

		m_wrapped->disconnect();
		m_connected = false;
	}
}


bool TLSSocket_OpenSSL::isConnected() const
{
	return m_wrapped->isConnected() && m_connected;
}


TLSSocket::size_type TLSSocket_OpenSSL::getBlockSize() const
{
	return 16384;  // 16 KB
}


const string TLSSocket_OpenSSL::getPeerName() const
{
	return m_wrapped->getPeerName();
}


const string TLSSocket_OpenSSL::getPeerAddress() const
{
	return m_wrapped->getPeerAddress();
}


void TLSSocket_OpenSSL::receive(string& buffer)
{
	const size_type size = receiveRaw(m_buffer, sizeof(m_buffer));

	if (size != 0)
		buffer = vmime::string(m_buffer, size);
	else
		buffer.clear();
}


void TLSSocket_OpenSSL::send(const string& buffer)
{
	sendRaw(buffer.data(), buffer.length());
}


TLSSocket::size_type TLSSocket_OpenSSL::receiveRaw(char* buffer, const size_type count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	int rc = SSL_read(m_ssl, buffer, static_cast <int>(count));

	if (m_ex.get())
		internalThrow();

	if (rc <= 0)
	{
		int error = SSL_get_error(m_ssl, rc);

		if (error == SSL_ERROR_WANT_WRITE || error == SSL_ERROR_WANT_READ)
		{
			m_status |= STATUS_WOULDBLOCK;
			return 0;
		}

		handleError(rc);
	}

	return rc;
}


void TLSSocket_OpenSSL::sendRaw(const char* buffer, const size_type count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	size_t size = count;

	while (size > 0)
	{
		int rc = SSL_write(m_ssl, buffer, static_cast <int>(size));

		if (m_ex.get())
			internalThrow();

		if (rc <= 0)
		{
			int error = SSL_get_error(m_ssl, rc);

			if (error == SSL_ERROR_WANT_WRITE || error == SSL_ERROR_WANT_READ)
			{
				platform::getHandler()->wait();
				continue;
			}

			handleError(rc);
		}
		else
		{
			buffer += rc;
			size -= rc;
		}
	}
}


TLSSocket_OpenSSL::size_type TLSSocket_OpenSSL::sendRawNonBlocking(const char* buffer, const size_type count)
{
	m_status &= ~STATUS_WOULDBLOCK;

	int rc = SSL_write(m_ssl, buffer, static_cast <int>(count));

	if (m_ex.get())
		internalThrow();

	if (rc <= 0)
	{
		int error = SSL_get_error(m_ssl, rc);

		if (error == SSL_ERROR_WANT_WRITE || error == SSL_ERROR_WANT_READ)
		{
			m_status |= STATUS_WOULDBLOCK;
			return 0;
		}

		handleError(rc);
	}

	return rc;
}


void TLSSocket_OpenSSL::handshake(ref <timeoutHandler> toHandler)
{
	if (toHandler)
		toHandler->resetTimeOut();

	// Start handshaking process
	m_toHandler = toHandler;

	if (!m_ssl)
		createSSLHandle();

	try
	{
		int rc;

		while ((rc = SSL_do_handshake(m_ssl)) <= 0)
		{
			const int err = SSL_get_error(m_ssl, rc);

 			if (err == SSL_ERROR_WANT_READ || err == SSL_ERROR_WANT_WRITE)
 			{
				// No data available yet
				platform::getHandler()->wait();
 			}
 			else
 			{
 				handleError(rc);
 			}

			// Check whether the time-out delay is elapsed
			if (m_toHandler && m_toHandler->isTimeOut())
			{
				if (!m_toHandler->handleTimeOut())
					throw exceptions::operation_timed_out();

				m_toHandler->resetTimeOut();
			}
		}
	}
	catch (...)
	{
		SSL_free(m_ssl);
		m_ssl = 0;
		m_toHandler = NULL;
		throw;
	}

	m_toHandler = NULL;

	// Verify server's certificate(s)
	ref <security::cert::certificateChain> certs = getPeerCertificates();
	if (certs == NULL)
		throw exceptions::tls_exception("No peer certificate.");

	m_session->getCertificateVerifier()->verify(certs, getPeerName());
	m_connected = true;
}


ref <security::cert::certificateChain> TLSSocket_OpenSSL::getPeerCertificates() const
{
	STACK_OF(X509)* chain = SSL_get_peer_cert_chain(m_ssl);

	if (chain == NULL)
		return NULL;

	int certCount = sk_X509_num(chain);

	if (certCount == 0)
		return NULL;

	bool error = false;
	std::vector <ref <security::cert::certificate> > certs;

	for (int i = 0; i < certCount && !error; i++)
	{
		ref <vmime::security::cert::X509Certificate> cert =
			vmime::security::cert::X509Certificate_OpenSSL::importInternal(sk_X509_value(chain, i));

		if (cert)
			certs.push_back(cert);
		else
			error = true;
	}

	if (error)
		return NULL;

	return vmime::create <security::cert::certificateChain>(certs);
}


void TLSSocket_OpenSSL::internalThrow()
{
	if (m_ex.get())
		throw *m_ex;
}


void TLSSocket_OpenSSL::handleError(int rc)
{
	if (rc > 0) return;

	internalThrow();

	int  sslError   = SSL_get_error(m_ssl, rc);
	long lastError  = ERR_get_error();

	switch (sslError)
	{
	case SSL_ERROR_ZERO_RETURN:
		return;

	case SSL_ERROR_SYSCALL:
	{
		if (lastError == 0)
		{
			if (rc == 0)
			{
				throw exceptions::tls_exception("SSL connection unexpectedly closed");
			}
			else
			{
				vmime::string msg;
				std::ostringstream oss(msg);
				oss << "The BIO reported an error: " << rc;
				oss.flush();
				throw exceptions::tls_exception(oss.str());
			}
		}
		break;
	}

	case SSL_ERROR_WANT_READ:

		BIO_set_retry_read(SSL_get_rbio(m_ssl));
		break;

	case SSL_ERROR_WANT_WRITE:

		BIO_set_retry_write(SSL_get_wbio(m_ssl));
		break;

	// This happens only for BIOs of type BIO_s_connect() or BIO_s_accept()
	case SSL_ERROR_WANT_CONNECT:
	case SSL_ERROR_WANT_ACCEPT:
	// SSL_CTX_set_client_cert_cb related, not used
	case SSL_ERROR_WANT_X509_LOOKUP:
	case SSL_ERROR_SSL:
	default:

		if (lastError == 0)
		{
			throw exceptions::tls_exception("Unexpected SSL IO error");
		}
		else
		{
			char buffer[256];
			ERR_error_string_n(lastError, buffer, sizeof(buffer));
			vmime::string msg(buffer);
			throw exceptions::tls_exception(msg);
		}

		break;
	}
}


unsigned int TLSSocket_OpenSSL::getStatus() const
{
	return m_status;
}


// Implementation of custom BIO methods


// static
int TLSSocket_OpenSSL::bio_write(BIO* bio, const char* buf, int len)
{
	BIO_clear_retry_flags(bio);

	if (buf == NULL || len <= 0)
		return -1;

	TLSSocket_OpenSSL *sok = reinterpret_cast <TLSSocket_OpenSSL*>(bio->ptr);

	if (!bio->init || !sok)
		return -1;

	try
	{
		const size_t n = sok->m_wrapped->sendRawNonBlocking(buf, len);

		if (n == 0 && sok->m_wrapped->getStatus() & socket::STATUS_WOULDBLOCK)
		{
			BIO_set_retry_write(bio);
			return -1;
		}

		return static_cast <int>(len);
	}
    catch (vmime::exception& e)
	{
		// Workaround for passing C++ exceptions from C BIO functions
		sok->m_ex.reset(e.clone());
		return -1;
	}
    catch (std::exception& e) // FIX by Elmue: Added support for std::exception
	{
		// Workaround for passing C++ exceptions from C BIO functions
        sok->m_ex.reset(new vmime::exception(e.what()));
		return -1;
	}
}


// static
int TLSSocket_OpenSSL::bio_read(BIO* bio, char* buf, int len)
{
	BIO_clear_retry_flags(bio);

	if (buf == NULL || len <= 0)
		return -1;

	TLSSocket_OpenSSL *sok = reinterpret_cast <TLSSocket_OpenSSL*>(bio->ptr);

	if (!bio->init || !sok)
		return -1;

	try
	{
		const size_t n = sok->m_wrapped->receiveRaw(buf, len);

		if (n == 0 || sok->m_wrapped->getStatus() & socket::STATUS_WOULDBLOCK)
		{
			BIO_set_retry_read(bio);
			return -1;
		}

		return static_cast <int>(n);
	}
    catch (vmime::exception& e)
	{
		// Workaround for passing C++ exceptions from C BIO functions
		sok->m_ex.reset(e.clone());
		return -1;
	}
    catch (std::exception& e) // FIX by Elmue: Added support for std::exception
	{
		// Workaround for passing C++ exceptions from C BIO functions
        sok->m_ex.reset(new vmime::exception(e.what()));
		return -1;
	}
}


// static
int TLSSocket_OpenSSL::bio_puts(BIO* bio, const char* str)
{
	return bio_write(bio, str, static_cast <int>(strlen(str)));
}


// static
long TLSSocket_OpenSSL::bio_ctrl(BIO* bio, int cmd, long num, void* ptr)
{
	long ret = 1;

	switch (cmd)
	{
	case BIO_CTRL_INFO:

		ret = 0;
		break;

	case BIO_CTRL_GET_CLOSE:

		ret = bio->shutdown;
		break;

	case BIO_CTRL_SET_CLOSE:

		bio->shutdown = static_cast <int>(num);
		break;

	case BIO_CTRL_PENDING:
	case BIO_CTRL_WPENDING:

		ret = 0;
		break;

	case BIO_CTRL_DUP:
	case BIO_CTRL_FLUSH:

		ret = 1;
		break;

	default:

		ret = 0;
		break;
	}

	return ret;
}


// static
int TLSSocket_OpenSSL::bio_create(BIO* bio)
{
	bio->init = 0;
	bio->num = 0;
	bio->ptr = NULL;
	bio->flags = 0;

	return 1;
}


// static
int TLSSocket_OpenSSL::bio_destroy(BIO* bio)
{
	if (bio == NULL)
		return 0;

	if (bio->shutdown)
	{
		bio->ptr = NULL;
		bio->init = 0;
		bio->flags = 0;
	}

	return 1;
}


} // tls
} // net
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_OPENSSL
