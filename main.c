#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "hook.h"

// address of msvbvm60.dll
static void *g_vb6;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    (void) hModule; (void) lpReserved;

    if(dwReason == DLL_PROCESS_ATTACH) {
        g_vb6 = LoadLibrary("msvbvm60.dll");
        vb6_hook_init(g_vb6);
    }
    return TRUE;
}
