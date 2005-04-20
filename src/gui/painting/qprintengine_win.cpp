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

#ifndef QT_NO_PRINTER

#include "qprinter_p.h"
#include "qprintengine_win_p.h"

#include <limits.h>

#include <private/qfont_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qvector.h>

#define QT_DEBUG_DRAW

static const struct {
    int winSizeName;
    QPrinter::PageSize qtSizeName;
} dmMapping[] = {
    { DMPAPER_LETTER,             QPrinter::Letter },
    { DMPAPER_LETTERSMALL,        QPrinter::Letter },
    { DMPAPER_TABLOID,            QPrinter::Tabloid },
    { DMPAPER_LEDGER,             QPrinter::Ledger },
    { DMPAPER_LEGAL,              QPrinter::Legal },
    { DMPAPER_EXECUTIVE,          QPrinter::Executive },
    { DMPAPER_A3,                 QPrinter::A3 },
    { DMPAPER_A4,                 QPrinter::A4 },
    { DMPAPER_A4SMALL,            QPrinter::A4 },
    { DMPAPER_A5,                 QPrinter::A5 },
    { DMPAPER_B4,                 QPrinter::B4 },
    { DMPAPER_B5,                 QPrinter::B5 },
    { DMPAPER_FOLIO,              QPrinter::Folio },
    { DMPAPER_ENV_10,             QPrinter::Comm10E },
    { DMPAPER_ENV_DL,             QPrinter::DLE },
    { DMPAPER_ENV_C3,             QPrinter::C5E },
    { DMPAPER_LETTER_EXTRA,       QPrinter::Letter },
    { DMPAPER_LEGAL_EXTRA,        QPrinter::Legal },
    { DMPAPER_TABLOID_EXTRA,      QPrinter::Tabloid },
    { DMPAPER_A4_EXTRA,           QPrinter::A4},
    { DMPAPER_LETTER_TRANSVERSE,  QPrinter::Letter},
    { DMPAPER_A4_TRANSVERSE,      QPrinter::A4},
    { DMPAPER_LETTER_EXTRA_TRANSVERSE, QPrinter::Letter },
    { DMPAPER_A_PLUS,             QPrinter::A4 },
    { DMPAPER_B_PLUS,             QPrinter::A3 },
    { DMPAPER_LETTER_PLUS,        QPrinter::Letter },
    { DMPAPER_A4_PLUS,            QPrinter::A4 },
    { DMPAPER_A5_TRANSVERSE,      QPrinter::A5 },
    { DMPAPER_B5_TRANSVERSE,      QPrinter::B5 },
    { DMPAPER_A3_EXTRA,           QPrinter::A3 },
    { DMPAPER_A5_EXTRA,           QPrinter::A5 },
    { DMPAPER_B5_EXTRA,           QPrinter::B5 },
    { DMPAPER_A2,                 QPrinter::A2 },
    { DMPAPER_A3_TRANSVERSE,      QPrinter::A3 },
    { DMPAPER_A3_EXTRA_TRANSVERSE,QPrinter::A3 },
    { 0, QPrinter::Custom }
};

static QPrinter::PageSize mapDevmodePageSize(int s)
{
    int i = 0;
    while ((dmMapping[i].winSizeName > 0) && (dmMapping[i].winSizeName != s))
        i++;
    return dmMapping[i].qtSizeName;
}

static int mapPageSizeDevmode(QPrinter::PageSize s)
{
    int i = 0;
 while ((dmMapping[i].winSizeName > 0) && (dmMapping[i].qtSizeName != s))
	i++;
    return dmMapping[i].winSizeName;
}

static struct {
    int winSourceName;
    QPrinter::PaperSource qtSourceName;
}  sources[] = {
    { DMBIN_ONLYONE,        QPrinter::OnlyOne },
    { DMBIN_LOWER,          QPrinter::Lower },
    { DMBIN_MIDDLE,         QPrinter::Middle },
    { DMBIN_MANUAL,         QPrinter::Manual },
    { DMBIN_ENVELOPE,       QPrinter::Envelope },
    { DMBIN_ENVMANUAL,      QPrinter::EnvelopeManual },
    { DMBIN_AUTO,           QPrinter::Auto },
    { DMBIN_TRACTOR,        QPrinter::Tractor },
    { DMBIN_SMALLFMT,       QPrinter::SmallFormat },
    { DMBIN_LARGEFMT,       QPrinter::LargeFormat },
    { DMBIN_LARGECAPACITY,  QPrinter::LargeCapacity },
    { DMBIN_CASSETTE,       QPrinter::Cassette },
    { DMBIN_FORMSOURCE,     QPrinter::FormSource },
    { 0, (QPrinter::PaperSource) -1 }
};

static QPrinter::PaperSource mapDevmodePaperSource(int s)
{
    int i = 0;
    while ((sources[i].winSourceName > 0) && (sources[i].winSourceName != s))
        i++;
    return sources[i].winSourceName ? sources[i].qtSourceName : (QPrinter::PaperSource) s;
}

static int mapPaperSourceDevmode(QPrinter::PaperSource s)
{
    int i = 0;
    while ((sources[i].qtSourceName >= 0) && (sources[i].qtSourceName != s))
        i++;
    return sources[i].winSourceName ? sources[i].winSourceName : s;
}

static BITMAPINFO *getWindowsBITMAPINFO( const QImage &image )
{
    int w, h, depth, ncols=2;

    w = image.width();
    h = image.height();
    depth = image.depth();

    if ( w == 0 || h == 0 || depth == 0 )           // invalid image or pixmap
        return 0;

    if ( depth > 1 && depth <= 8 ) {                    // set to nearest valid depth
        depth = 8;                                  //   2..7 ==> 8
        ncols = 256;
    }
    else if ( depth > 8 ) {
	// some windows printer drivers on 95/98 can't handle 32 bit DIBs,
	// so we have to use 24 bits in that case.
	if ( QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based )
	    depth = 24;
	else
	    depth = 32;
        ncols = 0;
    }

    int   bpl = ((w*depth+31)/32)*4;                // bytes per line
    int   bmi_len = sizeof(BITMAPINFO)+sizeof(RGBQUAD)*ncols;
    char *bmi_data = (char *)malloc( bmi_len );
    memset( bmi_data, 0, bmi_len );
    BITMAPINFO       *bmi = (BITMAPINFO*)bmi_data;
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    bmh->biSize           = sizeof(BITMAPINFOHEADER);
    bmh->biWidth          = w;
    bmh->biHeight	  = h;
    bmh->biPlanes         = 1;
    bmh->biBitCount       = depth;
    bmh->biCompression    = BI_RGB;
    bmh->biSizeImage      = bpl*h;
    bmh->biClrUsed        = ncols;
    bmh->biClrImportant   = 0;

    if ( ncols > 0  && !image.isNull()) {       // image with color map
        RGBQUAD *r = (RGBQUAD*)(bmi_data + sizeof(BITMAPINFOHEADER));
        ncols = qMin(ncols,image.numColors());
        for ( int i=0; i<ncols; i++ ) {
            QColor c = image.color(i);
            r[i].rgbRed = c.red();
            r[i].rgbGreen = c.green();
            r[i].rgbBlue = c.blue();
            r[i].rgbReserved = 0;
        }
    }

    return bmi;
}

QWin32PrintEngine::QWin32PrintEngine(QPrinter::PrinterMode mode)
    : QPaintEngine(*(new QWin32PrintEnginePrivate), PaintEngineFeatures(AllFeatures))
{
    Q_D(QWin32PrintEngine);
    d->docName = "document1";
    d->mode = mode;
    d->queryDefault();
    d->initialize();
}

bool QWin32PrintEngine::begin(QPaintDevice *)
{
    Q_D(QWin32PrintEngine);
    if (d->reinit) {
        d->resetDC();
    }

    // ### set default colors and stuff...

    bool ok = d->state == QPrinter::Idle;
    Q_ASSERT(d->hdc);

    // Assign the FILE: to get the query...
    if (d->fileName.isEmpty())
        d->fileName = d->port;

    QT_WA({
	DOCINFO di;
	memset(&di, 0, sizeof(DOCINFO));
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = reinterpret_cast<const wchar_t *>(d->docName.utf16());
	if (d->printToFile && !d->fileName.isEmpty())
	    di.lpszOutput = reinterpret_cast<const wchar_t *>(d->fileName.utf16());
	if (ok && StartDoc(d->hdc, &di) == SP_ERROR) {
	    qErrnoWarning("QWin32PrintEngine::begin: StartDoc failed");
	    ok = false;
	}
    } , {
	DOCINFOA di;
	memset(&di, 0, sizeof(DOCINFOA));
	di.cbSize = sizeof(DOCINFOA);
	QByteArray docNameA = d->docName.toLocal8Bit();
	di.lpszDocName = docNameA.data();
	QByteArray outfileA = d->fileName.toLocal8Bit();
	if (d->printToFile && !d->fileName.isEmpty())
	    di.lpszOutput = outfileA;
	if (ok && StartDocA(d->hdc, &di) == SP_ERROR) {
	    qErrnoWarning("QWin32PrintEngine::begin: StartDoc failed");
	    ok = false;
        }
    });

    if (QSysInfo::WindowsVersion & Qt::WV_DOS_based) {
	// StartPage resets DC on Win95/98
    }

    if (!ok) {
	if (d->hdc) {
	    DeleteDC(d->hdc);
	    d->hdc = 0;
	}
	d->state = QPrinter::Idle;
    } else {
	d->state = QPrinter::Active;
    }

    d->matrix = QMatrix();
    d->has_pen = true;
    d->pen = QColor(Qt::black);
    d->has_brush = false;

    updateMatrix(d->matrix);

    return ok;
}

bool QWin32PrintEngine::end()
{
    Q_D(QWin32PrintEngine);
    if (d->hdc) {
	if (d->state == QPrinter::Aborted) {
	    AbortDoc(d->hdc);
	} else {
	    EndPage(d->hdc);                 // end; printing done
	    EndDoc(d->hdc);
	}
    }
    d->state = QPrinter::Idle;
    return true;
}

