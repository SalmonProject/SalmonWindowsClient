
#include "stdafx.h"
#include "Common.hpp"
#include <gcroot.h>
#include "../Wrapper/cCertVerifier.hpp"

#pragma unmanaged

// This is a trick to hide the static managed instance from beeing visible outside the assembly
static gcroot<vmimeNET::Common::delTrace*> gf_Callback;

#pragma managed

static void TraceCallback(const char* s8_Trace)
{
    if (gf_Callback) 
        gf_Callback->Invoke(new String(UNI(s8_Trace).c_str()));
}

namespace vmimeNET {

// if f_Callback == null -> turn off trace output
void Common::SetTraceCallback(delTrace* f_Callback)
{
    #if VMIME_TRACE
        gf_Callback = f_Callback;

        if (f_Callback) vmime::SetTraceCallback(TraceCallback);
        else            vmime::SetTraceCallback(NULL);
    #else
        throw new Exception(L"Trace has been disabled by compiler switch VMIME_TRACE.");
    #endif
}

// Abort any communication with the server -> throws exception
// This function must be called from another thread!
void Common::CancelOperation()
{
    vmime::wrapper::cCommon::CancelOperation();
}

} // namespace vmimeNET

