/****************************************************************************
**
** Implementation of QMacPrintEngine class.
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

#include "qprintengine_mac.h"
#include "qprintengine_mac_p.h"
#include "qpaintdevicemetrics.h"


#define d d_func()
#define q q_func()

QMacPrintEngine::QMacPrintEngine(QPaintDevice *dev, QPrinter::PrinterMode mode) 
    : QCoreGraphicsPaintEngine(*(new QMacPrintEnginePrivate), dev)
{
    d->mode = mode;
    d->initialize();
}

bool QMacPrintEngine::begin(QPaintDevice *dev)
{
    if (!QCoreGraphicsPaintEngine::begin(dev))
        return false;
    Q_ASSERT_X(d->state == QPrinter::Idle, "QMacPrintEngine", "printer already active");
    bool ret = true;

    if (PMSessionValidatePrintSettings(d->session, d->settings, kPMDontWantBoolean) != noErr
        || PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean) != noErr
        || PMSessionBeginDocument(d->session, d->settings, d->format) != noErr) {
        d->state == QPrinter::Error;
        ret = false;
    }

    if (d->outputToFile) {
        QCFType<CFURLRef> outFile = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault,
                                                                  QCFString(d->outputFilename),
                                                                  kCFURLPOSIXPathStyle,
                                                                  false);
        if (PMSessionSetDestination(d->session, d->settings, kPMDestinationPreview,
                                    kPMDocumentFormatPDF, outFile) != noErr) {
            qWarning("problem setting file [%s]", d->outputFilename.utf8());
            ret = false;
        }
    }
    if (ret) {
        d->state = QPrinter::Active;
        d->newPage_helper();
    }
    return ret;
}

bool QMacPrintEngine::end()
{
    if (d->state != QPrinter::Idle) {
        PMSessionEndPage(d->session);
        PMSessionEndDocument(d->session);
        PMRelease(d->session);
    }
    d->state  = QPrinter::Idle;
    return true;
}

void QMacPrintEngine::setPrinterName(const QString &)
{
}

QString QMacPrintEngine::printerName() const
{
    return QString();
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

void QMacPrintEngine::setPrintProgram(const QString &) {}
QString QMacPrintEngine::printProgram() const {return QString(); }

void QMacPrintEngine::setDocName(const QString &) {}
QString QMacPrintEngine::docName() const {return QString(); }

void QMacPrintEngine::setCreator(const QString &) {}
QString QMacPrintEngine::creator() const{return QString(); }

void QMacPrintEngine::setOrientation(QPrinter::Orientation) {}
QPrinter::Orientation QMacPrintEngine::orientation() const { return (QPrinter::Orientation)0; }

void QMacPrintEngine::setPageSize(QPrinter::PageSize) {}
QPrinter::PageSize QMacPrintEngine::pageSize() const {return (QPrinter::PageSize)0;}

void QMacPrintEngine::setPageOrder(QPrinter::PageOrder) {}
QPrinter::PageOrder QMacPrintEngine::pageOrder() const {return (QPrinter::PageOrder)0; }

void QMacPrintEngine::setResolution(int) {}
int QMacPrintEngine::resolution() const {return 0;}

void QMacPrintEngine::setColorMode(QPrinter::ColorMode) {}
QPrinter::ColorMode QMacPrintEngine::colorMode() const {return (QPrinter::ColorMode) 0; }

void QMacPrintEngine::setFullPage(bool) {}
bool QMacPrintEngine::fullPage() const {return false;}

void QMacPrintEngine::setNumCopies(int) {}
int QMacPrintEngine::numCopies() const {return 0;}

void QMacPrintEngine::setCollateCopies(bool) {}
bool QMacPrintEngine::collateCopies() const {return false;}

void QMacPrintEngine::setPrintRange(QPrinter::PrintRange ) {}
QPrinter::PrintRange QMacPrintEngine::printRange() const {return (QPrinter::PrintRange) 0;}

void QMacPrintEngine::setPaperSource(QPrinter::PaperSource) {}
QPrinter::PaperSource QMacPrintEngine::paperSource()   const{return (QPrinter::PaperSource) 0;}

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
    if (PMGetAdjustedPaperRect(d->format, &macrect) == noErr)
        r.setCoords((int)macrect.left, (int)macrect.top, (int)macrect.right, (int)macrect.bottom);
    return r;
}

QRect QMacPrintEngine::pageRect() const
{
    QRect r;
    PMRect macrect;
    if (PMGetAdjustedPageRect(d->format, &macrect) == noErr)
        r.setCoords((int)macrect.left, (int)macrect.top, (int)macrect.right, (int)macrect.bottom);
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

    
    CFStringRef strings[1] = { kPMGraphicsContextCoreGraphics };
    QCFType<CFArrayRef> contextArray = CFArrayCreate(kCFAllocatorDefault,
                                                     reinterpret_cast<const void **>(strings),
                                                     1, &kCFTypeArrayCallBacks);
    bool contextOK = contextArray;
    if (contextOK)
        contextOK = PMSessionSetDocumentFormatGeneration(session, kPMDocumentFormatPDF,
                                                         contextArray, 0) == noErr;

    if (!settingsOK || !formatOK || !contextOK)
        state = QPrinter::Error;
}

bool QMacPrintEnginePrivate::newPage_helper()
{
    Q_ASSERT(d->state == QPrinter::Active);
    bool ret = true;
    
    if (PMSessionError(session) != noErr) {
        abort();
        return false;
    }
    OSStatus err = PMSessionBeginPage(session, format, 0);
    err = PMSessionGetGraphicsContext(session, kPMGraphicsContextCoreGraphics,
                                      reinterpret_cast<void **>(&hd));
    
    
    if (err != noErr) {
        state = QPrinter::Error;
        ret = false;
    }
    QRect page = q->pageRect();
    QRect paper = q->paperRect();
    CGContextScaleCTM(hd, 1, -1);
    CGContextTranslateCTM(hd, 0, -paper.height());
    if (ret) {
        if (q->fullPage()) {
            CGContextTranslateCTM(hd, page.x() - paper.x(), page.y() - paper.y());
        } else {
            CGContextTranslateCTM(hd, 0, 0);
        }
    }
    return ret;
}

