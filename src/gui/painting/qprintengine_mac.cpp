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

#include "qprintengine_mac.h"
#include "qprintengine_mac_p.h"
#include "qpaintdevicemetrics.h"


#define d d_func()
#define q q_func()

QMacPrintEngine::QMacPrintEngine(QPrinter::PrinterMode mode)
#if defined(QMAC_PRINTER_USE_QUICKDRAW)
    : QQuickDrawPaintEngine(*(new QMacPrintEnginePrivate))
#else
    : QCoreGraphicsPaintEngine(*(new QMacPrintEnginePrivate))
#endif
{
    d->mode = mode;
    d->initialize();
}

bool QMacPrintEngine::begin(QPaintDevice *dev)
{
#if defined(QMAC_PRINTER_USE_QUICKDRAW)
    if (!QQuickDrawPaintEngine::begin(dev))
        return false;
#else
    if (!QCoreGraphicsPaintEngine::begin(dev))
        return false;
#endif
    Q_ASSERT_X(d->state == QPrinter::Idle, "QMacPrintEngine", "printer already active");

    if (PMSessionValidatePrintSettings(d->session, d->settings, kPMDontWantBoolean) != noErr
        || PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean) != noErr) {
        d->state == QPrinter::Error;
        return false;
    }

    if (d->outputToFile) {
        QCFType<CFURLRef> outFile = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault,
                                                                  QCFString(d->outputFilename),
                                                                  kCFURLPOSIXPathStyle,
                                                                  false);
        if (PMSessionSetDestination(d->session, d->settings, kPMDestinationFile,
                                    kPMDocumentFormatPDF, outFile) != noErr) {
            qWarning("problem setting file [%s]", d->outputFilename.utf8());
            return false;
        }
    }

    if (PMSessionBeginDocument(d->session, d->settings, d->format) != noErr) {
        d->state == QPrinter::Error;
        return false;
    }

    d->state = QPrinter::Active;
    d->newPage_helper();
    return true;
}

bool QMacPrintEngine::end()
{
#if defined(QMAC_PRINTER_USE_QUICKDRAW)
    QQuickDrawPaintEngine::end();
#else
    d->hd = 0;
    QCoreGraphicsPaintEngine::end();
#endif
    if (d->state != QPrinter::Idle) {
        
        PMSessionEndPage(d->session);
        PMSessionEndDocument(d->session);
        PMRelease(d->session);
    }
    d->state  = QPrinter::Idle;
    return true;
}

void QMacPrintEngine::setPrinterName(const QString &name)
{
    OSStatus status = PMSessionSetCurrentPrinter(d->session, QCFString(name));
    if (status == noErr)
        qWarning("QMacPrintEngine::setPrinterName: Error setting printer %ld", status);
}

QString QMacPrintEngine::printerName() const
{
    QString ret;
    CFIndex currIndex;
    PMPrinter unused;
    QCFType<CFArrayRef> printerList;
    OSStatus status = PMSessionCreatePrinterList(d->session, &printerList, &currIndex, &unused);
    if (status != noErr)
        qWarning("QMacPrintEngine::printerName: Problem getting list of printers %ld", status);
    if (printerList)
        ret = QCFString::toQString(static_cast<CFStringRef>(CFArrayGetValueAtIndex(printerList,
                                                                                   currIndex)));
    return ret;
}

void QMacPrintEngine::setOutputToFile(bool toFile)
{
    d->outputToFile = toFile;
}

bool QMacPrintEngine::outputToFile() const
{
    return d->outputToFile;
}

void QMacPrintEngine::setOutputFileName(const QString &outputFileName)
{
    d->outputFilename = outputFileName;
}

QString QMacPrintEngine::outputFileName()const
{
    return d->outputFilename;
}

Qt::HANDLE
QMacPrintEngine::handle() const
{
    return d->qdHandle;
}

void QMacPrintEngine::setPrintProgram(const QString &) {}
QString QMacPrintEngine::printProgram() const { return QString(); }

void QMacPrintEngine::setDocName(const QString &) {}
QString QMacPrintEngine::docName() const { return QString(); }

void QMacPrintEngine::setCreator(const QString &) {}
QString QMacPrintEngine::creator() const { return QString(); }

void QMacPrintEngine::setOrientation(QPrinter::Orientation orientation)
{
    PMOrientation o = orientation == QPrinter::Portrait ? kPMPortrait : kPMLandscape;
    PMSetOrientation(d->format, o, false);
}

