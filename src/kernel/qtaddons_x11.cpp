/*
 * $XFree86: xc/lib/Xft/xftname.c,v 1.10 2001/03/30 18:50:18 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "qt_x11.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef QT_NO_XFTFREETYPE
#ifdef QT_NO_XFTNAMEUNPARSE

extern "C" {

#include <X11/Xft/XftFreetype.h>


typedef struct _XftObjectType {
    const char  *object;
    XftType     type;
} XftObjectType;


static const XftObjectType _XftObjectTypes[] = {
    { XFT_FAMILY,       XftTypeString, },
    { XFT_STYLE,        XftTypeString, },
    { XFT_SLANT,        XftTypeInteger, },
    { XFT_WEIGHT,       XftTypeInteger, },
    { XFT_SIZE,         XftTypeDouble, },
    { XFT_PIXEL_SIZE,   XftTypeDouble, },
    { XFT_ENCODING,     XftTypeString, },
    { XFT_SPACING,      XftTypeInteger, },
    { XFT_FOUNDRY,      XftTypeString, },
    { XFT_CORE,         XftTypeBool, },
    { XFT_ANTIALIAS,    XftTypeBool, },
    { XFT_XLFD,         XftTypeString, },
    { XFT_FILE,         XftTypeString, },
    { XFT_INDEX,        XftTypeInteger, },
    { XFT_RASTERIZER,   XftTypeString, },
    { XFT_OUTLINE,      XftTypeBool, },
    { XFT_SCALABLE,     XftTypeBool, },
    { XFT_RGBA,         XftTypeInteger, },
    { XFT_SCALE,        XftTypeDouble, },
    { XFT_RENDER,       XftTypeBool, },
};


#define NUM_OBJECT_TYPES    (sizeof _XftObjectTypes / sizeof _XftObjectTypes[0])


static Bool
_XftNameUnparseString (const char *string, char *escape, char **destp, int *lenp)
{
    int	    len = *lenp;
    char    *dest = *destp;
    char    c;

    while ((c = *string++))
    {
	if (escape && strchr (escape, c))
	{
	    if (len-- == 0)
		return False;
	    *dest++ = escape[0];
	}
	if (len-- == 0)
	    return False;
	*dest++ = c;
    }
    *destp = dest;
    *lenp = len;
    return True;
}

static Bool
_XftNameUnparseValue (XftValue v, char *escape, char **destp, int *lenp)
{
    char    temp[1024];

    switch (v.type) {
    case XftTypeVoid:
	return True;
    case XftTypeInteger:
	sprintf (temp, "%d", v.u.i);
	return _XftNameUnparseString (temp, 0, destp, lenp);
    case XftTypeDouble:
	sprintf (temp, "%g", v.u.d);
	return _XftNameUnparseString (temp, 0, destp, lenp);
    case XftTypeString:
	return _XftNameUnparseString (v.u.s, escape, destp, lenp);
    case XftTypeBool:
	return _XftNameUnparseString (v.u.b ? "True" : "False", 0, destp, lenp);

	/*
	  case XftTypeMatrix:
	  sprintf (temp, "%g %g %g %g",
	  v.u.m->xx, v.u.m->xy, v.u.m->yx, v.u.m->yy);
	  return _XftNameUnparseString (temp, 0, destp, lenp);
	*/
    }
    return False;
}

static Bool
_XftNameUnparseValueList (XftValueList *v, char *escape, char **destp, int *lenp)
{
    while (v)
    {
	if (!_XftNameUnparseValue (v->value, escape, destp, lenp))
	    return False;
	if ((v = v->next))
	    if (!_XftNameUnparseString (",", 0, destp, lenp))
		return False;
    }
    return True;
}

#define XFT_ESCAPE_FIXED    "\\-:,"
#define XFT_ESCAPE_VARIABLE "\\=_:,"

