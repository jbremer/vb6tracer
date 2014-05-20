#include <stdio.h>
#include <stdint.h>
#include <oaidl.h>
#include <wtypes.h>
#include "vb6.h"

#define H(mnemonic) \
    static void _pre_##mnemonic(uint32_t *esp, uint32_t *ebp, uint32_t *esi)

#define REPORT(fmt, ...) \
    (void) esp; (void) ebp; report("%x " fmt, esi, ##__VA_ARGS__)

H(OpenFile)
{
    REPORT("OpenFile %b", esp[2]);
}

H(FnLenStr)
{
    REPORT("FnLenStr %b", esp[0]);
}

H(ConcatStr)
{
    REPORT("ConcatStr %b %b", esp[1], esp[0]);
}

H(ConcatVar)
{
    REPORT("VarCat %v %v", esp[0], esp[1]);
}

H(FnInStr4)
{
    REPORT("FnInStr4 %Z %Z %u", esp[1], esp[2], esp[3]);
}

H(FnInStr4Var)
{
    REPORT("FnInStr4Var %v %v", esp[1], esp[2]);
}

H(CStr2Ansi)
{
    REPORT("CStr2Ansi %b", esp[1]);
}

H(CVarStr)
{
    REPORT("CVarStr %b", esp[0]);
}

H(CStr2Uni)
{
    if(esp[1] != 0) {
        REPORT("CStr2Uni %s", *(uint32_t *)(esp[1] - 4), esp[1]);
    }
}

H(EqStr)
{
    REPORT("EqStr %b %b", esp[0], esp[1]);
}

H(NeStr)
{
    REPORT("NeStr %b %b", esp[0], esp[1]);
}

H(LitStr)
{
    const wchar_t *s =
        *(const wchar_t **)(ebp[-0x54/4] + *(uint16_t *) esi * 4);
    REPORT("LitStr %b", s);
}

H(MemLdStr)
{
    const wchar_t *s = *(const wchar_t **)(ebp[-0x4c/4] + *(uint16_t *) esi);
    REPORT("MemLdStr %b", s);
}

H(IStStr)
{
    REPORT("IStStr %b", esp[0]);
}

H(FStStr)
{
    REPORT("FStStr %b", esp[0]);
}

H(FMemStStr)
{
    REPORT("FMemStStr %b", esp[0]);
}

H(FStStrCopy)
{
    REPORT("FStSTrCopy %b", esp[0]);
}

H(MemStStrCopy)
{
    REPORT("MemStStrCopy %b", esp[0]);
}

H(XorVar)
{
    REPORT("XorVar %v %v", esp[0], esp[1]);
}

H(LitI4)
{
    REPORT("LitI4 0x%x %u", esi[0], esi[0]);
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
    HOOK(LitI4),
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
