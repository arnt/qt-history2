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

#ifndef QBACKINGSTORE_P_H
#define QBACKINGSTORE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qpaintengine_raster_p.h"
#ifdef Q_WS_QWS
#include "private/qwidget_qws_p.h"
#endif

#ifdef Q_WS_WIN
class QBackingStoreDevice : public QPaintDevice
{
    QRasterPaintEngine engine;
    QWidget *tlw;

public:
    QBackingStoreDevice(QWidget *topLevel) : tlw(topLevel)  { engine.setFlushOnEnd(false); }
    QSize size() const { return engine.size(); }
    virtual int metric(PaintDeviceMetric metric) const {  return tlw->metric(metric); }
    QPaintEngine *paintEngine() const { return const_cast<QRasterPaintEngine *>(&engine); }
};
#endif

class QWidgetBackingStore
{
    QWidget *tlw;
    QRegion dirty;

#if defined(Q_WS_WIN)
    QBackingStoreDevice buffer;
#elif defined(Q_WS_QWS)
    QWSBackingStore buffer;
#else
    QPixmap buffer;
#endif
    QPoint tlwOffset;
    bool dirtyBufferSize;

    bool isOpaque(const QWidget *widget);
    bool hasBackground(const QWidget *widget);

    void paintToBuffer(const QRegion &rgn, QWidget *widget, const QPoint &offset, bool asRoot = true);
    void copyToScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, bool recursive = true);

    friend void qt_syncBackingStore(QRegion, QWidget *);
#if defined(Q_WS_X11) || defined(Q_WS_QWS)
    friend void qt_syncBackingStore(QWidget *);
#endif
    friend class QWidgetPrivate;
    friend class QWidget;
public:
    QWidgetBackingStore(QWidget *t);
    ~QWidgetBackingStore();
    void scrollRegion(const QRect &rect, int dx, int dy, QWidget *widget=0);
    void dirtyRegion(const QRegion &rgn, QWidget *widget=0);
    void cleanRegion(const QRegion &rgn, QWidget *widget=0);
#if defined(Q_WS_X11)
    QPixmap backingPixmap() const { return buffer; }
#elif defined(Q_WS_QWS)
    QPixmap *backingPixmap() const { return buffer.pixmap(); }
    void releaseBuffer();
#endif

    inline QPoint topLevelOffset() const { return tlwOffset; }
    static bool paintOnScreen(QWidget *);
};

#endif // QBACKINGSTORE_P_H