QPrinter::Orientation QMacPrintEngine::orientation() const
{
    PMOrientation orientation;
    PMGetOrientation(d->format, &orientation);
    return orientation == kPMPortrait ? QPrinter::Portrait : QPrinter::Landscape;
}

struct PaperSize
{
    int w;
    int h;
};

static const PaperSize sizes[] = {
    { 210, 297 },   // A4
    { 176, 250 },   // B5
    { 216, 279 },   // U.S. Letter
    { 216, 356 },   // U.S. Legal
    { 191, 254 },   // U.S. Executive
    { 841, 1189 },  // A0
    { 594, 841 },   // A1
    { 420, 594 },   // A2
    { 297, 420 },   // A3
    { 148, 210 },   // A5
    { 105, 148 },   // A6
    { 74, 105 },    // A7
    { 52, 74 },     // A8
    { 37, 52 },     // A9
    { 1000, 1414 }, // B0
    { 707, 1000 },  // B1
    { 31, 44 },     // B10
    { 500, 707 },   // B2
    { 353, 500 },   // B3
    { 250, 353 },   // B4
    { 125, 176 },   // B6
    { 88, 125 },    // B7
    { 62, 88 },     // B8
    { 44, 62 },     // B9
    { 162, 229 },   // C5E
    { 105, 241 },   // Comm10E
    { 110, 222 },   // DLE
    { 216, 330 },   // Folio
    { 432, 279 },   // Ledger
    { 279, 432 }   // Tabloid
};

void QMacPrintEngine::setPageSize(QPrinter::PageSize ps)
{
    PaperSize newSize = sizes[ps];
    QCFType<CFArrayRef> formats;
    PMPrinter printer;

    if (PMSessionGetCurrentPrinter(d->session, &printer) == noErr
        && PMSessionCreatePageFormatList(d->session, printer, &formats) == noErr) {
        CFIndex total = CFArrayGetCount(formats);
        PMPageFormat tmp;
        PMRect paper;
        for (CFIndex idx = 0; idx < total; ++idx) {
            tmp = static_cast<PMPageFormat>(
                                        const_cast<void *>(CFArrayGetValueAtIndex(formats, idx)));
            PMGetUnadjustedPaperRect(tmp, &paper);
            int wMM = int((paper.right - paper.left) / 72 * 25.4 + 0.5);
            int hMM = int((paper.bottom - paper.top) / 72 * 25.4 + 0.5);
            if (newSize.w == wMM && newSize.h == hMM) {
                PMCopyPageFormat(tmp, d->format);
                break;
            }
        }
    }
}

QPrinter::PageSize QMacPrintEngine::pageSize() const
{
    PMRect paper;
    PMGetUnadjustedPaperRect(d->format, &paper);
    int wMM = int((paper.right - paper.left) / 72 * 25.4 + 0.5);
    int hMM = int((paper.bottom - paper.top) / 72 * 25.4 + 0.5);
    for (int i = QPrinter::A4; i < QPrinter::NPageSize; ++i) {
        if (sizes[i].w == wMM && sizes[i].h == hMM)
            return (QPrinter::PageSize)i;
    }
    return QPrinter::Custom;
}

void QMacPrintEngine::setPageOrder(QPrinter::PageOrder) {}
QPrinter::PageOrder QMacPrintEngine::pageOrder() const {return (QPrinter::PageOrder)0; }

void QMacPrintEngine::setResolution(int) {}
int QMacPrintEngine::resolution() const {return 0;}

void QMacPrintEngine::setColorMode(QPrinter::ColorMode) {}
QPrinter::ColorMode QMacPrintEngine::colorMode() const {return (QPrinter::ColorMode) 0; }

void QMacPrintEngine::setFullPage(bool fullPage)
{
    d->fullPage = fullPage;
}

bool QMacPrintEngine::fullPage() const
{
    return d->fullPage;
}

void QMacPrintEngine::setNumCopies(int copies)
{
    PMSetCopies(d->settings, copies, false);
}

int QMacPrintEngine::numCopies() const
{
    return 1; /* Carbon handles # of copies for us */
}

void QMacPrintEngine::setCollateCopies(bool) {}
bool QMacPrintEngine::collateCopies() const { return false; }

void QMacPrintEngine::setPaperSource(QPrinter::PaperSource) {}
QPrinter::PaperSource QMacPrintEngine::paperSource() const
{
    return (QPrinter::PaperSource) 0;
}