Bool
XftNameUnparse (XftPattern *pat, char *dest, int len)
{
    unsigned int	i;
    XftPatternElt	*e;
    const XftObjectType *o;

    e = XftPatternFind (pat, XFT_FAMILY, False);
    if (e)
	{
	    if (!_XftNameUnparseValueList (e->values, XFT_ESCAPE_FIXED,
					   &dest, &len))
		return False;
	}
    e = XftPatternFind (pat, XFT_SIZE, False);
    if (e)
	{
	    if (!_XftNameUnparseString ("-", 0, &dest, &len))
		return False;
	    if (!_XftNameUnparseValueList (e->values, XFT_ESCAPE_FIXED, &dest, &len))
		return False;
	}
    for (i = 0; i < NUM_OBJECT_TYPES; i++)
	{
	    o = &_XftObjectTypes[i];
	    if (!strcmp (o->object, XFT_FAMILY) ||
		!strcmp (o->object, XFT_SIZE) ||
		!strcmp (o->object, XFT_FILE))
		continue;

	    e = XftPatternFind (pat, o->object, False);
	    if (e)
		{
		    if (!_XftNameUnparseString (":", 0, &dest, &len))
			return False;
		    if (!_XftNameUnparseString (o->object, XFT_ESCAPE_VARIABLE,
						&dest, &len))
			return False;
		    if (!_XftNameUnparseString ("=", 0, &dest, &len))
			return False;
		    if (!_XftNameUnparseValueList (e->values, XFT_ESCAPE_VARIABLE,
						   &dest, &len))
			return False;
		}
	}
    if (len == 0)
	return False;
    *dest = '\0';
    return True;
}

}

#endif // QT_NO_XFTNAMEUNPARSE




#ifndef QT_XFT2

#include <X11/Xft/Xft.h>

extern bool qt_use_xrender; // defined in qapplication_x11.cpp

extern "C" {
#define XFT_DRAW_N_SRC 2
struct _XftDraw {
    Display *dpy;
    Drawable drawable;
    Visual *visual;
    Colormap colormap;
    Region clip;
    Bool core_set;
    Bool render_set;
    Bool render_able;
    struct {
	Picture pict;
	struct {
	    Picture pict;
	    XRenderColor color;
	} src[XFT_DRAW_N_SRC];
    } render;
    struct {
	GC draw_gc;
	unsigned long fg;
	Font font;
    } core;
};

Picture XftDrawPicture( XftDraw *draw )
{
    if ( ! draw ) return 0;
    if ( ! draw->render_set ) {
	// force the RENDER Picture to be created...
	XftColor color;
	color.color.red = color.color.green = color.color.blue = color.color.alpha =
	    color.pixel = 0;
	XftDrawRect( draw, &color, -100, -100, 1, 1 );
    }
    return draw->render.pict;
}

XftDraw *XftDrawCreateAlpha( Display *display,
			     Pixmap pixmap,
			     int depth )
{
    // taken from Xft 1 sources, see copyright above
    XftDraw     *draw;

    draw = (XftDraw *) malloc (sizeof (XftDraw));
    if (!draw)
	return 0;
    draw->dpy = display;
    draw->drawable = pixmap;
    draw->visual = 0;
    draw->colormap = 0;
    draw->core_set = False;
    draw->clip = 0;

    // Qt addition - go ahead and create the render picture now
    draw->render_set = True;
    draw->render_able = False;

    if ( qt_use_xrender ) {
	draw->render_able = True;

	XRenderPictFormat *format = 0;
	XRenderPictFormat req;
	unsigned long mask = PictFormatType | PictFormatDepth | PictFormatAlphaMask;
	req.type = PictTypeDirect;
	req.depth = depth;
	req.direct.alphaMask = 0xff;
	format = XRenderFindFormat(draw->dpy, mask, &req, 0);
	if (format) {
	    draw->render.pict =
		XRenderCreatePicture(draw->dpy, draw->drawable, format, 0, 0);
	}

	// to keep Xft from trying to free zero pixmaps/pictures, we need to create
	// 2 more pictures (that are identical to draw->render.pict) :/
	draw->render.src[0].pict =
	    XRenderCreatePicture( draw->dpy, draw->drawable, format, 0, 0 );
	draw->render.src[1].pict =
	    XRenderCreatePicture( draw->dpy, draw->drawable, format, 0, 0 );
    }

    return draw;
}

}
#endif // QT_XFT2
#endif // QT_NO_XFTFREETYPE
