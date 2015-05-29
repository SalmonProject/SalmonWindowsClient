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
#include "../vmime/charset.hpp"


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS


#include "../gnutls/gnutls.h"
#include "../gnutls/x509.h"

#include <ctime>

#include "../vmime/security/cert/gnutls/X509Certificate_GnuTLS.hpp"

#include "../vmime/utility/outputStreamByteArrayAdapter.hpp"


namespace vmime {
namespace security {
namespace cert {


#ifndef VMIME_BUILDING_DOC

struct GnuTLSX509CertificateInternalData
{
	GnuTLSX509CertificateInternalData()
	{
		gnutls_x509_crt_init(&cert);
	}

	~GnuTLSX509CertificateInternalData()
	{
		gnutls_x509_crt_deinit(cert);
	}


	gnutls_x509_crt cert;
};

#endif // VMIME_BUILDING_DOC


X509Certificate_GnuTLS::X509Certificate_GnuTLS()
	: m_data(new GnuTLSX509CertificateInternalData)
{
}


X509Certificate_GnuTLS::X509Certificate_GnuTLS(const X509Certificate&)
	: X509Certificate(), m_data(NULL)
{
	// Not used
}


X509Certificate_GnuTLS::~X509Certificate_GnuTLS()
{
	delete m_data;
}


void* X509Certificate_GnuTLS::getInternalData()
{
	return &m_data->cert;
}


// static
ref <X509Certificate> X509Certificate::import(utility::inputStream& is)
{
	byteArray bytes;
	utility::stream::value_type chunk[4096];

	while (!is.eof())
	{
		const utility::stream::size_type len = is.read(chunk, sizeof(chunk));
		bytes.insert(bytes.end(), chunk, chunk + len);
	}

	return import(&bytes[0], bytes.size());
}


// static
ref <X509Certificate> X509Certificate::import
	(const byte_t* data, const size_t length)
{
	gnutls_datum buffer;
	buffer.data = const_cast <byte_t*>(data);
	buffer.size = static_cast <unsigned int>(length);

	// Try DER format
	ref <X509Certificate_GnuTLS> derCert = vmime::create <X509Certificate_GnuTLS>();

	if (gnutls_x509_crt_import(derCert->m_data->cert, &buffer, GNUTLS_X509_FMT_DER) >= 0)
		return derCert;

	// Try PEM format
	ref <X509Certificate_GnuTLS> pemCert = vmime::create <X509Certificate_GnuTLS>();

	if (gnutls_x509_crt_import(pemCert->m_data->cert, &buffer, GNUTLS_X509_FMT_PEM) >= 0)
		return pemCert;

	return NULL;
}


void X509Certificate_GnuTLS::write
	(utility::outputStream& os, const Format format) const
{
	size_t dataSize = 0;
	gnutls_x509_crt_fmt fmt = GNUTLS_X509_FMT_DER;

	switch (format)
	{
	case FORMAT_DER: fmt = GNUTLS_X509_FMT_DER; break;
	case FORMAT_PEM: fmt = GNUTLS_X509_FMT_PEM; break;
	}

	gnutls_x509_crt_export(m_data->cert, fmt, NULL, &dataSize);

	std::vector <byte_t> data(dataSize);

	gnutls_x509_crt_export(m_data->cert, fmt, &data[0], &dataSize);

	os.write(reinterpret_cast <utility::stream::value_type*>(&data[0]), dataSize);
}

const byteArray X509Certificate_GnuTLS::getSerialNumber() const
{
	char serial[64];
	size_t serialSize = sizeof(serial);

	gnutls_x509_crt_get_serial(m_data->cert, serial, &serialSize);

	return byteArray(serial, serial + serialSize);
}

// FIX by Elmue: Added support to get details about a certificate
// returns "C=US,O=VeriSign\, Inc.,OU=Class 1 Public Primary Certification Authority"
const string X509Certificate_GnuTLS::getIssuer() const
{
    char   buf[4096];
    size_t bufSize = sizeof(buf);
    if (gnutls_x509_crt_get_issuer_dn(m_data->cert, buf, &bufSize) != GNUTLS_E_SUCCESS)
        return "";
    
    return buf;
}

bool X509Certificate_GnuTLS::checkIssuer(ref <const X509Certificate> issuer_) const
{
	ref <const X509Certificate_GnuTLS> issuer =
		issuer_.dynamicCast <const X509Certificate_GnuTLS>();

	return (gnutls_x509_crt_check_issuer
			(m_data->cert, issuer->m_data->cert) >= 1);
}


bool X509Certificate_GnuTLS::verify(ref <const X509Certificate> caCert_) const
{
	ref <const X509Certificate_GnuTLS> caCert =
		caCert_.dynamicCast <const X509Certificate_GnuTLS>();

	unsigned int verify = 0;

	const int res = gnutls_x509_crt_verify
		(m_data->cert, &(caCert->m_data->cert), 1,
		 GNUTLS_VERIFY_ALLOW_X509_V1_CA_CRT,
		 &verify);

	return (res == 0 && verify == 0);
}

// FIX by Elmue: return of the non-matching names
bool X509Certificate_GnuTLS::verifyHostName(const string& hostname, string& s_NoMatch) const
{
	if (gnutls_x509_crt_check_hostname(m_data->cert, hostname.c_str()) != 0)
        return true;

    const int   MAX_CN               = 256;
    const char* OID_X520_COMMON_NAME = "2.5.4.3";

	char dnsname[MAX_CN];
	size_t dnsnamesize;
    int ret = 0;

	dnsnamesize = sizeof(dnsname);
	if (gnutls_x509_crt_get_dn_by_oid(m_data->cert, OID_X520_COMMON_NAME, 0, 0, dnsname, &dnsnamesize) >= 0) 
    {
        s_NoMatch = dnsname;
	}

	for (int i = 0; ret>=0; i++) 
    {
		dnsnamesize = sizeof(dnsname);
		ret = gnutls_x509_crt_get_subject_alt_name(m_data->cert, i, dnsname, &dnsnamesize, NULL);
		if (ret == GNUTLS_SAN_DNSNAME) 
        {
            if (s_NoMatch.length())
                s_NoMatch += ", ";

            s_NoMatch += dnsname;
		}
	}

    return false;
}


const datetime X509Certificate_GnuTLS::getActivationDate() const
{
	const time_t t = gnutls_x509_crt_get_activation_time(m_data->cert);
	return datetime(t);
}


const datetime X509Certificate_GnuTLS::getExpirationDate() const
{
	const time_t t = gnutls_x509_crt_get_expiration_time(m_data->cert);
	return datetime(t);
}


const byteArray X509Certificate_GnuTLS::getFingerprint(const DigestAlgorithm algo) const
{
	gnutls_digest_algorithm galgo;

	switch (algo)
	{
	case DIGEST_MD5:

		galgo = GNUTLS_DIG_MD5;
		break;

	default:
	case DIGEST_SHA1:

		galgo = GNUTLS_DIG_SHA;
		break;
	}

	size_t bufferSize = 0;
	gnutls_x509_crt_get_fingerprint
		(m_data->cert, galgo, NULL, &bufferSize);

	std::vector <byte_t> buffer(bufferSize);

	if (gnutls_x509_crt_get_fingerprint
		(m_data->cert, galgo, &buffer[0], &bufferSize) == 0)
	{
		byteArray res;
		res.insert(res.end(), &buffer[0], &buffer[0] + bufferSize);

		return res;
	}

	return byteArray();
}


const byteArray X509Certificate_GnuTLS::getEncoded() const
{
	byteArray bytes;
	utility::outputStreamByteArrayAdapter os(bytes);

	write(os, FORMAT_DER);

	return bytes;
}


const string X509Certificate_GnuTLS::getType() const
{
	return "X.509";
}


int X509Certificate_GnuTLS::getVersion() const
{
	return gnutls_x509_crt_get_version(m_data->cert);
}


bool X509Certificate_GnuTLS::equals(ref <const certificate> other) const
{
	ref <const X509Certificate_GnuTLS> otherX509 =
		other.dynamicCast <const X509Certificate_GnuTLS>();

	if (!otherX509)
		return false;

	const byteArray fp1 = getFingerprint(DIGEST_MD5);
	const byteArray fp2 = otherX509->getFingerprint(DIGEST_MD5);

	return fp1 == fp2;
}


} // cert
} // security
} // vmime


#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_TLS_SUPPORT && VMIME_TLS_SUPPORT_LIB_IS_GNUTLS
