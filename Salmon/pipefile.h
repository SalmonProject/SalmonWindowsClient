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

#ifndef _SALMON_INCLGUARD_PIPEFILE_H_
#define _SALMON_INCLGUARD_PIPEFILE_H_

#include <Windows.h>

typedef struct PIPEFILE
{
	HANDLE pipe;
	HANDLE process;
} PIPEFILE;

size_t pipeReadNice(char* buf, size_t bufSize, PIPEFILE* p);
void pcloseNice(PIPEFILE* p);
PIPEFILE* popenRNice(const char* cmdToExec);
int getlinePipe(char** lineptr, size_t* n, PIPEFILE* stream);

#endif //_SALMON_INCLGUARD_PIPEFILE_H_