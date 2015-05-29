
#pragma once

#include "../Wrapper/cPop3.hpp"
#include "EmailParser.hpp"

#pragma managed

using namespace System;
using namespace vmime::wrapper;

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

namespace vmimeNET
{
    // A managed wrapper for unmanaged Pop3 class
	public __gc class Pop3 : public IDisposable
	{
	public:
        __value enum eSecurity : Int32
	    {
            Normal    = cCommon::Secur_Normal,    // on normal port (default 110) unencrypted
            SSL       = cCommon::Secur_SSL,       // on secure port (default 995) using SSL
            TLS_opt   = cCommon::Secur_TLS_opt,   // on normal port (default 110) using TLS if possible
            TLS_force = cCommon::Secur_TLS_force, // on normal port (default 110) demands TLS otherwise fails
        };

        Pop3(String* s_Server, UInt16 u16_Port, eSecurity e_Security, bool b_AllowInvalidCerts);
        ~Pop3();
        void Dispose();

        void SetAuthData(String* s_User, String* s_Password);

        int          GetEmailCount();
        EmailParser* FetchEmailAt(int s32_Index);
        void         Close();

    private:
        cPop3* mpi_Pop3;
	};
}

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3