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
    Q_DECLARE_PRIVATE(QMacPrintEngine);
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
    virtual void updateBrush(const QBrush &brush, const QPoint &pt);
    virtual void updateFont(const QFont &font);
    virtual void updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush);
    virtual void updateXForm(const QWMatrix &matrix);
    virtual void updateClipRegion(const QRegion &region, bool clipEnabled);
    virtual void updateRenderHints(QPainter::RenderHints hints);
    virtual void drawLine(const QPoint &p1, const QPoint &ps);
    virtual void drawRect(const QRect &r);
    virtual void drawPoint(const QPoint &p);
    virtual void drawPoints(const QPointArray &pa, int index = 0, int npoints = -1);
    virtual void drawRoundRect(const QRect &r, int xRnd, int yRnd);
    virtual void drawEllipse(const QRect &r);
    virtual void drawArc(const QRect &r, int a, int alen);
    virtual void drawPie(const QRect &r, int a, int alen);
    virtual void drawChord(const QRect &r, int a, int alen);
    virtual void drawLineSegments(const QPointArray &, int index = 0, int nlines = -1);
    virtual void drawPolyline(const QPointArray &pa, int index = 0, int npoints = -1);
    virtual void drawPolygon(const QPointArray &pa, bool winding = false, int index = 0, int npoints = -1);
    virtual void drawConvexPolygon(const QPointArray &, int index = 0, int npoints = -1);
    virtual void drawCubicBezier(const QPointArray &, int index = 0);
    virtual void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::PixmapDrawingMode mode);
    virtual void drawTextItem(const QPoint &p, const QTextItem &ti, int textflags);
    virtual void drawTiledPixmap(const QRect &r, const QPixmap &pixmap, const QPoint &s,
				 Qt::PixmapDrawingMode mode);
    virtual void drawPath(const QPainterPath &);
    virtual QPainter::RenderHints supportedRenderHints() const;


private:
    virtual void updateInternal(QPainterState *state, bool updateGC = true);
    friend class QPrintDialogMac;
};
#endif
