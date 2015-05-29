
//
// Pop3 class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#include "stdafx.h"
#include "cPop3.hpp"
#include "cCertVerifier.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

#pragma unmanaged

#if VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

namespace vmime   {
namespace wrapper {

// u16_Server = hostname of POP3 server
// u16_Port   = 0 to use the default port, otherwise the user defined port
// e_Security = The encryption mode
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
// u16_ResIdCert = The IDR identifier in the RCDATA Resources of RootCA.txt. (IDR_ROOT_CA)
cPop3::cPop3(const wchar_t* u16_Server, vmime_uint16 u16_Port, cCommon::eSecurity e_Security,  
             bool  b_AllowInvalidCerts, vmime_uint16 u16_ResIdCert)
{
    ms_Server      = UTF(u16_Server);
    mu16_Port      = u16_Port;
    me_Security    = e_Security;
    mu16_ResIdCert = u16_ResIdCert;
    mb_AllowInvalidCerts = b_AllowInvalidCerts;
}

cPop3::~cPop3()
{
    // Erase password from memory
    memset((void*)ms_Password.c_str(), 0, ms_Password.length());
}

void cPop3::SetAuthData(const wchar_t* u16_User, const wchar_t* u16_Password)
{
    ms_User     = UTF(u16_User);
    ms_Password = UTF(u16_Password);
}

// Close()must be called explicitely, otherwise messages marked as deleted are not expunged from the server.
void cPop3::Close()
{
    try
    {
        if (mi_Inbox)
            mi_Inbox->close(true);
         
        // call the destructors
        mi_Inbox   = NULL;
        mi_Store   = NULL; // destructor disconnects from server
        mi_Session = NULL;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// private
void cPop3::Connect()
{
    if (mi_Inbox)
        return;

    const char* s8_Protocol;
    if (me_Security == cCommon::Secur_SSL) s8_Protocol = "pop3s";
    else                                   s8_Protocol = "pop3";

    if (mu16_Port == 0)
        mu16_Port = utility::url::UNSPECIFIED_PORT;

    utility::url i_Url(s8_Protocol, ms_Server, mu16_Port, "", ms_User, ms_Password);

    mi_Session = create <net::session>();
    mi_Store   = mi_Session->getStore(i_Url);

    ref <cCertVerifier> i_Verifier = create<cCertVerifier>(mb_AllowInvalidCerts, mu16_ResIdCert);
    mi_Store->setCertificateVerifier(i_Verifier);
    mi_Store->setTimeoutHandlerFactory(create<TimeoutFactory>());

    if (ms_User.length() || ms_Password.length())
    {
        mi_Store->setProperty("options.need-authentication", true);
    }

    if (me_Security == cCommon::Secur_TLS_opt || me_Security == cCommon::Secur_TLS_force)
    {
        mi_Store->setProperty("connection.tls", true); // use STLS
    }

    if (me_Security == cCommon::Secur_TLS_force)
    {
        mi_Store->setProperty("connection.tls.required", true);
    }

    // Try to authenticate via APOP (disabled)
    // Requires that the server sends a greeting like:
    // +OK POP3 server ready <1896.697170952@dbc.mtview.ca.us>
    // which is parsed as "<" id-left "@" id-right ">" 
    // otherwise APOP is not executed. 
    // Most servers do not send this type of greeting.
    mi_Store->setProperty("store.pop3.options.apop", false);
    // Some servers close the connection after an unsuccessful APOP command, 
    // so the fallback may not always work!
    mi_Store->setProperty("store.pop3.options.apop.fallback", true);

    // Send commands "USER" and "PASS" to the server if SASL fails
    mi_Store->setProperty("options.sasl.fallback", true);

    #if VMIME_TRACE
        TRACE("POP3 Using vmime %s", VMIME_VERSION);
    #endif

    mi_Store->connect();

    mi_Inbox = mi_Store->getDefaultFolder();
    mi_Inbox->open(net::folder::MODE_READ_WRITE);
}

int cPop3::GetEmailCount()
{
    try
    {
        Connect();

        return mi_Inbox->getMessageCount();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// Fetches only the header of the email (command TOP)
// u32_Index is zero-based!
// IMPORTANT: The returned instance must be freed with delete.
// We cannot return ref<cEmailParser> here because this cannot be stored in a Managed C++ class.
// returns NULL if index invalid
cEmailParser* cPop3::FetchEmailAt(int s32_Index)
{
    try
    {
        Connect();

        if (s32_Index < 0 || s32_Index >= mi_Inbox->getMessageCount())
            return NULL;
            
        // Email index on the server is one-based!
        ref<net::message> i_Msg = mi_Inbox->getMessage(s32_Index +1);

        // Send the TOP command to the server
        // FETCH_SIZE and FETCH_UID are not needed here because POP3 now gets Size and Uid automatically on demand
        // If GetSize() and GetUID() are never called in this class, there will be no additional server connection.
        mi_Inbox->fetchMessage(i_Msg, net::folder::FETCH_FULL_HEADER);

        return new cEmailParser(mi_Inbox, i_Msg, "POP3");
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}


} // namespace wrapper
} // namespace vmime

#endif // VMIME_HAVE_MESSAGING_FEATURES && VMIME_HAVE_MESSAGING_PROTO_POP3

