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

#ifndef QPRINTENGINE_H
#define QPRINTENGINE_H

#include "QtGui/qprinter.h"

class QPrintEngine
{
public:
    virtual void setPrinterName(const QString &) = 0;
    virtual QString printerName() const = 0;

    virtual void setOutputToFile(bool) = 0;
    virtual bool outputToFile() const = 0;

    virtual void setOutputFileName(const QString &) = 0;
    virtual QString outputFileName()const = 0;

    virtual void setPrintProgram(const QString &) = 0;
    virtual QString printProgram() const = 0;

    virtual void setDocName(const QString &) = 0;
    virtual QString docName() const = 0;

    virtual void setCreator(const QString &) = 0;
    virtual QString creator() const = 0;

    virtual void setOrientation(QPrinter::Orientation) = 0;
    virtual QPrinter::Orientation orientation()   const = 0;

    virtual void setPageSize(QPrinter::PageSize) = 0;
    virtual QPrinter::PageSize pageSize() const = 0;

    virtual void setPageOrder(QPrinter::PageOrder) = 0;
    virtual QPrinter::PageOrder pageOrder() const = 0;

    virtual void setResolution(int) = 0;
    virtual int resolution() const = 0;

    virtual void setColorMode(QPrinter::ColorMode) = 0;
    virtual QPrinter::ColorMode colorMode() const = 0;

    virtual void setFullPage(bool) = 0;
    virtual bool fullPage() const = 0;

    virtual void setCollateCopies(bool) = 0;
    virtual bool collateCopies() const = 0;

    virtual void setPaperSource(QPrinter::PaperSource) = 0;
    virtual QPrinter::PaperSource paperSource()   const = 0;

    virtual QList<int> supportedResolutions() const = 0;

#ifdef Q_WS_WIN
    virtual void setWinPageSize(short winPageSize) = 0;
    virtual short winPageSize() const = 0;
#endif

    virtual QRect paperRect() const = 0;
    virtual QRect pageRect() const = 0;

    virtual bool newPage() = 0;
    virtual bool abort() = 0;

    virtual void setNumCopies(int numCopies) = 0;
    virtual int numCopies() const = 0;

    virtual int metric(QPaintDevice::PaintDeviceMetric) const = 0;

    virtual QPrinter::PrinterState printerState() const = 0;

#ifdef Q_WS_WIN
    virtual HDC getDC() const { return 0; }
    virtual void releaseDC(HDC) { }
#endif

};

#endif // QPRINTENGINE_H
