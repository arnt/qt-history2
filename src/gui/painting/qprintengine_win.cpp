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

#include <private/qfontdata_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qvector.h>

#define d d_func()
#define q q_func()

// #define QT_DEBUG_DRAW

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
    : QWin32PaintEngine(*(new QWin32PrintEnginePrivate), PaintEngineFeatures(PixmapTransform
                                                                             | PixmapScale
                                                                             | UsesFontEngine
                                                                             | LinearGradients
                                                                             | AlphaFill
                                                                             | PainterPaths))
{
    d->docName = "document1";
    d->mode = mode;
    d->queryDefault();
    d->initialize();
}


void QWin32PrintEngine::updateClipRegion(const QRegion &clipRegion, bool clipEnabled)
{
    if (clipEnabled) {
	double xscale = ((float)metric(QPaintDevice::PdmPhysicalDpiX)) /
			((float)metric(QPaintDevice::PdmDpiX));
	double yscale = ((float)metric(QPaintDevice::PdmPhysicalDpiY)) /
			((float)metric(QPaintDevice::PdmDpiY));
	double xoff = 0;
	double yoff = 0;
	if (d->fullPage) {	// must adjust for margins
            xoff = - GetDeviceCaps(d->hdc, PHYSICALOFFSETX);
            yoff = - GetDeviceCaps(d->hdc, PHYSICALOFFSETY);
	}
	QRegion rgn = clipRegion * QMatrix(xscale, 0, 0, yscale, xoff, yoff);
	if (rgn.isEmpty())
            rgn = QRect(-0x1000000, -0x1000000, 1, 1);
        SelectClipRgn(d->hdc, rgn.handle());
    } else {
	SelectClipRgn(d->hdc, 0);
    }
}

bool QWin32PrintEngine::begin(QPaintDevice *dev)
{
    if (d->reinit)
        d->hdc = ResetDC(d->hdc, d->devMode);

    if (!QWin32PaintEngine::begin(dev)) {
	qWarning("QWin32PaintEngine::begin() failed...");
	return false;
    }

    d->forceGdi = true;

    bool ok = d->state == QPrinter::Idle;
//     if (ok && !d->hdc) {
// 	setup(0);
// 	if (!hdc)
// 	    ok = false;
//     }
    Q_ASSERT(d->hdc);

    // Assign the FILE: to get the query...
    if (d->fileName.isEmpty())
        d->fileName = d->port;

//     QT_WA({
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
//     } , {
// 	DOCINFOA di;
// 	memset(&di, 0, sizeof(DOCINFOA));
// 	di.cbSize = sizeof(DOCINFOA);
// 	QByteArray docNameA = doc_name.toLocal8Bit();
// 	di.lpszDocName = docNameA.data();
// 	QByteArray outfileA = output_filename.toLocal8Bit();
// 	if (output_file && !output_filename.isEmpty())
// 	    di.lpszOutput = outfileA.data();
// 	if (ok && StartDocA(hdc, &di) == SP_ERROR)
// 	    ok = false;
//     });

//     if (ok) {
// 	reinit(); // initialize latest changes before StartPage
        ok = StartPage(d->hdc) != SP_ERROR;
        d->setupPrinterMapping();

//     }
//     if (qWinVersion() & Qt::WV_DOS_based)
// 	// StartPage resets DC on Win95/98
        //d->setupPrinterMapping();

        //d->setupOriginMapping();

//     if (!ok) {
// 	if (hdc) {
// 	    DeleteDC(hdc);
// 	    hdc = 0;
// 	}
// 	state = PST_IDLE;
// 	return false;
//     } else {
	d->state = QPrinter::Active;
// 	painter = paint;
//     }
    return true;

}

