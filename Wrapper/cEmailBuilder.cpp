
//
// EmailBuilder class
// Copyright (C) Elmue
//
// This class is a wrapper that makes it easier to use the vmime library.
// NOTE: You should access this class only from the same thread that has created the instance.
//

#include "stdafx.h"
#include "cEmailBuilder.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

#pragma unmanaged

namespace vmime   {
namespace wrapper {

// u16_From = "jmiller@gmail.com"  or
// u16_From = "John Miller <jmiller@gmail.com>"
// u16_ResIdMime = The IDR identifier in the RCDATA Resources of "MimeTypes.txt". (IDR_MIME_TYPES)
cEmailBuilder::cEmailBuilder(const wchar_t* u16_From, const wchar_t* u16_Subject, vmime_uint16 u16_ResIdMime)
{
    try
    {
        // Pass the address of any function inside the DLL or Exe file and get the module handle
        HMODULE h_Module;
        if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (WCHAR*)&SetTraceCallback, &h_Module))
            throw std::exception("Could not obtain HINSTANCE of module.");

        // Load the file MimeTypes.txt from the binary resources in the executable
        HRSRC   h_Src  = FindResource  (h_Module, MAKEINTRESOURCE(u16_ResIdMime), RT_RCDATA);
        DWORD u32_Size = SizeofResource(h_Module, h_Src);
        HGLOBAL h_Glob = LoadResource  (h_Module, h_Src);
        char*  s8_Rsrc = (char*)LockResource(h_Glob); // Unlock is not required (see MSDN)
        if (!s8_Rsrc)
            throw std::exception("Compilation error: The resource RCDATA\\IDR_MIME_TYPES does not exist.");

        // Copy to a string because s8_Rsrc is not zero terminated!
        ms_ResourceData.assign(s8_Rsrc, u32_Size);

        // ------

        mi_Builder.setExpeditor(*GetMailbox(u16_From));
        mi_Builder.setSubject  (text(UTF(u16_Subject), charsets::UTF_8));

        mb_UseHtml = false;

