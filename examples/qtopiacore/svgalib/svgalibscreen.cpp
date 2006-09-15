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

#include "svgalibscreen.h"
#include "svgalibsurface.h"

#include <QVector>
#include <QApplication>
#include <QColor>
#include <QWidget>

bool SvgalibScreen::connect(const QString &displaySpec)
{
    Q_UNUSED(displaySpec);

    int mode = vga_getdefaultmode();
    if (mode <= 0) {
        qCritical("SvgalibScreen::connect(): invalid vga mode");
        return false;
    }

    vga_modeinfo *modeinfo = vga_getmodeinfo(mode);

    QScreen::lstep = modeinfo->linewidth;
    QScreen::dw = QScreen::w = modeinfo->width;
    QScreen::dh = QScreen::h = modeinfo->height;
    QScreen::d = modeinfo->bytesperpixel * 8;
    QScreen::size = QScreen::lstep * dh;
    QScreen::data = 0;

    const int dpi = 72;
    QScreen::physWidth = qRound(QScreen::dw * 25.4 / dpi);
    QScreen::physHeight = qRound(QScreen::dh * 25.4 / dpi);

    return true;
}

bool SvgalibScreen::initDevice()
{
    if (vga_init() != 0) {
        qCritical("SvgalibScreen::initDevice(): unable to initialize svgalib");
        return false;
    }

    int mode = vga_getdefaultmode();
    if (vga_setmode(mode) == -1) {
        qCritical("SvgalibScreen::initialize(): unable to set graphics mode");
        return false;
    }

    if (gl_setcontextvga(mode) != 0) {
        qCritical("SvgalibScreen::initDevice(): unable to set vga context");
        return false;
    }
    context = gl_allocatecontext();
    gl_getcontext(context);

    vga_modeinfo *modeinfo = vga_getmodeinfo(mode);
    if (!(modeinfo->flags & IS_LINEAR)) {
        qCritical("SvgalibScreen::initDevice(): graphics memory not linear");
        return false;
    }
    QScreen::data = vga_getgraphmem();

    QScreenCursor::initSoftwareCursor();
    return true;
}

void SvgalibScreen::shutdownDevice()
{
    gl_freecontext(context);
    vga_setmode(TEXT);
}

void SvgalibScreen::disconnect()
{
}

void SvgalibScreen::exposeRegion(QRegion region, int changing)
{
    QScreen::exposeRegion(region, changing);
    // flip between buffers if implementing a double buffered driver
}

void SvgalibScreen::solidFill(const QColor &color, const QRegion &reg)
{
    if (depth() != 32 || depth() != 16) {
        QScreen::solidFill(color, reg);
        return;
    }

    const QVector<QRect> rects = (reg & region()).rects();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        gl_fillbox(r.left(), r.top(), r.width(), r.height(), color.rgba());
    }
}

void SvgalibScreen::blit(const QImage &img, const QPoint &topLeft,
                         const QRegion &reg)
{
    bool do_fallback = true;

    if (depth() == 16 && img.format() == QImage::Format_RGB16)
        do_fallback = false;
    if (depth() == 32 && img.format() == QImage::Format_ARGB32_Premultiplied)
        do_fallback = false;

    if (do_fallback) {
        QScreen::blit(img, topLeft, reg);
        return;
    }

    const QVector<QRect> rects = (reg & region()).rects();

    for (int i = 0; i < rects.size(); ++i) {
        const QRect r = rects.at(i);
        gl_putboxpart(r.x(), r.y(), r.width(), r.height(),
                      img.width(), img.height(),
                      static_cast<void*>(const_cast<uchar*>(img.bits())),
                      r.x() - topLeft.x(), r.y() - topLeft.y());
    }
}

QWSWindowSurface* SvgalibScreen::createSurface(QWidget *widget) const
{
    static int onScreenPaint = -1;
    if (onScreenPaint == -1)
        onScreenPaint = qgetenv("QT_ONSCREEN_PAINT").toInt();

    if (onScreenPaint > 0 || widget->testAttribute(Qt::WA_PaintOnScreen))
        return new SvgalibSurface(widget);

    return QScreen::createSurface(widget);
}

QWSWindowSurface* SvgalibScreen::createSurface(const QString &key) const
{
    if (key == QLatin1String("svgalib"))
        return new SvgalibSurface;
    return QScreen::createSurface(key);
}
