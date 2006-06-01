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
#include "QtCore/qlibrary.h"
#include "QtCore/qstringlist.h"
#include "QtCore/qhash.h"
#include "QtCore/qabstractitemmodel.h"

class QPrinter;
class QPSPrintEnginePrivate;
class QCUPSSupport;

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

#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
    QCUPSSupport* cupsSupport();
#endif

private:
    Q_DISABLE_COPY(QPSPrintEngine)
};

#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
#include <cups/cups.h>

typedef int (*CupsGetDests)(cups_dest_t **dests);
typedef const char* (*CupsGetPPD)(const char *printer);
typedef int (*CupsMarkOptions)(ppd_file_t *ppd, int num_options, cups_option_t *options);
typedef ppd_file_t* (*PPDOpenFile)(const char *filename);
typedef void (*PPDMarkDefaults)(ppd_file_t *ppd);
typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);
typedef void (*PPDClose)(ppd_file_t *ppd);
typedef int (*PPDMarkOption)(ppd_file_t *ppd, const char *keyword, const char *option);
typedef void (*CupsFreeOptions)(int num_options, cups_option_t *options);
typedef void (*CupsSetDests)(int num_dests, cups_dest_t *dests);
typedef int (*CupsAddOption)(const char *name, const char *value, int num_options, cups_option_t **options);

class QCUPSSupport
{
public:
    QCUPSSupport(QPSPrintEnginePrivate *qq);
    ~QCUPSSupport();

    bool isAvailable() const;

    int availablePrintersCount() const;
    const cups_dest_t* availablePrinters() const;
    int currentPrinterIndex() const;
    const ppd_file_t* setCurrentPrinter(int index);

    const ppd_file_t* currentPPD() const;
    const ppd_option_t* ppdOption(const char *key) const;

//     const cups_option_t* printerOption(const QString &key) const;
    const ppd_option_t* pageSizes() const;

    int markOption(const char* name, const char* value);
    void saveOptions(QList<const ppd_option_t*> options, QList<const char*> markedOptions);

    QRect paperRect() const;
    QRect pageRect() const;

    QStringList options() const;

private:
    void collectMarkedOptions(QStringList& list, const ppd_group_t* group = 0) const;
    void collectMarkedOptionsHelper(QStringList& list, const ppd_group_t* group) const;

    QPSPrintEnginePrivate *q;
    QLibrary cupsLib;

    CupsGetDests _cupsGetDests;
    CupsGetPPD _cupsGetPPD;
    PPDOpenFile _ppdOpenFile;
    PPDMarkDefaults _ppdMarkDefaults;
    PPDClose _ppdClose;
    CupsMarkOptions _cupsMarkOptions;
    PPDMarkOption _ppdMarkOption;
    CupsFreeOptions _cupsFreeOptions;
    CupsSetDests _cupsSetDests;
    CupsAddOption _cupsAddOption;

    int prnCount;
    cups_dest_t *printers;
    const ppd_option_t* page_sizes;
    int currPrinterIndex;
    ppd_file_t *currPPD;
};

#endif // !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)


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

    bool duplex;
#if !defined(QT_NO_CUPS) && !defined(QT_NO_LIBRARY)
    QCUPSSupport cups;
#endif

};

#endif // QT_NO_PRINTER

#endif // QPRINTENGINE_PS_P_H
