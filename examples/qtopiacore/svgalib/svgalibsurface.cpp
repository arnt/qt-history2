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

#include <vgagl.h>

SvgalibSurface::SvgalibSurface() : QWSWindowSurface(), pdevice(0)
{
    setSurfaceFlags(Opaque);
}

SvgalibSurface::SvgalibSurface(QWidget *w)
    : QWSWindowSurface(w)
{
    setSurfaceFlags(Opaque);
    pdevice = new SvgalibPaintDevice(w);
}

SvgalibSurface::~SvgalibSurface()
{
    delete pdevice;
}

void SvgalibSurface::setGeometry(const QRect &rect)
{
    brect = rect;
    QWSWindowSurface::setGeometry(rect);
}

QPoint SvgalibSurface::painterOffset() const
{
    return brect.topLeft() + QWSWindowSurface::painterOffset();
}

void SvgalibSurface::scroll(const QRegion &region, int dx, int dy)
{
    const QVector<QRect> rects = region.rects();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        gl_copybox(r.left(), r.top(), r.width(), r.height(),
                   r.left() + dx, r.top() + dy);
    }
}

