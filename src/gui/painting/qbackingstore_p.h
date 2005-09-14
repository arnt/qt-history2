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

#include <private/qpaintengine_raster_p.h>

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

#ifdef Q_WS_WIN
    QBackingStoreDevice buffer;
#else
    QPixmap buffer;
#endif

    bool isOpaque(const QWidget *widget);
    bool hasBackground(const QWidget *widget);
    enum PaintFlags { AsRoot = 0x01, Recursive = 0x02 };
    void cleanBuffer(const QRegion &rgn, QWidget *widget, const QPoint &offset, uint flags);
    void cleanScreen(const QRegion &rgn, QWidget *widget, const QPoint &offset, uint flags);

    void paintWidget_sys(const QRegion &rgn, QWidget *widget);
    void updateWidget_sys(const QRegion &rgn, QWidget *widget);
    friend void qt_syncBackingStore(QRegion, QWidget *);
#ifdef Q_WS_X11
    friend void qt_syncBackingStore(QWidget *);
#endif
    friend class QWidgetPrivate;
    friend class QWidget;
public:
    QWidgetBackingStore(QWidget *t);
    ~QWidgetBackingStore();
    void scrollRegion(const QRegion &rgn, int dx, int dy, QWidget *widget=0);
    void dirtyRegion(const QRegion &rgn, QWidget *widget=0);
    void cleanRegion(const QRegion &rgn, QWidget *widget=0);
#ifndef Q_WS_WIN
    QPixmap backingPixmap() const { return buffer; }
#endif

    static bool paintOnScreen(QWidget *);
};

#endif // QBACKINGSTORE_P_H
