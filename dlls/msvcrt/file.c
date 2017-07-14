/*
 * Copyright 2017 Stefan Dösinger for CodeWeavers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

/* NOTE: The guest side uses mingw's headers. The host side uses Wine's headers. */

#include <windows.h>
#include <stdio.h>

#include "windows-user-services.h"
#include "dll_list.h"
#include "va_helper.h"
#include "msvcrt.h"

#ifndef QEMU_DLL_GUEST
#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(qemu_msvcrt);
#endif

#ifdef QEMU_DLL_GUEST

/* FIXME: Passing the IOB and FILE pointers like this only works when they have the same size
 * in guest and host. */
FILE * CDECL MSVCRT___iob_func(void)
{
    struct qemu_syscall call;
    call.id = QEMU_SYSCALL_ID(CALL___IOB_FUNC);

    qemu_syscall(&call);

    return (FILE *)call.iret;
}

#else

void qemu___iob_func(struct qemu_syscall *c)
{
    WINE_TRACE("\n");
    c->iret = QEMU_H2G(p___iob_func());
}

#endif

struct qemu_fprintf
{
    struct qemu_syscall super;
    uint64_t argcount, argcount_float;
    uint64_t file;
    uint64_t format;
    uint64_t MSVCRT_FILE_size;
    struct va_array args[1];
};

#ifdef QEMU_DLL_GUEST

static unsigned int count_printf_argsA(const char *format, char *fmts)
{
    unsigned int i, count = 0;
    BOOL fmt_start = FALSE;

    for (i = 0; format[i]; ++i)
    {
        if (!fmt_start)
        {
            if (format[i] != '%')
                continue;
            i++;
        }

        if (format[i] == '%')
            continue;

        switch (format[i])
        {
            case 'A':
            case 'a':
            case 'c':
            case 'd':
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
            case 'i':
            case 'n':
            case 'o':
            case 'p':
            case 's':
            case 'u':
            case 'X':
            case 'x':
                fmts[count++] = format[i];
                if (count == 256)
                    MSVCRT_exit(255);
                fmt_start = FALSE;
                break;

            default:
                fmt_start = TRUE;
                break;
        }
    }

    return count;
}

/* Looking up "stdout" requires a call out of the VM to get __iob, so do
 * the printf(...) -> fprintf(stdout, ...) wrapping outside the VM. */
static int CDECL vfprintf_helper(uint64_t op, FILE *file, const char *format, va_list args)
{
    struct qemu_fprintf *call;
    int ret;
    char fmts[256] = {0};
    unsigned int count = count_printf_argsA(format, fmts), i, arg = 0;
    union
    {
        double d;
        uint64_t i;
    } conv;

    call = MSVCRT_malloc(offsetof(struct qemu_fprintf, args[count]));
    call->super.id = op;
    call->argcount = count;
    call->file = (uint64_t)file;
    call->format = (uint64_t)format;
    call->MSVCRT_FILE_size = sizeof(FILE);
    call->argcount_float = 0;

    for (i = 0; i < count; ++i)
    {
        switch (fmts[i])
        {
            case 'A':
            case 'a':
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
                conv.d = va_arg(args, double);
                call->args[i].is_float = TRUE;
                call->args[i].arg = conv.i;
                call->argcount_float++;
                break;

            default:
                call->args[i].is_float = FALSE;
                call->args[i].arg = va_arg(args, uint64_t);
                break;
        }
    }

    qemu_syscall(&call->super);
    ret = call->super.iret;

    MSVCRT_free(call);

    return ret;
}

int CDECL MSVCRT_vfprintf(FILE *file, const char *format, va_list args)
{
    vfprintf_helper(QEMU_SYSCALL_ID(CALL_FPRINTF), file, format, args);
}

int CDECL MSVCRT_fprintf(FILE *file, const char *format, ...)
{
    int ret;
    va_list list;

    va_start(list, format);
    ret = vfprintf_helper(QEMU_SYSCALL_ID(CALL_FPRINTF), file, format, list);
    va_end(list);

    return ret;
}

int CDECL MSVCRT_printf(const char *format, ...)
{
    int ret;
    va_list list;

    va_start(list, format);
    ret = vfprintf_helper(QEMU_SYSCALL_ID(CALL_PRINTF), NULL, format, list);
    va_end(list);

    return ret;
}

