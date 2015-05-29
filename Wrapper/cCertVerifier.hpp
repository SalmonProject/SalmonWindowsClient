
//
// Certificate Verifier class
// Copyright (C) Elmue
//
// This class verifies the server X509 certificate against the root certificates in the Resource file RootCA.txt.
// You can optionally add user-trusted certificates by calling CertVerifier.setX509TrustedCerts()
// But this is not recommended because how will the user decide if he can trust a certificate that the server sends?
//


#pragma once

// NEVER include windows.h in a header file in a Managed C++ project !!
#include "cCommon.hpp"

#pragma unmanaged

using namespace vmime::security;

namespace vmime   {
namespace wrapper {

// Certificate verifier (X509)
class cCertVerifier : public cert::defaultCertificateVerifier
{
public:
    cCertVerifier(bool b_AllowInvalidCerts, vmime_uint16 u16_ResIdCert);

    // overrides virtual function in base class
	void verify(ref <cert::certificateChain> chain, const string& hostname);

private:
    void Init();

    bool mb_InitDone;
    bool mb_AllowInvalidCerts;

    std::string  ms_ResourceData;
};

} // namespace wrapper
} // namespace vmime
