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

class SvgalibSurface : public QWSWindowSurface
{
public:
    SvgalibSurface();
    SvgalibSurface(QWidget *w);
    ~SvgalibSurface();

    void setGeometry(const QRect &rect);
    QRect geometry() const { return brect; }

    bool isValidFor(const QWidget *) const { return true; }

    void scroll(const QRegion &region, int dx, int dy);

    const QString key() const { return QLatin1String("svgalib"); }
    const QByteArray data() const { return QByteArray(); }

    bool attach(const QByteArray &) { return true; }
    void detach() {}

    const QImage image() const { return QImage(); }
    QPaintDevice *paintDevice() { return pdevice; }
    QPoint painterOffset() const;

private:
    QRect brect;
    SvgalibPaintDevice *pdevice;
};

#endif // SVGALIBSURFACE_H
