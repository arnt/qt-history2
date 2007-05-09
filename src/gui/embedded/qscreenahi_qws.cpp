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

#include "qscreenahi_qws.h"

#ifndef QT_NO_QWS_AHI

#include <QtGui/qcolor.h>
#include <QtGui/qapplication.h>
#include <QtCore/qvector.h>
#include <QtCore/qvarlengtharray.h>

QAhiScreen::QAhiScreen(int displayId)
    : QScreen(displayId)
{
}

QAhiScreen::~QAhiScreen()
{
}

bool QAhiScreen::connect(const QString &displaySpec)
{
    Q_UNUSED(displaySpec);

    AhiSts_t status;

    status = AhiInit(0);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhiInit failed: %x", status);
        return false;
    }

    AhiDev_t device;
    AhiDevInfo_t info;

    status = AhiDevEnum(&device, &info, 0);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhiDevEnum failed: %x", status);
        return false;
    }

    status = AhiDevOpen(&context, device, "qscreenahi", AHIFLAG_USERLEVEL);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhiDevOpen failed: %x", status);
        return false;
    }

    AhiDispMode_t mode;

    status = AhiDispModeEnum(context, &mode, 0);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhiDispModeEnum failed: %x", status);
        return false;
    }

    status = AhiSurfAlloc(context, &surface, &mode.size, mode.pixFmt, 0);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhisurfAlloc failed: %x", status);
        return false;
    }

    if (QApplication::type() == QApplication::GuiServer) {
        status = AhiDispSurfSet(context, surface, 0);
        if (status != AhiStsOk) {
            qCritical("QAhiScreen::connect(): AhiDispSurfSet failed: %x",
                      status);
            return false;
        }

        status = AhiDispModeSet(context, &mode, 0);
        if (status != AhiStsOk) {
            qCritical("QAhiScreen::context(): AhiDispModeSet failed: %x",
                      status);
            return false;
        }
    }

    AhiSurfInfo_t surfaceInfo;

    status = AhiSurfInfo(context, surface, &surfaceInfo);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhiSurfInfo failed: %x", status);
        return false;
    }

    QScreen::data = 0;
    QScreen::w = QScreen::dw = surfaceInfo.size.cx;
    QScreen::h = QScreen::dh = surfaceInfo.size.cy;
    QScreen::lstep = surfaceInfo.stride;
    QScreen::size = surfaceInfo.sizeInBytes;

    switch (mode.pixFmt) {
    case AhiPix16bpp_565RGB:
        setPixelFormat(QImage::Format_RGB16);
        QScreen::d = 16;
        break;
    case AhiPix32bpp_8888ARGB:
        setPixelFormat(QImage::Format_ARGB32);
        QScreen::d = 32;
        break;
    default:
        qCritical("QAhiScreen::connect(): Unknown pixel format: %x",
                  mode.pixFmt);
        return false;
        break;
    }

    const int dpi = 72;
    QScreen::physWidth = qRound(QScreen::dw * 25.4 / dpi);
    QScreen::physHeight = qRound(QScreen::dh * 25.4 / dpi);

    return true;
}

void QAhiScreen::disconnect()
{
    AhiSurfFree(context, surface);
    AhiDevClose(context);
    AhiTerm();
}

bool QAhiScreen::initDevice()
{
    QScreenCursor::initSoftwareCursor();

    status = AhiDispState(context, AhiDispStateOn, 0);
    if (status != AhiStsOk) {
        qCritical("QAhiScreen::connect(): AhiDispState failed: %x", status);
        return false;
    }

    return true;
}

void QAhiScreen::shutdownDevice()
{
    AhiDispState(context, AhiDispStateOff, 0);
}

void QAhiScreen::setMode(int width, int height, int depth)
{
    Q_UNUSED(width);
    Q_UNUSED(height);
    Q_UNUSED(depth);
}