bool QWin32PrintEngine::end()
{
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
    Q_ASSERT(isActive());

    Q_ASSERT(d->hdc);

    if (!EndPage(d->hdc)) {
        qErrnoWarning("QWin32PrintEngine::newPage: EndPage failed");
        return false;
    }

    if (d->reinit) {
        if (!(d->hdc = ResetDC(d->hdc, d->devMode))) {
            qErrnoWarning("QWin32PrintEngine::newPage: ResetDC failed");
            return false;
        }
    }

    if (!StartPage(d->hdc)) {
        qErrnoWarning("Win32PrintEngine::newPage: StartPage failed");
        return false;
    }

    SetTextAlign(d->hdc, TA_BASELINE);
    d->setupPrinterMapping();
    d->setupOriginMapping();

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
            if (d->reinit)
                ResetDC(d->hdc, d->devMode);
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
        //d->setupPrinterMapping();
        //d->setupOriginMapping();
        //SetTextAlign(d->hdc, TA_BASELINE);
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

void QWin32PrintEngine::drawPixmap(const QRectF &targetRect,
                                   const QPixmap &originalPixmap,
                                   const QRectF &sr,
                                   Qt::PixmapDrawingMode mode)
{
#if defined QT_DEBUG_DRAW
    printf(" - QWin32PrintEngine::drawPixmap(), [%.2f,%.2f,%.2f,%.2f], size=[%d,%d], "
           "sr=[%.2f,%.2f,%.2f,%.2f], mode=%d\n",
           targetRect.x(), targetRect.y(), targetRect.width(), targetRect.height(),
           originalPixmap.width(), originalPixmap.height(),
           sr.x(), sr.y(), sr.width(), sr.height(),
           mode);
#endif

    QPixmap pixmap = originalPixmap;
    if (sr.x()!=0 || sr.y() != 0 || sr.size() != originalPixmap.size()) {
        QPixmap newPixmap(sr.size().toSize());
        QPainter p(&newPixmap);
        p.drawPixmap(QPointF(0, 0), originalPixmap, sr, Qt::CopyPixmap);
        p.end();
        pixmap = newPixmap;
    }

    if (mode != Qt::ComposePixmap) {
        pixmap.setMask(QBitmap());
    }

    // Turn of native transformations...
    QRectF rect(targetRect);
    QPainter *paint = painter();
    QPointF pos( rect.x(), rect.y() );
    QImage  image = pixmap.toImage();

    float w = pixmap.width();
    float h = pixmap.height();

    if ( pixmap.isQBitmap() ) {
        QColor bg = paint->background().color();
        QColor fg = paint->pen().color();
        image.convertDepth( 8 );
        if( image.color( 0 ) == QColor(Qt::black).rgb() &&
            image.color( 1 ) == QColor(Qt::white).rgb() ) {
            image.setColor( 1, bg.rgb() );
            image.setColor( 0, fg.rgb() );
        } else {
            image.setColor( 0, bg.rgb() );
            image.setColor( 1, fg.rgb() );
        }
    }

    double xs = 1.0;                    // x stretch
    double ys = 1.0;                    // y stretch
    if ( paint ) {
        bool wxf = paint->matrixEnabled();
        bool vxf = paint->viewTransformEnabled();
#ifndef QT_NO_IMAGE_TRANSFORMATION
        bool complexWxf = false;
#endif
        if ( wxf ) {
            QMatrix m = paint->matrix();
#ifndef QT_NO_IMAGE_TRANSFORMATION
            complexWxf = m.m12() != 0 || m.m21() != 0;
            if ( complexWxf ) {
                image.setAlphaBuffer( true );

                // When have to scale the image according to the rectangle before
                // the rotation takes place to avoid shearing the image.
                if (rect.width() != image.width() || rect.height() != image.height()) {
                    m = QMatrix( rect.width()/(double)image.width(), 0,
                                  0, rect.height()/(double)image.height(),
                                  0, 0 ) * m;
                }

                int origW = image.width();
                int origH = image.height();
                image = image.transform( m );
                w = image.width();
                h = image.height();
                rect.setWidth(w);
                rect.setHeight(h);

                // The image is already transformed. For the transformation
                // of pos, we need a modified world matrix:
                //   Let M be the original world matrix and T its true
                //   matrix of image transformation. The resulting new
                //   world matrix we are looking for has only the
                //   translation
                //     v = pos' - pos
                //       = M*pos - T*0 - pos
                //   whith pos' being the desired upper left corner of the
                //   transformed image.
                paint->save();
                QPointF p1 = QPointF(0,0) * QPixmap::trueMatrix( m, origW, origH );
                QPointF p2 = pos * paint->matrix();
                p1 = p2 - p1 - pos;
                paint->setMatrix( QMatrix( 1, 0, 0, 1, p1.x(), p1.y() ) );
            } else
#endif
                {
                    xs = m.m11();
                    ys = m.m22();
                }
        }
        if ( vxf ) {
            QRect vr = paint->viewport();
            QRect wr = paint->window();
            xs = xs * vr.width() / wr.width();
            ys = ys * vr.height() / wr.height();
        }
        if ( wxf || vxf ) {             // map position
            pos = pos * paint->matrix();
        }
#ifndef QT_NO_IMAGE_TRANSFORMATION
        if ( complexWxf )
            paint->restore();
#endif
    }

    float dw = xs * rect.width();
    float dh = ys * rect.height();
    BITMAPINFO *bmi = getWindowsBITMAPINFO( image );
    BITMAPINFOHEADER *bmh = (BITMAPINFOHEADER*)bmi;
    uchar *bits;

    QRegion oldClip;

    // Since we scale the image in the StretchXXX below, we scale the
    // bitmask to make the transparency clip region correct.
    if ( paint && image.hasAlphaBuffer() ) {
        QImage mask = image.createAlphaMask();
        QBitmap bm;
        bm = mask;
        xs = dw/(double)image.width();
        ys = dh/(double)image.height();
        if( xs!=1 || ys!=1 )
            bm = bm.transform( QMatrix( xs, 0, 0, ys, 0, 0 ) );
        QRegion r( bm );
        r.translate((int)pos.x(), (int)pos.y() );
        if ( paint->hasClipping() )
            r &= paint->clipRegion();
        paint->save();
        oldClip = paint->clipRegion();
        updateClipRegion(r, true);
    }

    bits = new uchar[bmh->biSizeImage];
    if ( bmh->biBitCount == 24 ) {
        int width = image.width();
        uchar *b = bits;
        uint lineFill = (3*width+3)/4*4 - 3*width;
        for( int y=image.height()-1; y >= 0 ; y-- ) {
            QRgb *s = (QRgb*)(image.scanLine( y ));
            for( int x=0; x < width; x++ ) {
                *b++ = qBlue( *s );
                *b++ = qGreen( *s );
                *b++ = qRed( *s );
                s++;
            }
            b += lineFill;
        }

    } else {
        uchar *b = bits;
        int w = (image.width()*image.depth() + 7)/8;
        int incr = w + ( (4-w) & 3 );
        for( int y=image.height()-1; y >= 0 ; y-- ) {
            memcpy( b, image.scanLine( y ), w );
            b += incr;
        }
    }

    int rc = GetDeviceCaps(d->hdc,RASTERCAPS);
    if ( (rc & RC_STRETCHDIB) != 0 ) {
        // StretchDIBits supported
        StretchDIBits( d->hdc,
                       qRound(pos.x()), qRound(pos.y()), qRound(dw), qRound(dh),
                       0, 0, qRound(w), qRound(h),
                       bits, bmi, DIB_RGB_COLORS, SRCCOPY );
    } else if ( (rc & RC_STRETCHBLT) != 0 ) {
        // StretchBlt supported
        HDC     hdcPrn = CreateCompatibleDC( d->hdc );
        HBITMAP hbm    = CreateDIBitmap( d->hdc, bmh, CBM_INIT,
                                         bits, bmi, DIB_RGB_COLORS );
        HBITMAP oldHbm = (HBITMAP)SelectObject( hdcPrn, hbm );
        StretchBlt( d->hdc, qRound(pos.x()), qRound(pos.y()), qRound(dw), qRound(dh),
                    hdcPrn, qRound(0), qRound(0), qRound(w), qRound(h),
                    SRCCOPY );
        SelectObject( hdcPrn, oldHbm );
        DeleteObject( hbm );
        DeleteObject( hdcPrn );
    }
    delete [] bits;
    free( bmi );

    if ( paint && image.hasAlphaBuffer() ) {
        updateClipRegion(oldClip, !oldClip.isEmpty());
        paint->restore();
    }
}

void QWin32PrintEnginePrivate::setupOriginMapping()
{
    if (fullPage) {
	POINT p;
	GetViewportOrgEx(hdc, &p);
	OffsetViewportOrgEx(hdc,
			    -p.x - GetDeviceCaps(hdc, PHYSICALOFFSETX),
			    -p.y - GetDeviceCaps(hdc, PHYSICALOFFSETY),
			    0);
    } else {
	POINT p;
	GetViewportOrgEx(hdc, &p);
	OffsetViewportOrgEx(hdc, -p.x, -p.y, 0);
    }
}

void QWin32PrintEnginePrivate::setupPrinterMapping()
{
    int mapMode = MM_ANISOTROPIC;
    if (GetDeviceCaps(hdc, LOGPIXELSX) == GetDeviceCaps(hdc, LOGPIXELSY))
	mapMode = MM_ISOTROPIC;

    int result;

    result = SetMapMode(hdc, mapMode);
    Q_ASSERT(result);

    // The following two lines are the cause of problems on Windows 9x,
    // for some reason, either one of these functions or both don't
    // have an effect.  This appears to be a bug with the Windows API
    // and as of yet I can't find a workaround.
    result = SetWindowExtEx(hdc, resolution, resolution, 0);
    Q_ASSERT(result);

    result = SetViewportExtEx(hdc, GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSY), 0);
    Q_ASSERT(result);
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
	GetProfileStringA("windows", "device", noPrinters.latin1(), buffer, 256);
        output = QString::fromLocal8Bit(buffer);
	if (output == noPrinters) { // no printers
	    qWarning("System has no default printer, are any printers installed?");
	    return;
	}
    });
    QStringList info = output.split(',');
    d->name = info.at(0);
    d->program = info.at(1);
    d->port = info.at(2);
}

