
//
// Smtp class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#pragma once

// NEVER include windows.h in a header file in a Managed C++ project !!
#include "cEmailBuilder.hpp"

#pragma unmanaged

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

namespace vmime   {
namespace wrapper {

class cSmtp
{
public:
    cSmtp(const wchar_t* u16_Server, vmime_uint16 u16_Port, cCommon::eSecurity e_Security, bool b_AllowInvalidCerts, vmime_uint16 u16_ResIdCert);
    virtual ~cSmtp();

    void SetAuthData(const wchar_t* u16_User, const wchar_t* u16_Password);
    bool Send(cEmailBuilder* pi_Email);
    
private:
    string             ms_User;
    string             ms_Password;
    string             ms_Server;
    vmime_uint16       mu16_Port;
    cCommon::eSecurity me_Security;
    vmime_uint16       mu16_ResIdCert;
    bool               mb_AllowInvalidCerts;
};

} // namespace wrapper
} // namespace vmime

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP
