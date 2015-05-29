
#include "stdafx.h"
#include "resource.h"
#include "Pop3.hpp"
#include "Common.hpp"

#pragma managed

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

namespace vmimeNET {

// A managed wrapper for unmanaged Pop3 class

// s_Server   = hostname of POP3 server
// u16_Port   = 0 to use the default port, otherwise the user defined port
// e_Security = The encryption mode
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
Pop3::Pop3(String* s_Server, UInt16 u16_Port, eSecurity e_Security, bool b_AllowInvalidCerts)
{
    try
    {
        mpi_Pop3 = new cPop3(StrW(s_Server), u16_Port, (cCommon::eSecurity)e_Security, b_AllowInvalidCerts, IDR_ROOT_CA);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

Pop3::~Pop3()
{
    Dispose();
}

void Pop3::Dispose()
{
    try
    {
        if (mpi_Pop3) delete mpi_Pop3;
        mpi_Pop3 = NULL;
    }
    catch (...)
    {
    }
}

void Pop3::SetAuthData(String* s_User, String* s_Password)
{
    try
    {
        mpi_Pop3->SetAuthData(StrW(s_User), StrW(s_Password));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

int Pop3::GetEmailCount()
{
    try
    {
        return mpi_Pop3->GetEmailCount();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// Fetches only the header of the email (command TOP)
// u32_Index is zero-based!
// returns null if index invalid
EmailParser* Pop3::FetchEmailAt(int s32_Index)
{
    try
    {   
        cEmailParser* p_Email = mpi_Pop3->FetchEmailAt(s32_Index);
        if (!p_Email)
            return NULL;

        return new EmailParser((IntPtr)p_Email);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// Close()must be called explicitely, otherwise messages marked as deleted are not expunged from the server.
void Pop3::Close()
{
    try
    {
        mpi_Pop3->Close();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

} // namespace vmimeNET

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3