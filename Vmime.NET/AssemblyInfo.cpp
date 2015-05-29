
#include "stdafx.h"

using namespace System;
using namespace System::Reflection;
using namespace System::Runtime::CompilerServices;

#if WIN64
    #if _DEBUG
        [assembly:AssemblyTitleAttribute  ("vmime.NET 64 Bit Debug")];
        [assembly:AssemblyProductAttribute("vmime.NET 64 Bit Debug")];
    #else
        [assembly:AssemblyTitleAttribute  ("vmime.NET 64 Bit Release")];
        [assembly:AssemblyProductAttribute("vmime.NET 64 Bit Release")];
    #endif
#else
    #if _DEBUG
        [assembly:AssemblyTitleAttribute  ("vmime.NET 32 Bit Debug")];
        [assembly:AssemblyProductAttribute("vmime.NET 32 Bit Debug")];
    #else
        [assembly:AssemblyTitleAttribute  ("vmime.NET 32 Bit Release")];
        [assembly:AssemblyProductAttribute("vmime.NET 32 Bit Release")];
    #endif
#endif

[assembly:AssemblyDescriptionAttribute("SMTP, POP3, IMAP library with SSL, TLS")];
[assembly:AssemblyConfigurationAttribute("")];
[assembly:AssemblyCopyrightAttribute("GNU GPL (Sourcecode on www.codeproject.com)")];
[assembly:AssemblyCompanyAttribute  ("ElmueSoft")];
[assembly:AssemblyTrademarkAttribute("ElmueSoft")];
[assembly:AssemblyVersionAttribute("0.9.2")];
[assembly:AssemblyKeyFileAttribute("KeyPair.snk")]
[assembly:CLSCompliant(true)];



