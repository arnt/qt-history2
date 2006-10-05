/**
   These are just defines we need to compile this across platforms
**/
/*
 *
 * Copyright © 2000 SuSE, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of SuSE not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  SuSE makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * SuSE DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL SuSE
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Keith Packard, SuSE, Inc.
 */

#ifndef _XRENDER_FAKE_H_
#define _XRENDER_FAKE_H_

#ifndef _XRENDER_H_
typedef double	XDouble;

typedef struct _XPointDouble {
    XDouble  x, y;
} XPointDouble;

#define XDoubleToFixed(f)    ((XFixed) ((f) * 65536))
#define XFixedToDouble(f)    (((XDouble) (f)) / 65536)

typedef int XFixed;

typedef struct _XPointFixed {
    XFixed  x, y;
} XPointFixed;

typedef struct _XLineFixed {
    XPointFixed	p1, p2;
} XLineFixed;

typedef struct _XTriangle {
    XPointFixed	p1, p2, p3;
} XTriangle;

typedef struct _XTrapezoid {
    XFixed  top, bottom;
    XLineFixed	left, right;
} XTrapezoid;

typedef struct _XSpanFix {
    XFixed	    left, right, y;
} XSpanFix;

typedef struct _XTrap {
    XSpanFix	    top, bottom;
} XTrap;

#endif

#endif /* _XRENDER_H_ */