QList<int> QMacPrintEngine::supportedResolutions() const
{
    Q_ASSERT_X(d->session, "QMacPrinterEngine::supporterdResolutions",
               "must have a valid printer session");
    UInt32 resCount;
    QList<int> resolutions;
    PMPrinter printer;
    if (PMSessionGetCurrentPrinter(d->session, &printer) == noErr) {
        PMResolution res;
        OSStatus status = PMPrinterGetPrinterResolutionCount(printer, &resCount);
        if (status  == kPMNotImplemented) {
            // *Sigh* we have to use the non-indexed version.
            if (PMPrinterGetPrinterResolution(printer, kPMMinSquareResolution, &res) == noErr)
                resolutions.append(int(res.hRes));
            if (PMPrinterGetPrinterResolution(printer, kPMMaxSquareResolution, &res) == noErr) {
                if (!resolutions.contains(int(res.hRes)))
                    resolutions.append(int(res.hRes));
            }
            if (PMPrinterGetPrinterResolution(printer, kPMDefaultResolution, &res) == noErr) {
                if (!resolutions.contains(int(res.hRes)))
                    resolutions.append(int(res.hRes));
            }
        } else if (status == noErr) {
            // According to the docs, index start at 1.
            for (UInt32 i = 1; i <= resCount; ++i) {
                if (PMPrinterGetIndexedPrinterResolution(printer, i, &res) == noErr)
                    resolutions.append(int(res.hRes));
            }
        } else {
            qWarning("QMacPrintEngine::supportedResolutions() unexpected error: %ld", status);
        }
    }
    return resolutions;
}

QRect QMacPrintEngine::paperRect() const
{
    QRect r;
    PMRect macrect;
    if (PMGetAdjustedPaperRect(d->format, &macrect) == noErr) {
        r.setCoords((int)macrect.left, (int)macrect.top, (int)macrect.right, (int)macrect.bottom);
        r.moveBy(-r.x(), -r.y());
    }
    
    return r;
}

QRect QMacPrintEngine::pageRect() const
{
    QRect r;
    PMRect macrect, macpaper;
    if (PMGetAdjustedPageRect(d->format, &macrect) == noErr
        && PMGetAdjustedPaperRect(d->format, &macpaper) == noErr) {
        r.setCoords(int(macrect.left), int(macrect.top), int(macrect.right), int(macrect.bottom));
        r.moveBy(int(-macpaper.left), int(-macpaper.top));
    }
    return r;
}

QString QMacPrintEngine::printerSelectionOption() const {return QString();}
void QMacPrintEngine::setPrinterSelectionOption(const QString &) {}

QPrinter::PrinterState QMacPrintEngine::printerState() const
{
    return d->state;
}

bool QMacPrintEngine::newPage()
{
    Q_ASSERT(d->state == QPrinter::Active);
    if (PMSessionEndPage(d->session) != noErr)  {
        d->state = QPrinter::Error;
        return false;
    }
    return d->newPage_helper();
}

bool QMacPrintEngine::abort()
{
    if (d->state == QPrinter::Active)
        return false;
    bool ret = end();
    d->state = QPrinter::Aborted;
    return ret;
}

static inline int qt_get_PDMWidth(PMPageFormat pformat, bool fullPage)
{
    int val = 0;
    PMRect r;
    if (fullPage) {
        if (PMGetAdjustedPaperRect(pformat, &r) == noErr)
            val = (int)(r.right - r.left);
    } else {
        if (PMGetAdjustedPageRect(pformat, &r) == noErr)
            val = (int)(r.right - r.left);
    }
    return val;
}

static inline int qt_get_PDMHeight(PMPageFormat pformat, bool fullPage)
{
    int val = 0;
    PMRect r;
    if (fullPage) {
        if (PMGetAdjustedPaperRect(pformat, &r) == noErr)
            val = (int)(r.bottom - r.top);
    } else {
        if (PMGetAdjustedPageRect(pformat, &r) == noErr)
            val = (int)(r.bottom - r.top);
    }
    return val;
}


