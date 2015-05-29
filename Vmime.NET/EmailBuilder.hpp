
#pragma once

#include "../Wrapper/cEmailBuilder.hpp"

#pragma managed

using namespace System;
using namespace vmime::wrapper;

namespace vmimeNET
{
    // A managed wrapper for unmanaged EmailBuilder class
	public __gc class EmailBuilder : public IDisposable
	{
	public:
        __value enum eHeaderField : Int32
		{
            ReturnPath   = cEmailBuilder::Head_ReturnPath,
            ReplyTo      = cEmailBuilder::Head_ReplyTo,
            Organization = cEmailBuilder::Head_Organization,
            UserAgent    = cEmailBuilder::Head_UserAgent,
        };

        EmailBuilder(String* s_From, String* s_Subject);
        ~EmailBuilder();
        void Dispose();

        void SetHeaderField(eHeaderField e_Field, String* s_Value);

        void AddTo (String* s_Email);
        void AddCc (String* s_Email);
        void AddBcc(String* s_Email);

        void AddAttachment    (String* s_Path, String* s_MimeType, String* s_FileName);
        void AddEmbeddedObject(String* s_Path, String* s_MimeType, String* s_CID);

        void SetPlainText(String* s_Plain);
        void SetHtmlText (String* s_Html);

        String* Generate();

        // As Managed C++ does not allow to declare friend classes we must expose this public
        __property IntPtr get_Internal();

    private:
        cEmailBuilder* mpi_Email;
	};
}

