/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#ifndef QVFBHDR_H

#include <qcolor.h>
#include <qrect.h>

#define QT_VFB_MOUSE_PIPE	"/tmp/.qtvfb_mouse"
#define QT_VFB_KEYBOARD_PIPE	"/tmp/.qtvfb_keyboard"

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
    unsigned int unicode;
    unsigned int modifiers;
    bool press;
    bool repeat;
};

#endif
