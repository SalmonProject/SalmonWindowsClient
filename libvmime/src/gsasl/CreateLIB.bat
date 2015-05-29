REM Execute this in Visual Studio SDK Command Prompt !
REM This creates the LIB file required for Visual Studio from the DEF file (DLL Export File)
REM The DEF file must be in the same directory as this script

REM If you replace the Dll by another version you must also replace the DEF file and execute this script anew!

lib /def:libgsasl-7.def   /out:libgsasl-7_32.lib   /nologo /machine:x86
lib /def:libgsasl-7.def   /out:libgsasl-7_64.lib   /nologo /machine:x64