#ifndef __VB6_HOOK__
#define __VB6_HOOK__

// for now just pre-hooks
typedef int (*vb6_hook_pre_t)(uint32_t *esp, uint32_t *ebp, uint32_t *esi);

int vb6_hook_init(void *vb6_handle);
int vb6_hook_ins(const char *mnemonic, vb6_hook_pre_t pre);

#endif
