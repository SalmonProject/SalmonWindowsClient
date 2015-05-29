
//
// Functionality common to all protocols
// Copyright (C) Elmue
//

#pragma once

// NEVER include windows.h in a header file in a Managed C++ project !!
#include "../libvmime/src/vmime/vmime.hpp"

#pragma unmanaged

// timeout in seconds to wait for an answer from the server
#define TRANSFER_TIMEOUT   30  

// NOTE: The entire vmime library uses UTF-8 internally.
// All strings passed to vmime code must be UTF-8 encoded.
// All strings returned from vmime (Trace, exceptions, etc..) must be converted back to Unicode
#define UTF(Unicode)  vmime::charset::WstringToUtf8(Unicode)
#define UNI(Utf)      vmime::charset::Utf8ToWstring(Utf)

// If a Unicode string is stored in a std::string -> copy it to a std::wstring
#define TOW(str)      wstring((const wchar_t*)str.c_str(), str.length()/2)

using namespace std;

namespace vmime   {
namespace wrapper {

class cCommon
{
public:
    // used for all protocols
    enum eSecurity
    {
        Secur_Normal,    // on normal port (default SMTP:  25, POP3: 110, IMAP: 143) unencrypted
        Secur_SSL,       // on secure port (default SMTP: 465, POP3: 995, IMAP: 993) using SSL
        Secur_TLS_opt,   // on normal port (default SMTP:  25, POP3: 110, IMAP: 143) using TLS if possible
        Secur_TLS_force, // on normal port (default SMTP: 587, POP3: 110, IMAP: 143) demands TLS otherwise fails
    };

    static void CancelOperation();
};

class TraceProgress : public utils::progressListener
{
public:
    TraceProgress(vmime_uint32 u32_Interval, const char* s8_Protocol);
    bool cancel() const;
    void start(const long predictedTotal);
    void progress(const long current, const long currentTotal);
    void stop(const long total);

private:
    vmime_uint32 mu32_LastTick;
    vmime_uint32 mu32_Interval;
    string       ms_Protocol;
};

class TimeoutHandler : public net::timeoutHandler
{
public:
    TimeoutHandler();
    void ModifyInterval(int s32_Timeout);
    void resetTimeOut();
    bool isTimeOut();
    bool handleTimeOut();

private:
    time_t m_LastUnix;
    time_t m_Interval;
};


class TimeoutFactory : public net::timeoutHandlerFactory
{
public:
    ref <net::timeoutHandler> create();
};

// This class assures that even in case of an exception the class is detroyed (avoid memory leaks)
template<class CLASS>
class GuardPtr
{
public:
    inline GuardPtr(CLASS* pi_Instance)
    {
        mpi_Instance = pi_Instance;
    }

    inline ~GuardPtr()
    {
        delete mpi_Instance;
    }

    inline CLASS* operator->() const
    {
        return mpi_Instance;
    }

    inline CLASS* Ptr() const
    {
        return mpi_Instance;
    }

private:
    CLASS* mpi_Instance;
};

} // namespace wrapper
} // namespace vmime

