
#pragma once

#include "../Wrapper/cImap.hpp"
#include "EmailParser.hpp"

#pragma managed

using namespace System;
using namespace vmime::wrapper;

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

namespace vmimeNET
{
    // A managed wrapper for unmanaged Imap class
    public __gc class Imap : public IDisposable
	{
	public:
        __value enum eSecurity : Int32
	    {
            Normal    = cCommon::Secur_Normal,    // on normal port (default 143) unencrypted
            SSL       = cCommon::Secur_SSL,       // on secure port (default 993) using SSL
            TLS_opt   = cCommon::Secur_TLS_opt,   // on normal port (default 143) using TLS if possible
            TLS_force = cCommon::Secur_TLS_force, // on normal port (default 143) demands TLS otherwise fails
        };

        Imap(String* s_Server, UInt16 u16_Port, eSecurity e_Security, bool b_AllowInvalidCerts);
        ~Imap();
        void Dispose();

        void SetAuthData(String* s_User, String* s_Password);

        String* EnumFolders() [];
        void    SelectFolder(String* s_Path);
        String* GetCurrentFolder();

        int          GetEmailCount();
        EmailParser* FetchEmailAt(int s32_Index);
        void         Close();

    private:
        cImap* mpi_Imap;
	};
}

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP