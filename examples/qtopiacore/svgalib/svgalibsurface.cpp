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

#include "svgalibsurface.h"
#include "svgalibpaintdevice.h"

SvgalibSurface::SvgalibSurface() : QWSOnScreenSurface(), pdevice(0)
{
    setSurfaceFlags(Opaque);
}

SvgalibSurface::SvgalibSurface(QWidget *w)
    : QWSOnScreenSurface(w)
{
    setSurfaceFlags(Opaque);
    pdevice = new SvgalibPaintDevice(w);
}

SvgalibSurface::~SvgalibSurface()
{
    delete pdevice;
}

