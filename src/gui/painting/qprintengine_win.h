/****************************************************************************
**
** Definition of QPrinter class.
**
** Copyright (C) 1992-2004 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_NO_PRINTER

#ifndef QPRINTENGINE_WIN_H
#define QPRINTENGINE_WIN_H

#include "qpaintengine_win.h"
#include "qprinter.h"
#include "qprintengine.h"

class QWin32PrintEnginePrivate;
class QPrinterPrivate;

class QWin32PrintEngine : public QWin32PaintEngine, public QPrintEngine
{
    Q_DECLARE_PRIVATE(QWin32PrintEngine);
public:
    QWin32PrintEngine(QPrinter::PrinterMode mode);

    // override QWin32PaintEngine
    bool begin(QPaintDevice *dev);
    bool end();
    void updateClipRegion(const QRegion &clip, bool clipEnabled);

    void drawPixmap(const QRect &r, const QPixmap &pm, const QRect &sr, Qt::PixmapDrawingMode mode);

    // Printer functions...
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
    QPrinter::Orientation orientation()   const;

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

    void setWinPageSize(short winPageSize);
    short winPageSize() const;

    QRect paperRect() const;
    QRect pageRect() const;

    QString printerSelectionOption() const;
    void setPrinterSelectionOption(const QString &);

    bool isActive() const;

    bool newPage();
    bool abort();
    int metric( int ) const;

    QPrinter::PrinterState printerState() const;

private:
    friend class QPrintDialogWin;
};

#endif // QPRINTENGINE_WIN_H

#endif // QT_NO_PRINTER
