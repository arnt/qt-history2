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

#ifdef QMAC_NO_COREGRAPHICS
//#  define QMAC_PRINTER_USE_QUICKDRAW
#endif

class QMacPrintEnginePrivate;
class QMacPrintEngine : 
#if defined(QMAC_PRINTER_USE_QUICKDRAW)
    public QQuickDrawPaintEngine, 
#else
    public QCoreGraphicsPaintEngine, 
#endif
    public QPrintEngine
{
    Q_DECLARE_PRIVATE(QMacPrintEngine);
public:
    QMacPrintEngine(QPrinter::PrinterMode mode);

    Qt::HANDLE handle() const;

    bool begin(QPaintDevice *dev);
    bool end();

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
private:
    friend class QPrintDialogMac;
};
#endif
