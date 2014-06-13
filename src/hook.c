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
#include <stdint.h>
#include <string.h>
#include <windows.h>
#include "vb6.h"

// hook data for redirecting certain hooks, there are 6 tables of 256 entries
static uint8_t g_hook_data[6][256][32];

// vb6 vm lookup table pointers
static uint8_t **g_table_00;
static uint8_t **g_table_fb;
static uint8_t **g_table_fc;
static uint8_t **g_table_fd;
static uint8_t **g_table_fe;
static uint8_t **g_table_ff;

static uint8_t ***g_tables[] = {
    &g_table_00, &g_table_fb, &g_table_fc,
    &g_table_fd, &g_table_fe, &g_table_ff,
};

// original vm lookup tables
static uint8_t *g_table_orig[6][256];

static int _vb6_locate_vm_tables(void *vb6_handle)
{
    // function we follow for the tables
    uint32_t (*proc_call_engine)(const uint8_t *data);
    *(FARPROC *) &proc_call_engine =
        GetProcAddress(vb6_handle, "ProcCallEngine");
    if(proc_call_engine == NULL) {
        fprintf(stderr, "[-] Unable to find ProcCallEngine!\n");
        return -1;
    }

    const uint8_t *ptr = (const uint8_t *) proc_call_engine;

    // we look for the "jmp lookup_table[eax*4]" instruction
    for (; *ptr != 0xff || ptr[1] != 0x24; ptr++);

    g_table_00 = *(uint8_t ***)(ptr + 3);
    printf("[+] table_00: 0x%p\n", g_table_00);

    // copy the original vm lookup table
    memcpy(g_table_orig[0], g_table_00, 256 * sizeof(uint8_t *));

    // mark the lookup table as RWX - by default it's marked RX, but we want
    // it to be writable, so just to make sure we make it RWX (rather than RW)
    unsigned long old_protection;
    VirtualProtect(g_table_00, 256 * sizeof(uint8_t *),
        PAGE_EXECUTE_READWRITE, &old_protection);

    // now we get the 2-byte lookup tables
    for (uint32_t idx = 0xfb; idx < 0x100; idx++) {
        // resolve address for this index
        ptr = g_table_00[idx];

        // again, look for the "jmp lookup_table[eax*4]" instruction
        for (; *ptr != 0xff || ptr[1] != 0x24; ptr++);

        // initialize the address of the correct 2-byte lookup table
        *g_tables[1 + idx - 0xfb] = *(uint8_t ***)(ptr + 3);

        // copy the original vm lookup table
        memcpy(g_table_orig[1 + idx - 0xfb], *g_tables[1 + idx - 0xfb],
            256 * sizeof(uint8_t *));

        // mark as RWX - just like we did for g_table_00
        VirtualProtect(*g_tables[1 + idx - 0xfb], 256 * sizeof(uint8_t *),
            PAGE_EXECUTE_READWRITE, &old_protection);

        printf("[+] table_%02x: 0x%p\n", idx, *g_tables[1 + idx - 0xfb]);
    }
    return 0;
}

int vb6_hook_init(void *vb6_handle)
{
    // int3 all the bytes
    memset(g_hook_data, 0xcc, sizeof(g_hook_data));

    return _vb6_locate_vm_tables(vb6_handle);
}

static void _vb6_set_pre_hook(uint8_t *orig, uint8_t *hook,
    vb6_hook_pre_t pre)
{
    // pushad
    *hook++ = 0x60;

    // lea eax, [esp+32]
    *hook++ = 0x8d; *hook++ = 0x44; *hook++ = 0xe4; *hook++ = 0x20;

    // push esi ; push ebp ; push esp
    *hook++ = 0x56;
    *hook++ = 0x55;
    *hook++ = 0x50;

    // call pre
    *hook++ = 0xe8;
    *(uint32_t *) hook = (uint8_t *) pre - hook - 4;
    hook += 4;

    // add esp, 12
    *hook++ = 0x83; *hook++ = 0xc4; *hook++ = 0x0c;

    // popad
    *hook++ = 0x61;

    // jmp orig_handler
    *hook++ = 0xe9;
    *(uint32_t *) hook = orig - hook - 4;
}

static int _vb6_hooks_ins(const char *mnemonic, vb6_hook_pre_t pre,
    vb6_insns_t *table, uint32_t table_index)
{
    int ret = 0;
    for (uint32_t idx = 0; idx < 256; idx++, table++) {
        if(table->mnemonic != NULL && !strcmp(table->mnemonic, mnemonic)) {
            _vb6_set_pre_hook(g_table_orig[table_index][idx],
                g_hook_data[table_index][idx], pre);

            uint8_t **lut = *g_tables[table_index];
            lut[idx] = g_hook_data[table_index][idx];

            ret++;
        }
    }
    return ret;
}

int vb6_hook_ins(const char *mnemonic, vb6_hook_pre_t pre)
{
    int ret = 0;
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_00, 0);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fb, 1);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fc, 2);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fd, 3);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_fe, 4);
    ret += _vb6_hooks_ins(mnemonic, pre, vb6_table_ff, 5);
    return ret;
}

static void _vb6_set_generic_pre_hook(uint8_t *orig, uint8_t *hook,
    vb6_hook_generic_pre_t gpre)
{
    // pushad
    *hook++ = 0x60;

    // lea ebx, [esp+32]
    *hook++ = 0x8d; *hook++ = 0x5c; *hook++ = 0xe4; *hook++ = 0x20;

    // push esi ; push ebp ; push esp ; push eax
    *hook++ = 0x56;
    *hook++ = 0x55;
    *hook++ = 0x53;
    *hook++ = 0x50;

    // call generic-pre
    *hook++ = 0xe8;
    *(uint32_t *) hook = (uint8_t *) gpre - hook - 4;
    hook += 4;

    // add esp, 16
    *hook++ = 0x83; *hook++ = 0xc4; *hook++ = 0x10;

    // popad
    *hook++ = 0x61;

    // jmp orig_handler
    *hook++ = 0xe9;
    *(uint32_t *) hook = orig - hook - 4;
}

int vb6_hook_generic_table00(vb6_hook_generic_pre_t pre)
{
    for (uint32_t idx = 0; idx < 0x100; idx++) {
        if(vb6_table_00[idx].mnemonic == NULL) continue;

        _vb6_set_generic_pre_hook(g_table_orig[0][idx],
            g_hook_data[0][idx], pre);

        g_table_00[idx] = g_hook_data[0][idx];
    }
    return 0;
}
