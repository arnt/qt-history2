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
    setActive(true);
    assignf(IsActive | DirtyFont);
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
    d->orient = orientation;
    PMOrientation o = orientation == QPrinter::Portrait ? kPMPortrait : kPMLandscape;
    PMSetOrientation(d->format, o, false);
}

QPrinter::Orientation QMacPrintEngine::orientation() const
{
    if (d->state == QPrinter::Idle)
        return d->orient;

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
                // reset the orientation and resolution as they are lost in the copy.
                setOrientation(d->orient);
                PMSetResolution(d->format, &d->resolution);
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
        r.translate(-r.x(), -r.y());
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
        r.translate(int(-macpaper.left), int(-macpaper.top));
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
        if (d->state == QPrinter::Active || orientation() == QPrinter::Portrait)
            val = qt_get_PDMWidth(d->format, fullPage());
        else
            val = qt_get_PDMHeight(d->format, fullPage());
        break;
    case QPaintDevice::PdmHeight:
        if (d->state == QPrinter::Active || orientation() == QPrinter::Portrait)
            val = qt_get_PDMHeight(d->format, fullPage());
        else
            val = qt_get_PDMWidth(d->format, fullPage());
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

    QRect page = q->pageRect();
    QRect paper = q->paperRect();
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

void QMacPrintEngine::updatePen(const QPen &pen)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updatePen(pen);
}

void QMacPrintEngine::updateBrush(const QBrush &brush, const QPointF &pt)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateBrush(brush, pt);
}

void QMacPrintEngine::updateInternal(QPainterState *newState, bool updateGC)
{
    d->paintEngine->state = state; // ### QPainter changes my state without me knowing in begin().
    d->paintEngine->updateInternal(newState, updateGC);
    QPaintEngine::updateInternal(newState, updateGC);
}

void QMacPrintEngine::updateFont(const QFont &font)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateFont(font);
}

void QMacPrintEngine::updateBackground(Qt::BGMode bgmode, const QBrush &bgBrush)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateBackground(bgmode, bgBrush);
}

void QMacPrintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateMatrix(matrix);
}

void QMacPrintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateClipRegion(region, op);
}

void QMacPrintEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateClipPath(path, op);
}

void QMacPrintEngine::updateRenderHints(QPainter::RenderHints hints)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->updateRenderHints(hints);
}

void QMacPrintEngine::drawLine(const QLineF &line)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawLine(line);
}

void QMacPrintEngine::drawRect(const QRectF &r)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawRect(r);
}

void QMacPrintEngine::drawPoint(const QPointF &p)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPoint(p);
}

void QMacPrintEngine::drawPoints(const QPolygonF &p)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPoints(p);
}

void QMacPrintEngine::drawEllipse(const QRectF &r)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawEllipse(r);
}

void QMacPrintEngine::drawLines(const QList<QLineF> &lines)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawLines(lines);
}

void QMacPrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPolygon(points, pointCount, mode);
}

void QMacPrintEngine::drawPixmap(const QRectF &r, const QPixmap &pm,
                                 const QRectF &sr, Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPixmap(r, pm, sr, mode);
}

void QMacPrintEngine::drawTextItem(const QPointF &p, const QTextItem &ti)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawTextItem(p, ti);
}

void QMacPrintEngine::drawTiledPixmap(const QRectF &dr, const QPixmap &pixmap,
                                      const QPointF &sr, Qt::PixmapDrawingMode mode)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawTiledPixmap(dr, pixmap, sr, mode);
}

void QMacPrintEngine::drawPath(const QPainterPath &path)
{
    Q_ASSERT(d->state == QPrinter::Active);
    d->paintEngine->drawPath(path);
}

QPainter::RenderHints QMacPrintEngine::supportedRenderHints() const
{
    Q_ASSERT(d->state == QPrinter::Active);
    return d->paintEngine->supportedRenderHints();
}