        // Set the content-type to "text/html"
        mi_Builder.constructTextPart(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_HTML));
        mi_HtmlPart = mi_Builder.getTextPart().dynamicCast<htmlTextPart>();
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------------------

void cEmailBuilder::SetHeaderField(eHeaderField e_Field, const wchar_t* u16_Value)
{
    try
    {
        switch (e_Field)
        {
            case Head_ReturnPath:   ms_ReturnPath   = UTF(u16_Value); break;
            case Head_ReplyTo:      ms_ReplyTo      = UTF(u16_Value); break;
            case Head_Organization: ms_Organization = UTF(u16_Value); break;
            case Head_UserAgent:    ms_UserAgent    = UTF(u16_Value); break;
        }
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------------------

// u16_Email = "jmiller@gmail.com"  or
// u16_Email = "John Miller <jmiller@gmail.com>"
void cEmailBuilder::AddTo(const wchar_t* u16_Email)
{
    try
    {
        mi_Builder.getRecipients().appendAddress(GetMailbox(u16_Email));
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

void cEmailBuilder::AddCc(const wchar_t* u16_Email)
{
    try
    {
        mi_Builder.getCopyRecipients().appendAddress(GetMailbox(u16_Email));
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

void cEmailBuilder::AddBcc(const wchar_t* u16_Email)
{
    try
    {
        mi_Builder.getBlindCopyRecipients().appendAddress(GetMailbox(u16_Email));
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------------------

// if u16_MimeType == "" -> get mime type from file extension in u16_Path.
// u16_FileName is the name that is stored in the email to be displayed for the recipient.
// if u16_FileName == "" -> get filename from u16_Path.
void cEmailBuilder::AddAttachment(const wchar_t* u16_Path, const wchar_t* u16_MimeType, const wchar_t* u16_FileName)
{
    try
    {
	    ref<fileAttachment> i_Att = create<fileAttachment>(UTF(u16_Path), GetMediaType(u16_MimeType, u16_Path));

        if (!u16_FileName[0])
        {
            u16_FileName = wcsrchr(u16_Path, '\\');
            if (!u16_FileName) 
                throw std::exception("A full path must be specified.");

            u16_FileName ++; // filename starts behind the last backslash
        }

        i_Att->getFileInfo().setFilename(UTF(u16_FileName));

	    mi_Builder.attach(i_Att);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// if u16_MimeType == "" or == NULL -> get mime type from file extension in u16_Path.
// u16_CID = "CompanyLogo" (A unique identifier for the embedded object)
// This may be referenced in HTML code (SetHtmlText()) like this: <img src="cid:CompanyLogo"/>
void cEmailBuilder::AddEmbeddedObject(const wchar_t* u16_Path, const wchar_t* u16_MimeType, const wchar_t* u16_CID)
{
    try
    {
        if (!mi_HtmlPart)
            throw std::exception("AddEmbeddedObject() must be called before BuildMessage() / Generate().");

        mi_HtmlPart->addObjectFile(UTF(u16_Path), GetMediaType(u16_MimeType, u16_Path), UTF(u16_CID));
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------------------

void cEmailBuilder::SetPlainText(const wchar_t* u16_Plain)
{
    try
    {
        ms_PlainText = UTF(u16_Plain);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

void cEmailBuilder::SetHtmlText(const wchar_t* u16_Html)
{
    try
    {
        if (!mi_HtmlPart)
            throw std::exception("SetHtmlText() must be called before BuildMessage() / Generate().");

        mi_HtmlPart->setText(create <stringContentHandler>(UTF(u16_Html)));
        mb_UseHtml = (u16_Html[0] != 0);
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// -----------------------------------

ref<message> cEmailBuilder::BuildMessage()
{
    try
    {
        ref <stringContentHandler> i_PlainTxt = create<stringContentHandler>(ms_PlainText);

        if (mb_UseHtml)
        {
            // content-type = "text/html"
            mi_HtmlPart->setPlainText(i_PlainTxt);
        }
        else
        {
            mi_HtmlPart = NULL;
            // Change the content-type to "text/plain"
            mi_Builder.constructTextPart(mediaType(mediaTypes::TEXT, mediaTypes::TEXT_PLAIN));

            ref<plainTextPart> i_PlainPart = mi_Builder.getTextPart().dynamicCast<plainTextPart>();
            i_PlainPart->setText(i_PlainTxt);
        }

        ref<message> i_Msg = mi_Builder.construct();

        if (ms_ReplyTo     .length()) i_Msg->getHeader()->ReplyTo()     ->setValue(ms_ReplyTo);
        if (ms_ReturnPath  .length()) i_Msg->getHeader()->ReturnPath()  ->setValue(ms_ReturnPath);
        if (ms_Organization.length()) i_Msg->getHeader()->Organization()->setValue(ms_Organization);
        if (ms_UserAgent   .length()) i_Msg->getHeader()->UserAgent()   ->setValue(ms_UserAgent);

        return i_Msg;
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

std::wstring cEmailBuilder::Generate()
{
    try
    {
        ref<message> i_Msg = BuildMessage();
        return UNI(i_Msg->generate());
    }
    catch (...)
    {
        // http://stackoverflow.com/questions/21346400/destructors-not-executed-no-stack-unwinding-when-exception-is-thrown
        throw; 
    }
}

// ############################################################################################
//                                     PRIVATE
// ############################################################################################

mediaType cEmailBuilder::GetMediaType(const wchar_t* u16_MimeType, const wchar_t* u16_Path)
{
    if (u16_MimeType[0]) // The user has specified a mime type -> use it
        return mediaType(UTF(u16_MimeType));

    const wchar_t* u16_Ext = wcsrchr(u16_Path, '.');
    if (!u16_Ext || !u16_Ext[0])
        throw std::exception("Either a file extension is required or the mime type must be specified.");

    // Unicode -> Ansi
    std::string s_Search = "\n";
    while ((++u16_Ext)[0])
    {
        s_Search.append(1, (char)u16_Ext[0]);
    }
    s_Search.append(1, '='); // e.g. "\nzip="

    int s32_Start = (int)ms_ResourceData.find(s_Search);
    if (s32_Start < 0)
        throw std::exception("The mime type could not be determined automatically from the file extension. Please specify the mime type manually.");

    s32_Start += s_Search.length();

    int s32_End = (int)ms_ResourceData.find('\n', s32_Start);
    if (s32_End < 0)
        throw std::exception("Resource 'MimeTypes.txt' is corrupt.");

    return mediaType(ms_ResourceData.substr(s32_Start, s32_End - s32_Start));
}

// Extracts Name and Email from "John Miller <jmiller@gmail.com>"
ref<mailbox> cEmailBuilder::GetMailbox(const wchar_t* u16_Address)
{
    string s_Address = UTF(u16_Address);

    int s32_Start = s_Address.find('<');
    int s32_End   = s_Address.find('>');

    if (s32_Start > 0 && s32_End > s32_Start)
    {
        string s_Email = s_Address.substr(s32_Start +1, s32_End - s32_Start -1);
        string s_Name  = s_Address.substr(0, s32_Start);

        return create<mailbox>(text(s_Name, charsets::UTF_8), s_Email);
    }
    else
    {
        return create<mailbox>(s_Address);
    }
}

} // namespace wrapper
} // namespace vmime