bool QWin32PrintEngine::newPage()
{
    Q_D(QWin32PrintEngine);
    Q_ASSERT(isActive());

    Q_ASSERT(d->hdc);

    bool transparent = GetBkMode(d->hdc) == TRANSPARENT;

    if (!EndPage(d->hdc)) {
        qErrnoWarning("QWin32PrintEngine::newPage: EndPage failed");
        return false;
    }

    if (d->reinit) {
        if (!d->resetDC()) {
            qErrnoWarning("QWin32PrintEngine::newPage: ResetDC failed");
            return false;
        }
    }

    if (!StartPage(d->hdc)) {
        qErrnoWarning("Win32PrintEngine::newPage: StartPage failed");
        return false;
    }

    SetTextAlign(d->hdc, TA_BASELINE);
    if (transparent)
        SetBkMode(d->hdc, TRANSPARENT);

    // ###
    return true;

    bool success = false;
    if (d->hdc && d->state == QPrinter::Active) {
//         bool restorePainter = false;
//         if ((qWinVersion()& Qt::WV_DOS_based) && painter && painter->isActive()) {
//             painter->save();               // EndPage/StartPage ruins the DC
//             restorePainter = true;
//         }
        if (EndPage(d->hdc) != SP_ERROR) {
	    // reinitialize the DC before StartPage if needed,
	    // because resetdc is disabled between calls to the StartPage and EndPage functions
	    // (see StartPage documentation in the Platform SDK:Windows GDI)
// 	    state = PST_ACTIVEDOC;
// 	    reinit();
// 	    state = PST_ACTIVE;
	    // start the new page now
            if (d->reinit) {
                if (!d->resetDC())
                    qErrnoWarning("QWin32PrintEngine::newPage(), ResetDC failed (2)");
            }
	    success = (StartPage(d->hdc) != SP_ERROR);
	}
        if (!success)
	    d->state = QPrinter::Aborted;

//         if (qWinVersion() & Qt::WV_DOS_based)
//         if (restorePainter) {
//             painter->restore();
// 	}

        if (!success)
            return false;
    }
    return true;
}

bool QWin32PrintEngine::abort()
{
    // do nothing loop.
    return false;
}


int QWin32PrintEngine::metric(QPaintDevice::PaintDeviceMetric m) const
{
    Q_D(const QWin32PrintEngine);
    Q_ASSERT(d->hdc);
    int val;
    int res = d->resolution;

    switch (m) {
    case QPaintDevice::PdmWidth:
        val = res
	      * GetDeviceCaps(d->hdc, d->fullPage ? PHYSICALWIDTH : HORZRES)
	      / GetDeviceCaps(d->hdc, LOGPIXELSX);
        break;
    case QPaintDevice::PdmHeight:
        val = res
	      * GetDeviceCaps(d->hdc, d->fullPage ? PHYSICALHEIGHT : VERTRES)
	      / GetDeviceCaps(d->hdc, LOGPIXELSY);
        break;
    case QPaintDevice::PdmDpiX:
        val = res;
        break;
    case QPaintDevice::PdmDpiY:
        val = res;
        break;
    case QPaintDevice::PdmPhysicalDpiX:
        val = GetDeviceCaps(d->hdc, LOGPIXELSX);
        break;
    case QPaintDevice::PdmPhysicalDpiY:
        val = GetDeviceCaps(d->hdc, LOGPIXELSY);
        break;
    case QPaintDevice::PdmWidthMM:
        if (!d->fullPage) {
            val = GetDeviceCaps(d->hdc, HORZSIZE);
        }
        else {
            float wi = 25.4 * GetDeviceCaps(d->hdc, PHYSICALWIDTH);
            val = qRound(wi / GetDeviceCaps(d->hdc,  LOGPIXELSX));
        }
        break;
    case QPaintDevice::PdmHeightMM:
        if (!d->fullPage) {
            val = GetDeviceCaps(d->hdc, VERTSIZE);
        }
        else {
            float hi = 25.4 * GetDeviceCaps(d->hdc, PHYSICALHEIGHT);
            val = qRound(hi / GetDeviceCaps(d->hdc,  LOGPIXELSY));
        }
        break;
    case QPaintDevice::PdmNumColors:
        {
	    int bpp = GetDeviceCaps(d->hdc, BITSPIXEL);
	    if(bpp==32)
		val = INT_MAX;
	    else if(bpp<=8)
		val = GetDeviceCaps(d->hdc, NUMCOLORS);
	    else
		val = 1 << (bpp * GetDeviceCaps(d->hdc, PLANES));
	}
        break;
    case QPaintDevice::PdmDepth:
        val = GetDeviceCaps(d->hdc, PLANES);
        break;
    default:
        qWarning("QPrinter::metric: Invalid metric command");
        return 0;
    }
    return val;
}

void QWin32PrintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QWin32PrintEngine);

    if (state.state() & DirtyTransform) {
        updateMatrix(state.matrix());
    }

    if (state.state() & DirtyPen) {
        d->pen = state.pen();
        d->has_pen = d->pen.style() != Qt::NoPen && d->pen.isSolid() && d->pen.brush().isOpaque();
    }

    if (state.state() & DirtyBrush) {
        QBrush brush = state.brush();
        d->has_brush = brush.style() == Qt::SolidPattern && brush.isOpaque();
        d->brush_color = brush.color();
    }

}

