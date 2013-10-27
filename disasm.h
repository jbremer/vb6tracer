#ifndef __VB6_DISASM__
#define __VB6_DISASM__

#include <stdint.h>

typedef struct _vb6_ins_t {
    const char     *mnemonic;
    const uint8_t  *raw;
    uint32_t        length;
} vb6_ins_t;

int32_t vb6_disasm(vb6_ins_t *i, const uint8_t *raw);

#endif
