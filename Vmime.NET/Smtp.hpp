
#pragma once

#include "../Wrapper/cSmtp.hpp"
#include "EmailBuilder.hpp"

#pragma managed

using namespace System;
using namespace vmime::wrapper;

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

namespace vmimeNET
{
    // A managed wrapper for unmanaged Smtp class
	public __gc class Smtp : public IDisposable
	{
	public:
        __value enum eSecurity : Int32
	    {
            Normal    = cCommon::Secur_Normal,    // on normal     port (default  25) unencrypted
            SSL       = cCommon::Secur_SSL,       // on secure     port (default 465) using SSL
            TLS_opt   = cCommon::Secur_TLS_opt,   // on normal     port (default  25) using TLS if possible
            TLS_force = cCommon::Secur_TLS_force, // on submission port (default 587) demands TLS otherwise fails
        };

        Smtp(String* s_Server, UInt16 u16_Port, eSecurity e_Security, bool b_AllowInvalidCerts);
        ~Smtp();
        void Dispose();

        void SetAuthData(String* s_User, String* s_Password);
        void Send(EmailBuilder* i_Email);

    private:
        cSmtp* mpi_Smtp;
	};
}

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP