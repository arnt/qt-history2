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

#include <private/qprintengine_mac_p.h>


#define d d_func()
#define q q_func()

QMacPrintEngine::QMacPrintEngine(QPrinter::PrinterMode mode) : QPaintEngine(*(new QMacPrintEnginePrivate))
{
    d->mode = mode;
    d->initialize();
}

bool QMacPrintEngine::begin(QPaintDevice *dev)
{
    d->paintEngine->begin(dev);
    Q_ASSERT_X(d->state == QPrinter::Idle, "QMacPrintEngine", "printer already active");

    if (PMSessionValidatePrintSettings(d->session, d->settings, kPMDontWantBoolean) != noErr
        || PMSessionValidatePageFormat(d->session, d->format, kPMDontWantBoolean) != noErr) {
        d->state == QPrinter::Error;
        return false;
    }

    if (!d->outputFilename.isEmpty()) {
        QCFType<CFURLRef> outFile = CFURLCreateWithFileSystemPath(kCFAllocatorSystemDefault,
                                                                  QCFString(d->outputFilename),
                                                                  kCFURLPOSIXPathStyle,
                                                                  false);
        if (PMSessionSetDestination(d->session, d->settings, kPMDestinationFile,
                                    kPMDocumentFormatPDF, outFile) != noErr) {
            qWarning("problem setting file [%s]", d->outputFilename.toUtf8().constData());
            return false;
        }
    }

    if (PMSessionBeginDocument(d->session, d->settings, d->format) != noErr) {
        d->state == QPrinter::Error;
        return false;
    }

    d->state = QPrinter::Active;
    d->newPage_helper();
    setActive(true);
    return true;
}

bool QMacPrintEngine::end()
{
    if(d->paintEngine->type() == QPaintEngine::CoreGraphics)
        static_cast<QCoreGraphicsPaintEngine*>(d->paintEngine)->d->hd = 0;
    d->paintEngine->end();
    if (d->state != QPrinter::Idle) {
        PMSessionEndPage(d->session);
        PMSessionEndDocument(d->session);
        PMRelease(d->session);
    }
    d->state  = QPrinter::Idle;
    return true;
}

QPaintEngine *
QMacPrintEngine::paintEngine() const
{
    return d->paintEngine;
}

Qt::HANDLE QMacPrintEngine::handle() const
{
    return d->qdHandle;
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

void QMacPrintEnginePrivate::setPageSize(QPrinter::PageSize ps)
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
                // reset the orientation and resolution as they are lost in the copy.
                q->setProperty(QPrintEngine::PPK_Orientation, d->orient);
                PMSetResolution(d->format, &d->resolution);
                break;
            }
        }
    }
}

QPrinter::PageSize QMacPrintEnginePrivate::pageSize() const
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

QList<QVariant> QMacPrintEnginePrivate::supportedResolutions() const
{
    Q_ASSERT_X(d->session, "QMacPrinterEngine::supporterdResolutions",
               "must have a valid printer session");
    UInt32 resCount;
    QList<QVariant> resolutions;
    PMPrinter printer;
    if (PMSessionGetCurrentPrinter(d->session, &printer) == noErr) {
        PMResolution res;
        OSStatus status = PMPrinterGetPrinterResolutionCount(printer, &resCount);
        if (status  == kPMNotImplemented) {
            // *Sigh* we have to use the non-indexed version.
            if (PMPrinterGetPrinterResolution(printer, kPMMinSquareResolution, &res) == noErr)
                resolutions.append(int(res.hRes));
            if (PMPrinterGetPrinterResolution(printer, kPMMaxSquareResolution, &res) == noErr) {
                QVariant var(int(res.hRes));
                if (!resolutions.contains(var))
                    resolutions.append(var);
            }
            if (PMPrinterGetPrinterResolution(printer, kPMDefaultResolution, &res) == noErr) {
                QVariant var(int(res.hRes));
                if (!resolutions.contains(var))
                    resolutions.append(var);
            }
        } else if (status == noErr) {
            // According to the docs, index start at 1.
            for (UInt32 i = 1; i <= resCount; ++i) {
                if (PMPrinterGetIndexedPrinterResolution(printer, i, &res) == noErr)
                    resolutions.append(QVariant(int(res.hRes)));
            }
        } else {
            qWarning("QMacPrintEngine::supportedResolutions() unexpected error: %ld", status);
        }
    }
    return resolutions;
}

