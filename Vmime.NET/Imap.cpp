
#include "stdafx.h"
#include "resource.h"
#include "Imap.hpp"
#include "Common.hpp"

#pragma managed

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

namespace vmimeNET {

// A managed wrapper for unmanaged Imap class

// s_Server   = hostname of IMAP server
// u16_Port   = 0 to use the default port, otherwise the user defined port
// e_Security = The encryption mode
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
Imap::Imap(String* s_Server, UInt16 u16_Port, eSecurity e_Security, bool b_AllowInvalidCerts)
{
    try
    {
        mpi_Imap = new cImap(StrW(s_Server), u16_Port, (cCommon::eSecurity)e_Security, b_AllowInvalidCerts, IDR_ROOT_CA);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

Imap::~Imap()
{
    Dispose();
}

void Imap::Dispose()
{
    try
    {
        if (mpi_Imap) delete mpi_Imap;
        mpi_Imap = NULL;
    }
    catch (...)
    {
    }
}

void Imap::SetAuthData(String* s_User, String* s_Password)
{
    try
    {
        mpi_Imap->SetAuthData(StrW(s_User), StrW(s_Password));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// --------------------------------------------------------------------

// Get all folders on the server
String* Imap::EnumFolders() []
{
    try
    {
        vector<wstring> i_Folders;
        mpi_Imap->EnumFolders(i_Folders);

        // The very awkward way of managed C++ to say "new String[n]"
        String* s_Array[] = static_cast<String*[]>(Array::CreateInstance(__typeof(String), i_Folders.size()));

        for (size_t i=0; i<i_Folders.size(); i++)
        {
            s_Array[i] = new String(i_Folders.at(i).c_str());
        }

        return s_Array;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// s_Path = "[Gmail]/Sent"
// ATTENTION: s_Path is case sensitive!
// If you never call this function the default folder is "INBOX".
void Imap::SelectFolder(String* s_Path)
{
    try
    {
        mpi_Imap->SelectFolder(StrW(s_Path));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

String* Imap::GetCurrentFolder()
{
    try
    {
        return new String(mpi_Imap->GetCurrentFolder().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// --------------------------------------------------------------------

// Gets the count of emails in the folder "INBOX" if SelectFolder() has not been called before.
int Imap::GetEmailCount()
{
    try
    {
        return mpi_Imap->GetEmailCount();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// Gets the emails in the folder "INBOX" if SelectFolder() has not been called before.
// Fetches only the header of the email (command FETCH HEADER).
// s32_Index is zero-based!
// returns null if index invalid
EmailParser* Imap::FetchEmailAt(int s32_Index)
{
    try
    {   
        cEmailParser* p_Email = mpi_Imap->FetchEmailAt(s32_Index);
        if (!p_Email)
            return NULL;

        return new EmailParser((IntPtr)p_Email);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// Disconect from the server
void Imap::Close()
{
    try
    {
        mpi_Imap->Close();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

} // namespace vmimeNET

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP