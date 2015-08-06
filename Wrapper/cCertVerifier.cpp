
//
// Certificate Verifier class
// Copyright (C) Elmue
//
// This class verifies the server X509 certificate against the root certificates in the Resource file RootCA.txt.
// You can optionally add user-trusted certificates by calling CertVerifier.setX509TrustedCerts()
// But this is not recommended because how will the user decide if he can trust a certificate that the server sends?
//

#include "stdafx.h"
#include "cCertVerifier.hpp"
#define NOMINMAX
#include <windows.h>  // NEVER include windows.h in a header file in a Managed C++ project !!

#pragma unmanaged

using namespace std;

namespace vmime   {
namespace wrapper {

// u16_ResIdCert = The IDR identifier in the RCDATA Resources of "RootCA.txt". (IDR_ROOT_CA)
// If the server sends a certificate that is not signed with a root certificate:
// b_AllowInvalidCerts = true  -> only write an ERROR to the Trace output.
// b_AllowInvalidCerts = false -> throw an exception and do not send the email.
cCertVerifier::cCertVerifier(bool b_AllowInvalidCerts, vmime_uint16 u16_ResIdCert)
{
    // Pass the address of any function inside the DLL or Exe file and get the module handle
    HMODULE h_Module;
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (WCHAR*)&SetTraceCallback, &h_Module))
        throw std::exception("Could not obtain HINSTANCE of module.");

    // Load the file RootCA.txt from the binary resources in the executable
    HRSRC   h_Src  = FindResource  (h_Module, MAKEINTRESOURCE(u16_ResIdCert), RT_RCDATA);
    DWORD u32_Size = SizeofResource(h_Module, h_Src);
    HGLOBAL h_Glob = LoadResource  (h_Module, h_Src);
    char*  s8_Rsrc = (char*)LockResource(h_Glob); // Unlock is not required (see MSDN)
    if (!s8_Rsrc)
        throw std::exception("Compilation error: The resource RCDATA\\IDR_ROOT_CA does not exist.");

    // Copy to a string because s8_Rsrc is not zero terminated!
    ms_ResourceData.assign(s8_Rsrc, u32_Size);

    // ------

    mb_AllowInvalidCerts = b_AllowInvalidCerts;
    mb_InitDone          = false;
}

void cCertVerifier::verify(ref <cert::certificateChain> chain, const string& hostname)
{
    #if VMIME_TRACE
        for (int i=0; i<(int)chain->getCount(); i++)
        {
            ref <cert::certificate> cert = chain->getAt(i);
            if (cert->getType() == "X.509")
            {
                ref <cert::X509Certificate> x509 = cert.dynamicCast<cert::X509Certificate>();

                TRACE("CERT Server X.509 certificate at chain pos %d: %s", i+1, x509->getIssuerString().c_str());
            }
            else
            {
                TRACE("CERT Server %s certificate at chain pos %d:", cert->getType().c_str(), i+1);
            }
        }
    #endif

    Init();

	try
	{
		defaultCertificateVerifier::verify(chain, hostname);
	}
	catch (exceptions::certificate_verification_exception& Ex)
	{
        if (mb_AllowInvalidCerts)
        {
            #if VMIME_TRACE
                TRACE("ERROR: The server's certificate cannot be trusted.\n%s", Ex.what());
            #endif
        }
        else
        {
            throw Ex;
        }
	}
}

// Load root certificates from resources (RootCA.txt)
void cCertVerifier::Init()
{
    if (mb_InitDone)
        return;

    vector <ref <cert::X509Certificate> > i_rootCAs;

    int s32_Start = 3; // Skip UTF8 BOM
    std::string s_File;

    while (TRUE)
    {
        int s32_End = (int)ms_ResourceData.find('\n', s32_Start); // first line = file name (UTF8)
        if (s32_End < 0)
            break;

        s_File = ms_ResourceData.substr(s32_Start, s32_End - s32_Start);

        s32_Start = s32_End +1;

        s32_End = (int)ms_ResourceData.find("-----END CERTIFICATE-----\n", s32_Start); // certificate PEM data (base64)
        if (s32_End < 0)
            break;

        s32_End += 25;

        vmime_uint8* u8_Data = (vmime_uint8*)ms_ResourceData.c_str() + s32_Start;
        ref <cert::X509Certificate> i_Cert = cert::X509Certificate::import(u8_Data, s32_End-s32_Start);

        s32_Start = s32_End +1;

        if (!i_Cert)
            throw exception(("Error loading root certificate " + s_File).c_str());

        i_rootCAs.push_back(i_Cert);
    }

    #if VMIME_TRACE
        TRACE("CERT %d Root Certificates (Certificate Authorities) loaded.", i_rootCAs.size()); 
    #endif

    setX509RootCAs(i_rootCAs);

    mb_InitDone = true;
}

} // namespace wrapper
} // namespace vmime
