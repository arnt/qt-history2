/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
#include <qpicture.h>

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
    bmh->biHeight         = h;
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

QAlphaPaintEngine::QAlphaPaintEngine(QAlphaPaintEnginePrivate &data, PaintEngineFeatures devcaps)
    : QPaintEngine(data, devcaps)
{

}

QAlphaPaintEngine::~QAlphaPaintEngine()
{

}

bool QAlphaPaintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QAlphaPaintEngine);
    Q_ASSERT(d->m_pass == 0);

    if (d->m_initstate) {
        delete d->m_initstate;
        d->m_initstate = 0;
    }

    d->m_pdev = pdev;
    d->m_savedcaps = gccaps;
    gccaps = AllFeatures;

    d->m_pic = new QPicture();
    d->m_picpainter = new QPainter(d->m_pic);
    d->m_picengine = d->m_picpainter->paintEngine();

    d->m_alphaPen = false;
    d->m_alphaBrush = false;
    d->m_alphaOpacity = false;
    d->m_hasalpha = false;

    // clear alpha region
    d->m_alphargn = QRegion();
    d->m_cliprgn = QRegion();
    d->m_pen = QPen();
    d->m_transform = QTransform();

    return true;
}

extern int qt_defaultDpi();

bool QAlphaPaintEngine::end()
{
    Q_D(QAlphaPaintEngine);
    Q_ASSERT(d->m_pass == 0);
    Q_ASSERT(d->m_initstate != 0);

    d->m_picpainter->end();
    
    // set clip region
    d->m_cliprgn = d->m_alphargn;

    // now replay the QPicture
    ++d->m_pass; // we are now doing pass #2

    // reset states
    gccaps = d->m_savedcaps;
    painter()->d_func()->updateState(d->m_initstate);

    begin(d->m_pdev);

    // make sure the output from QPicture is unscaled
    QTransform mtx = painter()->transform();
    mtx.scale(1.0f / (qreal(d->m_pdev->logicalDpiX()) / qreal(qt_defaultDpi())),
                      1.0f / (qreal(d->m_pdev->logicalDpiY()) / qreal(qt_defaultDpi())));
    painter()->setTransform(mtx);
    painter()->drawPicture(0, 0, *d->m_pic);

    d->m_cliprgn = QRegion();
    painter()->setClipPath(QPainterPath(), Qt::NoClip);

    QVector<QRect> rects = d->m_alphargn.rects();
    for (int i=0; i<rects.count(); ++i) {
        d->drawAlphaImage(rects.at(i));
    }
        
    --d->m_pass; // pass #2 finished

    return true;
}

void QAlphaPaintEngine::updateState(const QPaintEngineState &state)
{
    Q_D(QAlphaPaintEngine);
    Q_ASSERT(d->m_pass == 0);

    if (!d->m_initstate)
        d->m_initstate = new QPainterState(static_cast<QPainterState *>(this->state));

    DirtyFlags flags = state.state();
    if (flags & QPaintEngine::DirtyOpacity) {
        d->m_alphaOpacity = (state.opacity() != 1.0f);
    }

    if (flags & QPaintEngine::DirtyBrush) {
        if (state.brush().style() == Qt::NoBrush)
            d->m_alphaBrush = false;
        else
            d->m_alphaBrush = !state.brush().isOpaque();
    }
    
    if (flags & QPaintEngine::DirtyPen) {
        d->m_pen = state.pen();
        if (d->m_pen.style() == Qt::NoPen)
            d->m_alphaPen = false;
        else
            d->m_alphaPen = !d->m_pen.brush().isOpaque();
    }

    if (flags & QPaintEngine::DirtyTransform) {
        d->m_transform = state.transform();
    }

    d->m_hasalpha = d->m_alphaOpacity || d->m_alphaBrush || d->m_alphaPen;

    d->m_picengine->updateState(state);
}

void QAlphaPaintEngine::drawPath(const QPainterPath &path)
{
    Q_D(QAlphaPaintEngine);
    if (d->m_pass == 0) {
        if (d->m_hasalpha) {
            QRectF tr = d->addPenWidth(path.controlPointRect());
            d->addAlphaRect(d->m_transform.mapRect(tr));
        }
        d->m_picengine->drawPath(path);
    } else {
        QPaintEngine::drawPath(path);
    }
}

void QAlphaPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{
    Q_D(QAlphaPaintEngine);
    if (d->m_pass == 0) {
        if (d->m_hasalpha) {
            QPolygonF poly;
            for (int i=0; i<pointCount; ++i)
                poly.append(points[i]);
            QRectF tr = d->addPenWidth(poly.boundingRect());
            d->addAlphaRect(d->m_transform.mapRect(tr));
        }
        d->m_picengine->drawPolygon(points, pointCount, mode);
    } else {
        QPaintEngine::drawPolygon(points, pointCount, mode);
    }
}

void QAlphaPaintEngine::drawPixmap(const QRectF &r, const QPixmap &pm, const QRectF &sr)
{
    Q_D(QAlphaPaintEngine);
    Q_ASSERT(d->m_pass == 0);

    if (pm.hasAlpha() || d->m_alphaOpacity) {
        d->addAlphaRect(d->m_transform.mapRect(r));
    }

    d->m_picengine->drawPixmap(r, pm, sr);
}

void QAlphaPaintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(QAlphaPaintEngine);
    if (d->m_pass == 0) {
        if (d->m_alphaPen || d->m_alphaOpacity) {
            QRectF tr(p.x(), p.y() - textItem.ascent(), textItem.width() + 5, textItem.ascent() + textItem.descent() + 5);
            d->addAlphaRect(d->m_transform.mapRect(tr));
        }

        d->m_picengine->drawTextItem(p, textItem);
    } else {
        QPaintEngine::drawTextItem(p, textItem);
    }
}

void QAlphaPaintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pixmap, const QPointF &s)
{
    Q_D(QAlphaPaintEngine);
    if (d->m_pass == 0) {
        if (pixmap.hasAlpha() || d->m_alphaOpacity) {
            d->addAlphaRect(d->m_transform.mapRect(r));
        }
        d->m_picengine->drawTiledPixmap(r, pixmap, s);
    } else {
        QPaintEngine::drawTiledPixmap(r, pixmap, s);
    }
}

QRegion QAlphaPaintEngine::alphaClipping() const
{
    Q_D(const QAlphaPaintEngine);
    return d->m_cliprgn;
}

bool QAlphaPaintEngine::redirect() const
{
    Q_D(const QAlphaPaintEngine);
    return (d->m_pass == 0);
}

QAlphaPaintEnginePrivate::QAlphaPaintEnginePrivate()
    :   m_pass(0),
        m_pic(0),
        m_picengine(0),
        m_picpainter(0),
        m_initstate(0),
        m_alphaPen(false),
        m_alphaBrush(false),
        m_alphaOpacity(false),
        m_hasalpha(false)
{
        
}

QAlphaPaintEnginePrivate::~QAlphaPaintEnginePrivate()
{
    delete m_initstate;
    delete m_picpainter;
    delete m_pic;
}

QRectF QAlphaPaintEnginePrivate::addPenWidth(const QRectF &rect)
{
    QRectF br = rect;
    if (m_pen.style() != Qt::NoPen) {
        int w2 = m_pen.width() / 2;
        br.setCoords(br.left() - w2, br.top() - w2,
                    br.right() + w2, br.bottom() + w2);
    }
    return br;
}

void QAlphaPaintEnginePrivate::addAlphaRect(const QRectF &rect)
{
    QRect r;
    r.setLeft(rect.left());
    r.setTop(rect.top());
    r.setRight(rect.right() + 1);
    r.setBottom(rect.bottom() + 1);
    m_alphargn |= r;
}

void QAlphaPaintEnginePrivate::drawAlphaImage(const QRectF &rect)
{
    Q_Q(QAlphaPaintEngine);

    qreal dpiX = qMax(m_pdev->logicalDpiX(), 300);
    qreal dpiY = qMax(m_pdev->logicalDpiY(), 300);
    qreal xscale = (dpiX / m_pdev->logicalDpiX());
    qreal yscale = (dpiY / m_pdev->logicalDpiY());
    qreal txscale = qreal(dpiX) / qreal(qt_defaultDpi());
    qreal tyscale = qreal(dpiY) / qreal(qt_defaultDpi());

    QTransform picscale;
    picscale.scale(qreal(qt_defaultDpi()) / qreal(dpiX),
        qreal(qt_defaultDpi()) / qreal(dpiY));
    picscale.scale(xscale, yscale);

    QSize size((rect.width() * xscale), (rect.height() * yscale));
    int divw = (size.width() / 1024);
    int divh = (size.height() / 1024);
    divw += 1;
    divh += 1;

    int incx = rect.width() / divw;
    int incy = rect.height() / divh;

    for (int y=0; y<divh; ++y) {
        int ypos = (incy * y) + rect.y();
        int height = ( (y == (divh - 1)) ? (rect.height() - (incy * y)) : incy ) + 1;

        for (int x=0; x<divw; ++x) {
            int xpos = (incx * x) + rect.x();
            int width = ( (x == (divw - 1)) ? (rect.width() - (incx * x)) : incx ) + 1;

            QSize imgsize(width * xscale, height * yscale);
            QImage img(imgsize, QImage::Format_RGB32);
            img.setDotsPerMeterX((dpiX * 100) / 2.54f);
            img.setDotsPerMeterY((dpiY * 100) / 2.54f);
            img.fill(QColor(Qt::white).rgb());

            QPainter imgpainter(&img);
            imgpainter.setTransform(picscale);
            QPointF picpos(qreal(-xpos) * txscale, qreal(-ypos) * tyscale);
            imgpainter.drawPicture(picpos, *m_pic);
            imgpainter.end();

            q->painter()->setTransform(QTransform());
            QRect r(xpos, ypos, width, height);
            q->painter()->drawImage(r, img);
        }
    }
}

