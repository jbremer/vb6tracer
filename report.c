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
#include <string.h>
#include <windows.h>
#include "vb6.h"

static FILE *g_fp;

int utf8_encode(uint16_t c, uint8_t *out)
{
    if(c < 0x80) {
        *out = c & 0x7f;
        return 1;
    }
    else if(c < 0x800) {
        *out = 0xc0 + ((c >> 8) << 2) + (c >> 6);
        out[1] = 0x80 + (c & 0x3f);
        return 2;
    }
    else {
        *out = 0xe0 + (c >> 12);
        out[1] = 0x80 + (((c >> 8) & 0x1f) << 2) + ((c >> 6) & 0x3);
        out[2] = 0x80 + (c & 0x3f);
        return 3;
    }
}

static int _report_utf8x(char **out, uint16_t x)
{
    unsigned char buf[3];
    int len = utf8_encode(x, buf);
    if(*out != NULL) {
        memcpy(*out, buf, len);
        *out += len;
    }
    return len;
}

static int _report_character(char **out, uint16_t ch, int quoted)
{
    if(quoted == 0) {
        return _report_utf8x(out, ch);
    }

    switch (ch) {
    case '\t':
        return _report_utf8x(out, '\\') + _report_utf8x(out, 't');

    case '\r':
        return _report_utf8x(out, '\\') + _report_utf8x(out, 'r');

    case '\n':
        return _report_utf8x(out, '\\') + _report_utf8x(out, 'n');

    case '"':
        return _report_utf8x(out, '\\') + _report_utf8x(out, '"');

    default:
        return _report_utf8x(out, ch);
    }
}

static int _report_ascii(char **out, const char *s, int len, int quoted)
{
    if(len == 0) {
        return _report_ascii(out, "<empty>", 7, 0);
    }

    int ret = 0;

    if(quoted != 0) {
        ret += _report_utf8x(out, '"');
    }

    while (len-- != 0) {
        ret += _report_character(out, *s++, quoted);
    }

    if(quoted != 0) {
        ret += _report_utf8x(out, '"');
    }
    return ret;
}

static int _report_unicode(char **out, const wchar_t *s, int len, int quoted)
{
    if(len == 0) {
        return _report_ascii(out, "<empty>", 7, 0);
    }

    int ret = 0;

    if(quoted != 0) {
        ret += _report_utf8x(out, '"');
    }

    while (len-- != 0) {
        ret += _report_character(out, *s++, quoted);
    }

    if(quoted != 0) {
        ret += _report_utf8x(out, '"');
    }
    return ret;
}

static int _report_variant(char **out, const VARIANT *v)
{
    char buf[32]; int len;
    switch (v->vt) {
    case VT_NULL:
        return _report_ascii(out, "<null>", 6, 0);

    case VT_BOOL:
        if(v->boolVal != 0) {
            return _report_ascii(out, "True", 4, 0);
        }

        return _report_ascii(out, "False", 5, 0);

    case VT_I1: case VT_UI1:
        return _report_ascii(out, buf, sprintf(buf, "%u", v->bVal), 0);

    case VT_I2: case VT_UI2:
        return _report_ascii(out, buf, sprintf(buf, "%u", v->iVal), 0);

    case VT_I4: case VT_UI4: case VT_INT: case VT_UINT:
        return _report_ascii(out, buf, sprintf(buf, "%u", v->intVal), 0);

    case VT_I8: case VT_UI8:
        return _report_ascii(out, buf, sprintf(buf, "%I64u", v->llVal), 0);

    case VT_LPSTR:
        if(v->pcVal == NULL) return _report_ascii(out, "<null>", 6, 0);
        return _report_ascii(out, v->pcVal, strlen(v->pcVal), 1);

    case VT_LPWSTR:
        if(v->pbVal == NULL) return _report_ascii(out, "<null>", 6, 0);
        return _report_unicode(out, (const wchar_t *) v->pbVal,
                lstrlenW((const wchar_t *) v->pcVal), 1);

    case VT_BSTR:
        if(v->bstrVal == NULL) return _report_ascii(out, "<null>", 6, 0);

        len = *(int *)((uint8_t *) v->bstrVal - sizeof(int));
        return _report_unicode(out, v->bstrVal, len >> 1, 1);

    case VT_VARIANT:
        return _report_variant(out, v->pvarVal);

    default:
        sprintf(buf, "<VT_%d>", v->vt);
        return _report_ascii(out, buf, strlen(buf), 0);
    }
    return 0;
}

