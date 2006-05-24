/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QPRINTER_H
#define QPRINTER_H

#include <QtGui/qpaintdevice.h>
#include <QtCore/qstring.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_PRINTER

#if defined(B0)
#undef B0 // Terminal hang-up.  We assume that you do not want that.
#endif

class QPrinterPrivate;
class QPaintEngine;
class QPrintEngine;

class Q_GUI_EXPORT QPrinter : public QPaintDevice
{
    Q_DECLARE_PRIVATE(QPrinter)
public:
    enum PrinterMode { ScreenResolution, PrinterResolution, HighResolution };

    explicit QPrinter(PrinterMode mode = ScreenResolution);
    ~QPrinter();

    int devType() const;

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

    enum OutputFormat { NativeFormat, PdfFormat };

    // ### Qt 5: Merge with QAbstractPrintDialog::PrintRange
    enum PrintRange { AllPages, Selection, PageRange };

#ifdef QT3_SUPPORT
    enum PrinterOption { PrintToFile, PrintSelection, PrintPageRange };
#endif // QT3_SUPPORT

    void setOutputFormat(OutputFormat format);
    OutputFormat outputFormat() const;

    void setPrinterName(const QString &);
    QString printerName() const;

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

    void setFontEmbeddingEnabled(bool enable);
    bool fontEmbeddingEnabled() const;

    void setDoubleSidePrinting(bool enable);
    bool doubleSidePrinting();

#ifdef Q_WS_WIN
    void setWinPageSize(int winPageSize);
    int winPageSize() const;
#endif

    QRect paperRect() const;
    QRect pageRect() const;

#if !defined(Q_WS_WIN) || defined(qdoc)
    QString printerSelectionOption() const;
    void setPrinterSelectionOption(const QString &);
#endif

    bool newPage();
    bool abort();

    PrinterState printerState() const;

    QPaintEngine *paintEngine() const;
    QPrintEngine *printEngine() const;

#ifdef Q_WS_WIN
    HDC getDC() const;
    void releaseDC(HDC hdc) const;
#endif

    void setFromTo(int fromPage, int toPage);
    int fromPage() const;
    int toPage() const;

#ifndef QT_NO_PRINTDIALOG
    void setPrintRange(PrintRange range);
    PrintRange printRange() const;
#endif

#ifdef QT3_SUPPORT
#ifdef Q_WS_MAC
    QT3_SUPPORT bool pageSetup(QWidget *parent = 0);
    QT3_SUPPORT bool printSetup(QWidget *parent = 0);
#endif

    QT3_SUPPORT bool setup(QWidget *parent = 0);

    QT3_SUPPORT void setMinMax(int minPage, int maxPage);
    QT3_SUPPORT int minPage() const;
    QT3_SUPPORT int maxPage() const;

    QT3_SUPPORT void setCollateCopiesEnabled(bool);
    QT3_SUPPORT bool collateCopiesEnabled() const;

    QT3_SUPPORT void setOptionEnabled(PrinterOption, bool enable);
    QT3_SUPPORT bool isOptionEnabled(PrinterOption) const;

    inline QT3_SUPPORT QSize margins() const;
    inline QT3_SUPPORT void margins(uint *top, uint *left, uint *bottom, uint *right) const;

    inline QT3_SUPPORT bool aborted() { return printerState() == Aborted; }

    QT3_SUPPORT void setOutputToFile(bool);
    inline QT3_SUPPORT bool outputToFile() const { return !outputFileName().isEmpty(); }
#endif

protected:
    int metric(PaintDeviceMetric) const;
    void setEngines(QPrintEngine *printEngine, QPaintEngine *paintEngine);

private:
    Q_DISABLE_COPY(QPrinter)

    QPrinterPrivate *d_ptr;

    friend class QPrintDialogWin;
    friend class QAbstractPrintDialog;
};

#ifdef QT3_SUPPORT
inline QSize QPrinter::margins() const
{
    QRect page = pageRect();
    QRect paper = paperRect();
    return QSize(page.left() - paper.left(), page.top() - paper.top());
}

inline void QPrinter::margins(uint *top, uint *left, uint *bottom, uint *right) const
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
#endif

#endif // QT_NO_PRINTER

QT_END_HEADER

#endif // QPRINTER_H
