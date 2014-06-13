/*
 * VB6Tracer - VB6 Instrumentation
 * Copyright (C) 2013-2014 Jurriaan Bremer, Marion Marschalek
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <windows.h>
#include "vb6.h"

void spawn_debugger()
{
    const char *path = "X:\\ollydbg2\\ollydbg.exe";
    char fname[MAX_PATH];

    sprintf(fname, "\"%s\" -p %ld", path, GetCurrentProcessId());

    STARTUPINFO si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si)); si.cb = sizeof(si);
    CreateProcess(path, fname, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

    Sleep(5000);

    __asm__("int3\n");
}
