#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vb6.h"

static int is_jump_symbol(const uint8_t *addr, const char *sym)
{
    if(*addr == 0xff && addr[1] == 0x25) {
        addr = **(uint8_t ***)(addr + 2);
        const char *s = symbol(addr);
        return s != NULL && !strcmp(s, sym);
    }
    return 0;
}

int native(const uint8_t *fn)
{
    if(*fn == 0xff && fn[1] == 0x25) {
        const uint8_t *addr = **(uint8_t ***)(fn + 2);
        report("[x] Calling imported function.. %z", symbol(addr));
        return 0;
    }
    if(*fn == 0xba && fn[5] == 0xb9 && fn[10] == 0xff && fn[11] == 0xe1) {
        const uint8_t *addr = *(uint8_t **)(fn + 6);
        if(is_jump_symbol(addr, "MSVBVM60.DLL!ProcCallEngine") != 0) {
            report("[x] Calling VB6 Procedure.. 0x%x", *(uint32_t *)(fn + 1));
            return 0;
        }
    }
    if(*fn == 0x33 && fn[1] == 0xc0 && fn[2] == 0xba && fn[7] == 0x68 &&
            fn[12] == 0xc3) {
        const uint8_t *addr = *(uint8_t **)(fn + 8);
        if(is_jump_symbol(addr, "MSVBVM60.DLL!MethCallEngine") != 0) {
            report("[x] Calling VB6 Method.. 0x%x", *(uint32_t *)(fn + 3));
            return 0;
        }
    }
    if(*fn == 0xa1 && !memcmp(&fn[5], "\x0b\xc0\x74\x02\xff\xe0\x68", 7) &&
            fn[16] == 0xb8 && !memcmp(&fn[21], "\xff\xd0\xff\xe0", 4)) {
        const uint8_t *addr = *(uint8_t **)(fn + 17);
        if(is_jump_symbol(addr, "MSVBVM60.DLL!DllFunctionCall") != 0) {
            addr = *(uint8_t **)(fn + 12);
            const char *mod = *(const char **)(addr);
            const char *func = *(const char **)(addr + 4);
            report("[x] Calling dynamic function.. %z!%z", mod, func);
            return 0;
        }
    }
    report(".. unknown x86");
    return -1;
}
