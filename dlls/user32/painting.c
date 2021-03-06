/*
 * Copyright 2017 André Hentschel
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

#include "thunk/qemu_windows.h"

#include "windows-user-services.h"
#include "dll_list.h"
#include "qemu_user32.h"

#ifndef QEMU_DLL_GUEST
#include <wine/debug.h>
WINE_DEFAULT_DEBUG_CHANNEL(qemu_user32);
#endif

struct qemu_BeginPaint
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t lps;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI HDC WINAPI BeginPaint(HWND hwnd, PAINTSTRUCT *lps)
{
    struct qemu_BeginPaint call;
    call.super.id = QEMU_SYSCALL_ID(CALL_BEGINPAINT);
    call.hwnd = (ULONG_PTR)hwnd;
    call.lps = (ULONG_PTR)lps;

    qemu_syscall(&call.super);

    return (HDC)(ULONG_PTR)call.super.iret;
}

#else

void qemu_BeginPaint(struct qemu_syscall *call)
{
    struct qemu_BeginPaint *c = (struct qemu_BeginPaint *)call;
    PAINTSTRUCT copy, *paint = &copy;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    paint = QEMU_G2H(c->lps);
#else
    if (!c->lps)
        paint = NULL;
#endif

    c->super.iret = (ULONG_PTR)BeginPaint(QEMU_G2H(c->hwnd), paint);

#if GUEST_BIT != HOST_BIT
    if (c->lps)
        PAINTSTRUCT_h2g(QEMU_G2H(c->lps), paint);
#endif
}

#endif

struct qemu_EndPaint
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t lps;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI EndPaint(HWND hwnd, const PAINTSTRUCT *lps)
{
    struct qemu_EndPaint call;
    call.super.id = QEMU_SYSCALL_ID(CALL_ENDPAINT);
    call.hwnd = (ULONG_PTR)hwnd;
    call.lps = (ULONG_PTR)lps;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_EndPaint(struct qemu_syscall *call)
{
    struct qemu_EndPaint *c = (struct qemu_EndPaint *)call;
    PAINTSTRUCT copy, *paint = &copy;
    WINE_TRACE("\n");

#if GUEST_BIT == HOST_BIT
    paint = QEMU_G2H(c->lps);
#else
    if (c->lps)
    {
        PAINTSTRUCT_g2h(paint, QEMU_G2H(c->lps));
    }
    else
    {
        paint = NULL;
    }
#endif

    c->super.iret = (ULONG_PTR)EndPaint(QEMU_G2H(c->hwnd), paint);
}

#endif

struct qemu_GetDCEx
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t hrgnClip;
    uint64_t flags;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI HDC WINAPI GetDCEx(HWND hwnd, HRGN hrgnClip, DWORD flags)
{
    struct qemu_GetDCEx call;
    call.super.id = QEMU_SYSCALL_ID(CALL_GETDCEX);
    call.hwnd = (ULONG_PTR)hwnd;
    call.hrgnClip = (ULONG_PTR)hrgnClip;
    call.flags = (ULONG_PTR)flags;

    qemu_syscall(&call.super);

    return (HDC)(ULONG_PTR)call.super.iret;
}

#else

void qemu_GetDCEx(struct qemu_syscall *call)
{
    struct qemu_GetDCEx *c = (struct qemu_GetDCEx *)call;
    WINE_TRACE("\n");
    c->super.iret = (ULONG_PTR)GetDCEx(QEMU_G2H(c->hwnd), QEMU_G2H(c->hrgnClip), c->flags);
}

#endif

struct qemu_GetDC
{
    struct qemu_syscall super;
    uint64_t hwnd;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI HDC WINAPI GetDC(HWND hwnd)
{
    struct qemu_GetDC call;
    call.super.id = QEMU_SYSCALL_ID(CALL_GETDC);
    call.hwnd = (ULONG_PTR)hwnd;

    qemu_syscall(&call.super);

    return (HDC)(ULONG_PTR)call.super.iret;
}

#else

void qemu_GetDC(struct qemu_syscall *call)
{
    struct qemu_GetDC *c = (struct qemu_GetDC *)call;
    WINE_TRACE("\n");
    c->super.iret = (ULONG_PTR)GetDC(QEMU_G2H(c->hwnd));
}

#endif

struct qemu_GetWindowDC
{
    struct qemu_syscall super;
    uint64_t hwnd;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI HDC WINAPI GetWindowDC(HWND hwnd)
{
    struct qemu_GetWindowDC call;
    call.super.id = QEMU_SYSCALL_ID(CALL_GETWINDOWDC);
    call.hwnd = (ULONG_PTR)hwnd;

    qemu_syscall(&call.super);

    return (HDC)(ULONG_PTR)call.super.iret;
}

#else

void qemu_GetWindowDC(struct qemu_syscall *call)
{
    struct qemu_GetWindowDC *c = (struct qemu_GetWindowDC *)call;
    WINE_TRACE("\n");
    c->super.iret = (ULONG_PTR)GetWindowDC(QEMU_G2H(c->hwnd));
}

#endif

struct qemu_ReleaseDC
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t hdc;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI INT WINAPI ReleaseDC(HWND hwnd, HDC hdc)
{
    struct qemu_ReleaseDC call;
    call.super.id = QEMU_SYSCALL_ID(CALL_RELEASEDC);
    call.hwnd = (ULONG_PTR)hwnd;
    call.hdc = (ULONG_PTR)hdc;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ReleaseDC(struct qemu_syscall *call)
{
    struct qemu_ReleaseDC *c = (struct qemu_ReleaseDC *)call;
    WINE_TRACE("\n");
    c->super.iret = ReleaseDC(QEMU_G2H(c->hwnd), QEMU_G2H(c->hdc));
}

#endif

struct qemu_WindowFromDC
{
    struct qemu_syscall super;
    uint64_t hdc;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI HWND WINAPI WindowFromDC(HDC hdc)
{
    struct qemu_WindowFromDC call;
    call.super.id = QEMU_SYSCALL_ID(CALL_WINDOWFROMDC);
    call.hdc = (ULONG_PTR)hdc;

    qemu_syscall(&call.super);

    return (HWND)(ULONG_PTR)call.super.iret;
}

#else

void qemu_WindowFromDC(struct qemu_syscall *call)
{
    struct qemu_WindowFromDC *c = (struct qemu_WindowFromDC *)call;
    WINE_TRACE("\n");
    c->super.iret = (ULONG_PTR)WindowFromDC(QEMU_G2H(c->hdc));
}

#endif

struct qemu_LockWindowUpdate
{
    struct qemu_syscall super;
    uint64_t hwnd;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI LockWindowUpdate(HWND hwnd)
{
    struct qemu_LockWindowUpdate call;
    call.super.id = QEMU_SYSCALL_ID(CALL_LOCKWINDOWUPDATE);
    call.hwnd = (ULONG_PTR)hwnd;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_LockWindowUpdate(struct qemu_syscall *call)
{
    struct qemu_LockWindowUpdate *c = (struct qemu_LockWindowUpdate *)call;
    WINE_TRACE("\n");
    c->super.iret = LockWindowUpdate(QEMU_G2H(c->hwnd));
}

#endif

struct qemu_RedrawWindow
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t rect;
    uint64_t hrgn;
    uint64_t flags;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI RedrawWindow(HWND hwnd, const RECT *rect, HRGN hrgn, UINT flags)
{
    struct qemu_RedrawWindow call;
    call.super.id = QEMU_SYSCALL_ID(CALL_REDRAWWINDOW);
    call.hwnd = (ULONG_PTR)hwnd;
    call.rect = (ULONG_PTR)rect;
    call.hrgn = (ULONG_PTR)hrgn;
    call.flags = (ULONG_PTR)flags;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_RedrawWindow(struct qemu_syscall *call)
{
    struct qemu_RedrawWindow *c = (struct qemu_RedrawWindow *)call;
    WINE_TRACE("\n");
    c->super.iret = RedrawWindow(QEMU_G2H(c->hwnd), QEMU_G2H(c->rect), QEMU_G2H(c->hrgn), c->flags);
}

#endif

struct qemu_UpdateWindow
{
    struct qemu_syscall super;
    uint64_t hwnd;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI UpdateWindow(HWND hwnd)
{
    struct qemu_UpdateWindow call;
    call.super.id = QEMU_SYSCALL_ID(CALL_UPDATEWINDOW);
    call.hwnd = (ULONG_PTR)hwnd;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_UpdateWindow(struct qemu_syscall *call)
{
    struct qemu_UpdateWindow *c = (struct qemu_UpdateWindow *)call;
    WINE_TRACE("\n");
    c->super.iret = UpdateWindow(QEMU_G2H(c->hwnd));
}

#endif

struct qemu_InvalidateRgn
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t hrgn;
    uint64_t erase;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI InvalidateRgn(HWND hwnd, HRGN hrgn, BOOL erase)
{
    struct qemu_InvalidateRgn call;
    call.super.id = QEMU_SYSCALL_ID(CALL_INVALIDATERGN);
    call.hwnd = (ULONG_PTR)hwnd;
    call.hrgn = (ULONG_PTR)hrgn;
    call.erase = (ULONG_PTR)erase;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_InvalidateRgn(struct qemu_syscall *call)
{
    struct qemu_InvalidateRgn *c = (struct qemu_InvalidateRgn *)call;
    WINE_TRACE("\n");
    c->super.iret = InvalidateRgn(QEMU_G2H(c->hwnd), QEMU_G2H(c->hrgn), c->erase);
}

#endif

struct qemu_InvalidateRect
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t rect;
    uint64_t erase;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI InvalidateRect(HWND hwnd, const RECT *rect, BOOL erase)
{
    struct qemu_InvalidateRect call;
    call.super.id = QEMU_SYSCALL_ID(CALL_INVALIDATERECT);
    call.hwnd = (ULONG_PTR)hwnd;
    call.rect = (ULONG_PTR)rect;
    call.erase = (ULONG_PTR)erase;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_InvalidateRect(struct qemu_syscall *call)
{
    struct qemu_InvalidateRect *c = (struct qemu_InvalidateRect *)call;
    WINE_TRACE("\n");
    c->super.iret = InvalidateRect(QEMU_G2H(c->hwnd), QEMU_G2H(c->rect), c->erase);
}

#endif

struct qemu_ValidateRgn
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t hrgn;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI ValidateRgn(HWND hwnd, HRGN hrgn)
{
    struct qemu_ValidateRgn call;
    call.super.id = QEMU_SYSCALL_ID(CALL_VALIDATERGN);
    call.hwnd = (ULONG_PTR)hwnd;
    call.hrgn = (ULONG_PTR)hrgn;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ValidateRgn(struct qemu_syscall *call)
{
    struct qemu_ValidateRgn *c = (struct qemu_ValidateRgn *)call;
    WINE_TRACE("\n");
    c->super.iret = ValidateRgn(QEMU_G2H(c->hwnd), QEMU_G2H(c->hrgn));
}

#endif

struct qemu_ValidateRect
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t rect;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI ValidateRect(HWND hwnd, const RECT *rect)
{
    struct qemu_ValidateRect call;
    call.super.id = QEMU_SYSCALL_ID(CALL_VALIDATERECT);
    call.hwnd = (ULONG_PTR)hwnd;
    call.rect = (ULONG_PTR)rect;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ValidateRect(struct qemu_syscall *call)
{
    struct qemu_ValidateRect *c = (struct qemu_ValidateRect *)call;
    WINE_TRACE("\n");
    c->super.iret = ValidateRect(QEMU_G2H(c->hwnd), QEMU_G2H(c->rect));
}

#endif

struct qemu_GetUpdateRgn
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t hrgn;
    uint64_t erase;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI INT WINAPI GetUpdateRgn(HWND hwnd, HRGN hrgn, BOOL erase)
{
    struct qemu_GetUpdateRgn call;
    call.super.id = QEMU_SYSCALL_ID(CALL_GETUPDATERGN);
    call.hwnd = (ULONG_PTR)hwnd;
    call.hrgn = (ULONG_PTR)hrgn;
    call.erase = (ULONG_PTR)erase;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_GetUpdateRgn(struct qemu_syscall *call)
{
    struct qemu_GetUpdateRgn *c = (struct qemu_GetUpdateRgn *)call;
    WINE_TRACE("\n");
    c->super.iret = GetUpdateRgn(QEMU_G2H(c->hwnd), QEMU_G2H(c->hrgn), c->erase);
}

#endif

struct qemu_GetUpdateRect
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t rect;
    uint64_t erase;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI GetUpdateRect(HWND hwnd, LPRECT rect, BOOL erase)
{
    struct qemu_GetUpdateRect call;
    call.super.id = QEMU_SYSCALL_ID(CALL_GETUPDATERECT);
    call.hwnd = (ULONG_PTR)hwnd;
    call.rect = (ULONG_PTR)rect;
    call.erase = (ULONG_PTR)erase;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_GetUpdateRect(struct qemu_syscall *call)
{
    struct qemu_GetUpdateRect *c = (struct qemu_GetUpdateRect *)call;
    WINE_TRACE("\n");
    c->super.iret = GetUpdateRect(QEMU_G2H(c->hwnd), QEMU_G2H(c->rect), c->erase);
}

#endif

struct qemu_ExcludeUpdateRgn
{
    struct qemu_syscall super;
    uint64_t hdc;
    uint64_t hwnd;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI INT WINAPI ExcludeUpdateRgn(HDC hdc, HWND hwnd)
{
    struct qemu_ExcludeUpdateRgn call;
    call.super.id = QEMU_SYSCALL_ID(CALL_EXCLUDEUPDATERGN);
    call.hdc = (ULONG_PTR)hdc;
    call.hwnd = (ULONG_PTR)hwnd;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ExcludeUpdateRgn(struct qemu_syscall *call)
{
    struct qemu_ExcludeUpdateRgn *c = (struct qemu_ExcludeUpdateRgn *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = ExcludeUpdateRgn(QEMU_G2H(c->hdc), QEMU_G2H(c->hwnd));
}

#endif

struct qemu_ScrollWindowEx
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t dx;
    uint64_t dy;
    uint64_t rect;
    uint64_t clipRect;
    uint64_t hrgnUpdate;
    uint64_t rcUpdate;
    uint64_t flags;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI INT WINAPI ScrollWindowEx(HWND hwnd, INT dx, INT dy, const RECT *rect, const RECT *clipRect,
        HRGN hrgnUpdate, LPRECT rcUpdate, UINT flags)
{
    struct qemu_ScrollWindowEx call;
    call.super.id = QEMU_SYSCALL_ID(CALL_SCROLLWINDOWEX);
    call.hwnd = (ULONG_PTR)hwnd;
    call.dx = dx;
    call.dy = dy;
    call.rect = (ULONG_PTR)rect;
    call.clipRect = (ULONG_PTR)clipRect;
    call.hrgnUpdate = (ULONG_PTR)hrgnUpdate;
    call.rcUpdate = (ULONG_PTR)rcUpdate;
    call.flags = flags;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ScrollWindowEx(struct qemu_syscall *call)
{
    struct qemu_ScrollWindowEx *c = (struct qemu_ScrollWindowEx *)call;
    WINE_TRACE("\n");
    c->super.iret = ScrollWindowEx(QEMU_G2H(c->hwnd), c->dx, c->dy, QEMU_G2H(c->rect), QEMU_G2H(c->clipRect), QEMU_G2H(c->hrgnUpdate), QEMU_G2H(c->rcUpdate), c->flags);
}

#endif

struct qemu_ScrollWindow
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t dx;
    uint64_t dy;
    uint64_t rect;
    uint64_t clipRect;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI ScrollWindow(HWND hwnd, INT dx, INT dy, const RECT *rect, const RECT *clipRect)
{
    struct qemu_ScrollWindow call;
    call.super.id = QEMU_SYSCALL_ID(CALL_SCROLLWINDOW);
    call.hwnd = (ULONG_PTR)hwnd;
    call.dx = dx;
    call.dy = dy;
    call.rect = (ULONG_PTR)rect;
    call.clipRect = (ULONG_PTR)clipRect;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ScrollWindow(struct qemu_syscall *call)
{
    struct qemu_ScrollWindow *c = (struct qemu_ScrollWindow *)call;
    WINE_TRACE("\n");
    c->super.iret = ScrollWindow(QEMU_G2H(c->hwnd), c->dx, c->dy, QEMU_G2H(c->rect), QEMU_G2H(c->clipRect));
}

#endif

struct qemu_ScrollDC
{
    struct qemu_syscall super;
    uint64_t hdc;
    uint64_t dx;
    uint64_t dy;
    uint64_t scroll;
    uint64_t clip;
    uint64_t ret_update_rgn;
    uint64_t update_rect;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI ScrollDC(HDC hdc, INT dx, INT dy, const RECT *scroll, const RECT *clip, HRGN ret_update_rgn, LPRECT update_rect)
{
    struct qemu_ScrollDC call;
    call.super.id = QEMU_SYSCALL_ID(CALL_SCROLLDC);
    call.hdc = (ULONG_PTR)hdc;
    call.dx = (ULONG_PTR)dx;
    call.dy = (ULONG_PTR)dy;
    call.scroll = (ULONG_PTR)scroll;
    call.clip = (ULONG_PTR)clip;
    call.ret_update_rgn = (ULONG_PTR)ret_update_rgn;
    call.update_rect = (ULONG_PTR)update_rect;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

void qemu_ScrollDC(struct qemu_syscall *call)
{
    struct qemu_ScrollDC *c = (struct qemu_ScrollDC *)call;
    WINE_TRACE("\n");
    c->super.iret = ScrollDC(QEMU_G2H(c->hdc), c->dx, c->dy, QEMU_G2H(c->scroll), QEMU_G2H(c->clip), QEMU_G2H(c->ret_update_rgn), QEMU_G2H(c->update_rect));
}

#endif

struct qemu_PrintWindow
{
    struct qemu_syscall super;
    uint64_t hwnd;
    uint64_t hdcBlt;
    uint64_t nFlags;
};

#ifdef QEMU_DLL_GUEST

WINUSERAPI BOOL WINAPI PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags)
{
    struct qemu_PrintWindow call;
    call.super.id = QEMU_SYSCALL_ID(CALL_PRINTWINDOW);
    call.hwnd = (ULONG_PTR)hwnd;
    call.hdcBlt = (ULONG_PTR)hdcBlt;
    call.nFlags = (ULONG_PTR)nFlags;

    qemu_syscall(&call.super);

    return call.super.iret;
}

#else

/* TODO: Add PrintWindow to Wine headers? */
extern BOOL WINAPI PrintWindow(HWND hwnd, HDC hdcBlt, UINT nFlags);
void qemu_PrintWindow(struct qemu_syscall *call)
{
    struct qemu_PrintWindow *c = (struct qemu_PrintWindow *)call;
    WINE_FIXME("Unverified!\n");
    c->super.iret = PrintWindow(QEMU_G2H(c->hwnd), QEMU_G2H(c->hdcBlt), c->nFlags);
}

#endif

