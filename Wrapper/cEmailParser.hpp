
//
// EmailHeader class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#pragma once

// NEVER include windows.h in a header file in a Managed C++ project !!
#include "cCommon.hpp"

#pragma unmanaged

namespace vmime   {
namespace wrapper {

class cEmailParser : public object
{
public:
    cEmailParser(ref<net::folder> i_Folder, ref<net::message> i_Msg, const char* s8_Protocol);

    wstring             GetUID();
    vmime_uint32        GetSize();
    vmime_uint32        GetIndex();
    net::message::Flags GetFlags();

    wstring GetFrom   (wstring* ps_Name=NULL);
    wstring GetSender (wstring* ps_Name=NULL);
    wstring GetReplyTo(wstring* ps_Name=NULL);
    void    GetTo(vector<wstring>* pi_Emails, vector<wstring>* pi_Names=NULL);
    void    GetCc(vector<wstring>* pi_Emails, vector<wstring>* pi_Names=NULL);

    wstring GetSubject();
    wstring GetOrganization();
    wstring GetUserAgent();
    ref<const datetime> GetDate();

    wstring GetHtmlText();
    wstring GetPlainText();

    wstring GetEmail();

    vmime_uint32 GetEmbeddedObjectCount();
    bool         GetEmbeddedObjectAt(vmime_uint32 u32_Index, wstring& s_Id, wstring& s_ContentType, string& s_Data);

    vmime_uint32 GetAttachmentCount();
    bool         GetAttachmentAt(vmime_uint32 u32_Index, wstring& s_Name, wstring& s_ContentType, string& s_Data);

    // ------------------

    void Delete();

private:
    wstring             GetFieldTypeMailbox (const char* s8_Field, wstring* ps_Name=NULL);
    void                GetFieldTypeAddrList(const char* s8_Field, vector<wstring>* pi_Emails, vector<wstring>* pi_Names);
    wstring             GetFieldTypeText    (const char* s8_Field);
    ref<const datetime> GetFieldTypeDateTime(const char* s8_Field);

    ref<const htmlTextPart> GetHtmlTextPart();
    void RetrieveEntireMessage();

    ref<net::folder>   mi_Folder;   // The folder where this message has been downloaded
    ref<net::message>  mi_MsgHead;  // contains only the header of the email (faster to download)
    ref<messageParser> mi_Parser;   // contains the entire message (may be slow to get)
    string             ms_Message;  // contains the entire message
    string             ms_Protocol;
};

} // namespace wrapper
} // namespace vmime
