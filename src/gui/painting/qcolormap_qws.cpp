/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qcolormap.h"
#include "qcolor.h"
#include "qpaintdevice.h"
#include "qscreen_qws.h"
#include "qwsdisplay_qws.h"

class QColormapPrivate
{
public:
    QColormapPrivate()
        : mode(QColormap::Direct), depth(0), numcolors(0)
    { ref = 0; }
    ~QColormapPrivate()
    {}

    QAtomic ref;

    QColormap::Mode mode;
    int depth;
    int numcolors;
};

static QColormapPrivate *screenMap = 0;

void QColormap::initialize()
{
    screenMap = new QColormapPrivate;
    screenMap->ref = 1;

    screenMap->depth = QPaintDevice::qwsDisplay()->depth();
    if (screenMap->depth < 8) {
        screenMap->mode = QColormap::Indexed;
        screenMap->numcolors = 256;
    } else {
        screenMap->mode = QColormap::Direct;
        screenMap->numcolors = -1;
    }
}

void QColormap::cleanup()
{
    delete screenMap;
    screenMap = 0;
}

QColormap QColormap::instance(int /*screen*/)
{
    return QColormap();
}

QColormap::QColormap()
    : d(screenMap)
{ ++d->ref; }

QColormap::QColormap(const QColormap &colormap)
    :d (colormap.d)
{ ++d->ref; }

QColormap::~QColormap()
{
    if (!--d->ref)
        delete d;
}

QColormap::Mode QColormap::mode() const
{ return d->mode; }


int QColormap::depth() const
{ return d->depth; }


int QColormap::size() const
{
    return d->numcolors;
}

uint QColormap::pixel(const QColor &color) const
{
    QRgb rgb = color.rgba();
    if (d->mode == QColormap::Direct) {
        switch(d->depth) {
        case 16:
            return qt_convRgbTo16(rgb);
        case 24:
        case 32:
        {
            const int r = qRed(rgb);
            const int g = qGreen(rgb);
            const int b = qBlue(rgb);
            const int red_shift = 16;
            const int green_shift = 8;
            const int red_mask   = 0xff0000;
            const int green_mask = 0x00ff00;
            const int blue_mask  = 0x0000ff;
            const int tg = g << green_shift;
#ifndef QT_NO_QWS_DEPTH_32_BGR
            if (qt_screen->pixelType() == QScreen::BGRPixel) {
                const int tb = b << red_shift;
                return 0xff000000 | (r & blue_mask) | (tg & green_mask) | (tb & red_mask);
            }
#endif
            const int tr = r << red_shift;
            return 0xff000000 | (b & blue_mask) | (tg & green_mask) | (tr & red_mask);
        }
        }
    }
    return qt_screen->alloc(qRed(rgb), qGreen(rgb), qBlue(rgb));
}

const QColor QColormap::colorAt(uint pixel) const
{
    if (d->mode == Direct) {
        if (d->depth == 16) {
            pixel = qt_conv16ToRgb(pixel);
        }
        const int red_shift = 16;
        const int green_shift = 8;
        const int red_mask   = 0xff0000;
        const int green_mask = 0x00ff00;
        const int blue_mask  = 0x0000ff;
#ifndef QT_NO_QWS_DEPTH_32_BGR
        if (qt_screen->pixelType() == QScreen::BGRPixel) {
            return QColor((pixel & blue_mask),
                          (pixel & green_mask) >> green_shift,
                          (pixel & red_mask) >> red_shift);
        }
#endif
        return QColor((pixel & red_mask) >> red_shift,
                      (pixel & green_mask) >> green_shift,
                      (pixel & blue_mask));
    }
    Q_ASSERT_X(int(pixel) < qt_screen->numCols(), "QColormap::colorAt", "pixel out of bounds of palette");
    return QColor(qt_screen->clut()[pixel]);
}

const QVector<QColor> QColormap::colormap() const
{
    return QVector<QColor>();
}
