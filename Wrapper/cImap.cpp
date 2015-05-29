
//
// Imap class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#include "stdafx.h"
#include "cImap.hpp"
#include "cCertVerifier.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

#pragma unmanaged

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

namespace vmime   {
namespace wrapper {

// u16_Server = hostname of IMAP server
// u16_Port   = 0 to use the default port, otherwise the user defined port
// e_Security = The encryption mode
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
// u16_ResIdCert = The IDR identifier in the RCDATA Resources of RootCA.txt. (IDR_ROOT_CA)
cImap::cImap(const wchar_t* u16_Server, vmime_uint16 u16_Port, cCommon::eSecurity e_Security,  
             bool  b_AllowInvalidCerts, vmime_uint16 u16_ResIdCert)
{
    ms_Server      = UTF(u16_Server);
    mu16_Port      = u16_Port;
    me_Security    = e_Security;
    mu16_ResIdCert = u16_ResIdCert;
    ms_Folder      = L"INBOX";
    mb_AllowInvalidCerts = b_AllowInvalidCerts;
}

cImap::~cImap()
{
    // Erase password from memory
    memset((void*)ms_Password.c_str(), 0, ms_Password.length());
}

void cImap::SetAuthData(const wchar_t* u16_User, const wchar_t* u16_Password)
{
    ms_User     = UTF(u16_User);
    ms_Password = UTF(u16_Password);
}

// Disconect from the server
void cImap::Close()
{
    try
    {
        if (mi_Folder)
            mi_Folder->close(true); // disconnects one connection from the server
         
        // call the destructors
        mi_Folder  = NULL;
        mi_Store   = NULL; // destructor disconnects the last connection from the server
        mi_Session = NULL;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

//Salmon: effect deletion of emails without closing folder
void cImap::expunge()
{
	try
	{
		if (mi_Folder)
			mi_Folder->expunge(); // disconnects one connection from the server
	}
	catch (...)
	{
		// http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
		throw;
	}
}

void cImap::deleteMessages(vector<int> toDel)
{
	mi_Folder->deleteMessages(vmime::net::messageSet::byNumber(toDel));
}

// private
void cImap::Connect()
{
    if (mi_Store)
        return;

    const char* s8_Protocol;
    if (me_Security == cCommon::Secur_SSL) s8_Protocol = "imaps";
    else                                   s8_Protocol = "imap";

    if (mu16_Port == 0)
        mu16_Port = utility::url::UNSPECIFIED_PORT;

    utility::url i_Url(s8_Protocol, ms_Server, mu16_Port, "", ms_User, ms_Password);

    mi_Session = create <net::session>();

    ref<net::store> i_NewStore = mi_Session->getStore(i_Url);

    ref <cCertVerifier> i_Verifier = create<cCertVerifier>(mb_AllowInvalidCerts, mu16_ResIdCert);
    i_NewStore->setCertificateVerifier(i_Verifier);
    i_NewStore->setTimeoutHandlerFactory(create<TimeoutFactory>());

    if (ms_User.length() || ms_Password.length())
    {
        i_NewStore->setProperty("options.need-authentication", true);
    }

    if (me_Security == cCommon::Secur_TLS_opt || me_Security == cCommon::Secur_TLS_force)
    {
        i_NewStore->setProperty("connection.tls", true); // use STLS
    }

    if (me_Security == cCommon::Secur_TLS_force)
    {
        i_NewStore->setProperty("connection.tls.required", true);
    }

    // Send commands "USER" and "PASS" to the server if SASL fails
    i_NewStore->setProperty("options.sasl.fallback", true);

    #if VMIME_TRACE
        TRACE("IMAP Using vmime %s", VMIME_VERSION);
    #endif

    i_NewStore->connect();
    mi_Store = i_NewStore;
}

// --------------------------------------------------------------------

// Get all folders on the server
void cImap::EnumFolders(vector<wstring>& i_FolderList)
{
    try
    {
        Connect();

        vector<ref<net::folder>> i_Folders = mi_Store->getRootFolder()->getFolders(true); // recursive
        for (size_t i=0; i<i_Folders.size(); i++)
        {
            // The folder does not contain emails -> skip it
            if (i_Folders.at(i)->getType() & net::folder::TYPE_CONTAINS_MESSAGES)
            {
                net::folder::path i_Path = i_Folders.at(i)->getFullPath();
                i_FolderList.push_back(UNI(i_Path.toString('/')));
            }
        }
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

//well, the above function threw some exception (once out of like 100+ identical runs of the program), that appeared 
//to be a double free caused by the "smart" pointers...
//we don't even care about anything other than the default folder, so let's just use vmime's own getDefaultFolder().
void cImap::selectDefaultFolder()
{
	try
	{
		if (mi_Folder)
			return; // already selected

		Connect();

		mi_Folder = mi_Store->getDefaultFolder();
		mi_Folder->open(net::folder::MODE_READ_WRITE); // this creates a new connection to the server
		ms_Folder = L"INBOX";
	}
	catch (...)
	{
		// http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
		throw;
	}
}

// s_Path = "[Gmail]/Sent"
// ATTENTION: s_Path is case sensitive!
// If you never call this function the default folder is "INBOX".
void cImap::SelectFolder(const wstring s_Path)
{
    try
    {
        if (mi_Folder && ms_Folder == s_Path)
            return; // already selected

        Connect();

        if (s_Path.find('\\') != string::npos)
            throw exception("Please use slashes instead of backslashes in folder paths!");

        utility::path i_Path = utility::path::fromString('/', UTF(s_Path));
        ref<net::folder> i_NewFolder = mi_Store->getFolder(i_Path);

        i_NewFolder->open(net::folder::MODE_READ_WRITE); // this creates a new connection to the server

        if (mi_Folder)
            mi_Folder->close(true);

        mi_Folder = i_NewFolder;
        ms_Folder = s_Path;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

wstring cImap::GetCurrentFolder()
{
    return ms_Folder;
}

// --------------------------------------------------------------------

// Gets the count of emails in the folder "INBOX" if SelectFolder() has not been called before.
int cImap::GetEmailCount()
{
    try
    {
        if (!mi_Folder)
            SelectFolder(L"INBOX");

        return mi_Folder->getMessageCount();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// Gets the emails in the folder "INBOX" if SelectFolder() has not been called before.
// Fetches only the header of the email (command FETCH HEADER).
// s32_Index is zero-based!
// IMPORTANT: The returned instance must be freed with delete.
// We cannot return ref<cEmailParser> here because this cannot be stored in a Managed C++ class.
// returns NULL if index invalid
cEmailParser* cImap::FetchEmailAt(int s32_Index)
{
    try
    {
        if (!mi_Folder)
            SelectFolder(L"INBOX");

        if (s32_Index < 0 || s32_Index >= mi_Folder->getMessageCount())
            return NULL;
            
        // Email index on the server is one-based!
        ref<net::message> i_Msg = mi_Folder->getMessage(s32_Index +1);

        mi_Folder->fetchMessage(i_Msg, net::folder::FETCH_FULL_HEADER | 
                                       net::folder::FETCH_FLAGS | 
                                       net::folder::FETCH_SIZE  | 
                                       net::folder::FETCH_UID);

        return new cEmailParser(mi_Folder, i_Msg, "IMAP");
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}


} // namespace wrapper
} // namespace vmime

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_IMAP