void QWin32PrintEnginePrivate::initialize()
{
    if (hdc)
	release();
    Q_ASSERT(!hPrinter);
    Q_ASSERT(!hdc);
    Q_ASSERT(!devMode);
    Q_ASSERT(!pInfo);

    if(!OpenPrinterW((LPWSTR)name.utf16(), (LPHANDLE)&hPrinter, 0)) {
	qErrnoWarning("QWin32PrintEngine::initialize: OpenPrinter failed");
	return;
    }

    // Fetch the PRINTER_INFO_2 with DEVMODE data containing the
    // printer settings.
    DWORD infoSize, numBytes;
    GetPrinter(d->hPrinter, 2, NULL, 0, &infoSize);
    pInfo = (PRINTER_INFO_2 *)malloc(infoSize);
    if (!GetPrinter(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes)) {
	qErrnoWarning("QWin32PrintEngine::initialize: GetPrinter failed");
	return;
    }

    devMode = pInfo->pDevMode;

    hdc = CreateDC(reinterpret_cast<const wchar_t *>(program.utf16()),
		   reinterpret_cast<const wchar_t *>(name.utf16()), 0, devMode);

    switch(mode) {
    case QPrinter::ScreenResolution:
	d->resolution = GetDeviceCaps(qt_display_dc(), LOGPIXELSY);
	break;
    case QPrinter::HighResolution:
	d->resolution = GetDeviceCaps(hdc, LOGPIXELSY);
	break;
    default:
        break;
    }
    setupPrinterMapping();
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

QList<int> QWin32PrintEnginePrivate::queryResolutions() const
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
	numRes = DeviceCapabilitiesA(name.local8Bit(), port.local8Bit(), DC_ENUMRESOLUTIONS, 0, 0);
	enumRes = (LONG*)malloc(numRes * 2 * sizeof(LONG));
	errRes = DeviceCapabilitiesA(name.local8Bit(), port.local8Bit(), DC_ENUMRESOLUTIONS, (LPSTR)enumRes, 0);
    });

    QList<int> list;
    if (errRes == (DWORD)-1) {
        qErrnoWarning("QWin32PrintEngine::queryResolutions: DeviceCapabilities failed");
        return list;
    }

    for (uint i=0; i<numRes; ++i)
	list.append(enumRes[i*2]);
    return list;
}

