
//
// Functionality common to all protocols 
// Copyright (C) Elmue
//

#include "stdafx.h"
#include "cCommon.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

// ######################################################################################
//                                  Linker directives
// ######################################################################################

// Required Dll: 
// libgsasl-7.dll
#ifdef WIN64
    #pragma comment(lib, "../libvmime/src/gsasl/libgsasl-7_64.lib") // 64 Bit, Release
#else
    #pragma comment(lib, "../libvmime/src/gsasl/libgsasl-7_32.lib") // 32 Bit, Release
#endif

// Required Dll's: 
// libgnutls-28.dll, 
// libnettle-4-7.dll, 
// libhogweed-2-5.dll, 
// libgmp-10.dll, 
// libp11-kit-0.dll 
#if VMIME_TLS_SUPPORT_LIB_IS_GNUTLS
    #error GnuTLS does not run correctly on Windows.
    #error Additionally a 64 bit version does not exist.
#endif

// The following LIB's may be:
// -- dynamic -> require libeay32.dll and ssleay32.dll
// -- static  -> no Dlls required
// IMPORTANT: Read documentation about the required VC++ runtime!
#if VMIME_TLS_SUPPORT_LIB_IS_OPENSSL

    // You can compile against the Debug version of openssl by setting the following definition = 1
    // But these Lib files are not included, because their total size is 24 MB
    #define DEBUG_LIBS_PRESENT   0

    #ifdef WIN64
        #if _DEBUG && DEBUG_LIBS_PRESENT
            #pragma comment(lib, "../libvmime/src/openssl/libeay64MDd.lib") // MultiThreadedDll, 64 Bit, Debug
            #pragma comment(lib, "../libvmime/src/openssl/ssleay64MDd.lib")
        #else
            #pragma comment(lib, "../libvmime/src/openssl/libeay64MD.lib")  // MultiThreadedDll, 64 Bit, Release
            #pragma comment(lib, "../libvmime/src/openssl/ssleay64MD.lib")
        #endif
    #else
        #if _DEBUG && DEBUG_LIBS_PRESENT
            #pragma comment(lib, "../libvmime/src/openssl/libeay32MDd.lib") // MultiThreadedDll, 32 Bit, Debug
            #pragma comment(lib, "../libvmime/src/openssl/ssleay32MDd.lib")
        #else
            #pragma comment(lib, "../libvmime/src/openssl/libeay32MD.lib")  // MultiThreadedDll, 32 Bit, Release
            #pragma comment(lib, "../libvmime/src/openssl/ssleay32MD.lib")
        #endif
    #endif
    #pragma comment(lib, "Crypt32.lib") // Microsoft Crypt32.dll
#endif

#ifdef WIN64
    #if _DEBUG
        #pragma comment(lib, "../libvmime/Debug64/vmime.lib")
    #else
        #pragma comment(lib, "../libvmime/Release64/vmime.lib")
    #endif
#else
    #if _DEBUG
        #pragma comment(lib, "../libvmime/Debug32/vmime.lib")
    #else
        #pragma comment(lib, "../libvmime/Release32/vmime.lib")
    #endif
#endif

using namespace std;

namespace vmime   {
namespace wrapper {

// ######################################################################################
//                                     Common
// ######################################################################################

// Cancels any lengthy operation (communication with the server)
void cCommon::CancelOperation()
{
    platform::handler::setCancelFlag(true);
}

// ######################################################################################
//                                     Progress
// ######################################################################################

// u32_Interval = Interval (in ms) in which progress is written to Trace output
// u32_Interval = 0 -> trace all
// s8_Protocol = "SMTP", "POP3" or "IMAP"
TraceProgress::TraceProgress(vmime_uint32 u32_Interval, const char* s8_Protocol)
{
    mu32_Interval = u32_Interval;
    ms_Protocol   = s8_Protocol;
}

// This function is implemented in vmime but it is never called -> useless
bool TraceProgress::cancel() const
{
    return false;
}

void TraceProgress::start(const long predictedTotal)
{
    // Throw exception if the user has canceled
    platform::handler::checkCanceled();

    #if VMIME_TRACE
        TRACE("%s Message transfer starting.", ms_Protocol.c_str());
        mu32_LastTick = GetTickCount();
    #endif
}

void TraceProgress::progress(const long current, const long currentTotal)
{
    // Throw exception if the user has canceled
    platform::handler::checkCanceled();

    #if VMIME_TRACE
        if (currentTotal == 0)
            return; // avoid division by zero crash

        vmime_uint32 u32_Tick = GetTickCount();
        if (u32_Tick - mu32_LastTick < mu32_Interval)
            return;

        mu32_LastTick = u32_Tick;

        TRACE("%s Message transfer %d%%", ms_Protocol.c_str(), (100 * current) / currentTotal);
    #endif
}

void TraceProgress::stop(const long total)
{
    #if VMIME_TRACE
        TRACE("%s Message transfer finished. (%d Bytes transmitted)", ms_Protocol.c_str(), total);
    #endif
}


// ######################################################################################
//                                     Timeout
// ######################################################################################

TimeoutHandler::TimeoutHandler()
{
    m_Interval = TRANSFER_TIMEOUT;
}

void TimeoutHandler::ModifyInterval(int s32_Timeout) // Seconds
{
    m_Interval = (time_t)s32_Timeout;
}

void TimeoutHandler::resetTimeOut()
{
    m_LastUnix = ::time(NULL);
}

bool TimeoutHandler::isTimeOut()
{
    return (::time(NULL) - m_LastUnix >= m_Interval);
}

bool TimeoutHandler::handleTimeOut()
{
    return false;
}


ref<net::timeoutHandler> TimeoutFactory::create()
{
    return vmime::create<TimeoutHandler>();
}


} // namespace wrapper
} // namespace vmime