void QAhiScreen::blit(const QImage &image, const QPoint &topLeft,
                      const QRegion &reg)
{
    AhiSts_t status;

    status = AhiDrawRopSet(context, AHIMAKEROP3(AHIROPSRCCOPY));
    if (status != AhiStsOk) {
        qWarning("QAhiScreen::blit(): AhiDrawRopSet failed: %x", status);
        return;
    }

    status = AhiDrawSurfDstSet(context, surface, 0);
    if (status != AhiStsOk) {
        qWarning("QAhiScreen::blit(): AhiDrawSurfDstSet failed: %x", status);
        return;
    }

    AhiSize_t bitmapSize = { image.width(), image.height() };
    AhiBitmap_t bitmap = { bitmapSize,
                           (void*)(image.bits()), // XXX
                           image.bytesPerLine(),
                           AhiPix32bpp_8888ARGB };

    switch (pixelFormat()) {
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
        bitmap.pixFmt = AhiPix32bpp_8888ARGB;
        break;
    case QImage::Format_RGB16:
        bitmap.pixFmt = AhiPix16bpp_565RGB;
        break;
    default:
        break;
    }

    const QVector<QRect> rects = (reg & region()).rects();
    for (int i = 0; i < rects.size(); ++i) {
        const QRect rect = rects.at(i);

        AhiPoint_t src = { rect.x() - topLeft.x(), rect.y() - topLeft.y() };
        AhiRect_t dst = { rect.left(), rect.top(),
                          rect.x() + rect.width(),
                          rect.y() + rect.height() };

        status = AhiDrawBitmapBlt(context, &dst, &src, &bitmap, 0, 0);
        if (status != AhiStsOk)
            qWarning("QAhiScreen::blit(): AhiDrawBitmapBlt failed: %x",
                     status);
    }
}

void QAhiScreen::solidFill(const QColor &color, const QRegion &reg)
{
    AhiSts_t status;

    switch (pixelFormat()) {
    case QImage::Format_ARGB32_Premultiplied:
    case QImage::Format_ARGB32:
    case QImage::Format_RGB32:
        status = AhiDrawBrushFgColorSet(context, color.rgba());
        break;
    case QImage::Format_RGB16:
        status = AhiDrawBrushFgColorSet(context, qt_convRgbTo16(color.rgb()));
        break;
    default:
        qFatal("QAhiScreen::solidFill(): Not implemented for pixel format %d",
               int(pixelFormat()));
        break;
    }

    if (status != AhiStsOk) {
        qWarning("QAhiScreen::solidFill(): AhiDrawBrushFgColorSet failed: %x",
                 status);
        return;
    }

    status = AhiDrawBrushSet(context, 0, 0, 0, AHIFLAG_BRUSHSOLID);
    if (status != AhiStsOk) {
        qWarning("QAhiScreen::solidFill(): AhiDrawBrushSet failed: %x",
                 status);
        return;
    }

    status = AhiDrawRopSet(context, AHIMAKEROP3(AHIROPPATCOPY));
    if (status != AhiStsOk) {
        qWarning("QAhiScreen::solidFill(): AhiDrawRopSet failed: %x", status);
        return;
    }

    status = AhiDrawSurfDstSet(context, surface, 0);
    if (status != AhiStsOk) {
        qWarning("QAhiScreen::solidFill(): AhiDrawSurfDst failed: %x", status);
        return;
    }

    const QVector<QRect> rects = (reg & region()).rects();
    QVarLengthArray<AhiRect_t> ahiRects(rects.size());

    for (int i = 0; i < rects.size(); ++i) {
        const QRect rect = rects.at(i);
        ahiRects[i].left = rect.left();
        ahiRects[i].top = rect.top();
        ahiRects[i].right = rect.x() + rect.width();
        ahiRects[i].bottom = rect.y() + rect.height();
    }

    status = AhiDrawBitBltMulti(context, ahiRects.data(), 0, ahiRects.size());
    if (status != AhiStsOk)
        qWarning("QAhiScreen::solidFill(): AhiDrawBitBlt failed: %x", status);
}

#endif // QT_NO_QWS_AHI
