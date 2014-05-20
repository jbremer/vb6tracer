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

#endif