void QWin32PrintEnginePrivate::doReinit()
{
    if (state == QPrinter::Active)
        reinit = true;
    else
        hdc = ResetDC(hdc, devMode);
}

void QWin32PrintEngine::setPrinterName(const QString &name)
{
    d->name = name;
    d->initialize();
}

QString QWin32PrintEngine::printerName() const
{
    return d->name;
}

void QWin32PrintEngine::setOutputToFile(bool toFile)
{
    d->printToFile = toFile;
}

bool QWin32PrintEngine::outputToFile() const
{
    return d->printToFile;
}

void QWin32PrintEngine::setOutputFileName(const QString &name)
{
    if (isActive()) {
        qWarning("QWin32PrintEngine::setOutputFileName() cannot change filename while printing");
        return;
    }
    d->fileName = name;
}

QString QWin32PrintEngine::outputFileName()const
{
    return d->fileName;
}

void QWin32PrintEngine::setPrintProgram(const QString &)
{
    // Windows always uses the spooler so there is no point in
    // trying to set this.
}

QString QWin32PrintEngine::printProgram() const
{
    return d->program;
}

void QWin32PrintEngine::setDocName(const QString &name)
{
    if (isActive()) {
        qWarning("Cannot change document name while printing is active");
        return;
    }
    d->docName = name;
}

