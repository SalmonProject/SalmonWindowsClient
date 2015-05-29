
//
// EmailBuilder class
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

class cEmailBuilder : public object
{
public:
    enum eHeaderField
    {
        Head_ReturnPath,
        Head_ReplyTo,
        Head_Organization,
        Head_UserAgent,
    };

    cEmailBuilder(const wchar_t* u16_From, const wchar_t* u16_Subject, vmime_uint16 u16_ResIdMime);

    void SetHeaderField(eHeaderField e_Field, const wchar_t* u16_Value);

    void AddTo (const wchar_t* u16_Email);
    void AddCc (const wchar_t* u16_Email);
    void AddBcc(const wchar_t* u16_Email);

    void AddAttachment    (const wchar_t* u16_Path, const wchar_t* u16_MimeType, const wchar_t* u16_FileName);
    void AddEmbeddedObject(const wchar_t* u16_Path, const wchar_t* u16_MimeType, const wchar_t* u16_CID);

    void SetPlainText(const wchar_t* u16_Plain);
    void SetHtmlText (const wchar_t* u16_Html);

    ref<message> BuildMessage();
    wstring Generate();

private:
    mediaType    GetMediaType(const wchar_t* u16_MimeType, const wchar_t* u16_Path);
    ref<mailbox> GetMailbox(const wchar_t* u16_EmailWithName);

    messageBuilder     mi_Builder;
    ref<htmlTextPart>  mi_HtmlPart;

    bool    mb_UseHtml;
    string  ms_PlainText;
    string  ms_UserAgent;
    string  ms_ReturnPath;
    string  ms_ReplyTo;
    string  ms_Organization;
    string  ms_ResourceData;
};

} // namespace wrapper
} // namespace vmime
