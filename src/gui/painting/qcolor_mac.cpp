/****************************************************************************
**
** Implementation of QColor class for mac.
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

#include "qcolor.h"
#include "private/qcolor_p.h"
#include "string.h"
#include "qpaintdevice.h"
#include "qapplication.h"
#include "private/qapplication_p.h"

void qt_reset_color_avail()
{
}

/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

int QColor::maxColors()
{
    return -1;
}

int QColor::numBitPlanes()
{
    return 32;
}


void QColor::initialize()
{
    if (color_init)
        return;

    color_init = true;
}

void QColor::cleanup()
{
}


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

uint QColor::alloc()
{
    d.d32.pix = qRed(d.argb) << 16 | qGreen(d.argb) << 8 | qBlue(d.argb);
    return d.d32.pix;
}


void QColor::setSystemNamedColor(const QString& name)
{
    // setSystemNamedColor should look up rgb values from the built in
    // color tables first (see qcolor_p.cpp), and failing that, use
    // the window system's interface for translating names to rgb values...
    // we do this so that things like uic can load an XPM file with named colors
    // and convert it to a png without having to use window system functions...
    d.argb = qt_get_rgb_val(name.latin1());
    QRgb rgb;
    if (qt_get_named_rgb(name.latin1(), &rgb)) {
        d.argb = rgb;
        if (colormodel == d8) {
            d.d8.invalid = false;
            d.d8.dirty = true;
            d.d8.pix = 0;
        } else {
            alloc();
        }
    } else {
        // set to invalid color
        *this = QColor();
    }
}


int QColor::enterAllocContext()
{
    return 1;
}


void QColor::leaveAllocContext()
{
}


int QColor::currentAllocContext()
{
    return 0;
}


void QColor::destroyAllocContext(int)
{
}