void QWin32PrintEngine::updateMatrix(const QMatrix &matrix)
{
    Q_D(QWin32PrintEngine);

    QMatrix stretch(d->stretch_x, 0, 0, d->stretch_y,
                    d->fullPage ? -d->devPageRect.x() : 0,
                    d->fullPage ? -d->devPageRect.y() : 0);
    d->painterMatrix = matrix;
    d->matrix = d->painterMatrix * stretch;
}

void QWin32PrintEngine::drawPixmap(const QRectF &targetRect,
                                   const QPixmap &originalPixmap,
                                   const QRectF &sr)
{
    return;

//     Q_D(QWin32PrintEngine);
#if defined QT_DEBUG_DRAW
    printf(" - QWin32PrintEngine::drawPixmap(), "
           "[%.2f,%.2f,%.2f,%.2f], "
           "size=[%d,%d], "
           "sr=[%.2f,%.2f,%.2f,%.2f]\n",
           targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height(),
           originalPixmap.width(), originalPixmap.height(),
           sr.x(), sr.y(), sr.width(), sr.height());
#endif

    HBITMAP hbitmap = originalPixmap.toWinHBITMAP();
    HDC hbitmap_hdc = CreateCompatibleDC(qt_win_display_dc());
    HGDIOBJ null_bitmap = SelectObject(hbitmap_hdc, hbitmap);

//     int tx = qRound(targetRect.x() + d->matrix.dx());
//     int ty = qRound(targetRect.y() + d->matrix.dy());
//     int tw = qRound(targetRect.width());
//     int th = qRound(targetRect.height());

//     int sx = qRound(sr.x());
//     int sy = qRound(sr.y());
//     int sw = qRound(sr.width());
//     int sh = qRound(sr.height());

//     const BLENDFUNCTION bf = { AC_SRC_OVER,       // BlendOp
//                                0,                 // BlendFlags, must be zero
//                                255,               // SourceConstantAlpha, we use pr pixel
//                                AC_SRC_ALPHA       // AlphaFormat
//     };

//     if (!qAlphaBlend(d->hdc, tx, ty, tw, th, hbitmap_hdc, sx, sy, sw, sh, bf)) {
//         qWarning("AlphaBlending didn't see too successful...");
//     }

    SelectObject(hbitmap_hdc, null_bitmap);
    DeleteObject(hbitmap);
    DeleteDC(hbitmap_hdc);

//     SelectObject(mask_hdc, null_mask);
//     DeleteObject(mask);
//     DeleteDC(mask_hdc);
}


void QWin32PrintEnginePrivate::fillPath_dev(const QPainterPath &path, const QColor &color)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " --- QWin32PrintEnginePrivate::fillPath() bound:" << path.boundingRect() << color;
#endif

    if (!BeginPath(hdc))
        qErrnoWarning("QWin32PrintEnginePrivate::drawPath: BeginPath failed");

    // Drawing the subpaths
    int start = -1;
    for (int i=0; i<path.elementCount(); ++i) {
        const QPainterPath::Element &elm = path.elementAt(i);
        switch (elm.type) {
        case QPainterPath::MoveToElement:
            if (start >= 0
                && path.elementAt(start).x == path.elementAt(i-1).x
                && path.elementAt(start).y == path.elementAt(i-1).y)
                CloseFigure(hdc);
            start = i;
            MoveToEx(hdc, qRound(elm.x), qRound(elm.y), 0);
            break;
        case QPainterPath::LineToElement:
            LineTo(hdc, qRound(elm.x), qRound(elm.y));
            break;
        case QPainterPath::CurveToElement: {
            POINT pts[3] = {
                { qRound(elm.x), qRound(elm.y) },
                { qRound(path.elementAt(i+1).x), qRound(path.elementAt(i+1).y) },
                { qRound(path.elementAt(i+2).x), qRound(path.elementAt(i+2).y) }
            };
            i+=2;
            PolyBezierTo(hdc, pts, 3);
            break;
        }
        default:
            qFatal("QWin32PaintEngine::drawPath: Unhandled type: %d", elm.type);
        }
    }

    if (start >= 0
        && path.elementAt(start).x == path.elementAt(path.elementCount()-1).x
        && path.elementAt(start).y == path.elementAt(path.elementCount()-1).y)
        CloseFigure(hdc);

    if (!EndPath(hdc))
        qErrnoWarning("QWin32PaintEngine::drawPath: EndPath failed");


    SetPolyFillMode(hdc, path.fillRule() == Qt::WindingFill ? WINDING : ALTERNATE);
    HBRUSH brush = CreateSolidBrush(RGB(color.red(), color.green(), color.blue()));
    HGDIOBJ old_brush = SelectObject(hdc, brush);
    FillPath(hdc);
    DeleteObject(SelectObject(hdc, old_brush));
}