static unsigned int count_printf_argsW(const WCHAR *format, WCHAR *fmts)
{
    unsigned int i, count = 0;
    BOOL fmt_start = FALSE;

    for (i = 0; format[i]; ++i)
    {
        if (!fmt_start)
        {
            if (format[i] != '%')
                continue;
            i++;
        }

        if (format[i] == '%')
            continue;

        switch (format[i])
        {
            case 'A':
            case 'a':
            case 'c':
            case 'd':
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
            case 'i':
            case 'n':
            case 'o':
            case 'p':
            case 's':
            case 'u':
            case 'X':
            case 'x':
                fmts[count++] = format[i];
                if (count == 256)
                    MSVCRT_exit(255);
                fmt_start = FALSE;
                break;

            default:
                fmt_start = TRUE;
                break;
        }
    }

    return count;
}

static int CDECL vfwprintf_helper(uint64_t op, FILE *file, const WCHAR *format, va_list args)
{
    struct qemu_fprintf *call;
    int ret;
    WCHAR fmts[256] = {0};
    unsigned int count = count_printf_argsW(format, fmts), i, arg = 0;
    union
    {
        double d;
        uint64_t i;
    } conv;

    call = MSVCRT_malloc(offsetof(struct qemu_fprintf, args[count]));
    call->super.id = op;
    call->argcount = count;
    call->file = (uint64_t)file;
    call->format = (uint64_t)format;
    call->MSVCRT_FILE_size = sizeof(FILE);
    call->argcount_float = 0;

    for (i = 0; i < count; ++i)
    {
        switch (fmts[i])
        {
            case 'A':
            case 'a':
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
                conv.d = va_arg(args, double);
                call->args[i].is_float = TRUE;
                call->args[i].arg = conv.i;
                call->argcount_float++;
                break;

            default:
                call->args[i].is_float = FALSE;
                call->args[i].arg = va_arg(args, uint64_t);
                break;
        }
    }

    qemu_syscall(&call->super);
    ret = call->super.iret;

    MSVCRT_free(call);

    return ret;
}

int CDECL MSVCRT_vfwprintf(FILE *file, const WCHAR *format, va_list args)
{
    vfwprintf_helper(QEMU_SYSCALL_ID(CALL_FWPRINTF), file, format, args);
}

int CDECL MSVCRT_fwprintf(FILE *file, const WCHAR *format, ...)
{
    int ret;
    va_list list;

    va_start(list, format);
    ret = vfwprintf_helper(QEMU_SYSCALL_ID(CALL_FWPRINTF), file, format, list);
    va_end(list);

    return ret;
}

int CDECL MSVCRT_wprintf(const WCHAR *format, ...)
{
    int ret;
    va_list list;

    va_start(list, format);
    ret = vfwprintf_helper(QEMU_SYSCALL_ID(CALL_WPRINTF), NULL, format, list);
    va_end(list);

    return ret;
}

#else

struct printf_data
{
    void *file;
    void *fmt; 
    BOOL unicode;
};

static uint64_t printf_wrapper(void *ctx, ...)
{
    va_list list;
    const struct printf_data *data = ctx;
    int ret;

    va_start(list, ctx);
    if (data->unicode)
        ret = p_vfwprintf(data->file, data->fmt, list);
    else
        ret = p_vfprintf(data->file, data->fmt, list);
    va_end(list);

    return ret;
}

void qemu_fprintf(struct qemu_syscall *call)
{
    struct qemu_fprintf *c = (struct qemu_fprintf *)call;
    int ret;
    void *file;
    struct printf_data data;

    WINE_TRACE("(%lu floats/%lu args, format \"%s\"\n", c->argcount_float, c->argcount, (char *)QEMU_G2H(c->format));

    switch (c->super.id)
    {
        case QEMU_SYSCALL_ID(CALL_PRINTF):
            /* Don't put "stdout" here, it will call the Linux libc __iob_func export.
             * Plus, the size of FILE is different between Linux and Windows, and I
             * haven't found a nice way to get MSVCRT_FILE from Wine, other than
             * copypasting it, so grab the proper offset from the VM. */
            data.file = (BYTE *)p___iob_func() + c->MSVCRT_FILE_size;
            data.unicode = FALSE;
            break;

        case QEMU_SYSCALL_ID(CALL_FPRINTF):
            data.file = QEMU_G2H(c->file);
            data.unicode = FALSE;
            break;

        case QEMU_SYSCALL_ID(CALL_WPRINTF):
            data.file = (BYTE *)p___iob_func() + c->MSVCRT_FILE_size;
            data.unicode = TRUE;
            break;

        case QEMU_SYSCALL_ID(CALL_FWPRINTF):
            data.file = QEMU_G2H(c->file);
            data.unicode = TRUE;
            break;

        default:
            WINE_ERR("Unexpected op %lx\n", c->super.id);
    }
    data.fmt = QEMU_G2H(c->format);

    ret = call_va(printf_wrapper, &data, c->argcount, c->argcount_float, c->args);

    c->super.iret = ret;
}

