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

#ifndef QPRINTENGINE_PS_P_H
#define QPRINTENGINE_PS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of qpsprinter.cpp and qprinter_x11.cpp.
// This header file may change from version to version without notice,
// or even be removed.
//
// We mean it.
//

#include "qpaintengine.h"
#include "qprintengine.h"

#ifndef QT_NO_PRINTER

class QPrinter;
class QPSPrintEnginePrivate;

class Q_GUI_EXPORT QPSPrintEngine : public QPaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QPSPrintEngine)
public:
    // QPrinter uses these
    explicit QPSPrintEngine(QPrinter::PrinterMode m);
    ~QPSPrintEngine();

    virtual bool begin(QPaintDevice *pdev);
    virtual bool end();

    void updatePen(const QPen &pen);
    void updateBrush(const QBrush &brush, const QPointF &pt);
    void updateFont(const QFont &font);
    void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    void updateMatrix(const QMatrix &matrix);
    void updateClipRegion(const QRegion &region, Qt::ClipOperation op);

    virtual void drawLine(const QLineF &line);
    virtual void drawLines(const QLineF *lines, int lineCount);
    virtual void drawRect(const QRectF &r);
    virtual void drawPoint(const QPointF &p);
    virtual void drawEllipse(const QRectF &r);
    virtual void drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode);
    virtual void drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode);

    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr, Qt::PixmapDrawingMode mode);
    virtual void drawTextItem(const QPointF &p, const QTextItem &ti);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s,
				 Qt::PixmapDrawingMode mode);
    virtual void drawPath(const QPainterPath &);

    virtual QPaintEngine::Type type() const { return QPaintEngine::PostScript; }

    // Printer stuff...
    virtual void setPrinterName(const QString &);
    virtual QString printerName() const;

    virtual void setOutputToFile(bool);
    virtual bool outputToFile() const;

    virtual void setOutputFileName(const QString &);
    virtual QString outputFileName()const;

    virtual void setPrintProgram(const QString &);
    virtual QString printProgram() const;

    virtual void setDocName(const QString &);
    virtual QString docName() const;

    virtual void setCreator(const QString &);
    virtual QString creator() const;

    virtual void setOrientation(QPrinter::Orientation);
    virtual QPrinter::Orientation orientation() const;

    virtual void setPageSize(QPrinter::PageSize);
    virtual QPrinter::PageSize pageSize() const;

    virtual void setPageOrder(QPrinter::PageOrder);
    virtual QPrinter::PageOrder pageOrder() const;

    virtual void setResolution(int);
    virtual int resolution() const;

    virtual void setColorMode(QPrinter::ColorMode);
    virtual QPrinter::ColorMode colorMode() const;

    virtual void setFullPage(bool);
    virtual bool fullPage() const;

    virtual void setCollateCopies(bool);
    virtual bool collateCopies() const;

    virtual void setPaperSource(QPrinter::PaperSource);
    virtual QPrinter::PaperSource paperSource() const;

    virtual QList<int> supportedResolutions() const;

#ifdef Q_WS_WIN
    virtual void setWinPageSize(short winPageSize);
    virtual short winPageSize() const;
#endif

    virtual QRect paperRect() const;
    virtual QRect pageRect() const;

    virtual bool newPage();
    virtual bool abort();

    virtual void setNumCopies(int numCopies);
    virtual int numCopies() const;

    virtual int metric(QPaintDevice::PaintDeviceMetric metricType) const;

    virtual QPrinter::PrinterState printerState() const;

    virtual Qt::HANDLE handle() const { return 0; };

private:
    Q_DISABLE_COPY(QPSPrintEngine)
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_PS_P_H