void QWin32PrintEnginePrivate::fillPath(const QPainterPath &path, const QColor &color)
{
    fillPath_dev(path * matrix, color);
}

void QWin32PrintEnginePrivate::strokePath(const QPainterPath &path, const QColor &color)
{
    QPainterPathStroker stroker;
    stroker.setDashPattern(pen.style());
    stroker.setCapStyle(pen.capStyle());
    stroker.setJoinStyle(pen.joinStyle());

    QPainterPath stroke;

    qreal width = pen.widthF();
    if (width == 0) {
        stroker.setWidth(1);
        stroke = stroker.createStroke(path * matrix);
    } else {
        stroker.setWidth(width);
        stroker.setCurveThreshold(1 / (10 * matrix.m11() * matrix.m22()));
        stroke = stroker.createStroke(path) * matrix;
    }

    if (stroke.isEmpty())
        return;

    fillPath_dev(stroke, color);
}


void QWin32PrintEngine::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QWin32PrintEngine::drawPath(), bounds: " << path.boundingRect();
#endif

    Q_D(QWin32PrintEngine);

    if (d->has_brush)
        d->fillPath(path, d->brush_color);

    if (d->has_pen)
        d->strokePath(path, d->pen.color());
}


void QWin32PrintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QWin32PrintEngine::drawPolygon(), pointCount: " << pointCount;
#endif

    Q_ASSERT(pointCount > 1);

    QPainterPath path(points[0]);

    for (int i=1; i<pointCount; ++i) {
        path.lineTo(points[i]);
    }

    Q_D(QWin32PrintEngine);

    bool has_brush = d->has_brush;

    if (mode == PolylineMode)
        d->has_brush = false; // No brush for polylines
    else
        path.closeSubpath(); // polygons are should always be closed.

    drawPath(path);
    d->has_brush = has_brush;
}

void QWin32PrintEnginePrivate::queryDefault()
{
    /* Read the default printer name, driver and port with the intuitive function
     * Strings "windows" and "device" are specified in the MSDN under EnumPrinters()
     */
    QString noPrinters("qt_no_printers");
    QString output;
    QT_WA({
	ushort buffer[256];
	GetProfileStringW(L"windows", L"device",
	                  reinterpret_cast<const wchar_t *>(noPrinters.utf16()),
			  reinterpret_cast<wchar_t *>(buffer), 256);
	output = QString::fromUtf16(buffer);
	if (output == noPrinters) { // no printers
	    qWarning("System has no default printer, are any printers installed?");
	    return;
	}
    }, {
	char buffer[256];
	GetProfileStringA("windows", "device", noPrinters.toLatin1(), buffer, 256);
        output = QString::fromLocal8Bit(buffer);
	if (output == noPrinters) { // no printers
	    qWarning("System has no default printer, are any printers installed?");
	    return;
	}
    });
    QStringList info = output.split(',');
    name = info.at(0);
    program = info.at(1);
    port = info.at(2);
}

void QWin32PrintEnginePrivate::initialize()
{
    if (hdc)
	release();
    Q_ASSERT(!hPrinter);
    Q_ASSERT(!hdc);
    Q_ASSERT(!devMode);
    Q_ASSERT(!pInfo);

    if (name.isEmpty())
        return;

    bool ok;
    QT_WA( {
        ok = OpenPrinterW((LPWSTR)name.utf16(), (LPHANDLE)&hPrinter, 0);
    }, {
        ok = OpenPrinterA((LPSTR)name.toLatin1().data(), (LPHANDLE)&hPrinter, 0);
    } );

    if (!ok) {
	qErrnoWarning("QWin32PrintEngine::initialize: OpenPrinter failed");
	return;
    }

    // Fetch the PRINTER_INFO_2 with DEVMODE data containing the
    // printer settings.
    DWORD infoSize, numBytes;
    QT_WA( {
        GetPrinterW(hPrinter, 2, NULL, 0, &infoSize);
        pInfo = malloc(infoSize);
        if (!GetPrinterW(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes)) {
            qErrnoWarning("QWin32PrintEngine::initialize: GetPrinter failed");
            return;
        }
    }, {
        GetPrinterA(hPrinter, 2, NULL, 0, &infoSize);
        pInfo = malloc(infoSize);
        if (!GetPrinterA(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes)) {
            qErrnoWarning("QWin32PrintEngine::initialize: GetPrinter failed");
            return;
        }
    });

    QT_WA( {
        devMode = pInfoW()->pDevMode;
    }, {
        devMode = pInfoA()->pDevMode;
    } );

    QT_WA( {
        hdc = CreateDC(reinterpret_cast<const wchar_t *>(program.utf16()),
                       reinterpret_cast<const wchar_t *>(name.utf16()), 0, devModeW());
    }, {
        hdc = CreateDCA(program.toLatin1(), name.toLatin1(), 0, devModeA());
    } );

    Q_ASSERT(hPrinter);
    Q_ASSERT(hdc);
    Q_ASSERT(devMode);
    Q_ASSERT(pInfo);

    dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
    dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
    dpi_display = GetDeviceCaps(qt_win_display_dc(), LOGPIXELSY);

    switch(mode) {
    case QPrinter::ScreenResolution:
        resolution = dpi_display;
        stretch_x = dpi_x / double(dpi_display);
        stretch_y = dpi_y / double(dpi_display);
	break;
    case QPrinter::HighResolution:
        resolution = dpi_y;
        stretch_x = 1;
        stretch_y = 1;
	break;
    default:
        break;
    }

    devPaperRect = QRect(0, 0,
                         GetDeviceCaps(hdc, PHYSICALWIDTH),
                         GetDeviceCaps(hdc, PHYSICALHEIGHT));

    devPageRect = QRect(GetDeviceCaps(hdc, PHYSICALOFFSETX),
                        GetDeviceCaps(hdc, PHYSICALOFFSETY),
                        GetDeviceCaps(hdc, HORZRES),
                        GetDeviceCaps(hdc, VERTRES));

    qDebug() << "QWin32PrintEngine::initialize()" << endl
             << " - paperRect" << devPaperRect << endl
             << " - pageRect" << devPageRect << endl
             << " - stretch_x" << stretch_x << endl
             << " - stretch_y" << stretch_y << endl;
}