QWin32PrintEngine::QWin32PrintEngine(QPrinter::PrinterMode mode)
    : QAlphaPaintEngine(*(new QWin32PrintEnginePrivate),
                   PaintEngineFeatures(PrimitiveTransform
                                       | PixmapTransform
                                       | PerspectiveTransform
                                       | PainterPaths
                                       | Antialiasing
                                       | PaintOutsidePaintEvent))
{
    Q_D(QWin32PrintEngine);
    d->docName = QLatin1String("document1");
    d->mode = mode;
    d->queryDefault();
    d->initialize();
}

bool QWin32PrintEngine::begin(QPaintDevice *pdev)
{
    Q_D(QWin32PrintEngine);

    if (redirect()) {
        return QAlphaPaintEngine::begin(pdev);
    }

    if (d->reinit) {
       d->resetDC();
    }

    // ### set default colors and stuff...

    bool ok = d->state == QPrinter::Idle;

    if (!d->hdc)
        return false;

    // Assign the FILE: to get the query...
    if (d->fileName.isEmpty())
        d->fileName = d->port;

    QT_WA({
    d->devModeW()->dmCopies = d->num_copies;
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
    d->devModeA()->dmCopies = d->num_copies;
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

    if (StartPage(d->hdc) <= 0) {
        qErrnoWarning("QWin32PrintEngine::begin: StartPage failed");
        ok = false;
    }

    if (!ok) {
        d->state = QPrinter::Idle;
    } else {
        d->state = QPrinter::Active;
    }

    d->matrix = QTransform();
    d->has_pen = true;
    d->pen = QColor(Qt::black);
    d->has_brush = false;

    d->complex_xform = false;

    updateMatrix(d->matrix);

    return ok;
}

bool QWin32PrintEngine::end()
{
    Q_D(QWin32PrintEngine);

    if (redirect())
        QAlphaPaintEngine::end();

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
//          state = PST_ACTIVEDOC;
//          reinit();
//          state = PST_ACTIVE;
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
//      }

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

extern void qt_draw_text_item(const QPointF &pos, const QTextItemInt &ti, HDC hdc,
                              bool convertToText, const QTransform &matrix, const QPointF &topLeft);

void QWin32PrintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
    Q_D(const QWin32PrintEngine);

    if (redirect()) {
        QAlphaPaintEngine::drawTextItem(p, textItem);
        return;
    }

    QRgb brushColor = state->pen().brush().color().rgb();
    bool fallBack = state->pen().brush().style() != Qt::SolidPattern
                    || qAlpha(brushColor) != 0xff
                    || QT_WA_INLINE(false, d->txop >= QTransform::TxScale);


    if (!fallBack) {
        const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
        QFontEngine *fe = ti.fontEngine;

        // Try selecting the font to see if we get a substitution font
        SelectObject(d->hdc, fe->hfont);

        if (GetDeviceCaps(d->hdc, TECHNOLOGY) != DT_CHARSTREAM) {
            QT_WA({
                TCHAR n[64];
                GetTextFaceW(d->hdc, 64, n);
                fallBack = QString::fromUtf16((ushort *)n)
                        != QString::fromUtf16((ushort *)fe->logfont.lfFaceName);
            } , {
                char an[64];
                GetTextFaceA(d->hdc, 64, an);
                fallBack = QString::fromLocal8Bit(an)
                        != QString::fromLocal8Bit(((LOGFONTA*)(&fe->logfont))->lfFaceName);
            });
        }
    }


    if (fallBack) {
        QPaintEngine::drawTextItem(p, textItem);
        return ;
    }

    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

    // We only want to convert the glyphs to text if the entire string is latin1
    bool latin1String = true;
    for (int i=0;  i < ti.num_chars; ++i) {
        if (ti.chars[i].unicode() >= 0x100) {
            latin1String = false;
            break;
        }
    }

    COLORREF cf = RGB(qRed(brushColor), qGreen(brushColor), qBlue(brushColor));
    SelectObject(d->hdc, CreateSolidBrush(cf));
    SelectObject(d->hdc, CreatePen(PS_SOLID, 1, cf));
    SetTextColor(d->hdc, cf);

    qt_draw_text_item(p, ti, d->hdc, latin1String, d->matrix, d->devPaperRect.topLeft());
    DeleteObject(SelectObject(d->hdc,GetStockObject(HOLLOW_BRUSH)));
    DeleteObject(SelectObject(d->hdc,GetStockObject(BLACK_PEN)));
}

static inline qreal mmToInches(double mm)
{
    return mm*0.039370147;
}

static inline qreal inchesToMM(double in)
{
    return in/0.039370147;
}

int QWin32PrintEngine::metric(QPaintDevice::PaintDeviceMetric m) const
{
    Q_D(const QWin32PrintEngine);

    if (!d->hdc)
        return 0;

    int val;
    int res = d->resolution;

    switch (m) {
    case QPaintDevice::PdmWidth:
        val = res
              * GetDeviceCaps(d->hdc, d->fullPage ? PHYSICALWIDTH : HORZRES)
              / GetDeviceCaps(d->hdc, LOGPIXELSX);
        if (d->pageMarginsSet)
            val -= mmToInches(d->previousDialogMargins.left() +
                              d->previousDialogMargins.width());
        break;
    case QPaintDevice::PdmHeight:
        val = res
              * GetDeviceCaps(d->hdc, d->fullPage ? PHYSICALHEIGHT : VERTRES)
              / GetDeviceCaps(d->hdc, LOGPIXELSY);
        if (d->pageMarginsSet)
            val -= mmToInches(d->previousDialogMargins.top() +
                              d->previousDialogMargins.height());
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
            if (d->pageMarginsSet)
                val -= d->previousDialogMargins.left() +
                       d->previousDialogMargins.width();
        }
        break;
    case QPaintDevice::PdmHeightMM:
        if (!d->fullPage) {
            val = GetDeviceCaps(d->hdc, VERTSIZE);
        }
        else {
            float hi = 25.4 * GetDeviceCaps(d->hdc, PHYSICALHEIGHT);
            val = qRound(hi / GetDeviceCaps(d->hdc,  LOGPIXELSY));
            if (d->pageMarginsSet)
                val -= d->previousDialogMargins.top() +
                       d->previousDialogMargins.height();
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

    if (redirect()) {
        QAlphaPaintEngine::updateState(state);
        return;
    }

    if (state.state() & DirtyTransform) {
        updateMatrix(state.transform());
    }

    if (state.state() & DirtyPen) {
        d->pen = state.pen();
        d->has_pen = d->pen.style() != Qt::NoPen && d->pen.isSolid();
    }

    if (state.state() & DirtyBrush) {
        QBrush brush = state.brush();
        d->has_brush = brush.style() == Qt::SolidPattern;
        d->brush_color = brush.color();
    }

    if (state.state() & DirtyClipEnabled) {
        if (state.isClipEnabled())
            updateClipPath(painter()->clipPath(), Qt::ReplaceClip);
        else
            updateClipPath(QPainterPath(), Qt::NoClip);
    }

    if (state.state() & DirtyClipPath) {
        updateClipPath(state.clipPath(), state.clipOperation());
    }

    if (state.state() & DirtyClipRegion) {
        QPainterPath clipPath;
        QRegion clipRegion = state.clipRegion();
        clipPath.addRegion(clipRegion);
        updateClipPath(clipPath, state.clipOperation());
    }
}

void QWin32PrintEngine::updateClipPath(const QPainterPath &clipPath, Qt::ClipOperation op)
{
    Q_D(QWin32PrintEngine);

    bool doclip = true;
    if (op == Qt::NoClip) {
        SelectClipRgn(d->hdc, 0);
        doclip = false;
    }

    if (doclip) {
        QPainterPath xformed = clipPath * d->matrix;

        if (xformed.isEmpty()) {
            QRegion empty(-0x1000000, -0x1000000, 1, 1);
            SelectClipRgn(d->hdc, empty.handle());
        } else {
            d->composeGdiPath(xformed);
            const int ops[] = {
                -1,         // Qt::NoClip, covered above
                RGN_COPY,   // Qt::ReplaceClip
                RGN_AND,    // Qt::IntersectClip
                RGN_OR      // Qt::UniteClip
            };
            Q_ASSERT(op > 0 && unsigned(op) <= sizeof(ops) / sizeof(int));
            SelectClipPath(d->hdc, ops[op]);
        }
    }

    QPainterPath aclip;
    aclip.addRegion(alphaClipping());
    if (!aclip.isEmpty()) {
        QTransform tx;
        tx.scale(d->stretch_x, d->stretch_y);
        d->composeGdiPath(tx.map(aclip));
        SelectClipPath(d->hdc, RGN_DIFF);
    }
}

void QWin32PrintEngine::updateMatrix(const QTransform &m)
{
    Q_D(QWin32PrintEngine);

    QTransform stretch(d->stretch_x, 0, 0, d->stretch_y, d->origin_x, d->origin_y);
    d->painterMatrix = m;
    d->matrix = d->painterMatrix * stretch;

    if (d->matrix.m12() != 0 || d->matrix.m21() != 0 ||
        d->matrix.m13() != 0 || d->matrix.m23() != 0)
        d->txop = QTransform::TxRotShear;
    else if (d->matrix.m11() != 1 || d->matrix.m22() != 1)
        d->txop = QTransform::TxScale;
    else if (d->matrix.dx() != 0 || d->matrix.dy() != 0)
        d->txop = QTransform::TxTranslate;
    else
        d->txop = QTransform::TxNone;

    d->complex_xform = (d->txop > QTransform::TxScale);
}

void QWin32PrintEngine::drawPixmap(const QRectF &targetRect,
                                   const QPixmap &originalPixmap,
                                   const QRectF &sourceRect)
{
    Q_D(QWin32PrintEngine);

    if (redirect()) {
        QAlphaPaintEngine::drawPixmap(targetRect, originalPixmap, sourceRect);
        return;
    }

    const int tilesize = 2048;

    QRectF r = targetRect;
    QRectF sr = sourceRect;

    QPixmap pixmap = originalPixmap;
    if (sr.size() != pixmap.size()) {
        pixmap = pixmap.copy(sr.toRect());
    }

    qreal scaleX = 1.0f;
    qreal scaleY = 1.0f;

    QTransform scaleMatrix;
    scaleMatrix.scale(r.width() / pixmap.width(), r.height() / pixmap.height());
    QTransform adapted = QPixmap::trueMatrix(d->painterMatrix * scaleMatrix, 
        pixmap.width(), pixmap.height());

    qreal xform_offset_x = adapted.dx();
    qreal xform_offset_y = adapted.dy();

    if (d->complex_xform) {
        pixmap = pixmap.transformed(adapted);
        scaleX = d->stretch_x;
        scaleY = d->stretch_y;
    } else {
        scaleX = d->stretch_x * (r.width() / pixmap.width()) * d->painterMatrix.m11();
        scaleY = d->stretch_y * (r.height() / pixmap.height()) * d->painterMatrix.m22();
    }

    QPointF topLeft = r.topLeft() * d->painterMatrix;
    int tx = topLeft.x() * d->stretch_x + d->origin_x;
    int ty = topLeft.y() * d->stretch_y + d->origin_y;
    int tw = pixmap.width() * scaleX;
    int th = pixmap.height() * scaleY;

    xform_offset_x *= d->stretch_x;
    xform_offset_y *= d->stretch_y;

    int dc_state = SaveDC(d->hdc);

    int tilesw = pixmap.width() / tilesize;
    int tilesh = pixmap.height() / tilesize;
    ++tilesw;
    ++tilesh;

    int txinc = tw / tilesw;
    int tyinc = th / tilesh;

    for (int y = 0; y < tilesh; ++y) {
        int tposy = ty + (y * tyinc);
        int imgh = tilesize;
        int height = tyinc;
        if (y == (tilesh - 1)) {
            imgh = pixmap.height() - (y * tilesize);
            height = (th - (y * tyinc));
        }
        for (int x = 0; x < tilesw; ++x) {
            int tposx = tx + (x * txinc);
            int imgw = tilesize;
            int width = txinc;
            if (x == (tilesw - 1)) {
                imgw = pixmap.width() - (x * tilesize);
                width = (tw - (x * txinc));
            }

            QPixmap p = pixmap.copy(tilesize * x, tilesize * y, imgw, imgh);
            HBITMAP hbitmap = p.toWinHBITMAP(QPixmap::NoAlpha);
            HDC hbitmap_hdc = CreateCompatibleDC(qt_win_display_dc());
            HGDIOBJ null_bitmap = SelectObject(hbitmap_hdc, hbitmap);

            if (!StretchBlt(d->hdc, qRound(tposx - xform_offset_x), qRound(tposy - xform_offset_y), width, height,
                            hbitmap_hdc, 0, 0, p.width(), p.height(), SRCCOPY))
                qErrnoWarning("QWin32PrintEngine::drawPixmap, StretchBlt failed");

            SelectObject(hbitmap_hdc, null_bitmap);
            DeleteObject(hbitmap);
            DeleteDC(hbitmap_hdc);
        }
    }

    RestoreDC(d->hdc, dc_state);
}


void QWin32PrintEngine::drawTiledPixmap(const QRectF &r, const QPixmap &pm, const QPointF &pos)
{
    if (redirect()) {
        QAlphaPaintEngine::drawTiledPixmap(r, pm, pos);
        return;
    }

    QRectF rectAtOrigin(0, 0, r.width(), r.height());
    QImage image(rectAtOrigin.size().toSize(), QImage::Format_ARGB32_Premultiplied);
    QPainter p(&image);
    p.drawTiledPixmap(rectAtOrigin, pm, pos);
    p.end();
    drawPixmap(r, QPixmap::fromImage(image), rectAtOrigin);

}


void QWin32PrintEnginePrivate::composeGdiPath(const QPainterPath &path)
{
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
}


void QWin32PrintEnginePrivate::fillPath_dev(const QPainterPath &path, const QColor &color)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " --- QWin32PrintEnginePrivate::fillPath() bound:" << path.boundingRect() << color;
#endif

    composeGdiPath(path);

    HBRUSH brush = CreateSolidBrush(RGB(color.red(), color.green(), color.blue()));
    HGDIOBJ old_brush = SelectObject(hdc, brush);
    FillPath(hdc);
    DeleteObject(SelectObject(hdc, old_brush));
}

void QWin32PrintEnginePrivate::strokePath_dev(const QPainterPath &path, const QColor &color,
                                              Qt::PenStyle penStyle)
{
    composeGdiPath(path);

    int gdiPenStyle;
    switch (penStyle)
    {
    case Qt::DashLine:
        gdiPenStyle = PS_DASH;
        break;
    case Qt::DashDotLine:
        gdiPenStyle = PS_DASHDOT;
        break;
    case Qt::DashDotDotLine:
        gdiPenStyle = PS_DASHDOTDOT;
        break;
    case Qt::DotLine:
        gdiPenStyle = PS_DOT;
        break;
    default:
        gdiPenStyle = PS_SOLID;
        break;
    };

    HPEN pen = CreatePen(gdiPenStyle, 0, RGB(color.red(), color.green(), color.blue()));
    HGDIOBJ old_pen = SelectObject(hdc, pen);
    StrokePath(hdc);
    DeleteObject(SelectObject(hdc, old_pen));
}


void QWin32PrintEnginePrivate::fillPath(const QPainterPath &path, const QColor &color)
{
    fillPath_dev(path * matrix, color);
}

void QWin32PrintEnginePrivate::strokePath(const QPainterPath &path, const QColor &color)
{
    QPainterPathStroker stroker;
    stroker.setDashPattern(pen.dashPattern());
    stroker.setDashOffset(pen.dashOffset());
    stroker.setCapStyle(pen.capStyle());
    stroker.setJoinStyle(pen.joinStyle());
    stroker.setMiterLimit(pen.miterLimit());

    QPainterPath stroke;

    qreal width = pen.widthF();
    if (pen.isCosmetic()) {
        // We do not support custom dash patterns for stroked paths with solid colored, cosmetic pens
        strokePath_dev(path * matrix, color, pen.style());
    } else {
        stroker.setWidth(width);
        stroke = stroker.createStroke(path) * matrix;
        if (stroke.isEmpty())
            return;

        fillPath_dev(stroke, color);
    }
}


void QWin32PrintEngine::drawPath(const QPainterPath &path)
{
#ifdef QT_DEBUG_DRAW
    qDebug() << " - QWin32PrintEngine::drawPath(), bounds: " << path.boundingRect();
#endif

    Q_D(QWin32PrintEngine);

    if (redirect()) {
        QAlphaPaintEngine::drawPath(path);
        return;
    }

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

    if (redirect()) {
        QAlphaPaintEngine::drawPolygon(points, pointCount, mode);
        return;
    }

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
    QString noPrinters(QLatin1String("qt_no_printers"));
    QString output;
    QT_WA({
        ushort buffer[256];
        GetProfileStringW(L"windows", L"device",
                          reinterpret_cast<const wchar_t *>(noPrinters.utf16()),
                          reinterpret_cast<wchar_t *>(buffer), 256);
        output = QString::fromUtf16(buffer);
        if (output == noPrinters) { // no printers
            qWarning("QPrinter: System has no default printer, are any printers installed?");
            return;
        }
    }, {
        char buffer[256];
        GetProfileStringA("windows", "device", noPrinters.toLatin1(), buffer, 256);
        output = QString::fromLocal8Bit(buffer);
        if (output == noPrinters) { // no printers
            qWarning("QPrinter: System has no default printer, are any printers installed?");
            return;
        }
    });
    QStringList info = output.split(QLatin1Char(','));
    if(name.isEmpty())
        name = info.at(0);
    if(program.isEmpty())
        program = info.at(1);
    if(port.isEmpty())
        port = info.at(2);
}

QWin32PrintEnginePrivate::~QWin32PrintEnginePrivate()
{
    if (hdc)
        release();
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

    txop = QTransform::TxNone;

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
        hMem = GlobalAlloc(GHND, infoSize);
        pInfo = GlobalLock(hMem);
        if (!GetPrinterW(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes)) {
            qErrnoWarning("QWin32PrintEngine::initialize: GetPrinter failed");
            return;
        }
    }, {
        GetPrinterA(hPrinter, 2, NULL, 0, &infoSize);
        hMem = GlobalAlloc(GHND, infoSize);
        pInfo = GlobalLock(hMem);
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
    Q_ASSERT(pInfo);

    if (devMode) {
        QT_WA( {
            num_copies = devModeW()->dmCopies;
        }, {
            num_copies = devModeA()->dmCopies;
        } );
    }

    initHDC();

#ifdef QT_DEBUG_DRAW
    qDebug() << "QWin32PrintEngine::initialize()" << endl
             << " - paperRect" << devPaperRect << endl
             << " - pageRect" << devPageRect << endl
             << " - stretch_x" << stretch_x << endl
             << " - stretch_y" << stretch_y << endl
             << " - origin_x" << origin_x << endl
             << " - origin_y" << origin_y << endl;
#endif
}

void QWin32PrintEnginePrivate::initHDC()
{
    Q_ASSERT(hdc);

    dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
    dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
    dpi_display = GetDeviceCaps(qt_win_display_dc(), LOGPIXELSY);

    switch(mode) {
    case QPrinter::ScreenResolution:
        resolution = dpi_display;
        stretch_x = dpi_x / double(dpi_display);
        stretch_y = dpi_y / double(dpi_display);
        break;
    case QPrinter::PrinterResolution:
    case QPrinter::HighResolution:
        resolution = dpi_y;
        stretch_x = 1;
        stretch_y = 1;
        break;
    default:
        break;
    }

    initDevRects();
}

void QWin32PrintEnginePrivate::initDevRects()
{
    devPaperRect = QRect(0, 0,
                         GetDeviceCaps(hdc, PHYSICALWIDTH),
                         GetDeviceCaps(hdc, PHYSICALHEIGHT));
    devPhysicalPageRect = QRect(GetDeviceCaps(hdc, PHYSICALOFFSETX),
                                GetDeviceCaps(hdc, PHYSICALOFFSETY),
                                GetDeviceCaps(hdc, HORZRES),
                                GetDeviceCaps(hdc, VERTRES));
    if (!pageMarginsSet)
        devPageRect = devPhysicalPageRect;
    else
        devPageRect = devPaperRect.adjusted(qRound(mmToInches(previousDialogMargins.left() / 100.0) * dpi_x),
                                            qRound(mmToInches(previousDialogMargins.top() / 100.0) * dpi_y),
                                            -qRound(mmToInches(previousDialogMargins.width() / 100.0) * dpi_x),
                                            -qRound(mmToInches(previousDialogMargins.height() / 100.0) * dpi_y));
    updateOrigin();
}

void QWin32PrintEnginePrivate::setPageMargins(int marginLeft, int marginTop, int marginRight, int marginBottom)
{
    pageMarginsSet = true;
    previousDialogMargins = QRect(marginLeft, marginTop, marginRight, marginBottom);

    devPageRect = devPaperRect.adjusted(qRound(mmToInches(marginLeft / 100.0) * dpi_x),
                                        qRound(mmToInches(marginTop / 100.0) * dpi_y),
                                        - qRound(mmToInches(marginRight / 100.0) * dpi_x),
                                        - qRound(mmToInches(marginBottom / 100.0) * dpi_y));
    fullPage = true;
    updateOrigin();
}

QRect QWin32PrintEnginePrivate::getPageMargins()
{
    if (pageMarginsSet)
        return previousDialogMargins;
    else
        return QRect(qRound(inchesToMM(devPhysicalPageRect.left()) * 100.0 / dpi_x),
                     qRound(inchesToMM(devPhysicalPageRect.top()) * 100.0 / dpi_y),
                     qRound(inchesToMM(devPaperRect.right() - devPhysicalPageRect.right()) * 100.0 / dpi_x),
                     qRound(inchesToMM(devPaperRect.bottom() - devPhysicalPageRect.bottom()) * 100.0 / dpi_y));
}

void QWin32PrintEnginePrivate::release()
{
    if (hdc == 0)
        return;

    if (globalDevMode) { // Devmode comes from print dialog
        GlobalUnlock(globalDevMode);
    } else {            // Devmode comes from initialize...
        // devMode is a part of the same memory block as pInfo so one free is enough...
        GlobalUnlock(hMem);
        GlobalFree(hMem);
        ClosePrinter(hPrinter);
    }
    DeleteDC(hdc);

    hdc = 0;
    hPrinter = 0;
    pInfo = 0;
    hMem = 0;
    devMode = 0;
}

QList<QVariant> QWin32PrintEnginePrivate::queryResolutions() const
{
    // Read the supported resolutions of the printer.
    DWORD numRes;
    LONG *enumRes;
    DWORD errRes;
    QList<QVariant> list;

    QT_WA({
        numRes = DeviceCapabilities(reinterpret_cast<const wchar_t *>(name.utf16()),
                                    reinterpret_cast<const wchar_t *>(port.utf16()),
                                    DC_ENUMRESOLUTIONS, 0, 0);
        if (numRes == (DWORD)-1)
            return list;
        enumRes = (LONG*)malloc(numRes * 2 * sizeof(LONG));
        errRes = DeviceCapabilities(reinterpret_cast<const wchar_t *>(name.utf16()),
                                    reinterpret_cast<const wchar_t *>(port.utf16()),
                                    DC_ENUMRESOLUTIONS, (LPWSTR)enumRes, 0);
    }, {
        numRes = DeviceCapabilitiesA(name.toLocal8Bit(), port.toLocal8Bit(), DC_ENUMRESOLUTIONS, 0, 0);
        if (numRes == (DWORD)-1)
            return list;
        enumRes = (LONG*)malloc(numRes * 2 * sizeof(LONG));
        errRes = DeviceCapabilitiesA(name.toLocal8Bit(), port.toLocal8Bit(), DC_ENUMRESOLUTIONS, (LPSTR)enumRes, 0);
    });

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
    if (state == QPrinter::Active) {
        reinit = true;
    } else {
        resetDC();
        initDevRects();
    }
}

void QWin32PrintEnginePrivate::updateOrigin()
{
    if (fullPage) {
        // subtract physical margins to make (0,0) absolute top corner of paper
        // then add user defined margins
        origin_x = -devPhysicalPageRect.x();
        origin_y = -devPhysicalPageRect.y();
        if (pageMarginsSet) {
            origin_x += devPageRect.left();
            origin_y += devPageRect.top();
        }
    } else {
        origin_x = 0;
        origin_y = 0;
    }
}

void QWin32PrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant &value)
{
    Q_D(QWin32PrintEngine);
    switch (key) {
    case PPK_CollateCopies:
        {
            if (!d->devMode)
                break;
            short collate = value.toBool() ? DMCOLLATE_TRUE : DMCOLLATE_FALSE;
            QT_WA( { d->devModeW()->dmCollate = collate; },
                   { d->devModeA()->dmCollate = collate; } );
            d->doReinit();
        }
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
            qWarning("QWin32PrintEngine: Cannot change document name while printing is active");
            return;
        }
        d->docName = value.toString();
        break;

    case PPK_FullPage:
        d->fullPage = value.toBool();
        d->updateOrigin();
        break;

    case PPK_NumberOfCopies:
        d->num_copies = value.toInt();
        QT_WA( { d->devModeW()->dmCopies = d->num_copies; },
               { d->devModeA()->dmCopies = d->num_copies; });
        d->doReinit();
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
            qWarning("QWin32PrintEngine: Cannot change filename while printing");
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

            QList<QVariant> v = property(PPK_PaperSources).toList();
            if (v.contains(value))
                dmMapped = mapPaperSourceDevmode(QPrinter::PaperSource(value.toInt()));

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
        if(d->name.isEmpty())
            d->queryDefault();
        d->initialize();
        break;

    case PPK_Resolution:
        {
            d->resolution = value.toInt();

            d->stretch_x = d->dpi_x / double(d->resolution);
            d->stretch_y = d->dpi_y / double(d->resolution);
        }
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
            if (!d->devMode) {
                value = QPrinter::Color;
            } else {
                int mode;
                QT_WA( {
                    mode = d->devModeW()->dmColor;
                }, {
                    mode = d->devModeA()->dmColor;
                } );
                value = mode == DMCOLOR_COLOR ? QPrinter::Color : QPrinter::GrayScale;
            }
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
            if (!d->devMode) {
                value = QPrinter::Portrait;
            } else {
                int o;
                QT_WA( { o = d->devModeW()->dmOrientation; }, { o = d->devModeA()->dmOrientation; } );
                value = o == DMORIENT_LANDSCAPE ? QPrinter::Landscape : QPrinter::Portrait;
            }
        }
        break;

    case PPK_OutputFileName:
        value = d->fileName;
        break;

    case PPK_PageRect:
        value = QTransform(1/d->stretch_x, 0, 0, 1/d->stretch_y, 0, 0).mapRect(d->devPageRect);
        break;

    case PPK_PageSize:
        if (!d->devMode) {
            value = QPrinter::A4;
        } else {
            QT_WA( {
                value = mapDevmodePageSize(d->devModeW()->dmPaperSize);
            }, {
                value = mapDevmodePageSize(d->devModeA()->dmPaperSize);
            } );
        }
        break;

    case PPK_PaperRect:
        value = QTransform(1/d->stretch_x, 0, 0, 1/d->stretch_y, 0, 0).mapRect(d->devPaperRect);
        break;

    case PPK_PaperSource:
        if (!d->devMode) {
            value = QPrinter::Auto;
        } else {
            QT_WA( {
                value = mapDevmodePaperSource(d->devModeW()->dmDefaultSource);
            }, {
                value = mapDevmodePaperSource(d->devModeA()->dmDefaultSource);
            } );
        }
        break;

    case PPK_PrinterName:
        value = d->name;
        break;

    case PPK_Resolution:
        if (d->resolution || !d->name.isEmpty())
            value = d->resolution;
        break;

    case PPK_SupportedResolutions:
        value = d->queryResolutions();
        break;

    case PPK_WindowsPageSize:
        if (!d->devMode) {
            value = -1;
        } else {
            QT_WA( {
                value = d->devModeW()->dmPaperSize;
            }, {
                value = d->devModeA()->dmPaperSize;
            } );
        }
        break;

    case PPK_PaperSources:
        {
            int available, count;
            WORD *data;

            QT_WA({
                available = DeviceCapabilitiesW((const WCHAR *)d->name.utf16(), (const WCHAR *)d->port.utf16(), DC_BINS, 0,
                                                d->devModeW());
            }, {
                available = DeviceCapabilitiesA(d->name.toLatin1(), d->port.toLatin1(), DC_BINS, 0,
                                                d->devModeA());
            });

            if (available <= 0)
                break;
            data = (WORD *) malloc(available * sizeof(WORD));

            QT_WA({
                count = DeviceCapabilitiesW((const WCHAR *)d->name.utf16(), (const WCHAR *)d->port.utf16(), DC_BINS, (WCHAR *)data,
                                            d->devModeW());
            }, {
                count = DeviceCapabilitiesA(d->name.toLatin1(), d->port.toLatin1(), DC_BINS,
                                            (char *) data, d->devModeA());
            });

            QList<QVariant> out;
            for (int i=0; i<count; ++i) {
                QPrinter::PaperSource src = mapDevmodePaperSource(data[i]);
                if (src != -1)
                    out << (int) src;
            }
            value = out;
            free(data);
        }
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

            num_copies = devModeW()->dmCopies;
        }, {
            DEVMODEA *dm = (DEVMODEA*) GlobalLock(globalDevmode);
            release();
            globalDevMode = globalDevmode;
            devMode = dm;
            hdc = CreateDCA(program.toLatin1(), name.toLatin1(), 0, dm);

            num_copies = devModeA()->dmCopies;
        } );
    }

    initHDC();
}

#endif // QT_NO_PRINTER