int QMacPrintEngine::metric(int m) const
{
    int val = 1;
    switch (m) {
    case QPaintDeviceMetrics::PdmWidth:
        val = qt_get_PDMWidth(d->format, fullPage());
        break;
    case QPaintDeviceMetrics::PdmHeight:
        val = qt_get_PDMHeight(d->format, fullPage());
        break;
    case QPaintDeviceMetrics::PdmWidthMM:
        val = metric(QPaintDeviceMetrics::PdmWidth);
        val = int((val * 254 + 5 * d->resolution.hRes) / (10 * d->resolution.hRes));
        break;
    case QPaintDeviceMetrics::PdmHeightMM:
        val = metric(QPaintDeviceMetrics::PdmHeight);
        val = int((val * 254 + 5 * d->resolution.vRes) / (10 * d->resolution.vRes));
        break;
    case QPaintDeviceMetrics::PdmPhysicalDpiX:
    case QPaintDeviceMetrics::PdmPhysicalDpiY: {
        PMPrinter printer;
        if(PMSessionGetCurrentPrinter(d->session, &printer) == noErr) {
            PMResolution resolution;
            PMPrinterGetPrinterResolution(printer, kPMCurrentValue, &resolution);
            val = (int)resolution.vRes;
            break;
        }
        //otherwise fall through
    }
    case QPaintDeviceMetrics::PdmDpiY:
        val = (int)d->resolution.vRes;
        break;
    case QPaintDeviceMetrics::PdmDpiX:
        val = (int)d->resolution.hRes;
        break;
    case QPaintDeviceMetrics::PdmNumColors:
        val = (1 << metric(QPaintDeviceMetrics::PdmDepth));
        break;
    case QPaintDeviceMetrics::PdmDepth:
        val = 24;
        break;
    default:
        val = 0;
#if defined(QT_CHECK_RANGE)
        qWarning("QPrinter::metric: Invalid metric command");
#endif
    }
    return val;
}

void QMacPrintEnginePrivate::initialize()
{
    Q_ASSERT(!format);
    Q_ASSERT(!settings);
    Q_ASSERT(!session);

    fullPage = false;
    outputToFile = false;

    if (PMCreateSession(&session) != noErr)
        session = 0;

    PMTag res;
    if (mode == QPrinter::HighResolution)
        res = kPMMaxSquareResolution;
    else
        res = kPMDefaultResolution;

    PMPrinter printer;
    if (session && PMSessionGetCurrentPrinter(session, &printer) == noErr) {
        OSStatus ret = PMPrinterGetPrinterResolution(printer, res, &resolution);
        Q_ASSERT(ret == noErr);
        Q_UNUSED(ret);
    }

    bool settingsOK = PMCreatePrintSettings(&settings) == noErr;
    if (settingsOK)
        settingsOK = PMSessionDefaultPrintSettings(session, settings) == noErr;


    bool formatOK = PMCreatePageFormat(&format) == noErr;
    if (formatOK) {
        formatOK = PMSessionDefaultPageFormat(session, format) == noErr;
        formatOK = PMSetResolution(format, &resolution) == noErr;
    }

#if !defined(QMAC_PRINTER_USE_QUICKDRAW)
    CFStringRef strings[1] = { kPMGraphicsContextCoreGraphics };
    QCFType<CFArrayRef> contextArray = CFArrayCreate(kCFAllocatorDefault,
                                                     reinterpret_cast<const void **>(strings),
                                                     1, &kCFTypeArrayCallBacks);
    bool contextOK = contextArray;
    if (contextOK) 
        contextOK = PMSessionSetDocumentFormatGeneration(session, kPMDocumentFormatPDF,
                                                         contextArray, 0) == noErr;
    if(!contextOK) 
        state = QPrinter::Error;
    else
#endif
        if (!settingsOK || !formatOK)
            state = QPrinter::Error;
}

bool QMacPrintEnginePrivate::newPage_helper()
{
    Q_ASSERT(d->state == QPrinter::Active);

    if (PMSessionError(session) != noErr) {
        abort();
        return false;
    }
    OSStatus err = PMSessionBeginPage(session, format, 0);
#if defined(QMAC_PRINTER_USE_QUICKDRAW)
    err = PMSessionGetGraphicsContext(session, kPMGraphicsContextQuickdraw,
                                      reinterpret_cast<void **>(&qdHandle));
#else
    err = PMSessionGetGraphicsContext(session, kPMGraphicsContextCoreGraphics,
                                      reinterpret_cast<void **>(&hd));
#endif

    if (err != noErr) {
        state = QPrinter::Error;
        return false;
    }

    QRect page = q->pageRect();
    QRect paper = q->paperRect();
#if !defined(QMAC_PRINTER_USE_QUICKDRAW)
    CGContextScaleCTM(hd, 1, -1);
    CGContextTranslateCTM(hd, 0, -paper.height());
    if (!fullPage)
        CGContextTranslateCTM(hd, page.x() - paper.x(), page.y() - paper.y());
    orig_xform = CGContextGetCTM(hd);
    setClip(0);
#else
    QMacSavedPortInfo mp(d->pdev);
    SetOrigin(page.x() - paper.x(), page.y() - paper.y());
#endif
    return true;
}

