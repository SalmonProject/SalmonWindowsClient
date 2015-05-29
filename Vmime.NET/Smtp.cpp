
#include "stdafx.h"
#include "resource.h"
#include "Smtp.hpp"
#include "Common.hpp"

#pragma managed


#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP

namespace vmimeNET {

// A managed wrapper for unmanaged Smtp class

// s_Server   = hostname of SMTP server
// u16_Port   = 0 to use the default port, otherwise the user defined port
// e_Security = The encryption mode
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
Smtp::Smtp(String* s_Server, UInt16 u16_Port, eSecurity e_Security, bool b_AllowInvalidCerts)
{
    try
    {
        mpi_Smtp = new cSmtp(StrW(s_Server), u16_Port, (cCommon::eSecurity)e_Security, b_AllowInvalidCerts, IDR_ROOT_CA);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

Smtp::~Smtp()
{
    Dispose();
}

void Smtp::Dispose()
{
    try
    {
        if (mpi_Smtp) delete mpi_Smtp;
        mpi_Smtp = NULL;
    }
    catch (...)
    {
    }
}

void Smtp::SetAuthData(String* s_User, String* s_Password)
{
    try
    {
        mpi_Smtp->SetAuthData(StrW(s_User), StrW(s_Password));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

void Smtp::Send(EmailBuilder* i_Email)
{
    try
    {
        mpi_Smtp->Send((cEmailBuilder*)(void*)i_Email->Internal);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

} // namespace vmimeNET

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_SMTP