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

#ifndef QPRINTENGINE_MAC_H
#define QPRINTENGINE_MAC_H

#include "qprinter.h"
#include "qprintengine.h"

#include <qpaintengine_mac.h>

#include <private/qpainter_p.h>

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

    void setPrinterName(const QString &);
    QString printerName() const;

    void setOutputToFile(bool);
    bool outputToFile() const;

    void setOutputFileName(const QString &);
    QString outputFileName()const;

    void setPrintProgram(const QString &);
    QString printProgram() const;

    void setDocName(const QString &);
    QString docName() const;

    void setCreator(const QString &);
    QString creator() const;

    void setOrientation(QPrinter::Orientation);
    QPrinter::Orientation orientation() const;

    void setPageSize(QPrinter::PageSize);
    QPrinter::PageSize pageSize() const;

    void setPageOrder(QPrinter::PageOrder);
    QPrinter::PageOrder pageOrder() const;

    void setResolution(int);
    int resolution() const;

    void setColorMode(QPrinter::ColorMode);
    QPrinter::ColorMode colorMode() const;

    void setFullPage(bool);
    bool fullPage() const;

    void setNumCopies(int);
    int numCopies() const;

    void setCollateCopies(bool);
    bool collateCopies() const;

    void setPaperSource(QPrinter::PaperSource);
    QPrinter::PaperSource paperSource()   const;

    QList<int> supportedResolutions() const;

    QRect paperRect() const;
    QRect pageRect() const;

    QString printerSelectionOption() const;
    void setPrinterSelectionOption(const QString &);

    QPrinter::PrinterState printerState() const;

    bool newPage();
    bool abort();
    int metric( int ) const;

    //forwarded functions
    virtual void updatePen(const QPen &pen);
    virtual void updateBrush(const QBrush &brush, const QPointF &pt);
    virtual void updateFont(const QFont &font);
    virtual void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    virtual void updateMatrix(const QMatrix &matrix);
    virtual void updateClipRegion(const QRegion &region, Qt::ClipOperation op);
    virtual void updateClipPath(const QPainterPath &path, Qt::ClipOperation op);
    virtual void updateRenderHints(QPainter::RenderHints hints);
    virtual void drawLine(const QLineF &line);
    virtual void drawLines(const QList<QLineF> &points);
    virtual void drawRect(const QRectF &r);
    virtual void drawPoint(const QPointF &p);
    virtual void drawPoints(const QPolygon &p);
    virtual void drawEllipse(const QRectF &r);
    virtual void drawPolygon(const QPolygon &p, PolygonDrawMode mode);
    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr, Qt::PixmapDrawingMode mode);
    virtual void drawTextItem(const QPointF &p, const QTextItem &ti, int textflags);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
				 Qt::PixmapDrawingMode mode);
    virtual void drawPath(const QPainterPath &);
    virtual QPainter::RenderHints supportedRenderHints() const;


private:
    virtual void updateInternal(QPainterState *state, bool updateGC = true);
    friend class QPrintDialog;
    friend class QPageSetupDialog;
};
#endif
