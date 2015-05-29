//
// FIX by Elmue:
// Replaced iconv library that does not exist as 64 Bit version.
// The iconv library is not required because Windows has built-in character conversion.
//

#include "../vmime/config.hpp"


#if VMIME_CHARSETCONV_LIB_IS_WIN

#include "../vmime/charsetConverter_Win.hpp"

#include "../vmime/exception.hpp"
#include "../vmime/utility/inputStreamStringAdapter.hpp"
#include "../vmime/utility/outputStreamStringAdapter.hpp"

#if (WIN32 || WIN64)
#define NOMINMAX
    #include <windows.h>
    #include "../vmime/platforms/windows/windowsCodepages.hpp"
#else
    #error Please use the compiler switch VMIME_CHARSETCONV_LIB_IS_WIN only on Windows!
#endif

#define CP_UNICODE   1200

namespace vmime
{

// static
ref <charsetConverter> charsetConverter::createGenericConverter
	(const charset& source, const charset& dest,
	 const charsetConverterOptions& opts)
{
	return vmime::create <charsetConverter_Win>(source, dest, opts);
}


charsetConverter_Win::charsetConverter_Win
	(const charset& source, const charset& dest, const charsetConverterOptions& opts)
	: m_source(source), m_dest(dest)
{
}


void charsetConverter_Win::convert(utility::inputStream& in, utility::outputStream& out)
{
    char s8_Buffer[32000];

    string s_In, s_Out;
    while (!in.eof())
    {
        size_t len = in.read(s8_Buffer, sizeof(s8_Buffer));
        s_In.append(s8_Buffer, len);
    }

    convert(s_In, s_Out);

    out.write(s_Out.data(), s_Out.length());
}


void charsetConverter_Win::convert(const string& s_In, string& s_Out)
{
	if (m_source == m_dest)
	{
		// No conversion needed
		s_Out = s_In;
		return;
	}

    int s32_CpSrc = getCodePage(m_source.getName().c_str());
    int s32_CpDst = getCodePage(m_dest  .getName().c_str());

    wstring s_UniBuffer;

    WCHAR* u16_UniPtr = NULL;
    int    s32_UniLen = 0;

    if (s32_CpSrc == CP_UNICODE)
    {
        u16_UniPtr = (WCHAR*)s_In.c_str();
        s32_UniLen = s_In.length() / 2;
    }
    else
    {
        int s32_BufSize = s_In.length() * 2; // in wide characters
        s_UniBuffer.resize(s32_BufSize);

        u16_UniPtr = (WCHAR*)s_UniBuffer.c_str();
        s32_UniLen = MultiByteToWideChar(s32_CpSrc, 0, s_In.c_str(), s_In.length(), u16_UniPtr, s32_BufSize);
    }

    if (s32_CpDst == CP_UNICODE)
    {
        s_Out.assign((const char*)u16_UniPtr, s32_UniLen *2);
    }
    else
    {
        int s32_BufSize = s32_UniLen * 6; // in multibyte characters

        string s_Buffer;
        s_Buffer.resize(s32_BufSize);
        char* s8_Buffer = (char*)s_Buffer.c_str();

        int s32_Len = WideCharToMultiByte(s32_CpDst, 0, u16_UniPtr, s32_UniLen, s8_Buffer, s32_BufSize, 0, NULL);

        s_Out.assign(s8_Buffer, s32_Len);
    }
}

int charsetConverter_Win::getCodePage(const char* s8_Name)
{
    if (stricmp(s8_Name, charsets::WSTRING) == 0) 
        return CP_UNICODE;

    // "cp1252" --> return 1252
    if ((s8_Name[0] == 'c' || s8_Name[0] == 'C') && 
        (s8_Name[1] == 'p' || s8_Name[1] == 'P'))
        return atoi(s8_Name + 2);

    return vmime::platforms::windows::windowsCodepages::getByName(s8_Name); // throws
}


ref <utility::charsetFilteredOutputStream> charsetConverter_Win::getFilteredOutputStream(utility::outputStream& os)
{
	return NULL;
}

}; // namsepace

#endif // VMIME_CHARSETCONV_LIB_IS_WIN
