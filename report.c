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
    int ret = 0;
    while (len-- != 0) {
        ret += _report_utf8x(out, *(unsigned char *) s++);
    }
    return ret;
}

static int _report_unicode(char **out, const wchar_t *s, int len)
{
    int ret = 0;
    while (len-- != 0) {
        ret += _report_utf8x(out, *(unsigned short *) s++);
    }
    return ret;
}

static int _report_variant(char **out, const VARIANT *v)
{
    (void) out; (void) v;
    // TODO
    return 0;
}

static int _report_sprintf(char *out, const char *fmt, va_list args)
{
    int ret = 0;
    while (*fmt != 0) {
        if(*fmt != '%') {
            ret += _report_utf8x(&out, *fmt++);
            continue;
        }
        if(*++fmt == 'z') {
            const char *s = va_arg(args, const char *);
            if(s == NULL) return -1;

            ret += _report_ascii(&out, s, strlen(s));
        }
        else if(*fmt == 'Z') {
            const wchar_t *s = va_arg(args, const wchar_t *);
            if(s == NULL) return -1;

            ret += _report_unicode(&out, s, lstrlenW(s));
        }
        else if(*fmt == 's') {
            int len = va_arg(args, int);
            const char *s = va_arg(args, const char *);
            if(s == NULL) return -1;

            ret += _report_ascii(&out, s, len);
        }
        else if(*fmt == 'S') {
            int len = va_arg(args, int);
            const wchar_t *s = va_arg(args, const wchar_t *);
            if(s == NULL) return -1;

            ret += _report_unicode(&out, s, len);
        }
        else if(*fmt == 'd') {
            char s[32];
            sprintf(s, "%d", va_arg(args, int));
            ret += _report_ascii(&out, s, strlen(s));
        }
        else if(*fmt == 'u') {
            char s[32];
            sprintf(s, "%u", va_arg(args, int));
            ret += _report_ascii(&out, s, strlen(s));
        }
        else if(*fmt == 'x') {
            char s[16];
            sprintf(s, "%x", va_arg(args, int));
            ret += _report_ascii(&out, s, strlen(s));
        }
        else if(*fmt == 'b') {
            wchar_t *s = va_arg(args, wchar_t *);
            uint32_t len = *(uint32_t *)((uint8_t *) s - sizeof(uint32_t));
            ret += _report_unicode(&out, s, len);
        }
        else if(*fmt == 'v') {
            ret += _report_variant(&out, va_arg(args, VARIANT *));
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
