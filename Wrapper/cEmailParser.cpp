
//
// Email class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#include "stdafx.h"
#include "cEmailParser.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

#pragma unmanaged

namespace vmime   {
namespace wrapper {

// Do not use this constructor "manually"! 
// Call cPop3::FetchEmailAt() to get an instance of cEmailParser instead!
// s8_Protocol = "POP3" or "IMAP" for Trace output
cEmailParser::cEmailParser(ref<net::folder> i_Folder, ref<net::message> i_Msg, const char* s8_Protocol)
{
    mi_Folder   = i_Folder;
    mi_MsgHead  = i_Msg;
    ms_Protocol = s8_Protocol;
}

// Deletes the email on the server.
// Connects to the server (POP3 command DELE)
// ATTENTION: If cPop3::Close() is not called afterwards the email(s) will not be deleted!
// On IMAP deletes immediately.
void cEmailParser::Delete()
{
    try
    {
        // POP3 does not support deleting by UID
        // IMAP allows deleting by UID or by Number. We chose by UID because it is safer.
        if (ms_Protocol == "POP3")
            mi_Folder->deleteMessages(net::messageSet::byNumber(mi_MsgHead->getNumber()));
        else
            mi_Folder->deleteMessages(net::messageSet::byUID(mi_MsgHead->getUID()));
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// Gets the size of the email in Bytes
// POP3 connects to the server (POP3 command LIST)
// IMAP does not connect anew.
vmime_uint32 cEmailParser::GetSize()
{
    try
    {
        return mi_MsgHead->getSize();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// Gets the unique identifier that the server has assigned to this email.
// POP3 connects to the server (POP3 command UIDL)
// IMAP does not connect anew.
wstring cEmailParser::GetUID()
{
    try
    {
        return UNI(mi_MsgHead->getUID());
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// #############################################################################################
//                                   public Message Header
//                  uses only the meail header (fast to download, POP3 uses TOP command)
// #############################################################################################

// Gets the zero-based index of this email in the server's folder (e.g. Inbox)
vmime_uint32 cEmailParser::GetIndex()
{
    try
    {
        return mi_MsgHead->getNumber() -1;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

net::message::Flags cEmailParser::GetFlags()
{
    try
    {
        return (net::message::Flags)mi_MsgHead->getFlags();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// returns the pure email address and optionally the name of "John Miller <jmiller@gmail.com>"
wstring cEmailParser::GetFrom(wstring* ps_Name) // = NULL
{
    try
    {
        return GetFieldTypeMailbox(fields::FROM, ps_Name);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}
wstring cEmailParser::GetSender(wstring* ps_Name) // = NULL
{
    try
    {
        return GetFieldTypeMailbox(fields::SENDER, ps_Name);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}
wstring cEmailParser::GetReplyTo(wstring* ps_Name) // = NULL
{
    try
    {
        return GetFieldTypeMailbox(fields::REPLY_TO, ps_Name);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

void cEmailParser::GetTo(vector<wstring>* pi_Emails, vector<wstring>* pi_Names) // = NULL
{
    try
    {
        GetFieldTypeAddrList(fields::TO, pi_Emails, pi_Names);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

void cEmailParser::GetCc(vector<wstring>* pi_Emails, vector<wstring>* pi_Names) // = NULL
{
    try
    {
        GetFieldTypeAddrList(fields::CC, pi_Emails, pi_Names);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

wstring cEmailParser::GetSubject()
{
    try
    {
        return GetFieldTypeText(fields::SUBJECT);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

wstring cEmailParser::GetOrganization()
{
    try
    {
        return GetFieldTypeText(fields::ORGANIZATION);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

wstring cEmailParser::GetUserAgent()
{
    try
    {
        return GetFieldTypeText(fields::USER_AGENT);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// returns NULL if no DATE field in the header!
ref<const datetime> cEmailParser::GetDate()
{
    try
    {
        return GetFieldTypeDateTime(fields::DATE);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// #############################################################################################
//                                         private
// #############################################################################################

// returns the pure email address and optionally the name of "John Miller <jmiller@gmail.com>"
// returns empty strings if the field does not exist
// May be used for the fields: FROM, SENDER, REPLY_TO, DELIVERED_TO
wstring cEmailParser::GetFieldTypeMailbox(const char* s8_Field, wstring* ps_Name)
{
    try
    {
        ref<const mailbox> i_Mailbox = mi_MsgHead->getHeader()->findField(s8_Field)->getValue().dynamicCast<const mailbox>();

        if (ps_Name)  
           *ps_Name = TOW(i_Mailbox->getName().getConvertedText(charsets::WSTRING));

        return UNI(i_Mailbox->getEmail().toString());
    }
    catch (exceptions::no_such_field&)
    {
        if (ps_Name)  
           *ps_Name  = L"";

        return L"";
    }
}

// returns the pure email address and the name of "John Miller <jmiller@gmail.com>" in two vectors
// May be used for the fields: TO, CC, BCC
void cEmailParser::GetFieldTypeAddrList(const char* s8_Field, vector<wstring>* pi_Emails, vector<wstring>* pi_Names)
{
    try
    {
        ref<const addressList> i_AddrList = mi_MsgHead->getHeader()->findField(s8_Field)->getValue().dynamicCast<const addressList>();
        ref<mailboxList>    i_MailboxList = i_AddrList->toMailboxList();

        for (size_t i=0; i<i_MailboxList->getMailboxCount(); i++)
        {
            ref<mailbox> i_Mailbox = i_MailboxList->getMailboxAt(i);

            if (pi_Emails) pi_Emails->push_back(UNI(i_Mailbox->getEmail().toString()));
            if (pi_Names)  pi_Names ->push_back(TOW(i_Mailbox->getName() .getConvertedText(charsets::WSTRING)));
        }        
    }
    catch (exceptions::no_such_field&)
    {
    }
}

// returns an empty string if the field does not exist
// May be used for the fields: SUBJECT, ORGANIZATION, USER_AGENT, CONTENT_DESCRIPTION, MIME_VERSION, CONTENT_LOCATION
wstring cEmailParser::GetFieldTypeText(const char* s8_Field)
{
    try
    {
        ref<const text> i_Text = mi_MsgHead->getHeader()->findField(s8_Field)->getValue().dynamicCast<const text>();
        return TOW(i_Text->getConvertedText(charsets::WSTRING));
    }
    catch (exceptions::no_such_field&)
    {
        return L"";
    }
}

// returns the datetime in local time and the timezone in minutes of deviation from GMT
// returns NULL if the field does not exist
// May be used for the fields: DATE
ref <const datetime> cEmailParser::GetFieldTypeDateTime(const char* s8_Field)
{
    try
    {
        return mi_MsgHead->getHeader()->findField(s8_Field)->getValue().dynamicCast<const datetime>();
    }
    catch (exceptions::no_such_field&)
    {
        return NULL;
    }
}

// #############################################################################################
//                                   public Message Body
//    requires the entire message (may be many Megabytes to download, POP3 uses RETR command)
// #############################################################################################

// Gets the entire email (for example to store it in a *.eml file)
wstring cEmailParser::GetEmail()
{
    try
    {
        RetrieveEntireMessage();
        return UNI(ms_Message);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

wstring cEmailParser::GetPlainText()
{
    try
    {
        RetrieveEntireMessage();

        string s_Text;
        utility::outputStreamStringAdapter i_Stream(s_Text);

	    for (size_t i=0; i<mi_Parser->getTextPartCount(); i++)
	    {
		    const textPart& i_Part = *mi_Parser->getTextPartAt(i);

		    if (i_Part.getType().getSubType() == mediaTypes::TEXT_HTML)
		    {
			    const htmlTextPart& i_Html = dynamic_cast<const htmlTextPart&>(i_Part);

                i_Html.getPlainText()->extract(i_Stream);
                return charset::CpToWstring(s_Text, i_Html.getCharset());
            }

		    if (i_Part.getType().getSubType() == mediaTypes::TEXT_PLAIN)
		    {
			    const plainTextPart& i_Plain = dynamic_cast<const plainTextPart&>(i_Part);

                i_Plain.getText()->extract(i_Stream);
                return charset::CpToWstring(s_Text, i_Plain.getCharset());
            }
        }
        return L"";
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

wstring cEmailParser::GetHtmlText()
{
    try
    {
        RetrieveEntireMessage();

	    ref<const htmlTextPart> i_Html = GetHtmlTextPart();
        if (!i_Html)
            return L"";

        string s_Text;
        utility::outputStreamStringAdapter i_Stream(s_Text);

        i_Html->getText()->extract(i_Stream);
        return charset::CpToWstring(s_Text, i_Html->getCharset());
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------

vmime_uint32 cEmailParser::GetEmbeddedObjectCount()
{
    try
    {
        RetrieveEntireMessage();

	    ref<const htmlTextPart> i_Html = GetHtmlTextPart();
        if (!i_Html)
            return 0;

        return i_Html->getObjectCount();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// NOTE: s_Data is a string but may contain binary data (e.g. a GIF file)!
// returns false if the object with the given index does not exist
bool cEmailParser::GetEmbeddedObjectAt(vmime_uint32 u32_Index, wstring& s_Id, wstring& s_MimeType, string& s_Data)
{
    try
    {
        RetrieveEntireMessage();

	    ref<const htmlTextPart> i_Html = GetHtmlTextPart();
        
        if (!i_Html || u32_Index >= i_Html->getObjectCount())
        {
            s_Id.clear();
            s_MimeType.clear();
            s_Data.clear();
            return false;
        }

        const htmlTextPart::embeddedObject& i_Object = *i_Html->getObjectAt(u32_Index);

        s_Id       = UNI(i_Object.getId());
        s_MimeType = UNI(i_Object.getType().generate());

        utility::outputStreamStringAdapter i_Stream(s_Data);
        i_Object.getData()->extract(i_Stream);
        return true;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------

vmime_uint32 cEmailParser::GetAttachmentCount()
{
    try
    {
        RetrieveEntireMessage();

        return (vmime_uint32)mi_Parser->getAttachmentCount();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// NOTE: s_Data is a string but may contain binary data (e.g. a PDF file)!
// returns false if the attachment with the given index does not exist
bool cEmailParser::GetAttachmentAt(vmime_uint32 u32_Index, wstring& s_Name, wstring& s_MimeType, string& s_Data)
{
    try
    {
        RetrieveEntireMessage();

        if (u32_Index >= mi_Parser->getAttachmentCount())
        {
            s_Name.clear();
            s_MimeType.clear();
            s_Data.clear();
            return false;
        }

        const attachment& i_Attach = *mi_Parser->getAttachmentAt(u32_Index);

        s_Name     = TOW(i_Attach.getName().getConvertedText(charsets::WSTRING));
        s_MimeType = UNI(i_Attach.getType().generate());

        utility::outputStreamStringAdapter i_Stream(s_Data);
        i_Attach.getData()->extract(i_Stream);
        return true;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// #############################################################################################
//                                         private
// #############################################################################################

// private
void cEmailParser::RetrieveEntireMessage()
{
    if (mi_Parser)
        return;

	std::ostringstream i_Stream;
	utility::outputStreamAdapter i_Adapt(i_Stream);

    TraceProgress i_Progress(1000, ms_Protocol.c_str()); // Trace progress once a second

    // This connects to the server and downloads the entire message (which may be slow for large emails)
    mi_MsgHead->extract(i_Adapt, &i_Progress);

    ms_Message = i_Stream.str();
    mi_Parser  = create<messageParser>(ms_Message);
}

ref<const htmlTextPart> cEmailParser::GetHtmlTextPart()
{
	for (size_t i=0; i<mi_Parser->getTextPartCount(); i++)
	{
		const textPart& i_Part = *mi_Parser->getTextPartAt(i);

		if (i_Part.getType().getSubType() == mediaTypes::TEXT_HTML)
		{
			return create<const htmlTextPart>(dynamic_cast<const htmlTextPart&>(i_Part));
        }
    }
    return NULL;
}


} // namespace wrapper
} // namespace vmime

