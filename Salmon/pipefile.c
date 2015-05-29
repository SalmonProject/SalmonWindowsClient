//Copyright 2015 The Salmon Censorship Circumvention Project
//
//This file is part of the Salmon Client (Windows).
//
//The Salmon Client (Windows) is free software; you can redistribute it and / or
//modify it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 3 of the License, or
//(at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//GNU General Public License for more details.
//
//The full text of the license can be found at:
//http://www.gnu.org/licenses/gpl.html

#include "salmon_constants.h"

#include "pipefile.h"

size_t pipeReadNice(char* buf, size_t bufSize, PIPEFILE* p)
{
	DWORD bytesRead = 0;
	ReadFile(p->pipe, buf, bufSize, &bytesRead, NULL);
	return (size_t)bytesRead;
}

void pcloseNice(PIPEFILE* p)
{
	CloseHandle(p->pipe);
	CloseHandle(p->process);
	free(p);
}

//just check whether the specified program can be popen()'d
BOOL popenCheckExe(const char* cmdToExec)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE initReadTheirStdout, readTheirStdout, writeTheirStdout;
	CreatePipe(&initReadTheirStdout, &writeTheirStdout, &sa, 0);
	DuplicateHandle(GetCurrentProcess(), initReadTheirStdout, GetCurrentProcess(), &readTheirStdout, 0, FALSE, DUPLICATE_SAME_ACCESS);
	CloseHandle(initReadTheirStdout);

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = writeTheirStdout;
	si.hStdInput = NULL;
	si.hStdError = NULL;

	WCHAR toExecW[600]; //EXEC_VPNCMD_BUFSIZE = 600
	mbstowcs(toExecW, cmdToExec, 600);
	PROCESS_INFORMATION pi; //out param
	if (!CreateProcess(NULL, toExecW, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		return FALSE;

	CloseHandle(pi.hThread);
	CloseHandle(writeTheirStdout);
	CloseHandle(readTheirStdout);
	CloseHandle(pi.hProcess);

	return TRUE;
}

PIPEFILE* popenRNice(const char* cmdToExec)
{
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	HANDLE initReadTheirStdout, readTheirStdout, writeTheirStdout;
	CreatePipe(&initReadTheirStdout, &writeTheirStdout, &sa, 0);
	DuplicateHandle(GetCurrentProcess(), initReadTheirStdout, GetCurrentProcess(), &readTheirStdout, 0, FALSE, DUPLICATE_SAME_ACCESS);
	CloseHandle(initReadTheirStdout);

	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdOutput = writeTheirStdout;
	si.hStdInput = NULL;
	si.hStdError = NULL;

	WCHAR toExecW[600]; //EXEC_VPNCMD_BUFSIZE = 600
	mbstowcs(toExecW, cmdToExec, 600);
	PROCESS_INFORMATION pi; //out param
	if (!CreateProcess(NULL, toExecW, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
	{
		DWORD retSize;
		LPTSTR pTemp = NULL;

		retSize = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			NULL, GetLastError(), LANG_NEUTRAL, (LPTSTR)&pTemp, 0, NULL);

		MessageBoxW(NULL, pTemp, L"Error running SoftEther; is it installed correctly?", MB_OK);
		LocalFree(pTemp);
		return 0;
	}

	CloseHandle(pi.hThread);
	CloseHandle(writeTheirStdout);

	PIPEFILE* ret = (PIPEFILE*)malloc(sizeof(PIPEFILE));
	ret->pipe = readTheirStdout;
	ret->process = pi.hProcess;
	return ret;
}

size_t popenOneShotR(const char* cmdToExec, char* buf, size_t bufSize)
{
	PIPEFILE* p = popenRNice(cmdToExec);

	DWORD curBytesRead = 0;
	size_t bytesReadTotal = 0;
	do
	{
		curBytesRead = pipeReadNice(&buf[bytesReadTotal], bufSize - (bytesReadTotal + 1), p);
		bytesReadTotal += curBytesRead;
	} while (bytesReadTotal < bufSize - 1 && curBytesRead != 0);
	*(buf + (bytesReadTotal < bufSize ? bytesReadTotal : bufSize - 1)) = 0;

	pcloseNice(p);

	return bytesReadTotal;
}

int getlinePipe(char** lineptr, size_t* n, PIPEFILE* stream)
{
	if (!*lineptr)
	{
		*lineptr = (char*)malloc(1024);
		*n = 1024;
	}

	int readSoFar = 0;
	while (1)
	{
		if (readSoFar > *n - 2)
		{
			realloc(*lineptr, *n * 2);
			*n *= 2;
		}

		DWORD didWeRead;
		if (!ReadFile(stream->pipe, (*lineptr) + readSoFar, 1, &didWeRead, NULL) || didWeRead < 1)
			return -1;

		readSoFar++;
		if ((*lineptr)[readSoFar - 1] == '\n')
		{
			(*lineptr)[readSoFar] = 0;
			return readSoFar;
		}
	}
	return -1;
}
