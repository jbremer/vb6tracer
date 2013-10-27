#ifndef __VB6_TABLES__
#define __VB6_TABLES__

#include <stdint.h>

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

#endif