QString QWin32PrintEngine::docName() const
{
    return d->docName;
}

void QWin32PrintEngine::setCreator(const QString &)
{
    // Does not make sense on windows.
}

QString QWin32PrintEngine::creator() const
{
    return QString();
}

void QWin32PrintEngine::setOrientation(QPrinter::Orientation orient)
{
    Q_ASSERT(d->devMode);
    d->devMode->dmOrientation = orient == QPrinter::Landscape
        ? DMORIENT_LANDSCAPE
        : DMORIENT_PORTRAIT;
    d->doReinit();
}

QPrinter::Orientation QWin32PrintEngine::orientation() const
{
    return d->devMode->dmOrientation == DMORIENT_LANDSCAPE
        ? QPrinter::Landscape
        : QPrinter::Portrait;
}

void QWin32PrintEngine::setPageSize(QPrinter::PageSize size)
{
    Q_ASSERT(d->devMode);
    d->devMode->dmPaperSize = mapPageSizeDevmode(size);
    d->doReinit();
}

QPrinter::PageSize QWin32PrintEngine::pageSize() const
{
    Q_ASSERT(d->devMode);
    return mapDevmodePageSize(d->devMode->dmPaperSize);
}

void QWin32PrintEngine::setPageOrder(QPrinter::PageOrder)
{

}

QPrinter::PageOrder QWin32PrintEngine::pageOrder() const
{
    return QPrinter::FirstPageFirst;
}

void QWin32PrintEngine::setResolution(int resolution)
{
    d->resolution = resolution;
    d->setupPrinterMapping();
}

int QWin32PrintEngine::resolution() const
{
    return d->resolution;
}

void QWin32PrintEngine::setColorMode(QPrinter::ColorMode mode)
{
    Q_ASSERT(d->devMode);
    d->devMode->dmColor = mode == QPrinter::Color
        ? DMCOLOR_COLOR
        : DMCOLOR_MONOCHROME;
    d->doReinit();
}

QPrinter::ColorMode QWin32PrintEngine::colorMode() const
{
    Q_ASSERT(d->devMode);
    return d->devMode->dmColor == DMCOLOR_COLOR
        ? QPrinter::Color
        : QPrinter::GrayScale;
}

void QWin32PrintEngine::setFullPage(bool fullPage)
{
    d->fullPage = fullPage;
    d->setupOriginMapping();
}

bool QWin32PrintEngine::fullPage() const
{
    return d->fullPage;
}

void QWin32PrintEngine::setCollateCopies(bool)
{
    d->doReinit();
}

bool QWin32PrintEngine::collateCopies() const
{
    return false;
}

void QWin32PrintEngine::setPaperSource(QPrinter::PaperSource src)
{
    Q_ASSERT(d->devMode);
    int dmMapped = DMBIN_AUTO;

    DWORD caps = DeviceCapabilities((TCHAR*)d->name.utf16(), 0, DC_BINS, 0, 0);
    // Skip it altogether if it's not supported...
    if (caps != DWORD(-1) && caps != 0) {
        LPTSTR bins = (LPTSTR)(new WORD[caps]);
        if (DeviceCapabilities((TCHAR*)d->name.utf16(), 0, DC_BINS, bins, 0)) {
            bool ok = false;
            int source = mapPaperSourceDevmode(src);
            for (DWORD i=0; i<caps; i++)
                ok |= (bins[i] == (WORD)source);
            if (ok)
                dmMapped = source;
        }
        delete [] bins;
    }
    d->devMode->dmDefaultSource = dmMapped;
    d->doReinit();
}

