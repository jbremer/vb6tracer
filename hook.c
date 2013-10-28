#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include "hook.h"
#include "disasm.h"
#include "vb6_tables.h"

// hook data for redirecting certain hooks, there are 6 tables of 256 entries
static uint8_t hook_data[][256 * 6 * 32];

// vb6 vm lookup tables
static uint8_t **g_table_00;
static uint8_t **g_table_fb;
static uint8_t **g_table_fc;
static uint8_t **g_table_fd;
static uint8_t **g_table_fe;
static uint8_t **g_table_ff;

static int _vb6_locate_vm_tables(void *vb6_handle)
{
    // function we follow for the tables
    uint32_t (*proc_call_engine)(const uint8_t *data);
    *(FARPROC *) &proc_call_engine =
        GetProcAddress(vb6_handle, "ProcCallEngine");
    if(proc_call_engine == NULL) return -1;

    const uint8_t *ptr = (const uint8_t *) proc_call_engine;

    // we look for the "jmp lookup_table[eax*4]" instruction
    for (; *ptr != 0xff || ptr[1] != 0x24; ptr++);

    g_table_00 = *(uint8_t ***)(ptr + 3);

    static uint8_t ***tables[] = {
        &g_table_fb, &g_table_fc, &g_table_fd, &g_table_fe, &g_table_ff,
    };

    // now we get the 2-byte lookup tables
    for (uint32_t idx = 0xfb; idx < 0x100; idx++) {
        // resolve address for this index
        ptr = g_table_00[idx];

        // again, look for the "jmp lookup_table[eax*4]" instruction
        for (; *ptr != 0xff || ptr[1] != 0x24; ptr++);

        // initialize the address of the correct 2-byte lookup table
        *tables[idx - 0xfb] = *(uint8_t ***)(ptr + 3);
    }
    return 0;
}

int vb6_hook_init(void *vb6_handle)
{
    return _vb6_locate_vm_tables(vb6_handle);
}

static int _vb6_hooks_ins(const char *mnemonic, vb6_hook_pre_t pre,
    vb6_insns_t *table)
{
    int ret = 0;
    for (uint32_t idx = 0; idx < 256; idx++, table++) {
        if(table->mnemonic != NULL && !strcmp(table->mnemonic, mnemonic)) {
            // ...
        }
    }
    return ret;
}

int vb6_hook_ins(const char *mnemonic, vb6_hook_pre_t pre)
{
    int ret = 0;
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_00);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fb);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fc);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fd);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fe);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_ff);
    return ret;
}