void QWin32PrintEnginePrivate::release()
{
    Q_ASSERT(hdc);

    if (globalDevMode) { // Devmode comes from print dialog
        GlobalUnlock(globalDevMode);
    } else {            // Devmode comes from initialize...
        // devMode is a part of the same memory block as pInfo so one free is enough...
        free(pInfo);
        ClosePrinter(hPrinter);
    }
    DeleteDC(hdc);

    hdc = 0;
    hPrinter = 0;
    pInfo = 0;
    devMode = 0;
}

QList<QVariant> QWin32PrintEnginePrivate::queryResolutions() const
{
    // Read the supported resolutions of the printer.
    DWORD numRes;
    LONG *enumRes;
    DWORD errRes;

    QT_WA({
	numRes = DeviceCapabilities(reinterpret_cast<const wchar_t *>(name.utf16()),
	                            reinterpret_cast<const wchar_t *>(port.utf16()),
				    DC_ENUMRESOLUTIONS, 0, 0);
	enumRes = (LONG*)malloc(numRes * 2 * sizeof(LONG));
        errRes = DeviceCapabilities(reinterpret_cast<const wchar_t *>(name.utf16()),
	                            reinterpret_cast<const wchar_t *>(port.utf16()),
				    DC_ENUMRESOLUTIONS, (LPWSTR)enumRes, 0);
    }, {
	numRes = DeviceCapabilitiesA(name.toLocal8Bit(), port.toLocal8Bit(), DC_ENUMRESOLUTIONS, 0, 0);
	enumRes = (LONG*)malloc(numRes * 2 * sizeof(LONG));
	errRes = DeviceCapabilitiesA(name.toLocal8Bit(), port.toLocal8Bit(), DC_ENUMRESOLUTIONS, (LPSTR)enumRes, 0);
    });

    QList<QVariant> list;
    if (errRes == (DWORD)-1) {
        qErrnoWarning("QWin32PrintEngine::queryResolutions: DeviceCapabilities failed");
        return list;
    }

    for (uint i=0; i<numRes; ++i)
	list.append(int(enumRes[i*2]));
    return list;
}

void QWin32PrintEnginePrivate::doReinit()
{
    if (state == QPrinter::Active)
        reinit = true;
    else
        resetDC();
}

