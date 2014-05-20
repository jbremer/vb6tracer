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

static int _report_ascii(char **out, const char *s, int len)
{
    if(len == 0) {
        return _report_ascii(out, "<empty>", 7);
    }

    int ret = 0;
    while (len-- != 0) {
        ret += _report_utf8x(out, *(unsigned char *) s++);
    }
    return ret;
}

static int _report_unicode(char **out, const wchar_t *s, int len)
{
    if(len == 0) {
        return _report_ascii(out, "<empty>", 7);
    }

    int ret = 0;
    while (len-- != 0) {
        ret += _report_utf8x(out, *(unsigned short *) s++);
    }
    return ret;
}

static int _report_variant(char **out, const VARIANT *v)
{
    char buf[32]; int len;
    switch (v->vt) {
    case VT_NULL:
        return _report_ascii(out, "<null>", 6);

    case VT_BOOL:
        if(v->boolVal != 0) {
            return _report_ascii(out, "True", 4);
        }

        return _report_ascii(out, "False", 5);

    case VT_I1: case VT_UI1:
        return _report_ascii(out, buf, sprintf(buf, "%u", v->bVal));

    case VT_I2: case VT_UI2:
        return _report_ascii(out, buf, sprintf(buf, "%u", v->iVal));

    case VT_I4: case VT_UI4: case VT_INT: case VT_UINT:
        return _report_ascii(out, buf, sprintf(buf, "%u", v->intVal));

    case VT_I8: case VT_UI8:
        return _report_ascii(out, buf, sprintf(buf, "%I64u", v->llVal));

    case VT_LPSTR:
        if(v->pcVal == NULL) return _report_ascii(out, "<null>", 6);
        return _report_ascii(out, v->pcVal, strlen(v->pcVal));

    case VT_LPWSTR:
        if(v->pbVal == NULL) return _report_ascii(out, "<null>", 6);
        return _report_unicode(out, (const wchar_t *) v->pbVal,
                lstrlenW((const wchar_t *) v->pcVal));

    case VT_BSTR:
        if(v->bstrVal == NULL) return _report_ascii(out, "<null>", 6);

        len = *(int *)((uint8_t *) v->bstrVal - sizeof(int));
        return _report_unicode(out, v->bstrVal, len);

    case VT_VARIANT:
        return _report_variant(out, v->pvarVal);

    default:
        sprintf(buf, "<VT_%d>", v->vt);
        return _report_ascii(out, buf, strlen(buf));
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
                ret += _report_ascii(&out, "<null>", 6);
                break;
            }

            ret += _report_ascii(&out, s, strlen(s));
            break;

        case 'Z':
            w = va_arg(args, const wchar_t *);
            if(w == NULL) {
                ret += _report_ascii(&out, "<null>", 6);
                break;
            }

            ret += _report_unicode(&out, w, lstrlenW(w));
            break;

        case 's':
            len = va_arg(args, int);
            s = va_arg(args, const char *);
            if(s == NULL) {
                ret += _report_ascii(&out, "<null>", 6);
                break;
            }

            ret += _report_ascii(&out, s, len);
            break;

        case 'S':
            len = va_arg(args, int);
            w = va_arg(args, const wchar_t *);
            if(w == NULL) {
                ret += _report_ascii(&out, "<null>", 6);
                break;
            }

            ret += _report_unicode(&out, w, len);
            break;

        case 'd':
            sprintf(buf, "%d", va_arg(args, int));
            ret += _report_ascii(&out, buf, strlen(buf));
            break;

        case 'u':
            sprintf(buf, "%u", va_arg(args, int));
            ret += _report_ascii(&out, buf, strlen(buf));
            break;

        case 'x':
            sprintf(buf, "%08x", va_arg(args, int));
            ret += _report_ascii(&out, buf, strlen(buf));
            break;

        case 'b':
            w = va_arg(args, wchar_t *);
            if(w == NULL) {
                ret += _report_ascii(&out, "<null>", 6);
                break;
            }

            len = *(int *)((uint8_t *) w - sizeof(int));
            ret += _report_unicode(&out, w, len >> 1);
            break;

        case 'v':
            v = va_arg(args, const VARIANT *);
            if(v == NULL) {
                ret += _report_ascii(&out, "<null>", 6);
                break;
            }

            ret += _report_variant(&out, v);
            break;
        }
        fmt++;
    }
    _report_utf8x(&out, '\n');
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

        fwrite(buf, 1, len, g_fp);
        fflush(g_fp);
    }
}
