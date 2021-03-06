/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVFBHDR_H
#define QVFBHDR_H

#include <QtGui/qcolor.h>
#include <QtCore/qrect.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#define QT_VFB_MOUSE_PIPE           "/tmp/.qtvfb_mouse-%1"
#define QT_VFB_KEYBOARD_PIPE        "/tmp/.qtvfb_keyboard-%1"
#define QT_VFB_MAP                  "/tmp/.qtvfb_map-%1"

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
    int viewerVersion;
    int serverVersion;
};

struct QVFbKeyData
{
    unsigned int keycode;
    Qt::KeyboardModifiers modifiers;
    unsigned short int unicode;
    bool press;
    bool repeat;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QVFBHDR_H