QPrinter::PrinterState QMacPrintEngine::printerState() const
{
    return d->state;
}

bool QMacPrintEngine::newPage()
{
    Q_ASSERT(d->state == QPrinter::Active);
    OSStatus err = PMSessionEndPage(d->session);
    if (err != noErr)  {
        qWarning("QMacPrintEngine::newPage: Cannot end current page. %ld", err);
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


int QMacPrintEngine::metric(QPaintDevice::PaintDeviceMetric m) const
{
    int val = 1;
    switch (m) {
    case QPaintDevice::PdmWidth:
        if (d->state == QPrinter::Active
            || property(PPK_Orientation).toInt() == QPrinter::Portrait)
            val = qt_get_PDMWidth(d->format, property(PPK_FullPage).toBool());
        else
            val = qt_get_PDMHeight(d->format, property(PPK_FullPage).toBool());
        break;
    case QPaintDevice::PdmHeight:
        if (d->state == QPrinter::Active
            || property(PPK_Orientation).toInt() == QPrinter::Portrait)
            val = qt_get_PDMHeight(d->format, property(PPK_FullPage).toBool());
        else
            val = qt_get_PDMWidth(d->format, property(PPK_FullPage).toBool());
        break;
    case QPaintDevice::PdmWidthMM:
        val = metric(QPaintDevice::PdmWidth);
        val = int((val * 254 + 5 * d->resolution.hRes) / (10 * d->resolution.hRes));
        break;
    case QPaintDevice::PdmHeightMM:
        val = metric(QPaintDevice::PdmHeight);
        val = int((val * 254 + 5 * d->resolution.vRes) / (10 * d->resolution.vRes));
        break;
    case QPaintDevice::PdmPhysicalDpiX:
    case QPaintDevice::PdmPhysicalDpiY: {
        PMPrinter printer;
        if(PMSessionGetCurrentPrinter(d->session, &printer) == noErr) {
            PMResolution resolution;
            PMPrinterGetPrinterResolution(printer, kPMCurrentValue, &resolution);
            val = (int)resolution.vRes;
            break;
        }
        //otherwise fall through
    }
    case QPaintDevice::PdmDpiY:
        val = (int)d->resolution.vRes;
        break;
    case QPaintDevice::PdmDpiX:
        val = (int)d->resolution.hRes;
        break;
    case QPaintDevice::PdmNumColors:
        val = (1 << metric(QPaintDevice::PdmDepth));
        break;
    case QPaintDevice::PdmDepth:
        val = 24;
        break;
    default:
        val = 0;
        qWarning("QPrinter::metric: Invalid metric command");
    }
    return val;
}

void QMacPrintEnginePrivate::initialize()
{
    Q_ASSERT(!format);
    Q_ASSERT(!settings);
    Q_ASSERT(!session);

#if 0 //always use coregraphics for now until the bugs are kicked out
#if !defined(QMAC_NO_COREGRAPHICS)
    if(!qgetenv("QT_MAC_USE_QUICKDRAW"))
        paintEngine = new QCoreGraphicsPaintEngine();
    else
#endif
        paintEngine = new QQuickDrawPaintEngine();
#else
    paintEngine = new QCoreGraphicsPaintEngine();
#endif

    q->gccaps = paintEngine->gccaps;

    fullPage = false;

    if (PMCreateSession(&session) != noErr)
        session = 0;

    PMTag res;
    if (mode == QPrinter::HighResolution)
        res = kPMMaxSquareResolution;
    else
        res = kPMDefaultResolution;

    PMPrinter printer;
    if (session && PMSessionGetCurrentPrinter(session, &printer) == noErr) {
        if(PMPrinterGetPrinterResolution(printer, res, &resolution) != noErr)
            qWarning("QPrinter::initialize: Cannot get printer resolution");
    }

    bool settingsOK = PMCreatePrintSettings(&settings) == noErr;
    if (settingsOK)
        settingsOK = PMSessionDefaultPrintSettings(session, settings) == noErr;


    bool formatOK = PMCreatePageFormat(&format) == noErr;
    if (formatOK) {
        formatOK = PMSessionDefaultPageFormat(session, format) == noErr;
        formatOK = PMSetResolution(format, &resolution) == noErr;
    }

    if(paintEngine->type() == QPaintEngine::CoreGraphics) {
        CFStringRef strings[1] = { kPMGraphicsContextCoreGraphics };
        QCFType<CFArrayRef> contextArray = CFArrayCreate(kCFAllocatorDefault,
                                                         reinterpret_cast<const void **>(strings),
                                                     1, &kCFTypeArrayCallBacks);
        OSStatus err = PMSessionSetDocumentFormatGeneration(session, kPMDocumentFormatPDF,
                                                            contextArray, 0);
        if(err != noErr) {
            qWarning("QMacPrintEngine::initialize: Cannot set format generation to PDF: %ld", err);
            state = QPrinter::Error;
        }
    }
    if (!settingsOK || !formatOK) {
        qWarning("QMacPrintEngine::initialize: Unable to initialize QPainter");
        state = QPrinter::Error;
    }
}

bool QMacPrintEnginePrivate::newPage_helper()
{
    Q_ASSERT(d->state == QPrinter::Active);

    if (PMSessionError(session) != noErr) {
        abort();
        return false;
    }
    if(PMSessionBeginPage(session, format, 0) != noErr) {
        state = QPrinter::Error;
        return false;
    }

    QRect page = q->property(QPrintEngine::PPK_PageRect).toRect();
    QRect paper = q->property(QPrintEngine::PPK_PaperRect).toRect();
    if(paintEngine->type() == QPaintEngine::CoreGraphics) {
        CGContextRef cgContext;
        OSStatus err = PMSessionGetGraphicsContext(session, kPMGraphicsContextCoreGraphics,
                                                   reinterpret_cast<void **>(&cgContext));
        if(err != noErr) {
            qWarning("QMacPrintEngine::newPage: Cannot retrieve CoreGraphics context. %ld", err);
            state = QPrinter::Error;
            return false;
        }
        QCoreGraphicsPaintEngine *cgEngine = static_cast<QCoreGraphicsPaintEngine*>(d->paintEngine);
        cgEngine->d->hd = cgContext;
        CGContextScaleCTM(cgContext, 1, -1);
        CGContextTranslateCTM(cgContext, 0, -paper.height());
        if (!fullPage)
            CGContextTranslateCTM(cgContext, page.x() - paper.x(), page.y() - paper.y());
        cgEngine->d->orig_xform = CGContextGetCTM(cgContext);
        cgEngine->d->setClip(0);
    } else {
        OSStatus err = PMSessionGetGraphicsContext(session, kPMGraphicsContextQuickdraw,
                                                   reinterpret_cast<void **>(&qdHandle));
        if(err != noErr) {
            qWarning("QMacPrintEngine::newPage: Cannot retrieve QuickDraw context. %ld", err);
            state = QPrinter::Error;
            return false;
        }
        QMacSavedPortInfo mp(d->pdev);
        SetOrigin(page.x() - paper.x(), page.y() - paper.y());
    }
    return true;
}


void QMacPrintEngine::updateState(const QPaintEngineState &state)
{
    d->paintEngine->updateState(state);
}

void QMacPrintEngine::drawRects(const QRectF *r, int num)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawRects(r, num);
}

void QMacPrintEngine::drawPoints(const QPointF *points, int pointCount)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPoints(points, pointCount);
}

