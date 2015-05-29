
#include "stdafx.h"
#include "resource.h"
#include "EmailParser.hpp"
#include "Common.hpp"

#pragma managed

namespace vmimeNET {

// A managed wrapper for unmanaged EmailParser class

// Do not use this constructor "manually"! 
// Call Pop3.FetchEmailAt() to get an instance of EmailParser instead!
EmailParser::EmailParser(IntPtr p_Internal)
{
    try
    {
        mpi_Email = (cEmailParser*)(void*)p_Internal;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

EmailParser::~EmailParser()
{
    Dispose();
}

void EmailParser::Dispose()
{
    try
    {
        if (mpi_Email) delete mpi_Email;
        mpi_Email = NULL;
    }
    catch (...)
    {
    }
}

// Deletes the email on the server. 
// Connects to the server (POP3 command DELE)
// ATTENTION: If Pop3.Close() is not called afterwards the email(s) will not be deleted!
void EmailParser::Delete()
{
    try
    {
        mpi_Email->Delete();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// Gets the unique identifier that the server has assigned to this email.
// Connects to the server (POP3 command UIDL)
String* EmailParser::GetUID()
{
    try
    {
        return new String(mpi_Email->GetUID().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// Gets the size of the email in Bytes
// Connects to the server (POP3 command LIST)
UInt32 EmailParser::GetSize()
{
    try
    {
        return mpi_Email->GetSize();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// #############################################################################################
//                                       Message Header
//                  uses only the meail header (fast to download, POP3 uses TOP command)
// #############################################################################################

// Gets the zero-based index of this email in the server's folder (e.g. Inbox)
UInt32 EmailParser::GetIndex()
{
    try
    {
        return mpi_Email->GetIndex();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

EmailParser::eFlags EmailParser::GetFlags()
{
    try
    {
        return (eFlags)mpi_Email->GetFlags();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// For the email "John Miller <jmiller@gmail.com>"
// returns String[] with 
// String[0] = "jmiller@gmail.com" and 
// String[1] = "John Miller"
String* EmailParser::GetFrom() []
{
    try
    {
        wstring s_Name;
        wstring s_Email = mpi_Email->GetFrom(&s_Name);

        // The very awkward way of managed C++ to say "new String[2]"
	    String* s_Array[] = static_cast<String*[]>(Array::CreateInstance(__typeof(String), 2));
        s_Array[0] = new String(s_Email.c_str());
        s_Array[1] = new String(s_Name .c_str());
        return s_Array;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}
// For the email "John Miller <jmiller@gmail.com>"
// returns String[] with 
// String[0] = "jmiller@gmail.com" and 
// String[1] = "John Miller"
String* EmailParser::GetSender() []
{
    try
    {
        wstring s_Name;
        wstring s_Email = mpi_Email->GetSender(&s_Name);

        // The very awkward way of managed C++ to say "new String[2]"
	    String* s_Array[] = static_cast<String*[]>(Array::CreateInstance(__typeof(String), 2));
        s_Array[0] = new String(s_Email.c_str());
        s_Array[1] = new String(s_Name .c_str());
        return s_Array;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}
// For the email "John Miller <jmiller@gmail.com>"
// returns String[] with 
// String[0] = "jmiller@gmail.com" and 
// String[1] = "John Miller"
String* EmailParser::GetReplyTo() []
{
    try
    {
        wstring s_Name;
        wstring s_Email = mpi_Email->GetReplyTo(&s_Name);

        // The very awkward way of managed C++ to say "new String[2]"
	    String* s_Array[] = static_cast<String*[]>(Array::CreateInstance(__typeof(String), 2));
        s_Array[0] = new String(s_Email.c_str());
        s_Array[1] = new String(s_Name .c_str());
        return s_Array;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}


void EmailParser::GetTo(List<String*>* i_Emails, List<String*>* i_Names)
{
    try
    {
        vector<wstring> i_VectMails;
        vector<wstring> i_VectNames;

        mpi_Email->GetTo(&i_VectMails, &i_VectNames);

        for (size_t i=0; i<i_VectMails.size(); i++)
        {
            i_Emails->Add(new String(i_VectMails.at(i).c_str()));
            i_Names ->Add(new String(i_VectNames.at(i).c_str()));
        }
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}
void EmailParser::GetCc(List<String*>* i_Emails, List<String*>* i_Names)
{
    try
    {
        vector<wstring> i_VectMails;
        vector<wstring> i_VectNames;

        mpi_Email->GetCc(&i_VectMails, &i_VectNames);

        for (size_t i=0; i<i_VectMails.size(); i++)
        {
            i_Emails->Add(new String(i_VectMails.at(i).c_str()));
            i_Names ->Add(new String(i_VectNames.at(i).c_str()));
        }
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}


String* EmailParser::GetSubject()
{
    try
    {
        return new String(mpi_Email->GetSubject().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

String* EmailParser::GetOrganization()
{
    try
    {
        return new String(mpi_Email->GetOrganization().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

String* EmailParser::GetUserAgent()
{
    try
    {
        return new String(mpi_Email->GetUserAgent().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// returns local time and the timezone in minutes (+/-) deviation from GMT.
// returns DateTime::MinValue (00:00 at January 1, 0001) if no DATE field in the header!
DateTime EmailParser::GetDate(Int32* s32_Timezone)
{
    try
    {
        DateTime i_Date = DateTime::MinValue;
        *s32_Timezone  = 0;

        vmime::ref<const vmime::datetime> p_Date = mpi_Email->GetDate();
        if (p_Date)
        {
            i_Date = DateTime(p_Date->getYear(), p_Date->getMonth(),  p_Date->getDay(),
                              p_Date->getHour(), p_Date->getMinute(), p_Date->getSecond());

            *s32_Timezone = p_Date->getZone();
        }

        return i_Date;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// #############################################################################################
//                                     Message Body
//    requires the entire message (may be many Megabytes to download, POP3 uses RETR command)
// #############################################################################################

// Gets the entire email (for example to store it in a *.eml file)
String* EmailParser::GetEmail()
{
    try
    {
        return new String(mpi_Email->GetEmail().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

String* EmailParser::GetHtmlText()
{
    try
    {
        return new String(mpi_Email->GetHtmlText().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

String* EmailParser::GetPlainText()
{
    try
    {
        return new String(mpi_Email->GetPlainText().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// -----------------------

UInt32 EmailParser::GetEmbeddedObjectCount()
{
    try
    {
        return mpi_Email->GetEmbeddedObjectCount();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

bool EmailParser::GetEmbeddedObjectAt(UInt32 u32_Index, String** s_Id, String** s_MimeType, [Out] Byte (*u8_Data)__gc[])
{
    try
    {
        wstring id, mimeType;
        string  data;
        bool b_Success = mpi_Email->GetEmbeddedObjectAt(u32_Index, id, mimeType, data);

        *s_Id       = new String(id.c_str());
        *s_MimeType = new String(mimeType.c_str());

        *u8_Data = new Byte[data.size()];
        Marshal::Copy((IntPtr)(void*)data.c_str(), *u8_Data, 0, (int)data.size());

        return b_Success;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// -----------------------

UInt32 EmailParser::GetAttachmentCount()
{
    try
    {
        return mpi_Email->GetAttachmentCount();
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

bool EmailParser::GetAttachmentAt(UInt32 u32_Index, String** s_Name, String** s_MimeType, [Out] Byte (*u8_Data)__gc[])
{
    try
    {
        wstring name, mimeType;
        string  data;
        bool b_Success = mpi_Email->GetAttachmentAt(u32_Index, name, mimeType, data);

        *s_Name     = new String(name.c_str());
        *s_MimeType = new String(mimeType.c_str());

        *u8_Data = new Byte[data.size()];
        Marshal::Copy((IntPtr)(void*)data.c_str(), *u8_Data, 0, (int)data.size());

        return b_Success;
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}


} // namespace vmimeNET