void QWin32PrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QWin32PrintEngine);
    switch (key) {
    case PPK_CollateCopies:
        d->doReinit();
        break;

    case PPK_ColorMode:
        {
            if (!d->devMode)
                break;
            int cm = value.toInt() == QPrinter::Color ? DMCOLOR_COLOR : DMCOLOR_MONOCHROME;
            QT_WA( { d->devModeW()->dmColor = cm; }, { d->devModeA()->dmColor = cm; } );
            d->doReinit();
        }
        break;

    case PPK_Creator:

        break;

    case PPK_DocumentName:
        if (isActive()) {
            qWarning("Cannot change document name while printing is active");
            return;
        }
        d->docName = value.toString();
        break;

    case PPK_FullPage:
        d->fullPage = value.toBool();
        break;

    case PPK_NumberOfCopies:
        if (!d->devMode)
            break;
        QT_WA( {
            d->devModeW()->dmCopies = value.toInt();
        }, {
            d->devModeA()->dmCopies = value.toInt();
        } );
        break;

    case PPK_Orientation:
        {
            if (!d->devMode)
                break;
            int o = value.toInt() == QPrinter::Landscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
            QT_WA( { d->devModeW()->dmOrientation = o; }, { d->devModeA()->dmOrientation = o; } );
            d->doReinit();
        }
        break;

    case PPK_OutputFileName:
        if (isActive()) {
            qWarning("QWin32PrintEngine: cannot change filename while printing");
        } else {
            d->fileName = value.toString();
            d->printToFile = !value.toString().isEmpty();
        }
        break;

    case PPK_PageSize:
        if (!d->devMode)
            break;
        QT_WA( {
            d->devModeW()->dmPaperSize = mapPageSizeDevmode(QPrinter::PageSize(value.toInt()));
        }, {
            d->devModeA()->dmPaperSize = mapPageSizeDevmode(QPrinter::PageSize(value.toInt()));
        } );
        d->doReinit();
        break;

    case PPK_PaperSource:
        {
            if (!d->devMode)
                break;
            int dmMapped = DMBIN_AUTO;

            DWORD caps;
            QT_WA( {
                caps = DeviceCapabilitiesW((TCHAR*)d->name.utf16(), 0, DC_BINS, 0, 0);
            }, {
                caps = DeviceCapabilitiesA(d->name.toLatin1(), 0, DC_BINS, 0, 0);
            } );
            // Skip it altogether if it's not supported...
            if (caps != DWORD(-1) && caps != 0) {
                WORD *bins = new WORD[caps];
                bool gotCaps;
                QT_WA( {
                    gotCaps = DeviceCapabilitiesW((wchar_t *)d->name.utf16(), 0, DC_BINS, (wchar_t *) bins, 0);
                }, {
                    gotCaps = DeviceCapabilitiesA(d->name.toLatin1(), 0, DC_BINS, (char*) bins, 0);
                } );
                if (gotCaps) {
                    bool ok = false;
                    int source = mapPaperSourceDevmode(QPrinter::PaperSource(value.toInt()));
                    for (DWORD i=0; i<caps; i++)
                        ok |= (bins[i] == (WORD)source);
                    if (ok)
                        dmMapped = source;
                }
                delete [] bins;
            }
            QT_WA( {
                d->devModeW()->dmDefaultSource = dmMapped;
            }, {
                d->devModeA()->dmDefaultSource = dmMapped;
            } );
            d->doReinit();
        }
        break;

    case PPK_PrinterName:
        d->name = value.toString();
        d->initialize();
        break;

    case PPK_Resolution:
        d->resolution = value.toInt();
        d->stretch_x = d->resolution / double(d->dpi_display);
        d->stretch_y = d->resolution / double(d->dpi_display);
        // ### matrix update?
        break;

    case PPK_SelectionOption:

        break;

    case PPK_SupportedResolutions:

        break;


    case PPK_WindowsPageSize:
        if (!d->devMode)
            break;
        QT_WA( {
            d->devModeW()->dmPaperSize = value.toInt();
        }, {
            d->devModeA()->dmPaperSize = value.toInt();
        } );
        d->doReinit();
        break;

    default:
        // Do nothing
        break;
    }
}

QVariant QWin32PrintEngine::property(PrintEnginePropertyKey key) const
{
    Q_D(const QWin32PrintEngine);
    QVariant value;
    switch (key) {

    case PPK_CollateCopies:
        value = false;
        break;

    case PPK_ColorMode:
        {
            if (!d->devMode)
                value = QPrinter::GrayScale;
            int mode;
            QT_WA( {
                mode = d->devModeW()->dmColor;
            }, {
                mode = d->devModeA()->dmColor;
            } );
            value = mode == DMCOLOR_COLOR ? QPrinter::Color : QPrinter::GrayScale;
        }
        break;

    case PPK_DocumentName:
        value = d->docName;
        break;

    case PPK_FullPage:
        value = d->fullPage;
        break;

    case PPK_NumberOfCopies:
        value = 1;
        break;

    case PPK_Orientation:
        {
            if (!d->devMode)
                value = QPrinter::Portrait;
            int o;
            QT_WA( { o = d->devModeW()->dmOrientation; }, { o = d->devModeA()->dmOrientation; } );
            value = o == DMORIENT_LANDSCAPE ? QPrinter::Landscape : QPrinter::Portrait;
        }
        break;

    case PPK_OutputFileName:
        value = d->fileName;
        break;

    case PPK_PageRect:
        {
            value = QMatrix(1/d->stretch_x, 0, 0, 1/d->stretch_y, 0, 0).mapRect(d->devPageRect);
        }
        break;

    case PPK_PageSize:
        if (!d->devMode)
            value = QPrinter::A4;
        QT_WA( {
            value = mapDevmodePageSize(d->devModeW()->dmPaperSize);
        }, {
            value = mapDevmodePageSize(d->devModeA()->dmPaperSize);
        } );
        break;

    case PPK_PaperRect:
        {
            value = QMatrix(1/d->stretch_x, 0, 0, 1/d->stretch_y, 0, 0).mapRect(d->devPaperRect);
        }
        break;

    case PPK_PaperSource:
        if (!d->devMode)
            value = QPrinter::Auto;
        QT_WA( {
            value = mapDevmodePaperSource(d->devModeW()->dmDefaultSource);
        }, {
            value = mapDevmodePaperSource(d->devModeA()->dmDefaultSource);
        } );
        break;

    case PPK_PrinterName:
        value = d->name;
        break;

    case PPK_Resolution:
        value = d->resolution;
        break;

    case PPK_SupportedResolutions:
        value = d->queryResolutions();
        break;

    case PPK_WindowsPageSize:
        if (!d->devMode)
            value = -1;
        QT_WA( {
            value = d->devModeW()->dmPaperSize;
        }, {
            value = d->devModeA()->dmPaperSize;
        } );
        break;

    default:
        // Do nothing
        break;
    }
    return value;
}

