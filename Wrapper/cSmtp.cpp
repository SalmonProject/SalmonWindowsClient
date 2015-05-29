
//
// Smtp class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#include "stdafx.h"
#include "cSmtp.hpp"
#include "cCertVerifier.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

#pragma unmanaged

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

namespace vmime   {
namespace wrapper {

// u16_Server = hostname of SMTP server
// u16_Port   = 0 to use the default port, otherwise the user defined port
// e_Security = The encryption mode
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
// u16_ResIdCert = The IDR identifier in the RCDATA Resources of RootCA.txt. (IDR_ROOT_CA)
cSmtp::cSmtp(const wchar_t* u16_Server, vmime_uint16 u16_Port, cCommon::eSecurity e_Security, 
             bool  b_AllowInvalidCerts, vmime_uint16 u16_ResIdCert)
{
    ms_Server      = UTF(u16_Server);
    mu16_Port      = u16_Port;
    me_Security    = e_Security;
    mu16_ResIdCert = u16_ResIdCert;
    mb_AllowInvalidCerts = b_AllowInvalidCerts;
}

cSmtp::~cSmtp()
{
    // Erase password from memory
    memset((void*)ms_Password.c_str(), 0, ms_Password.length());
}

void cSmtp::SetAuthData(const wchar_t* u16_User, const wchar_t* u16_Password)
{
    ms_User     = UTF(u16_User);
    ms_Password = UTF(u16_Password);
}

// throws
bool cSmtp::Send(cEmailBuilder* pi_Email)
{
    try
    {
        const char* s8_Protocol;
        //if (me_Security == cCommon::Secur_SSL) s8_Protocol = "smtps";
        //else                                   s8_Protocol = "smtp";
		s8_Protocol = "smtp";

        if (mu16_Port == 0)
        {
            if (me_Security == cCommon::Secur_TLS_force) 
                mu16_Port = 587;
            else
                mu16_Port = utility::url::UNSPECIFIED_PORT;
        }

        utility::url i_Url(s8_Protocol, ms_Server, mu16_Port, "", ms_User, ms_Password);

        ref <net::session>   i_Session = create <net::session>();
	    ref <net::transport> i_Transp  = i_Session->getTransport(i_Url);

        ref <cCertVerifier> i_Verifier = create<cCertVerifier>(mb_AllowInvalidCerts, mu16_ResIdCert);
        i_Transp->setCertificateVerifier(i_Verifier);
        i_Transp->setTimeoutHandlerFactory(create<TimeoutFactory>());

        if (ms_User.length() || ms_Password.length())
        {
            i_Transp->setProperty("options.need-authentication", true);
        }

        if (me_Security == cCommon::Secur_TLS_opt || me_Security == cCommon::Secur_TLS_force)
        {
            i_Transp->setProperty("connection.tls", true); // use STARTTLS
        }

        if (me_Security == cCommon::Secur_TLS_force)
        {
            i_Transp->setProperty("connection.tls.required", true);
        }

        // "options.sasl.fallback" not available for SMTP

        TraceProgress i_Progress(1000, "SMTP"); // Trace progress once a second

        #if VMIME_TRACE
            TRACE("SMTP Using vmime %s", VMIME_VERSION);
        #endif

	    i_Transp->connect();
        i_Transp->send(pi_Email->BuildMessage(), &i_Progress);
        i_Transp->disconnect();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        //throw; 
		return false;
    }
	return true;
}

} // namespace wrapper
} // namespace vmime

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

