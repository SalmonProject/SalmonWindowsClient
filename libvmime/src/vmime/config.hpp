//
//
// This file is missing in GitHub.
// This file has entirely been created by Elmue.
//

#pragma once

// ------------------------------

// Create a static vmime.lib file rather than a vmime.dll.
// Do NOT create a DLL -> it will break your ass on Visual Studio!
#define VMIME_STATIC   1

// The following lines were originally in the files export.hpp, export-dynamic.hpp and export-static.hpp
// There is no need to have 3 files for just 15 lines of code. These lines are better here.
#if VMIME_STATIC
#  define VMIME_EXPORT
#else
#  ifndef VMIME_EXPORT
#    ifdef vmime_EXPORTS
        /* We are building this library */
#      define VMIME_EXPORT __declspec(dllexport)
#    else
        /* We are using this library */
#      define VMIME_EXPORT __declspec(dllimport)
#    endif
#  endif
#endif

// ------------------------------

// warning C4267: 'argument' : conversion from 'size_t' to 'unsigned long', possible loss of data
#pragma warning(disable: 4267)

// warning C4996: 'std::_Copy_opt' was declared deprecated
#pragma warning(disable: 4996)

typedef   signed char    vmime_int8;
typedef unsigned char    vmime_uint8;
typedef   signed short   vmime_int16;
typedef unsigned short   vmime_uint16;
typedef   signed int     vmime_int32;
typedef unsigned int     vmime_uint32;
typedef   signed __int64 vmime_int64;
typedef unsigned __int64 vmime_uint64;

#define _WIN32_WINNT    0x0501  // minimum version Windows XP

// This define is used by vmime code to distinguish Windows from Linux.
// It's name is annoying because it must be defined for 32 Bit AND for 64 Bit compilations.
// Please do not confuse _WIN32 with WIN32 and WIN64 which exist only on the given platform!
#undef  _WIN32
#define _WIN32

#define VMIME_PACKAGE "libvmime"
#define VMIME_VERSION "0.9.2"
#define VMIME_API     "1.0.0"

#define VMIME_PLATFORM_IS_WINDOWS        1
#define VMIME_PLATFORM_IS_POSIX          0

#define VMIME_BYTE_ORDER_BIG_ENDIAN      0
#define VMIME_BYTE_ORDER_LITTLE_ENDIAN   1  // Windows is always little endian

#define VMIME_CHARSETCONV_LIB_IS_ICONV   0  // requires libiconv library
#define VMIME_CHARSETCONV_LIB_IS_ICU     0
#define VMIME_CHARSETCONV_LIB_IS_WIN     1  // requires Windows (uses MultiByteToWideChar)

// required for file attachments and Maildir
#define VMIME_HAVE_FILESYSTEM_FEATURES   1

// Enable SASL support (otherwise exception "All authentication methods failed)
#define VMIME_HAVE_SASL_SUPPORT          1 // requires libgsasl library

#define VMIME_HAVE_TLS_SUPPORT           1
#define VMIME_TLS_SUPPORT_LIB_IS_OPENSSL 1 // requires openssl library
#define VMIME_TLS_SUPPORT_LIB_IS_GNUTLS  0 // requires gnutls  library (does not work correctly on Windows!)
#define VMIME_HAVE_GNUTLS_PRIORITY_FUNCS 1

// VMIME_HAVE_MESSAGING_FEATURES must be enabled, otherwise lots of errors!
#define VMIME_HAVE_MESSAGING_FEATURES       1 // enable IMAP, POP3, SMTP...
#define VMIME_HAVE_MESSAGING_PROTO_POP3     1
#define VMIME_HAVE_MESSAGING_PROTO_IMAP     1
#define VMIME_HAVE_MESSAGING_PROTO_SMTP     1
#define VMIME_HAVE_MESSAGING_PROTO_MAILDIR  0 // store emails in folders on disk
#define VMIME_HAVE_MESSAGING_PROTO_SENDMAIL 0 // starts an external process
#define VMIME_SENDMAIL_PATH      ""


#define VMIME_HAVE_GETADDRINFO    1 // getaddrinfo() exists (Winsock 2.0)
#define VMIME_HAVE_GETNAMEINFO    1 // getnameinfo() exists (Winsock 2.0)
#define VMIME_HAVE_PTHREAD        0 // PTHREAD_LIB
#define VMIME_HAVE_LOCALTIME_R    0 // Unix
#define VMIME_HAVE_LOCALTIME_S    1 // Windows
#define VMIME_HAVE_GMTIME_R       0 // Unix
#define VMIME_HAVE_GMTIME_S       1 // Windows
#define VMIME_HAVE_STRCPY_S       1 // strcpy_s() exists
#define VMIME_HAVE_SIZE_T         1 // size_t exists


// ----------- Trace Output --------------

// Trace output via TRACE() is written to the user defined callback vmime::traceCallback
// (also works in non-console applications)
// Depending on what you do with the Trace output, you can decide to print it only in Debug builds or in both
#if _DEBUG
    #define VMIME_TRACE   1
#else // NDEBUG
    #define VMIME_TRACE   1
#endif


#if VMIME_TRACE
    // The log level is an integer between 0 and 9. Higher values mean more verbosity. 
    // Higher values should be used with care, since they may reveal sensitive information.
    // Use a log level of 10 to enable all debugging options.
    #define GNUTLS_LOGLEVEL   3
    #define GNUTLS_DEBUG      1

    #define TRACE   vmime::Trace  // See base.cpp
#else
    #define TRACE   Error:Enclose_all_TRACE_macros_in_if_VMIME_TRACE_endif
#endif

// --------------------------------
