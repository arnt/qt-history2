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

#ifndef SVGALIBSURFACE_H
#define SVGALIBSURFACE_H

#include "svgalibpaintengine.h"
#include "svgalibpaintdevice.h"
#include <private/qwindowsurface_qws_p.h>

class SvgalibPaintDevice;

class SvgalibSurface : public QWSOnScreenSurface
{
public:
    SvgalibSurface();
    SvgalibSurface(QWidget *w);
    ~SvgalibSurface();

    const QString key() const { return QLatin1String("svgalib"); }
    QPaintDevice *paintDevice() { return pdevice; }

private:
    SvgalibPaintDevice *pdevice;
};

#endif // SVGALIBSURFACE_H
