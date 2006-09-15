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

#include "svgalibpaintdevice.h"
#include "svgalibpaintengine.h"

#include <QApplication>
#include <QDesktopWidget>

SvgalibPaintDevice::SvgalibPaintDevice(QWidget *w)
    : QCustomRasterPaintDevice(w)
{
    pengine = new SvgalibPaintEngine;
}

SvgalibPaintDevice::~SvgalibPaintDevice()
{
    delete pengine;
}

int SvgalibPaintDevice::metric(PaintDeviceMetric m) const
{
    if (m == PdmWidth)
        return QApplication::desktop()->screenGeometry().width();
    else if (m == PdmHeight)
        return QApplication::desktop()->screenGeometry().height();
    return QCustomRasterPaintDevice::metric(m);
}