#endif

struct qemu_sprintf
{
    struct qemu_syscall super;
    uint64_t argcount, argcount_float;
    uint64_t str;
    uint64_t format;
    struct va_array args[1];
};

#ifdef QEMU_DLL_GUEST

static int CDECL vsprintf_helper(uint64_t op, char *str, const char *format, va_list args)
{
    struct qemu_sprintf *call;
    int ret;
    char fmts[256] = {0};
    unsigned int count = count_printf_argsA(format, fmts), i, arg = 0;
    union
    {
        double d;
        uint64_t i;
    } conv;

    call = MSVCRT_malloc(offsetof(struct qemu_sprintf, args[count]));
    call->super.id = op;
    call->argcount = count;
    call->str = (uint64_t)str;
    call->format = (uint64_t)format;
    call->argcount_float = 0;

    for (i = 0; i < count; ++i)
    {
        switch (fmts[i])
        {
            case 'A':
            case 'a':
            case 'E':
            case 'e':
            case 'F':
            case 'f':
            case 'G':
            case 'g':
                conv.d = va_arg(args, double);
                call->args[i].is_float = TRUE;
                call->args[i].arg = conv.i;
                call->argcount_float++;
                break;

            default:
                call->args[i].is_float = FALSE;
                call->args[i].arg = va_arg(args, uint64_t);
                break;
        }
    }

    qemu_syscall(&call->super);
    ret = call->super.iret;

    MSVCRT_free(call);

    return ret;
}

int CDECL MSVCRT_sprintf(char *str, const char *format, ...)
{
    int ret;
    va_list list;

    va_start(list, format);
    ret = vsprintf_helper(QEMU_SYSCALL_ID(CALL_SPRINTF), str, format, list);
    va_end(list);

    return ret;
}

#else

struct sprintf_data
{
    void *dst;
    void *fmt;
};

static uint64_t sprintf_wrapper(void *ctx, ...)
{
    va_list list;
    const struct sprintf_data *data = ctx;
    int ret;

    va_start(list, ctx);
    ret = p_vsprintf(data->dst, data->fmt, list);
    va_end(list);

    return ret;
}

void qemu_sprintf(struct qemu_syscall *call)
{
    struct qemu_sprintf *c = (struct qemu_sprintf *)call;
    struct sprintf_data data;
    int ret;

    WINE_FIXME("(%lu floats/%lu args, format \"%s\"\n", c->argcount_float, c->argcount, (char *)QEMU_G2H(c->format));

    data.dst = QEMU_G2H(c->str);
    data.fmt = QEMU_G2H(c->format);
    ret = call_va(sprintf_wrapper, &data, c->argcount, c->argcount_float, c->args);

    c->super.iret = ret;
}

#endif

struct qemu_fwrite
{
    struct qemu_syscall super;
    uint64_t str;
    uint64_t size, count;
    uint64_t file;
};


#ifdef QEMU_DLL_GUEST

size_t CDECL MSVCRT_fwrite(const void *str, size_t size, size_t count, FILE *file)
{
    struct qemu_fwrite call;
    call.super.id = QEMU_SYSCALL_ID(CALL_FWRITE);
    call.str = (uint64_t)str;
    call.size = (uint64_t)size;
    call.count = (uint64_t)count;
    call.file = (uint64_t)file;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_fwrite(struct qemu_syscall *call)
{
    struct qemu_fwrite *c = (struct qemu_fwrite *)call;
    WINE_TRACE("\n");
    c->super.iret = (uint64_t)p_fwrite(QEMU_G2H(c->str), c->size, c->count, QEMU_G2H(c->file));
}

#endif

struct qemu_puts
{
    struct qemu_syscall super;
    uint64_t str;
};

#ifdef QEMU_DLL_GUEST

int CDECL MSVCRT_puts(const char *str)
{
    struct qemu_puts call;
    call.super.id = QEMU_SYSCALL_ID(CALL_PUTS);
    call.str = (uint64_t)str;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_puts(struct qemu_syscall *call)
{
    struct qemu_puts *c = (struct qemu_puts *)call;
    WINE_TRACE("\n");
    c->super.iret = p_puts(QEMU_G2H(c->str));
}

#endif
