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
#include "vb6.h"

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
