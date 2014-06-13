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

#ifndef VB6_H
#define VB6_H

#include <stdint.h>

// vb6_tables.c
typedef struct _vb6_insns_t {
    int32_t     length;
    const char *mnemonic;
} vb6_insns_t;

extern vb6_insns_t vb6_table_00[256];
extern vb6_insns_t vb6_table_fb[256];
extern vb6_insns_t vb6_table_fc[256];
extern vb6_insns_t vb6_table_fd[256];
extern vb6_insns_t vb6_table_fe[256];
extern vb6_insns_t vb6_table_ff[256];

// disasm.c

typedef struct _vb6_ins_t {
    const char     *mnemonic;
    const uint8_t  *raw;
    uint32_t        length;
} vb6_ins_t;

int32_t vb6_disasm(vb6_ins_t *i, const uint8_t *raw);

// hook.c

// for now just pre-hooks
typedef void (*vb6_hook_pre_t)(uint32_t *esp, uint32_t *ebp, uint32_t *esi);
typedef int (*vb6_hook_generic_pre_t)(uint32_t eax, uint32_t *esp,
    uint32_t *ebp, uint32_t *esi);

int vb6_hook_init(void *vb6_handle);

int vb6_hook_ins(const char *mnemonic, vb6_hook_pre_t pre);

int vb6_hook_generic_table00(vb6_hook_generic_pre_t pre);

// misc.c

void spawn_debugger();

// insns.c

int vb6_set_hooks();

// report.c

int report_init(const char *path);
void report_close();

// Modifiers:
// z, zero-terminated ascii string
// Z, zero-terminated unicode string
// s, ascii string with length
// S, unicode string with length
// d, 32-bit signed integer
// u, 32-bit unsigned integer
// x, 32-bit hex value
// b, bstr object
// v, variant object
void report(const char *fmt, ...);
void hexdump(const void *addr, int length, const char *msg);

// x86dump.c

const char *symbol(const uint8_t *addr);
void x86dump(const uint8_t *addr, const char *msg);

// native.c

int native(const uint8_t *fn);

#endif
