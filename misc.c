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
