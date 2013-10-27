
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
