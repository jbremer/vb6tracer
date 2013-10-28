#ifndef __VB6_HOOK__
#define __VB6_HOOK__

#include <stdint.h>

// for now just pre-hooks
typedef int (*vb6_hook_pre_t)(uint32_t *esp, uint32_t *ebp, uint32_t *esi);
typedef int (*vb6_hook_generic_pre_t)(uint32_t eax, uint32_t *esp,
    uint32_t *ebp, uint32_t *esi);

int vb6_hook_init(void *vb6_handle);

int vb6_hook_ins(const char *mnemonic, vb6_hook_pre_t pre);
int vb6_hook_generic_table00(vb6_hook_generic_pre_t pre);

void spawn_debugger();

#endif