void QMacPrintEngine::drawEllipse(const QRectF &r)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawEllipse(r);
}

void QMacPrintEngine::drawLines(const QLineF *lines, int lineCount)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawLines(lines, lineCount);
}

void QMacPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPolygon(points, pointCount, mode);
}

void QMacPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPixmap(r, pm, sr);
}

void QMacPrintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawTextItem(p, ti);
}

void QMacPrintEngine::drawTiledPixmap(const QRectF &dr, const QPixmap &pixmap, const QPointF &sr)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawTiledPixmap(dr, pixmap, sr);
}

void QMacPrintEngine::drawPath(const QPainterPath &path)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPath(path);
}


void QMacPrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    switch (key) {
    case PPK_CollateCopies:
        break;
    case PPK_ColorMode:
        break;
    case PPK_Creator:
        break;
    case PPK_DocumentName:
        break;
    case PPK_PageOrder:
        break;
    case PPK_PaperSource:
        break;
    case PPK_SelectionOption:
        break;
    case PPK_Resolution:  // ###
        break;

    case PPK_FullPage:
        d->fullPage = value.toBool();
        break;
    case PPK_NumberOfCopies:
        PMSetCopies(d->settings, value.toInt(), false);
        break;
    case PPK_Orientation: {
        d->orient = QPrinter::Orientation(value.toInt());
        PMOrientation o = d->orient == QPrinter::Portrait ? kPMPortrait : kPMLandscape;
        PMSetOrientation(d->format, o, false);
        break; }
    case PPK_OutputFileName:
        d->outputFilename = value.toString();
        break;
    case PPK_PageSize:
        d->setPageSize(QPrinter::PageSize(value.toInt()));
        break;
    case PPK_PrinterName: {
        OSStatus status = PMSessionSetCurrentPrinter(d->session, QCFString(value.toString()));
        if (status == noErr)
            qWarning("QMacPrintEngine::setPrinterName: Error setting printer %ld", status);
        break; }
    default:
        break;
    }
}

QVariant QMacPrintEngine::property(PrintEnginePropertyKey key) const
{
    QVariant ret;

    switch (key) {
    case PPK_CollateCopies:
        ret = false;
        break;
    case PPK_ColorMode:
        break;
    case PPK_Creator:
        break;
    case PPK_DocumentName:
        break;
    case PPK_FullPage:
        ret = d->fullPage;
        break;
    case PPK_NumberOfCopies:
        ret = 1;
        break;
    case PPK_Orientation:
        if (d->state == QPrinter::Idle) {
            ret = d->orient;
        } else {
            PMOrientation orientation;
            PMGetOrientation(d->format, &orientation);
            ret = orientation == kPMPortrait ? QPrinter::Portrait : QPrinter::Landscape;
        }
        break;
    case PPK_OutputFileName:
        ret = d->outputFilename;
        break;
    case PPK_PageOrder:
        break;
    case PPK_PaperSource:
        break;
    case PPK_PageRect: {
        QRect r;
        PMRect macrect, macpaper;
        if (PMGetAdjustedPageRect(d->format, &macrect) == noErr
                && PMGetAdjustedPaperRect(d->format, &macpaper) == noErr) {
            r.setCoords(int(macrect.left), int(macrect.top), int(macrect.right), int(macrect.bottom));
            r.translate(int(-macpaper.left), int(-macpaper.top));
        }
        ret = r;
        break; }
    case PPK_PageSize:
        ret = d->pageSize();
        break;
    case PPK_PaperRect: {
        QRect r;
        PMRect macrect;
        if (PMGetAdjustedPaperRect(d->format, &macrect) == noErr) {
            r.setCoords((int)macrect.left, (int)macrect.top, (int)macrect.right, (int)macrect.bottom);
            r.translate(-r.x(), -r.y());
        }
        ret = r;
        break; }
    case PPK_PrinterName: {
        CFIndex currIndex;
        PMPrinter unused;
        QCFType<CFArrayRef> printerList;
        OSStatus status = PMSessionCreatePrinterList(d->session, &printerList, &currIndex, &unused);
        if (status != noErr)
            qWarning("QMacPrintEngine::printerName: Problem getting list of printers %ld", status);
        if (printerList)
            ret = QCFString::toQString(static_cast<CFStringRef>(CFArrayGetValueAtIndex(printerList,
                                                                                       currIndex)));
        break; }
    case PPK_Resolution: // ###
        break;
    case PPK_SupportedResolutions:
        ret = d->supportedResolutions();
        break;
    default:
        break;
    }
    return ret;
}
