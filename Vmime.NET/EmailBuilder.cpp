
#include "stdafx.h"
#include "resource.h"
#include "EmailBuilder.hpp"
#include "Common.hpp"

#pragma managed

namespace vmimeNET {

// A managed wrapper for unmanaged EmailBuilder class

// s_From = "jmiller@gmail.com"  or
// s_From = "John Miller <jmiller@gmail.com>"
EmailBuilder::EmailBuilder(String* s_From, String* s_Subject)
{
    try
    {
        mpi_Email = new cEmailBuilder(StrW(s_From), StrW(s_Subject), IDR_MIME_TYPES);
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

EmailBuilder::~EmailBuilder()
{
    Dispose();
}

void EmailBuilder::Dispose()
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

// As Managed C++ does not allow to declare friend classes we must expose this public
IntPtr EmailBuilder::get_Internal()
{
    return (IntPtr)mpi_Email;
}

void EmailBuilder::SetHeaderField(eHeaderField e_Field, String* s_Value)
{
    try
    {
        mpi_Email->SetHeaderField((cEmailBuilder::eHeaderField)e_Field, StrW(s_Value));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// s_Email = "jmiller@gmail.com"  or
// s_Email = "John Miller <jmiller@gmail.com>"
void EmailBuilder::AddTo(String* s_Email)
{
    try
    {
        mpi_Email->AddTo(StrW(s_Email));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// s_Email = "jmiller@gmail.com"  or
// s_Email = "John Miller <jmiller@gmail.com>"
void EmailBuilder::AddCc(String* s_Email)
{
    try
    {
        mpi_Email->AddCc(StrW(s_Email));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

// s_Email = "jmiller@gmail.com"  or
// s_Email = "John Miller <jmiller@gmail.com>"
void EmailBuilder::AddBcc(String* s_Email)
{
    try
    {
        mpi_Email->AddBcc(StrW(s_Email));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

void EmailBuilder::AddAttachment(String* s_Path, String* s_MimeType, String* s_FileName)
{
    try
    {
        mpi_Email->AddAttachment(StrW(s_Path), StrW(s_MimeType), StrW(s_FileName));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

void EmailBuilder::AddEmbeddedObject(String* s_Path, String* s_MimeType, String* s_CID)
{
    try
    {
        mpi_Email->AddEmbeddedObject(StrW(s_Path), StrW(s_MimeType), StrW(s_CID));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

void EmailBuilder::SetPlainText(String* s_Plain)
{
    try
    {
        mpi_Email->SetPlainText(StrW(s_Plain));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

void EmailBuilder::SetHtmlText(String* s_Html)
{
    try
    {
        mpi_Email->SetHtmlText(StrW(s_Html));
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

String* EmailBuilder::Generate()
{
    try
    {
        return new String(mpi_Email->Generate().c_str());
    }
    catch (std::exception& Ex)
    {
        throw new Exception(UNI(Ex.what()).c_str());
    }
}

} // namespace vmimeNET