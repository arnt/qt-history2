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

#ifndef QPRINTER_H
#define QPRINTER_H

#include "qpaintdevice.h"
#include "qstring.h"

#ifndef QT_NO_PRINTER

#if defined(B0)
#undef B0 // Terminal hang-up.  We assume that you do not want that.
#endif

class QPrinterPrivate;
class QPaintEngine;

class Q_GUI_EXPORT QPrinter : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QPrinter)
public:
    enum PrinterMode { ScreenResolution, PrinterResolution, HighResolution };

    QPrinter(PrinterMode mode = ScreenResolution);
   ~QPrinter();

    enum Orientation { Portrait, Landscape };

    enum PageSize    { A4, B5, Letter, Legal, Executive,
		       A0, A1, A2, A3, A5, A6, A7, A8, A9, B0, B1,
		       B10, B2, B3, B4, B6, B7, B8, B9, C5E, Comm10E,
		       DLE, Folio, Ledger, Tabloid, Custom, NPageSize = Custom };

    enum PageOrder   { FirstPageFirst,
		       LastPageFirst };

    enum ColorMode   { GrayScale,
		       Color };

    enum PaperSource { OnlyOne,
		       Lower,
		       Middle,
		       Manual,
		       Envelope,
                       EnvelopeManual,
		       Auto,
		       Tractor,
		       SmallFormat,
                       LargeFormat,
		       LargeCapacity,
		       Cassette,
		       FormSource };

    enum PrinterState { Idle,
                        Active,
                        Aborted,
                        Error };
#ifdef QT_COMPAT
    enum PrintRange { AllPages, Selection, PageRange };
    enum PrinterOption { PrintToFile, PrintSelection, PrintPageRange };
#endif // QT_COMPAT

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

    void setOrientation(Orientation);
    Orientation orientation() const;

    void setPageSize(PageSize);
    PageSize pageSize() const;

    void setPageOrder(PageOrder);
    PageOrder pageOrder() const;

    void setResolution(int);
    int resolution() const;

    void setColorMode(ColorMode);
    ColorMode colorMode() const;

    void setCollateCopies(bool collate);
    bool collateCopies() const;

    void setFullPage(bool);
    bool fullPage() const;

    void setNumCopies(int);
    int numCopies() const;

    void setPaperSource(PaperSource);
    PaperSource paperSource() const;

    QList<int> supportedResolutions() const;

#ifdef Q_WS_WIN
    void setWinPageSize(short winPageSize);
    short winPageSize() const;
#endif

    QRect paperRect() const;
    QRect pageRect() const;

#ifndef Q_WS_WIN
    QString printerSelectionOption() const;
    void setPrinterSelectionOption(const QString &);
#endif

    bool newPage();
    bool abort();

    PrinterState printerState() const;

    QPaintEngine *paintEngine() const;

#ifdef Q_WS_WIN
    HDC getDC() const;
    void releaseDC(HDC hdc);
#endif

#if defined (QT_COMPAT)
#ifdef Q_WS_MAC
    QT_COMPAT bool pageSetup(QWidget *parent = 0);
    QT_COMPAT bool printSetup(QWidget *parent = 0);
#endif

    QT_COMPAT bool setup(QWidget *parent = 0);

    QT_COMPAT void setFromTo(int fromPage, int toPage);
    QT_COMPAT int fromPage() const;
    QT_COMPAT int toPage() const;

    QT_COMPAT void setMinMax(int minPage, int maxPage);
    QT_COMPAT int minPage() const;
    QT_COMPAT int maxPage() const;

    QT_COMPAT void setCollateCopiesEnabled(bool);
    QT_COMPAT bool collateCopiesEnabled() const;

    QT_COMPAT void setPrintRange(PrintRange range);
    QT_COMPAT PrintRange printRange() const;

    QT_COMPAT void setOptionEnabled(PrinterOption, bool enable);
    QT_COMPAT bool isOptionEnabled(PrinterOption) const;

    inline QT_COMPAT QSize margins() const;
    inline QT_COMPAT void margins(uint *top, uint *left, uint *bottom, uint *right) const;

    inline QT_COMPAT bool aborted() { return printerState() == Aborted; }
#endif

protected:
    int         metric(PaintDeviceMetric) const;

private:
    Q_DISABLE_COPY(QPrinter)

    QPrinterPrivate *d_ptr;

    friend class QPrintDialogWin;
};

#ifdef QT_COMPAT
Q_GUI_EXPORT QSize QPrinter::margins() const
{
    QRect page = pageRect();
    QRect paper = paperRect();
    return QSize(page.left() - paper.left(), page.top() - paper.top());
}

Q_GUI_EXPORT void QPrinter::margins(uint *top, uint *left, uint *bottom, uint *right) const
{
    QRect page = pageRect();
    QRect paper = paperRect();
    if (top)
        *top = page.top() - paper.top();
    if (left)
        *left = page.left() - paper.left();
    if (bottom)
        *bottom = paper.bottom() - page.bottom();
    if (right)
        *right = paper.right() - page.right();
}
#endif // QT_COMPAT

#endif // QT_NO_PRINTER

#endif // QPRINTER_H
