
#pragma once

#include <string>

#pragma managed

using namespace System;
using namespace System::Runtime::InteropServices;  // Marshal

namespace vmimeNET
{
	public __gc class Common
	{
	public:
        __delegate void delTrace(String* s_Text);
        static void SetTraceCallback(delTrace* f_Callback);

        static void CancelOperation();

    private:
        // make constructor invisible from outside the assembly
        Common()
        {
        }
	};

    // --------------------------------------------------------------------------------

    // Convert managed Strings to unmanaged strings (Wide Char)
	private class StrW
	{
	public:
        StrW(String* s_String)
        {
            mu16_String = (wchar_t*)Marshal::StringToHGlobalUni(s_String).ToPointer();
        }

        ~StrW()
        {
            Marshal::FreeHGlobal(mu16_String);
        }

        operator wchar_t*() const
        {
            return mu16_String;
        }

        operator std::wstring() const
        {
            return mu16_String;
        }

    private:
        wchar_t* mu16_String;
	};
}


