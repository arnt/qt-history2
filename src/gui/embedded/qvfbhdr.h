/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVFBHDR_H
#define QVFBHDR_H

#ifndef QT_H
#include "qcolor.h"
#include "qrect.h"
#endif // QT_H

#define QT_VFB_MOUSE_PIPE        "/tmp/.qtvfb_mouse-%1"
#define QT_VFB_KEYBOARD_PIPE        "/tmp/.qtvfb_keyboard-%1"

struct QVFbHeader
{
    int width;
    int height;
    int depth;
    int linestep;
    int dataoffset;
    QRect update;
    bool dirty;
    int  numcols;
    QRgb clut[256];
};

struct QVFbKeyData
{
    unsigned int keycode;
    unsigned int modifiers;
    unsigned short int unicode;
    bool press;
    bool repeat;
};

#endif
