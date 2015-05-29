//
// FIX by Elmue:
// Replaced iconv library that does not exist as 64 Bit version.
// The iconv library is not required because Windows has built-in character conversion.
//

#ifndef VMIME_charsetConverter_Win_HPP_INCLUDED
#define VMIME_charsetConverter_Win_HPP_INCLUDED


#include "../vmime/config.hpp"


#if VMIME_CHARSETCONV_LIB_IS_WIN


#include "../vmime/charsetConverter.hpp"


namespace vmime
{


/** A generic charset converter which uses Windows MultiByteToWideChar.
  */

class charsetConverter_Win : public charsetConverter
{
public:
	charsetConverter_Win(const charset& source, const charset& dest, const charsetConverterOptions& opts = charsetConverterOptions());

	void convert(const string& s_In, string& s_Out);
	void convert(utility::inputStream& in, utility::outputStream& out);

	ref <utility::charsetFilteredOutputStream> getFilteredOutputStream(utility::outputStream& os);

private:
    int getCodePage(const char* s8_Name);

	charset m_source;
	charset m_dest;
};


} // namespace


#endif // VMIME_CHARSETCONV_LIB_IS_WIN

#endif // VMIME_charsetConverter_Win_HPP_INCLUDED
