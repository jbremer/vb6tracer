#include <stdio.h>
#include <stdint.h>
#include "vb6_tables.h"
#include "disasm.h"

int32_t vb6_disasm(vb6_ins_t *i, const uint8_t *raw)
{
    i->raw = raw;

#define TBL(idx) \
    case 0x##idx: \
        i->length = vb6_table_##idx[*raw].length; \
        i->mnemonic = vb6_table_##idx[*raw].mnemonic; \
        break;

    switch (*raw++) {
        TBL(fb); TBL(fc); TBL(fd); TBL(fe); TBL(ff);

    default:
        i->length = vb6_table_00[*--raw].length;
        i->mnemonic = vb6_table_00[*raw].mnemonic;
        break;
    }

    return i->length;
}
