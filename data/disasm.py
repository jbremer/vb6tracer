"""
VB6Tracer - VB6 Instrumentation
Copyright (C) 2013-2014 Jurriaan Bremer, Marion Marschalek

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
"""

print '#include "vb6_tables.h"'

last_table_index = None
for line in open('opcodes.txt'):
    opcode, length, mnemonic = line.split()[:3]
    opcode = int(opcode, 16)

    if mnemonic in ('DOC', 'Unknown'):
        continue

    if (opcode >> 8) != last_table_index:
        if not last_table_index is None:
            print '};'

        last_table_index = opcode >> 8
        print 'vb6_insns_t vb6_table_%02x[256] = {' % last_table_index

    print '    [%d] = {%d, "%s"},' % (opcode & 0xff, int(length, 16),
                                      mnemonic)

print '};'
