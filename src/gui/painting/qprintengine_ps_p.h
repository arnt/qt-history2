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

#ifndef QT_NO_PRINTER

#include "private/qpdf_p.h"
#include "qplatformdefs.h"

class QPrinter;
class QPSPrintEnginePrivate;

class QPSPrintEngine : public QPdfBaseEngine
{
    Q_DECLARE_PRIVATE(QPSPrintEngine)
public:
    // QPrinter uses these
    explicit QPSPrintEngine(QPrinter::PrinterMode m);
    ~QPSPrintEngine();


    virtual bool begin(QPaintDevice *pdev);
    virtual bool end();

    void setBrush();

    virtual void drawImage(const QRectF &r, const QImage &img, const QRectF &sr, Qt::ImageConversionFlags);
    virtual void drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr);
    virtual void drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s);

    virtual void drawImageInternal(const QRectF &r, QImage img, bool bitmap);
    
    virtual QPaintEngine::Type type() const { return QPaintEngine::PostScript; }

    // Printer stuff...
    void setProperty(PrintEnginePropertyKey key, const QVariant &value);
    QVariant property(PrintEnginePropertyKey key) const;

    virtual bool newPage();
    virtual bool abort();

    virtual int metric(QPaintDevice::PaintDeviceMetric metricType) const;

    virtual QPrinter::PrinterState printerState() const;

    virtual Qt::HANDLE handle() const { return 0; };

private:
    Q_DISABLE_COPY(QPSPrintEngine)
};


class QPSPrintEnginePrivate : public QPdfBaseEnginePrivate {
public:
    QPSPrintEnginePrivate(QPrinter::PrinterMode m);
    ~QPSPrintEnginePrivate();

    void emitHeader(bool finished);
    void emitPages();
    void drawImage(qreal x, qreal y, qreal w, qreal h, const QImage &img, const QImage &mask);
    void flushPage(bool last = false);
    QRect paperRect() const;
    QRect pageRect() const;

    int         pageCount;
    bool        epsf;
    QByteArray     fontsUsed;

    // the device the output is in the end streamed to.
    QIODevice *outDevice;
    int fd;

    // stores the descriptions of the n first pages.
    QByteArray buffer;

    bool firstPage;

    QRect boundingBox;

    bool        collate;
    int         copies;
    QString printerName;
    QString outputFileName;
    QString selectionOption;
    QString printProgram;
    QString title;
    QString creator;
    QPrinter::Orientation orientation;
    QPrinter::PageSize pageSize;
    QPrinter::PageOrder pageOrder;
    int resolution;
    QPrinter::ColorMode colorMode;
    bool fullPage;
    QPrinter::PaperSource paperSource;
    QPrinter::PrinterState printerState;
    bool embedFonts;

    pid_t pid;
};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_PS_P_H
