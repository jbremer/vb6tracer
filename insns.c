#include <stdio.h>
#include <stdint.h>
#include <oaidl.h>
#include <wtypes.h>
#include "vb6.h"

static void _pre_OpenFile(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("OpenFile %b", esp[2]);
}

static void _pre_FnLenStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("FnLenStr %b", esp[0]);
}

static void _pre_ConcatStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("ConcatStr %b %b", esp[1], esp[0]);
}

static void _pre_ConcatVar(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("VarCat %v %v", esp[0], esp[1]);
}

static void _pre_FnInStr4(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("FnInStr4 %Z %Z %u", esp[1], esp[2], esp[3]);
}

static void _pre_FnInStr4Var(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("FnInStr4Var %v %v", esp[1], esp[2]);
}

static void _pre_CStr2Ansi(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("CStr2Ansi %b", esp[1]);
}

static void _pre_CVarStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("CVarStr %b", esp[0]);
}

static void _pre_CStr2Uni(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    if(esp[1] != 0) {
        report("CStr2Uni %s", *(uint32_t *)(esp[1] - 4), esp[1]);
    }
}

static void _pre_EqStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("EqStr %b %b", esp[0], esp[1]);
}

static void _pre_NeStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("NeStr %b %b", esp[0], esp[1]);
}

static void _pre_LitStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) esp;

    const wchar_t *s =
        *(const wchar_t **)(ebp[-0x54/4] + *(uint16_t *) esi * 4);
    report("LitStr %b", s);
}

static void _pre_MemLdStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) esp;

    const wchar_t *s = *(const wchar_t **)(ebp[-0x4c/4] + *(uint16_t *) esi);
    report("MemLdStr %b", s);
}

static void _pre_IStStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("IStStr %b", esp[0]);
}

static void _pre_FStStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("FStStr %b", esp[0]);
}

static void _pre_FMemStStr(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("FMemStStr %b", esp[0]);
}

static void _pre_FStStrCopy(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("FStSTrCopy %b", esp[0]);
}

static void _pre_MemStStrCopy(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("MemStStrCopy %b", esp[0]);
}

static void _pre_XorVar(uint32_t *esp, uint32_t *ebp, uint32_t *esi)
{
    (void) ebp; (void) esi;

    report("XorVar %v %v", esp[0], esp[1]);
}

#define HOOK(fn) {#fn, _pre_##fn}

static struct _hooks_t {
    const char *mnemonic;
    vb6_hook_pre_t pre;
} g_hooks[] = {
    HOOK(OpenFile),
    HOOK(FnLenStr),
    HOOK(ConcatStr),
    HOOK(ConcatVar),
    HOOK(EqStr),
    HOOK(NeStr),
    HOOK(LitStr),
    HOOK(MemLdStr),
    HOOK(FnInStr4),
    HOOK(FnInStr4Var),
    HOOK(CStr2Uni),
    HOOK(CStr2Ansi),
    HOOK(CVarStr),
    HOOK(IStStr),
    HOOK(FStStr),
    HOOK(FMemStStr),
    HOOK(FStStrCopy),
    HOOK(MemStStrCopy),
    HOOK(XorVar),
};

int vb6_set_hooks()
{
    for (uint32_t idx = 0; idx < sizeof(g_hooks)/sizeof(g_hooks[0]); idx++) {
        report("[x] Installing %z hook", g_hooks[idx].mnemonic);

        if(vb6_hook_ins(g_hooks[idx].mnemonic, g_hooks[idx].pre) == 0) {
            report("[!] Error installing hook for %z!",
                g_hooks[idx].mnemonic);
            exit(1);
        }
    }
    return 0;
}