static int _report_sprintf(char *out, const char *fmt, va_list args)
{
    int ret = 0, len; char buf[32];
    const char *s; const wchar_t *w; const VARIANT *v;
    while (*fmt != 0) {
        if(*fmt != '%') {
            ret += _report_utf8x(&out, *fmt++);
            continue;
        }
        switch (*++fmt) {
        case 'z':
            s = va_arg(args, const char *);
            if(s == NULL) {
                ret += _report_ascii(&out, "<null>", 6, 0);
                break;
            }

            ret += _report_ascii(&out, s, strlen(s), 0);
            break;

        case 'Z':
            w = va_arg(args, const wchar_t *);
            if(w == NULL) {
                ret += _report_ascii(&out, "<null>", 6, 0);
                break;
            }

            ret += _report_unicode(&out, w, lstrlenW(w), 0);
            break;

        case 's':
            len = va_arg(args, int);
            s = va_arg(args, const char *);
            if(s == NULL) {
                ret += _report_ascii(&out, "<null>", 6, 0);
                break;
            }

            ret += _report_ascii(&out, s, len, 1);
            break;

        case 'S':
            len = va_arg(args, int);
            w = va_arg(args, const wchar_t *);
            if(w == NULL) {
                ret += _report_ascii(&out, "<null>", 6, 0);
                break;
            }

            ret += _report_unicode(&out, w, len, 1);
            break;

        case 'd':
            sprintf(buf, "%d", va_arg(args, int));
            ret += _report_ascii(&out, buf, strlen(buf), 0);
            break;

        case 'u':
            sprintf(buf, "%u", va_arg(args, int));
            ret += _report_ascii(&out, buf, strlen(buf), 0);
            break;

        case 'x':
            sprintf(buf, "%08x", va_arg(args, int));
            ret += _report_ascii(&out, buf, strlen(buf), 0);
            break;

        case 'b':
            w = va_arg(args, wchar_t *);
            if(w == NULL) {
                ret += _report_ascii(&out, "<null>", 6, 0);
                break;
            }

            len = *(int *)((uint8_t *) w - sizeof(int));
            ret += _report_unicode(&out, w, len >> 1, 1);
            break;

        case 'v':
            v = va_arg(args, const VARIANT *);
            if(v == NULL) {
                ret += _report_ascii(&out, "<null>", 6, 0);
                break;
            }

            ret += _report_variant(&out, v);
            break;
        }
        fmt++;
    }
    ret += _report_utf8x(&out, '\n');
    return ret;
}

int report_init(const char *path)
{
    g_fp = fopen(path, "wb");
    if(g_fp == NULL) {
        fprintf(stderr, "[-] Error opening report file.\n");
        return -1;
    }

    return 0;
}

void report_close()
{
    if(g_fp != NULL) {
        fclose(g_fp);
    }
}

void report(const char *fmt, ...)
{
    va_list args; int len;
    va_start(args, fmt);
    len = _report_sprintf(NULL, fmt, args);
    if(len > 0) {
        char buf[len + 1];
        _report_sprintf(buf, fmt, args);
        va_end(args);

        static CRITICAL_SECTION cs; static int init = 0;
        if(init == 0) {
            InitializeCriticalSection(&cs);
            init = 1;
        }

        EnterCriticalSection(&cs);
        fwrite(buf, 1, len, g_fp);
        fflush(g_fp);
        LeaveCriticalSection(&cs);
    }
}

void hexdump(const void *addr, int length, const char *msg)
{
    report("[x] Hexdump 0x%x..0x%x: %z",
        addr, (uintptr_t) addr + length, msg);

    if(length > 0x1000) length = 0x1000;

    const uint8_t *ptr = (const uint8_t *) addr; char buf[128];
    for (uint32_t offset = 0; length > 0; offset += 16, length -= 16) {
        memset(buf, ' ', sizeof(buf));
        sprintf(buf, "%04x  ", offset);
        for (int i = 0; i < (length > 16 ? 16 : length); i++, ptr++) {
            sprintf(buf + i * 3 + 6 + (i > 7), "%02x ", *ptr);
            buf[i * 3 + 9 + (i > 7)] = ' ';
            buf[16 * 3 + 9 + i] = *ptr >= 0x20 && *ptr < 0x7e ? *ptr : '.';
        }
        buf[8 * 3 + 6] = buf[16 * 3 + 7] = buf[16 * 3 + 8] = ' ';
        buf[16 * 4 + 7] = 0;
        report("%z", buf);
    }
}