QPrinter::PaperSource QWin32PrintEngine::paperSource() const
{
    Q_ASSERT(d->devMode);
    return mapDevmodePaperSource(d->devMode->dmDefaultSource);
}

QList<int> QWin32PrintEngine::supportedResolutions() const
{
    return d->queryResolutions();
}

void QWin32PrintEngine::setWinPageSize(short winPageSize)
{
    Q_ASSERT(d->devMode);
    d->devMode->dmPaperSize = winPageSize;
    d->doReinit();
}

short QWin32PrintEngine::winPageSize() const
{
    Q_ASSERT(d->devMode);
    return d->devMode->dmPaperSize;
}

QRect QWin32PrintEngine::paperRect() const
{
    double scale = d->resolution / (double) GetDeviceCaps(d->hdc, LOGPIXELSY);
    return QRect(0, 0,
                 int(GetDeviceCaps(d->hdc, PHYSICALWIDTH) * scale),
                 int(GetDeviceCaps(d->hdc, PHYSICALHEIGHT) * scale));
}

QRect QWin32PrintEngine::pageRect() const
{
    double scale = d->resolution / (double) GetDeviceCaps(d->hdc, LOGPIXELSY);
    int pagex = GetDeviceCaps(d->hdc, PHYSICALOFFSETX);
    int pagey = GetDeviceCaps(d->hdc, PHYSICALOFFSETY);
    int pagew = GetDeviceCaps(d->hdc, HORZRES) - pagex;
    int pageh = GetDeviceCaps(d->hdc, VERTRES) - pagey;
    return QRect(int(pagex*scale), int(pagey*scale), int(pagew*scale), int(pageh*scale));
}

bool QWin32PrintEngine::isActive() const
{
    return d->state == QPrinter::Active;
}

QPrinter::PrinterState QWin32PrintEngine::printerState() const
{
    return d->state;
}

void QWin32PrintEngine::setNumCopies(int copies)
{
    d->devMode->dmCopies = copies;
}

int QWin32PrintEngine::numCopies() const
{
    return 1;
}

HDC QWin32PrintEngine::getDC() const
{
    return d->hdc;
}

void QWin32PrintEngine::releaseDC(HDC)
{

}

HGLOBAL *QWin32PrintEnginePrivate::createDevNames()
{
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

#if defined QT_PRINTDIALOG_DEBUG
    printf("QPrintDialogWinPrivate::createDevNames()\n"
           " -> wDriverOffset: %d\n"
           " -> wDeviceOffset: %d\n"
           " -> wOutputOffset: %d\n",
           dn->wDriverOffset,
           dn->wDeviceOffset,
           dn->wOutputOffset);

    printf("QPrintDialogWinPrivate::createDevNames(): %s, %s, %s\n",
           QString::fromUtf16((ushort*)(dn) + dn->wDriverOffset).latin1(),
           QString::fromUtf16((ushort*)(dn) + dn->wDeviceOffset).latin1(),
           QString::fromUtf16((ushort*)(dn) + dn->wOutputOffset).latin1());
#endif // QT_PRINTDIALOG_DEBUG

    GlobalUnlock(hGlobal);

    return hGlobal;
}

void QWin32PrintEnginePrivate::readDevnames(HGLOBAL globalDevnames)
{
    if (globalDevnames) {
        DEVNAMES *dn = (DEVNAMES*) GlobalLock(globalDevnames);
        name = QString::fromUtf16((ushort*)(dn) + dn->wDeviceOffset);
        port = QString::fromUtf16((ushort*)(dn) + dn->wOutputOffset);
        program = QString::fromUtf16((ushort*)(dn) + dn->wDriverOffset);
        GlobalUnlock(globalDevnames);
    }
}

void QWin32PrintEnginePrivate::readDevmode(HGLOBAL globalDevmode)
{
    if (globalDevmode) {
        DEVMODE *dm = (DEVMODE*) GlobalLock(globalDevmode);
        release();
        globalDevMode = globalDevmode;
        devMode = dm;
        hdc = CreateDC(reinterpret_cast<const wchar_t *>(program.utf16()),
		       reinterpret_cast<const wchar_t *>(name.utf16()), 0, dm);
        setupPrinterMapping();
    }
}

#endif // QT_NO_PRINTER
