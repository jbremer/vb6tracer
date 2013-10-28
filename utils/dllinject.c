#include <stdio.h>
#include <windows.h>

int main(int argc, char *argv[])
{
    if(argc != 3) {
        printf("Usage: %s <dll> <app> [args..]\n", argv[0]);
        printf("(args currently not supported!)\n");
        return 1;
    }

    FARPROC load_library_a =
        GetProcAddress(GetModuleHandle("kernel32"), "LoadLibraryA");
    if(load_library_a == NULL) {
        fprintf(stderr, "Error resolving LoadLibraryA?!\n");
        return 1;
    }

    STARTUPINFO si; PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    si.cb = sizeof(si);

    if(CreateProcessA(argv[2], argv[2], NULL, NULL, FALSE, CREATE_SUSPENDED,
            NULL, NULL, &si, &pi) == FALSE) {
        fprintf(stderr, "Error launching process: %d!\n", GetLastError());
        return 1;
    }

    void *lib = VirtualAllocEx(pi.hProcess, NULL, strlen(argv[1]) + 1,
        MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if(lib == NULL) {
        fprintf(stderr, "Error allocating memory in the process: %d!\n",
            GetLastError());
        goto error;
    }

    unsigned long bytes_written;
    if(WriteProcessMemory(pi.hProcess, lib, argv[1], strlen(argv[1]) + 1,
            &bytes_written) == FALSE ||
            bytes_written != strlen(argv[1]) + 1) {
        fprintf(stderr, "Error writing lib to the process: %d\n",
            GetLastError());
        goto error;
    }

    if(QueueUserAPC((PAPCFUNC) load_library_a, pi.hThread,
            (ULONG_PTR) lib) == 0) {
        fprintf(stderr, "Error queueing APC to the process: %d\n",
            GetLastError());
        goto error;
    }

    printf("[x] Injected successfully!\n");

    ResumeThread(pi.hThread);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 0;

error:
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return 1;
}