QPrinter::PrinterState QWin32PrintEngine::printerState() const
{
    return d_func()->state;
}

HDC QWin32PrintEngine::getDC() const
{
    return d_func()->hdc;
}

void QWin32PrintEngine::releaseDC(HDC) const
{

}

HGLOBAL *QWin32PrintEnginePrivate::createDevNames()
{
    QT_WA( {
        int size = sizeof(DEVNAMES)
                   + program.length() * 2 + 2
                   + name.length() * 2 + 2
                   + port.length() * 2 + 2;
        HGLOBAL *hGlobal = (HGLOBAL *) GlobalAlloc(GMEM_MOVEABLE, size);
        DEVNAMES *dn = (DEVNAMES*) GlobalLock(hGlobal);

        dn->wDriverOffset = sizeof(DEVNAMES) / sizeof(TCHAR);
        dn->wDeviceOffset = dn->wDriverOffset + program.length() + 1;
        dn->wOutputOffset = dn->wDeviceOffset + name.length() + 1;

        memcpy((ushort*)dn + dn->wDriverOffset, program.utf16(), program.length() * 2 + 2);
        memcpy((ushort*)dn + dn->wDeviceOffset, name.utf16(), name.length() * 2 + 2);
        memcpy((ushort*)dn + dn->wOutputOffset, port.utf16(), port.length() * 2 + 2);
        dn->wDefault = 0;

        GlobalUnlock(hGlobal);

//         printf("QPrintDialogWinPrivate::createDevNames()\n"
//                " -> wDriverOffset: %d\n"
//                " -> wDeviceOffset: %d\n"
//                " -> wOutputOffset: %d\n",
//                dn->wDriverOffset,
//                dn->wDeviceOffset,
//                dn->wOutputOffset);

//         printf("QPrintDialogWinPrivate::createDevNames(): %s, %s, %s\n",
//                QString::fromUtf16((ushort*)(dn) + dn->wDriverOffset).latin1(),
//                QString::fromUtf16((ushort*)(dn) + dn->wDeviceOffset).latin1(),
//                QString::fromUtf16((ushort*)(dn) + dn->wOutputOffset).latin1());

        return hGlobal;
    }, {
        int size = sizeof(DEVNAMES)
                   + program.length() + 2
                   + name.length() + 2
                   + port.length() + 2;
        HGLOBAL *hGlobal = (HGLOBAL *) GlobalAlloc(GMEM_MOVEABLE, size);
        DEVNAMES *dn = (DEVNAMES*) GlobalLock(hGlobal);

        dn->wDriverOffset = sizeof(DEVNAMES);
        dn->wDeviceOffset = dn->wDriverOffset + program.length() + 1;
        dn->wOutputOffset = dn->wDeviceOffset + name.length() + 1;

        memcpy((char*)dn + dn->wDriverOffset, program.toLatin1(), program.length() + 2);
        memcpy((char*)dn + dn->wDeviceOffset, name.toLatin1(), name.length() + 2);
        memcpy((char*)dn + dn->wOutputOffset, port.toLatin1(), port.length() + 2);
        dn->wDefault = 0;

        GlobalUnlock(hGlobal);
        return hGlobal;
    } );
}

void QWin32PrintEnginePrivate::readDevnames(HGLOBAL globalDevnames)
{
    if (globalDevnames) {
        QT_WA( {
            DEVNAMES *dn = (DEVNAMES*) GlobalLock(globalDevnames);
            name = QString::fromUtf16((ushort*)(dn) + dn->wDeviceOffset);
            port = QString::fromUtf16((ushort*)(dn) + dn->wOutputOffset);
            program = QString::fromUtf16((ushort*)(dn) + dn->wDriverOffset);
            GlobalUnlock(globalDevnames);
        }, {
            DEVNAMES *dn = (DEVNAMES*) GlobalLock(globalDevnames);
            name = QString::fromLatin1((char*)(dn) + dn->wDeviceOffset);
            port = QString::fromLatin1((char*)(dn) + dn->wOutputOffset);
            program = QString::fromLatin1((char*)(dn) + dn->wDriverOffset);
            GlobalUnlock(globalDevnames);
        } );
    }
}

void QWin32PrintEnginePrivate::readDevmode(HGLOBAL globalDevmode)
{
    if (globalDevmode) {
        QT_WA( {
            DEVMODE *dm = (DEVMODE*) GlobalLock(globalDevmode);
            release();
            globalDevMode = globalDevmode;
            devMode = dm;
            hdc = CreateDC(reinterpret_cast<const wchar_t *>(program.utf16()),
                           reinterpret_cast<const wchar_t *>(name.utf16()), 0, dm);
        }, {
            DEVMODEA *dm = (DEVMODEA*) GlobalLock(globalDevmode);
            release();
            globalDevMode = globalDevmode;
            devMode = dm;
            hdc = CreateDCA(program.toLatin1(), name.toLatin1(), 0, dm);
        } );
    }
}

#endif // QT_NO_PRINTER
