/****************************************************************************
**
** Definition of QMacPrintEngine class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
class QMacPrintEngine : public QPrintEngine, public QCoreGraphicsPaintEngine
{
    Q_DECLARE_PRIVATE(QMacPrintEngine);
public:
    QMacPrintEngine(QPaintDevice *dev, QPrinter::PrinterMode mode);

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

    void setPrintRange(QPrinter::PrintRange range);
    QPrinter::PrintRange printRange() const;

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
};
#endif
