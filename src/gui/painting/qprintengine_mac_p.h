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

#ifndef QPRINTENGINE_MAC_P_H
#define QPRINTENGINE_MAC_P_H

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

#ifndef QT_NO_PRINTER

#include "qprinter.h"
#include "qprintengine.h"
#include <private/qpaintengine_mac_p.h>
#include <private/qpainter_p.h>

class QPrinterPrivate;
class QMacPrintEnginePrivate;
class QMacPrintEngine : public QPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QMacPrintEngine)
public:
    QMacPrintEngine(QPrinter::PrinterMode mode);

    Qt::HANDLE handle() const;

    bool begin(QPaintDevice *dev);
    bool end();
    virtual QPaintEngine::Type type() const { return QPaintEngine::MacPrinter; }

    QPaintEngine *paintEngine() const;

    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;

    QPrinter::PrinterState printerState() const;

    bool newPage();
    bool abort();
    int metric(QPaintDevice::PaintDeviceMetric) const;

    //forwarded functions

    void updateState(const QPaintEngineState &state);

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
    void updateRenderHints(QPainter::RenderHints hints);
    virtual void drawLines(const QLineF *lines, int lineCount);
    virtual void drawRects(const QRectF *r, int num);
    virtual void drawPoints(const QPointF *p, int pointCount);
    virtual void drawEllipse(const QRectF &r);
    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);
    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr, Qt::PixmapDrawingMode mode);
    virtual void drawTextItem(const QPointF &p, const QTextItem &ti);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
				 Qt::PixmapDrawingMode mode);
    virtual void drawPath(const QPainterPath &);
    virtual QPainter::RenderHints supportedRenderHints() const;


private:
    virtual void updateInternal(QPainterState *state, bool updateGC = true);
    friend class QPrintDialog;
    friend class QPageSetupDialog;
};

class QMacPrintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QMacPrintEngine)
public:
    QPrinter::PrinterMode mode;
    QPrinter::PrinterState state;
    QPrinter::Orientation orient;
    PMPageFormat format;
    PMPrintSettings settings;
    PMPrintSession session;
    PMResolution resolution;
    QString outputFilename;
    bool fullPage;
    GWorldPtr qdHandle;
    QPaintEngine *paintEngine;
    QMacPrintEnginePrivate() : mode(QPrinter::ScreenResolution), state(QPrinter::Idle),
                               orient(QPrinter::Portrait), format(0), settings(0), session(0),
                               qdHandle(0), paintEngine(0) {}
    void initialize();
    bool newPage_helper();
    void setPageSize(QPrinter::PageSize ps);
    QPrinter::PageSize pageSize() const;
    QList<QVariant> supportedResolutions() const;

};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_WIN_P_H
