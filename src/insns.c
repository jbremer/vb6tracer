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
#include <oaidl.h>
#include <wtypes.h>
#include "vb6.h"

#define H(mnemonic) \
    static void _pre_##mnemonic(uint32_t *esp, uint32_t *ebp, uint32_t *esi)

#define REPORT(fmt, ...) \
    (void) esp; (void) ebp; report("%x " fmt, esi - 1, ##__VA_ARGS__)

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

H(ImpAdCallFPR4)
{
    uint8_t *fn = *(uint8_t **)(ebp[-0x54/4] + *(uint16_t *) esi * 4);
    REPORT("ImpAdCallI2 fn 0x%x", fn);
    if(fn != NULL && native(fn) < 0) {
        x86dump(fn, "ImpAdCallFPR4");
    }
}

H(ImpAdCallI2)
{
    uint8_t *fn = *(uint8_t **)(ebp[-0x54/4] + *(uint16_t *) esi * 4);
    REPORT("ImpAdCallI2 fn 0x%x", fn);
    if(fn != NULL && native(fn) < 0) {
        x86dump(fn, "ImpAdCallI2");
    }
}

H(VCallHresult)
{
    const uint8_t *fn =
        *(uint8_t **)(*(uint32_t *) ebp[-0x4c/4] + *(uint16_t *) esi);
    REPORT("VCallHresult fn 0x%x", fn);
    if(fn != NULL && native(fn) < 0) {
        x86dump(fn, "VCallHresult");
    }
}

H(ThisVCallHresult)
{
    uint8_t *fn = *(uint8_t **)(*(uint32_t *) ebp[8/4] + *(uint16_t *) esi);
    REPORT("ThisVCallHresult fn 0x%x", fn);
    if(fn != NULL && native(fn) < 0) {
        x86dump(fn, "ThisVCallHresult");
    }
}

H(ImpAdCallCbFrame)
{
    const uint8_t *fn =
        *(uint8_t **)(ebp[-0x54/4] + *((uint16_t *) esi + 1) * 4);
    REPORT("ImpAdCallCbFrame fn 0x%x", fn);
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
    // const wchar_t *s = *(const wchar_t **)(ebp[-0x4c/4] + *(uint16_t *) esi);
    REPORT("MemLdStr");
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

H(LitI2_Byte)
{
    REPORT("LitI2_Byte %u", (uint16_t)(signed char) *esi);
}

H(LitVarI2)
{
    REPORT("LitVarI2 %u", *((uint16_t *) esi + 1));
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
    HOOK(ImpAdCallFPR4),
    HOOK(ImpAdCallI2),
    HOOK(VCallHresult),
    HOOK(ThisVCallHresult),
    HOOK(ImpAdCallCbFrame),
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
    HOOK(LitI2_Byte),
    HOOK(LitVarI2),
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
