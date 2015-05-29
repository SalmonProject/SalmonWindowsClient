
#pragma once

#include "../Wrapper/cEmailParser.hpp"

#pragma managed

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace vmime::wrapper;

namespace vmimeNET
{
    // A managed wrapper for unmanaged EmailParser class
    public __gc class EmailParser : public IDisposable
	{
	public:
        __value enum eFlags
        {
            None    = 0,
		    Seen    = vmime::net::message::FLAG_SEEN,    // Message has been seen.
		    Recent  = vmime::net::message::FLAG_RECENT,  // Message has been recently received.
		    Deleted = vmime::net::message::FLAG_DELETED, // Message is marked for deletion.
		    Replied = vmime::net::message::FLAG_REPLIED, // User replied to this message.
		    Marked  = vmime::net::message::FLAG_MARKED,  // Used-defined flag.
		    Passed  = vmime::net::message::FLAG_PASSED,  // Message has been resent/forwarded/bounced.
		    Draft   = vmime::net::message::FLAG_DRAFT,   // Message is marked as a 'draft'.
        };

        EmailParser(IntPtr p_Internal);
        ~EmailParser();
        void Dispose();

        String*   GetUID();
        UInt32    GetSize();
        UInt32    GetIndex();
        eFlags    GetFlags();

        String*   GetFrom   () [];
        String*   GetSender () [];
        String*   GetReplyTo() [];
        void      GetTo(List<String*>* i_Emails, List<String*>* i_Names);
        void      GetCc(List<String*>* i_Emails, List<String*>* i_Names);

        String*   GetSubject();
        String*   GetOrganization();
        String*   GetUserAgent();
        DateTime  GetDate([Out] Int32* s32_Timezone);

        String*   GetHtmlText();
        String*   GetPlainText();

        String*   GetEmail();

        UInt32    GetEmbeddedObjectCount();
        bool      GetEmbeddedObjectAt(UInt32 u32_Index, [Out] String** s_Id, [Out] String** s_MimeType, [Out] Byte (*u8_Data) __gc[]);

        UInt32    GetAttachmentCount();
        bool      GetAttachmentAt(UInt32 u32_Index, [Out] String** s_Name, [Out] String** s_MimeType, [Out] Byte (*u8_Data) __gc[]);

        // ------------------

        void Delete();


    private:
        cEmailParser* mpi_Email;
	};
}

