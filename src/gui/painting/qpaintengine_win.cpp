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

#include "qbitmap.h"
#include "qbrush.h"
#include "qcolormap.h"
#include "qlibrary.h"
#include "qpaintdevice.h"
#include "qpainter.h"
#include "qpainter_p.h"
#include "qpainterpath.h"
#include "qpainterpath_p.h"
#include "qpen.h"
#include "qpixmap.h"
#include "qpixmapcache.h"
#include "qpolygon.h"
#include "qt_windows.h"
#include "qtextlayout.h"
#include "qvarlengtharray.h"
#include "qwidget.h"

#include <private/qfontengine_p.h>
#include <private/qpaintengine_win_p.h>
#include <private/qtextengine_p.h>
#include <private/qwidget_p.h>

#include <qdebug.h>

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979
#endif

//## this is only defined if winver > 5 on mingw
#ifndef GRADIENT_FILL_TRIANGLE
#define GRADIENT_FILL_TRIANGLE 0x02
#endif

#define QT_NO_NATIVE_XFORM
// #define QT_NO_NATIVE_GRADIENT
// #define QT_NO_NATIVE_PATH
// #define QT_NO_NATIVE_ALPHA

// #define QT_DEBUG_DRAW

#define d d_func()
#define q q_func()

// For alpha blending, we must load the AlphaBlend() function at run time.
#if !defined (AC_SRC_ALPHA)
#define AC_SRC_ALPHA 0x01
#endif

#if !defined (SHADEBLENDCAPS)
#define SHADEBLENDCAPS 120
#endif

#if !defined (SB_CONST_ALPHA)
#define SB_CONST_ALPHA 0x00000001
#endif

#if !defined (SB_GRAD_TRI)
#define SB_GRAD_TRI         0x00000020
#endif

Q_DECLARE_TYPEINFO(POINT, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(RECT, Q_PRIMITIVE_TYPE);

static void qt_resolve_gdiplus();
static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features();
static void qMaskedBlt(HDC hdcDest, int x, int y, int w, int h,
                       HDC hdcSrc, int sx, int sy,
                       HDC hdcMask);

#define COLOR_VALUE(c) RGB(c.red(),c.green(),c.blue())

static HPEN stock_nullPen;
static HPEN stock_blackPen;
static HPEN stock_whitePen;
static HBRUSH stock_nullBrush;
static HBRUSH stock_blackBrush;
static HBRUSH stock_whiteBrush;
static HFONT  stock_sysfont;

/********************************************************************************
 * GDI functions we need to dynamically resolve due for 9x based.
 */

typedef int (__stdcall *PtrModifyWorldTransform)(HDC hdc, const XFORM *, uint);
typedef int (__stdcall *PtrSetGraphicsMode)(HDC hdc, uint);
typedef int (__stdcall *PtrSetWorldTransform)(HDC hdc, const XFORM *);

typedef int (__stdcall *PtrAlphaBlend)(HDC, int, int, int, int,
                                           HDC, int, int, int, int,
                                           BLENDFUNCTION);
typedef int (__stdcall *PtrGradientFill)(HDC, PTRIVERTEX, ulong, void *, ulong, ulong);

static PtrModifyWorldTransform qModifyWorldTransform = 0;
static PtrSetGraphicsMode qSetGraphicsMode = 0;
static PtrSetWorldTransform qSetWorldTransform = 0;

static PtrAlphaBlend qAlphaBlend = 0;
static PtrGradientFill qGradientFill = 0;


void qt_resolve_gdi_functions()
{
    static bool resolved = false;
    if (resolved)
        return;
    resolved = true;

    QLibrary gdi32("gdi32");
    gdi32.load();
    if (gdi32.isLoaded()) {
        qSetGraphicsMode = (PtrSetGraphicsMode) gdi32.resolve("SetGraphicsMode");
        qSetWorldTransform = (PtrSetWorldTransform) gdi32.resolve("SetWorldTransform");
        qModifyWorldTransform = (PtrModifyWorldTransform) gdi32.resolve("ModifyWorldTransform");
    }

    QLibrary img32("msimg32");
    img32.load();
    if (img32.isLoaded()) {
        qAlphaBlend = (PtrAlphaBlend) img32.resolve("AlphaBlend");
        qGradientFill = (PtrGradientFill) img32.resolve("GradientFill");
    }

#ifdef QT_DEBUG_DRAW
    printf("qSetGraphicsMode..........: %p\n", qSetGraphicsMode);
    printf("qSetWorldTransform........: %p\n", qSetWorldTransform);
    printf("qModifyWorldTransform.....: %p\n", qModifyWorldTransform);
    printf("qAlphaBlend...............: %p\n", qAlphaBlend);
#endif
}

QWin32PaintEngine::QWin32PaintEngine(QWin32PaintEnginePrivate &dptr,
                                     PaintEngineFeatures caps)
    : QPaintEngine(dptr, caps)
{
    d->flags |= IsStartingUp;
}

QWin32PaintEngine::QWin32PaintEngine()
    : QPaintEngine(*(new QWin32PaintEnginePrivate), qt_decide_paintengine_features())
{
    d->flags |= IsStartingUp;
}

QWin32PaintEngine::~QWin32PaintEngine()
{
}

bool QWin32PaintEngine::begin(QPaintDevice *pdev)
{
    if (isActive()) {                                // already active painting
        qWarning("QWin32PaintEngine::begin: Painter is already active."
               "\n\tYou must end() the painter before a second begin()\n");
        return true;
    }

    // Set up the polygon clipper.
    const int BUFFERZONE = 100;
    d->polygonClipper.setBoundingRect(QRect(-BUFFERZONE,
                                            -BUFFERZONE,
                                            pdev->width() + 2*BUFFERZONE,
                                            pdev->height() + 2 * BUFFERZONE));

    d->ellipseHack = false;
    d->advancedMode = false;

    setActive(true);

    Q_ASSERT(pdev);
    if (!d->hdc) {
        d->hdc = pdev->getDC();
        if (pdev->devType() == QInternal::Widget) {
            QWidget *w = static_cast<QWidget *>(pdev);
            d->usesWidgetDC = (d->hdc != 0);
            if (!d->usesWidgetDC) {
                if (w->testAttribute(Qt::WA_PaintUnclipped)) {
                    d->hdc = GetWindowDC(w->winId());
                    if (w->isWindow()) {
                        int dx = w->geometry().x() - w->frameGeometry().x();
                        int dy = w->geometry().y() - w->frameGeometry().y();
#ifndef Q_OS_TEMP
                        SetWindowOrgEx(d->hdc, -dx, -dy, 0);
#else
                        //                    MoveWindow(w->winId(), w->frameGeometry().x(), w->frameGeometry().y(), w->frameGeometry().width(), w->frameGeometry().height(), false);
                        //                    MoveWindow(w->winId(), w->frameGeometry().x() - 50, w->frameGeometry().y() - 50, w->frameGeometry().width(), w->frameGeometry().height(), false);
#endif
                    }
                } else {
                    d->hdc = GetDC((w->windowType() == Qt::Desktop) ? 0 : w->winId());
                }
                const_cast<QWidgetPrivate *>(w->d)->hd = (Qt::HANDLE)d->hdc;
            }
        }
        Q_ASSERT(d->hdc);
    }

    QRegion region = systemClip();
    if (!region.isEmpty())
        SelectClipRgn(d->hdc, region.handle());

    HPALETTE hpal = QColormap::hPal();
    if (hpal) {
        d->holdpal = SelectPalette(d->hdc, hpal, true);
        RealizePalette(d->hdc);
    }

    SetBkMode(d->hdc, TRANSPARENT);
    SetTextAlign(d->hdc, TA_BASELINE);
    SetTextColor(d->hdc, RGB(0, 0, 0));
    SelectObject(d->hdc, stock_nullBrush);
    SelectObject(d->hdc, stock_blackPen);

    setDirty(QPaintEngine::DirtyBackground);
    setDirty(QPaintEngine::DirtyBrush);

    d->pColor = RGB(0, 0, 0);
    d->bColor = RGB(0, 0, 0);

    // force a call to switch advanced mode on/off
    d->setNativeMatrix(QMatrix());

    return true;
}

bool QWin32PaintEngine::end()
{
    if (!isActive()) {
        qWarning("QWin32PaintEngine::end: Missing begin() or begin() failed");
        return false;
    }

    if (d->hpen) {
        SelectObject(d->hdc, stock_blackPen);
        DeleteObject(d->hpen);
        d->hpen = 0;
    }
    if (d->hbrush) {
        SelectObject(d->hdc, stock_nullBrush);
        DeleteObject(d->hbrush);
        if (d->hbrushbm && !d->pixmapBrush)
            DeleteObject(d->hbrushbm);
        d->hbrush = 0;
        d->hbrushbm = 0;
        d->pixmapBrush = d->nocolBrush = false;
    }

    SelectObject(d->hdc, stock_sysfont);

    if (d->holdpal) {
        SelectPalette(d->hdc, d->holdpal, true);
        RealizePalette(d->hdc);
    }

    if (d->pdev->devType() == QInternal::Widget && !d->usesWidgetDC) {
        QWidget *w = static_cast<QWidget*>(d->pdev);
        ReleaseDC((w->windowType() == Qt::Desktop) ? 0 : w->winId(), d->hdc);
        const_cast<QWidgetPrivate*>(w->d)->hd = 0;
    } else {
        d->pdev->releaseDC(d->hdc);
    }

    SelectClipRgn(d->hdc, 0);

    d->hdc = 0;

    d->matrix = QMatrix();
    d->noNativeXform = false;
    d->advancedMode = false;
    d->penStyle = Qt::SolidLine;
    d->brushStyle = Qt::SolidPattern;
    d->txop = QPainterPrivate::TxNone;

    setActive(false);
    return true;
}

void QWin32PaintEngine::drawPath(const QPainterPath &p)
{
#ifdef QT_NO_NATIVE_PATH
    Q_ASSERT(!"QWin32PaintEngine::drawPath(), QT_NO_NATIVE_PATH is defined...\n");
    return;
#endif

    d->composeGdiPath(p);

    if (p.fillRule() == Qt::WindingFill)
        SetPolyFillMode(d->hdc, WINDING);

    bool pen = d->penStyle != Qt::NoPen;
    bool brush = d->brushStyle != Qt::NoBrush;

    // Workaround for filling gradients
    if (d->brushStyle == Qt::LinearGradientPattern
        || (d->brushStyle == Qt::SolidPattern && d->brush.color().alpha() != 255)) {
        HRGN oldRegion = 0;
        int gotRegion = GetClipRgn(d->hdc, oldRegion);
        SelectClipPath(d->hdc, RGN_AND);
        if (d->brushStyle == Qt::LinearGradientPattern)
            d->fillGradient(p.boundingRect().toRect());
        else
            d->fillAlpha(p.boundingRect().toRect(), d->brush.color());
        brush = false;
        if (gotRegion <= 0) { // No path originally
            SelectClipRgn(d->hdc, 0);
        } else if (gotRegion == 1){ // Reset and release original path
            Q_ASSERT(oldRegion);
            SelectClipRgn(d->hdc, oldRegion);
            DeleteObject(oldRegion);
        }
        if (pen)
            d->composeGdiPath(p);
    }

    if (pen && brush)
        StrokeAndFillPath(d->hdc);
    else if (pen)
        StrokePath(d->hdc);
    else if (brush)
        FillPath(d->hdc);

    if (p.fillRule() == Qt::WindingFill)
        SetPolyFillMode(d->hdc, ALTERNATE);
}


void QWin32PaintEngine::initialize()
{
    qt_resolve_gdi_functions();
    stock_nullPen    = (HPEN)GetStockObject(NULL_PEN);
    stock_blackPen   = (HPEN)GetStockObject(BLACK_PEN);
    stock_whitePen   = (HPEN)GetStockObject(WHITE_PEN);
    stock_nullBrush  = (HBRUSH)GetStockObject(NULL_BRUSH);
    stock_blackBrush = (HBRUSH)GetStockObject(BLACK_BRUSH);
    stock_whiteBrush = (HBRUSH)GetStockObject(WHITE_BRUSH);
    stock_sysfont    = (HFONT)GetStockObject(SYSTEM_FONT);
}


void QWin32PaintEngine::cleanup()
{
}


void QWin32PaintEngine::drawPixmap(const QRectF &rf, const QPixmap &sourcePm, const QRectF &srf,
                                   Qt::PixmapDrawingMode mode)
{
#if defined QT_DEBUG_DRAW
    printf(" - QWin32PaintEngine::drawPixmap(), [%.2f,%.2f,%.2f,%.2f], size=[%d,%d], depth=%d, "
           "sr=[%.2f,%.2f,%.2f,%.2f], mode=%d\n",
           rf.x(), rf.y(), rf.width(), rf.height(),
           sourcePm.width(), sourcePm.height(), sourcePm.depth(),
           srf.x(), srf.y(), srf.width(), srf.height(),
           mode);
#endif

    QRect r = rf.toRect();
    QRect sr = srf.toRect();

    Q_ASSERT(isActive());

    bool stretch = r.width() != sr.width() || r.height() != sr.height();

    const QPixmap *pixmap = &sourcePm;
    bool srcIsAlpha = pixmap->hasAlphaChannel();
    bool targetIsAlpha = d->pdev->devType() == QInternal::Pixmap
                         && static_cast<QPixmap *>(d->pdev)->hasAlphaChannel();

    // Source needs to be alpha if target is.
    if (targetIsAlpha && !srcIsAlpha) {
        QString key = QString("qt_src_not_alpha: sn=%1").arg(pixmap->serialNumber());
        QPixmap *cached = QPixmapCache::find(key);
        if (!cached) {
            QImage im = pixmap->toImage();
            im = im.convertDepth(32);
            im.setAlphaBuffer(true);
            QPixmap tmp = im;
            QPixmapCache::insert(key, tmp);
            pixmap = QPixmapCache::find(key);
            Q_ASSERT_X(pixmap,
                       "QWin32PaintEngine::drawPixmap()",
                       "We just inserted pixmap into cache so it should have been there");
        }
    }

    bool alphaPen = d->pen.color().alpha() != 255;
    bool win9x = QSysInfo::WindowsVersion & QSysInfo::WV_DOS_based;

    if (pixmap->depth() == 1 && (alphaPen || win9x)) {
        QRegion region(*static_cast<const QBitmap*>(pixmap));
        region.translate(qRound(r.x()), qRound(r.y()));
        updateClipRegion(region, Qt::IntersectClip);
        if (alphaPen) {
            d->fillAlpha(r, d->pen.color());
        } else {
            // Since qMaskedBlt doesn't handle bitmaps too well.
            updateBrush(d->pen.color(), QPointF());
            updatePen(Qt::NoPen);
            drawRects(&r, 1);
            setDirty(QPaintEngine::DirtyPen|QPaintEngine::DirtyBrush);
        }
        setDirty(DirtyClip);
        return;
    }

    QPixmap *pm = (QPixmap*)pixmap;
    QBitmap *mask = (QBitmap*)pm->mask();
    if (!mask && pixmap->depth() == 1 && d->bgMode == Qt::TransparentMode)
        mask = (QBitmap*) pm;

    HDC pm_dc = pm->getDC();

    if (mode == Qt::CopyPixmapNoMask)
        mask = 0;

    Q_ASSERT(pm_dc);

    if (qAlphaBlend && (srcIsAlpha || targetIsAlpha) && mode == Qt::ComposePixmap) {
        const BLENDFUNCTION bf = { AC_SRC_OVER,       // BlendOp
                                   0,                 // BlendFlags, must be zero
                                   255,               // SourceConstantAlpha, we use pr pixel
                                   AC_SRC_ALPHA       // AlphaFormat
        };
        if (!qAlphaBlend(d->hdc, r.x(), r.y(), r.width(), r.height(),
                         pm_dc, sr.x(), sr.y(), sr.width(), sr.height(), bf)) {
            qErrnoWarning("QWin32PaintEngine::drawPixmap, AlphaBlend failed");
        }
    } else if (mask && mode == Qt::ComposePixmap) {
        if (stretch) {
            QImage imageData = pixmap->toImage();
            QImage imageMask = imageData.createAlphaMask();
            QBitmap tmpbm = imageMask;
            QBitmap bm(sr.width(), sr.height());
            {
                QPainter p(&bm);
                p.drawPixmap(QRectF(0, 0, srf.width(), srf.height()).toRect(),
                             tmpbm, sr, Qt::CopyPixmapNoMask);
            }
            QMatrix xform = QMatrix(r.width()/(double)sr.width(), 0,
                                    0, r.height()/(double)sr.height(),
                                    0, 0);
            bm = bm.transform(xform);
            QRegion region(bm);
            region.translate(r.x(), r.y());
            if (state->painter->hasClipping())
                region &= state->painter->clipRegion();
            state->painter->save();
            state->painter->setClipRegion(region);
            updateState(state);
            StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
                       pm_dc, sr.x(), sr.y(), sr.width(), sr.height(),
                       SRCCOPY);
            state->painter->restore();
        } else {
            QT_WA( {
                if (!MaskBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
                             pm_dc, sr.x(), sr.y(), mask->hbm(), sr.x(), sr.y(),
                             MAKEROP4(0x00aa0000, SRCCOPY)))
                    qErrnoWarning("QWin32PaintEngine::drawPixmap: MaskBlt failed");
            }, {
                HDC maskdc = mask->getDC();
                qMaskedBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
                           pm_dc, sr.x(), sr.y(),
                           maskdc);
                mask->releaseDC(maskdc);
            } );
        }
    } else {
        if (stretch) {
            Q_ASSERT((GetDeviceCaps(d->hdc, RASTERCAPS) & RC_STRETCHBLT) != 0);
            if (!StretchBlt(d->hdc, r.x(), r.y(), r.width(), r.height(),
                            pm_dc, sr.x(), sr.y(), sr.width(), sr.height(),
                            SRCCOPY)) {
                qErrnoWarning("QWin32PaintEngine::drawPixmap: StretchBlt failed");
            }
        } else {
            Q_ASSERT((GetDeviceCaps(d->hdc, RASTERCAPS) & RC_BITBLT) != 0);
            if (!BitBlt(d->hdc, r.x(), r.y(), sr.width(), sr.height(),
                        pm_dc, sr.x(), sr.y(),
                        SRCCOPY)) {
                qErrnoWarning("QWin32PaintEngine::drawPixmap: BitBlt failed");
            }
        }
    }
    pm->releaseDC(pm_dc);
}

void QWin32PaintEngine::drawTextItem(const QPointF &pos, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    if (!ti.num_glyphs)
        return;

    switch(ti.fontEngine->type()) {
    case QFontEngine::Multi:
        drawTextItemMulti(pos, ti);
        break;
    case QFontEngine::Win:
    default:
        drawTextItemWin(pos, ti);
        break;
    }
}

void QWin32PaintEngine::drawTextItemMulti(const QPointF &p, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);
    QFontEngineMulti *multi = static_cast<QFontEngineMulti *>(ti.fontEngine);
    QGlyphLayout *glyphs = ti.glyphs;
    int which = glyphs[0].glyph >> 24;

    qreal x = p.x();
    qreal y = p.y();

    int start = 0;
    int end, i;
    for (end = 0; end < ti.num_glyphs; ++end) {
        const int e = glyphs[end].glyph >> 24;
        if (e == which)
            continue;

        // set the high byte to zero
        for (i = start; i < end; ++i)
            glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

        // draw the text
        QTextItemInt ti2 = ti;
        ti2.glyphs = ti.glyphs + start;
        ti2.num_glyphs = end - start;
        ti2.fontEngine = multi->engine(which);
        ti2.f = ti.f;
        drawTextItem(QPointF(x, y), ti2);

        // reset the high byte for all glyphs and advance to the next sub-string
        const int hi = which << 24;
        for (i = start; i < end; ++i) {
            glyphs[i].glyph = hi | glyphs[i].glyph;
            x += glyphs[i].advance.x();
        }

        // change engine
        start = end;
        which = e;
    }

    // set the high byte to zero
    for (i = start; i < end; ++i)
        glyphs[i].glyph = glyphs[i].glyph & 0xffffff;

    // draw the text
    QTextItemInt ti2 = ti;
    ti2.glyphs = ti.glyphs + start;
    ti2.num_glyphs = end - start;
    ti2.fontEngine = multi->engine(which);
    ti2.f = ti.f;
    drawTextItem(QPointF(x,y), ti2);

    // reset the high byte for all glyphs
    const int hi = which << 24;
    for (i = start; i < end; ++i)
        glyphs[i].glyph = hi | glyphs[i].glyph;
}

void QWin32PaintEngine::drawTextItemWin(const QPointF &pos, const QTextItem &textItem)
{
    const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

#ifdef QT_DEBUG_DRAW
        printf(" - QWin32PaintEngine::drawTextItem(), (%.2f,%.2f), string=%s\n",
               pos.x(), pos.y(), QString::fromRawData(ti.chars, ti.num_chars).toLatin1().data());
#endif

    QPointF p = pos;

    if (d->txop > QPainterPrivate::TxTranslate) {
        d->setNativeMatrix(d->matrix);
    } else if (d->txop == QPainterPrivate::TxTranslate) {
        p = p * d->matrix;
    }

    QFontEngine *fe = ti.fontEngine;
    QPainterState *state = painterState();

    double scale = 1.;
    int angle = 0;
    bool transform = false;
    qreal x = p.x();
    qreal y = p.y();

    if (state->txop >= QPainterPrivate::TxScale
        && !(QSysInfo::WindowsVersion & QSysInfo::WV_NT_based)) {
        // Draw rotated and sheared text on Windows 95, 98

        // All versions can draw rotated text natively. Scaling can be done with window/viewport transformations.
        // Shearing transformations are done by QPainter.

        // rotation + scale + translation
        scale = sqrt(state->matrix.m11()*state->matrix.m22()
                      - state->matrix.m12()*state->matrix.m21());
        angle = qRound(1800*acos(state->matrix.m11()/scale)/M_PI);
        if (state->matrix.m12() < 0)
            angle = 3600 - angle;

        transform = true;
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut) || scale != 1. || angle) {
        LOGFONT lf = fe->logfont;
        lf.lfUnderline = (ti.flags & QTextItem::Underline);
        lf.lfStrikeOut = (ti.flags & QTextItem::StrikeOut);
        if (angle) {
            lf.lfOrientation = -angle;
            lf.lfEscapement = -angle;
        }
        if (scale != 1.) {
            lf.lfHeight = (int) (lf.lfHeight*scale);
            lf.lfWidth = (int) (lf.lfWidth*scale);
        }
        HFONT hf = QT_WA_INLINE(CreateFontIndirectW(&lf), CreateFontIndirectA((LOGFONTA*)&lf));
        SelectObject(d->hdc, hf);
    } else {
        SelectObject(d->hdc, fe->hfont);
    }

    unsigned int options =  fe->ttf ? ETO_GLYPH_INDEX : 0;

    QGlyphLayout *glyphs = ti.glyphs;

#if 0
    // ###### should be moved to the printer GC
    if(p->pdev->devType() == QInternal::Printer) {
        // some buggy printer drivers can't handle glyph indices correctly for latin1
        // If the string is pure latin1, we output the string directly, not the glyph indices.
        // There must be a better way to get this working, but currently I can't think of one.
        const QChar *uc = engine->string.unicode() + ti.position;
        int l = engine->length(si - &engine->items[0]);
        int i = 0;
        bool latin = (l == ti.num_glyphs);
        while (latin && i < l) {
            if(uc[i].unicode() >= 0x100)
                latin = false;
            ++i;
        }
        if(latin) {
            glyphs = (glyph_t *)uc;
            options = 0;
        }
    }
#endif

    int xo = qRound(x);

    if (!(ti.flags & QTextItem::RightToLeft)) {
        // hack to get symbol fonts working on Win95. See also QFontEngine constructor
        if (fe->useTextOutA) {
            // can only happen if !ttf
            for(int i = 0; i < ti.num_glyphs; i++) {
                QString str(QChar(glyphs->glyph));
                QByteArray cstr = str.toLocal8Bit();
                TextOutA(d->hdc, qRound(x + glyphs->offset.x()), qRound(y + glyphs->offset.y()),
                         cstr.data(), cstr.length());
                x += qRound(glyphs->advance.x());
                glyphs++;
            }
        } else {
            bool haveOffsets = true;
            qreal w = 0;
            for(int i = 0; i < ti.num_glyphs; i++) {
                if (glyphs[i].offset.x() != 0 || glyphs[i].offset.y() != 0 || glyphs[i].space_18d6 != 0) {
                    haveOffsets = true;
                    break;
                }
                w += glyphs[i].advance.x();
            }

            if (haveOffsets || transform) {
                for(int i = 0; i < ti.num_glyphs; i++) {
                    wchar_t chr = glyphs->glyph;
                    qreal xp = x + glyphs->offset.x();
                    qreal yp = y + glyphs->offset.y();
                    if (transform)
                        state->painter->matrix().map(xp, yp, &xp, &yp);
                    ExtTextOutW(d->hdc, qRound(xp), qRound(yp), options, 0, &chr, 1, 0);
                    x += glyphs->advance.x() + ((qreal)glyphs->space_18d6) / 64.;
                    y += glyphs->advance.y();
                    glyphs++;
                }
            } else {
                // fast path
                QVarLengthArray<wchar_t> g(ti.num_glyphs);
                for (int i = 0; i < ti.num_glyphs; ++i)
                    g[i] = glyphs[i].glyph;
                // fast path
                ExtTextOutW(d->hdc,
                            qRound(x + glyphs->offset.x()),
                            qRound(y + glyphs->offset.y()),
                            options, 0, g.data(), ti.num_glyphs, 0);
                x += w;
            }
        }
    } else {
        int i = ti.num_glyphs;
        while(i--) {
            x += glyphs[i].advance.x() + ((qreal)glyphs[i].space_18d6) / 64.;
            y += glyphs[i].advance.y();
        }
        i = 0;
        while(i < ti.num_glyphs) {
            x -= glyphs[i].advance.x();
            y -= glyphs[i].advance.y();

            int xp = qRound(x+glyphs[i].offset.x());
            int yp = qRound(y+glyphs[i].offset.y());
            ExtTextOutW(d->hdc, xp, yp, options, 0, reinterpret_cast<wchar_t *>(&glyphs[i].glyph), 1, 0);

            if (glyphs[i].nKashidas) {
                QChar ch(0x640); // Kashida character
                QGlyphLayout g[8];
                int nglyphs = 7;
                ti.fontEngine->stringToCMap(&ch, 1, g, &nglyphs, 0);
                for (uint k = 0; k < glyphs[i].nKashidas; ++k) {
                    x -= g[0].advance.x();
                    y -= g[0].advance.y();

                    int xp = qRound(x+g[0].offset.x());
                    int yp = qRound(y+g[0].offset.y());
                    ExtTextOutW(d->hdc, xp, yp, options, 0, reinterpret_cast<wchar_t *>(&g[0].glyph), 1, 0);
                }
            } else {
                x -= ((qreal)glyphs[i].space_18d6) / 64;
            }
            ++i;
        }
    }

    if (ti.flags & (QTextItem::Underline|QTextItem::StrikeOut) || scale != 1. || angle)
        DeleteObject(SelectObject(d->hdc, fe->hfont));

    if (ti.flags & (QTextItem::Overline)) {
        int lw = qRound(fe->lineThickness());
        int yp = qRound(y - fe->ascent() - 1);
        Rectangle(d->hdc, xo, yp, qRound(x), yp + lw);

    }

    if (d->txop >= QPainterPrivate::TxTranslate)
        d->setNativeMatrix(QMatrix());
}

void QWin32PaintEngine::updatePen(const QPen &pen)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updatePen(), style=%d, color=%p, calls=%d\n",
           pen.style(), pen.color().rgba(), ++counter);
#endif
    d->pen = pen;
    d->penStyle = pen.style();

    d->pColor = COLOR_VALUE(pen.color());
    d->pWidth = pen.width();
    HANDLE oldPen = d->hpen;



    int s;

    switch (d->penStyle) {
    case Qt::NoPen:             s = PS_NULL;        break;
    case Qt::SolidLine:         s = PS_SOLID;        break;
    case Qt::DashLine:          s = PS_DASH;        break;
#ifndef Q_OS_TEMP
    case Qt::DotLine:           s = PS_DOT;         break;
    case Qt::DashDotLine:            s = PS_DASHDOT;         break;
    case Qt::DashDotDotLine:    s = PS_DASHDOTDOT;         break;
#endif
        default:
            s = PS_SOLID;
            qWarning("QPainter::updatePen: Invalid pen style");
    }
#ifndef Q_OS_TEMP
    if (((pen.width() != 0) || pen.width() > 1) &&
         (QSysInfo::WindowsVersion & QSysInfo::WV_NT_based || d->penStyle == Qt::SolidLine)) {
        LOGBRUSH lb;
        lb.lbStyle = 0;
        lb.lbColor = d->pColor;
        lb.lbHatch = 0;
        int pst = PS_GEOMETRIC | s;
        switch (pen.capStyle()) {
            case Qt::SquareCap:
                pst |= PS_ENDCAP_SQUARE;
                break;
            case Qt::RoundCap:
                pst |= PS_ENDCAP_ROUND;
                break;
            case Qt::FlatCap:
                pst |= PS_ENDCAP_FLAT;
                break;
	    default:
                pst |= PS_ENDCAP_SQUARE;
                qWarning("QPainter::updatePen: Invalid cap style");
        }
        switch (pen.joinStyle()) {
            case Qt::BevelJoin:
                pst |= PS_JOIN_BEVEL;
                break;
            case Qt::RoundJoin:
                pst |= PS_JOIN_ROUND;
                break;
            case Qt::MiterJoin:
                pst |= PS_JOIN_MITER;
                break;
	    default:
                pst |= PS_JOIN_BEVEL;
                qWarning("QPainter::updatePen: Invalid join style");
        }
        d->hpen = ExtCreatePen(pst, (DWORD)d->pWidth, &lb, 0, 0);
    }
    else
#endif
    {
        d->hpen = CreatePen(s, qRound(pen.width()), d->pColor);
    }

    SetTextColor(d->hdc, d->pColor);
    SelectObject(d->hdc, d->hpen);

    if (oldPen)
        DeleteObject(oldPen);

    return;
}


void QWin32PaintEngine::updateBrush(const QBrush &brush, const QPointF &bgOrigin)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updateBrush(), style=%d, color=%p, calls=%d\n",
           brush.style(), brush.color().rgba(), ++counter);
#endif
    d->brush = brush;
    d->brushStyle = brush.style();

#ifndef Q_OS_TEMP
    static short d1_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static short d2_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static short d3_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static short d4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static short d5_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static short d6_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static short d7_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static short *dense_patterns[]
        = { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };
#else
    static DWORD d1_pat[]     = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static DWORD d2_pat[]     = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static DWORD d3_pat[]     = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static DWORD d4_pat[]     = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static DWORD d5_pat[]     = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static DWORD d6_pat[]     = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static DWORD d7_pat[]     = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static DWORD *dense_patterns[]
        = { d1_pat, d2_pat, d3_pat, d4_pat, d5_pat, d6_pat, d7_pat };

    static DWORD hor_pat[]    = { 0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0xff };
    static DWORD ver_pat[]    = { 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef, 0xef };
    static DWORD cross_pat[]  = { 0xef, 0xef, 0xef, 0x00, 0xef, 0xef, 0xef, 0xef };
    static DWORD bdiag_pat[]  = { 0x7f, 0xbf, 0xdf, 0xef, 0xf7, 0xfb, 0xfd, 0xfe };
    static DWORD fdiag_pat[]  = { 0xfe, 0xfd, 0xfb, 0xf7, 0xef, 0xdf, 0xbf, 0x7f };
    static DWORD dcross_pat[] = { 0x7e, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0x7e };
    static DWORD *hatch_patterns[]
        = { hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };
#endif

    d->bColor           = COLOR_VALUE(brush.color());
    HBRUSH oldBrush = d->hbrush;
    HBITMAP hbrushbm_old    = d->hbrushbm;
    bool    pixmapBrush_old = d->pixmapBrush;

    d->pixmapBrush = d->nocolBrush = false;
    d->hbrushbm = 0;

    if (d->brushStyle == Qt::NoBrush) {
        d->hbrush = stock_nullBrush;
    } else if (d->brushStyle == Qt::SolidPattern) {                        // create solid brush
        d->hbrush = CreateSolidBrush(d->bColor);
#ifndef Q_OS_TEMP
    } else if ((d->brushStyle >= Qt::Dense1Pattern && d->brushStyle <= Qt::Dense7Pattern) ||
                (d->brushStyle == Qt::TexturePattern)) {
        if (d->brushStyle == Qt::TexturePattern) {
            // The brush pixmap can never be a multi cell pixmap
            d->hbrushbm = brush.texture().hbm();
            d->pixmapBrush = true;
            d->nocolBrush = brush.texture().depth() == 1;
        } else {
            short *bm = dense_patterns[d->brushStyle - Qt::Dense1Pattern];
            d->hbrushbm = CreateBitmap(8, 8, 1, 1, bm);
            d->nocolBrush = true;
        }
        d->hbrush = CreatePatternBrush(d->hbrushbm);
        if (!d->pixmapBrush) // hbm is owned by the pixmap, and will be deleted by it later.
            DeleteObject(d->hbrushbm);
        d->hbrushbm = 0;
    } else if (d->brushStyle == Qt::LinearGradientPattern) {
        d->hbrush = stock_nullBrush;
    } else {                                        // one of the hatch brushes
        int s;
        switch (d->brushStyle) {
            case Qt::HorPattern:
                s = HS_HORIZONTAL;
                break;
            case Qt::VerPattern:
                s = HS_VERTICAL;
                break;
            case Qt::CrossPattern:
                s = HS_CROSS;
                break;
            case Qt::BDiagPattern:
                s = HS_BDIAGONAL;
                break;
            case Qt::FDiagPattern:
                s = HS_FDIAGONAL;
                break;
            case Qt::DiagCrossPattern:
                s = HS_DIAGCROSS;
                break;
            default:
                s = HS_HORIZONTAL;
        }
        d->hbrush = CreateHatchBrush(s, d->bColor);
    }
#else
    } else {
        if (d->brushStyle == Qt::TexturePattern) {
            // The brush pixmap can never be a multi cell pixmap
            d->hbrushbm = brush.pixmap()->hbm();
            d->pixmapBrush = true;
            d->nocolBrush = brush.pixmap()->depth() == 1;
            d->hbrush = CreatePatternBrush(d->hbrushbm);
            DeleteObject(d->hbrushbm);
        } else {
            struct {
                BITMAPINFOHEADER bmi;
                COLORREF palette[2];
                DWORD bitmapData[8];

            } bitmapBrush;

            memset(&bitmapBrush, 0, sizeof(bitmapBrush));

            bitmapBrush.bmi.biSize     = sizeof(BITMAPINFOHEADER);
            bitmapBrush.bmi.biWidth    =
            bitmapBrush.bmi.biHeight   = 8;
            bitmapBrush.bmi.biPlanes   =
            bitmapBrush.bmi.biBitCount = 1;
            bitmapBrush.bmi.biClrUsed  = 0;
            QRgb *coltbl = (QRgb*)bitmapBrush.palette;
            coltbl[0] = d->bColor.rgba();
            coltbl[1] = Qt::color0.rgba();

            static DWORD *pattern = hatch_patterns[0]; // Qt::HorPattern

            if (d->brushStyle >= Qt::Dense1Pattern && d->brushStyle <= Qt::Dense7Pattern)
                pattern = dense_patterns[d->brushStyle - Qt::Dense1Pattern];
            else if (d->brushStyle >= Qt::HorPattern && d->brushStyle <= Qt::DiagCrossPattern)
                pattern = hatch_patterns[d->brushStyle - Qt::HorPattern];

            memcpy(bitmapBrush.bitmapData, pattern, 64);
            d->hbrush = CreateDIBPatternBrushPt(&bitmapBrush, DIB_RGB_COLORS);
        }
    }
#endif
    SetBrushOrgEx(d->hdc, qRound(bgOrigin.x()), qRound(bgOrigin.y()), 0);
    SelectObject(d->hdc, d->hbrush);

    if (oldBrush && oldBrush != stock_nullBrush) {
        if (hbrushbm_old && !pixmapBrush_old)
            DeleteObject(hbrushbm_old);        // delete last brush pixmap
        DeleteObject(oldBrush);              // delete last brush
    }
}

void QWin32PaintEngine::updateBackground(Qt::BGMode mode, const QBrush &bgBrush)
{
    Q_ASSERT(isActive());
    d->bgMode = mode;

    SetBkColor(d->hdc, COLOR_VALUE(bgBrush.color()));
    SetBkMode(d->hdc, mode == Qt::TransparentMode ? TRANSPARENT : OPAQUE);
}


void QWin32PaintEngine::updateMatrix(const QMatrix &mtx)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updateMatrix(), [%.1f %.1f %.1f %.1f %.1f %.1f], calls=%d\n",
           mtx.m11(), mtx.m12(), mtx.m21(), mtx.m22(), mtx.dx(), mtx.dy(), ++counter);
#endif

    if (mtx.m12() != 0 || mtx.m21() != 0)
        d->txop = QPainterPrivate::TxRotShear;
    else if (mtx.m11() != 1 || mtx.m22() != 1)
        d->txop = QPainterPrivate::TxScale;
    else if (mtx.dx() != 0 || mtx.dy() != 0)
        d->txop = QPainterPrivate::TxTranslate;
    else
        d->txop = QPainterPrivate::TxNone;

    d->matrix = mtx;
    d->invMatrix = mtx.inverted();

#ifndef QT_NO_NATIVE_XFORM
    d->setNativeMatrix(mtx);
#endif
}

static const int qt_clip_operations[] = {
    0,              // Qt::NoClip
    RGN_COPY,       // Qt::ReplaceClip
    RGN_AND,        // Qt::IntersectClip
    RGN_OR          // Qt::UniteClip
};

#ifdef QT_DEBUG_DRAW
static const char *qt_clip_operation_names[] = {
    "NoClip",
    "ReplaceClip",
    "IntersectClip",
    "UniteClip"
};
#endif

void QWin32PaintEngine::updateClipRegion(const QRegion &region, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    static int counter = 0;
    printf(" - QWin32PaintEngine::updateClipRegion, size=%d, [%d %d %d %d], %s, calls=%d\n",
           region.rects().size(),
           region.boundingRect().x(),
           region.boundingRect().y(),
           region.boundingRect().width(),
           region.boundingRect().height(),
           qt_clip_operation_names[op],
           ++counter);
#endif
    // Sanity check since we use it blindly below.
    Q_ASSERT(op >= 0 && op <= Qt::UniteClip);

    if (op == Qt::NoClip) {
        SelectClipRgn(d->hdc, 0);
        return;
    }

    QRegion rgn = region;

    // Setting an empty clip region on windows disables clipping, so we do the
    // nice hack of just setting a 1 pixel clip region far away to avoid anything
    // from being drawn.
    if (region.isEmpty())
        rgn = QRegion(-0x1000000, -0x1000000, 1, 1);
    ExtSelectClipRgn(d->hdc, rgn.handle(), qt_clip_operations[op]);
}

void QWin32PaintEngine::updateClipPath(const QPainterPath &path, Qt::ClipOperation op)
{
#ifdef QT_DEBUG_DRAW
    QRectF bounds = path.boundingRect();
    printf(" - QWin32PaintEngine::updateClipPath, size=%d, [%.2f %.2f %.2f %.2f], %s\n",
           path.elementCount(), bounds.x(), bounds.y(), bounds.width(), bounds.height(),
           qt_clip_operation_names[op]);
#endif


#ifdef QT_NO_NATIVE_PATH
    QPaintEngine::updateClipPath(path, op);
    return;
#endif
    // Sanity check since we use it blindly below.
    Q_ASSERT(op >= 0 && op <= Qt::UniteClip);

    if (op == Qt::NoClip) {
        SelectClipRgn(d->hdc, 0);
    } else if (path.isEmpty()) {
        updateClipRegion(QRegion(), op);
    } else {
        d->composeGdiPath(path
#ifndef QT_NO_NATIVE_XFORM
                          * d->matrix
#endif
                          );
        SelectClipPath(d->hdc, qt_clip_operations[op]);
    }
}

void QWin32PaintEngine::updateFont(const QFont &font)
{
    if (state->pfont)
        delete state->pfont;
#undef d
    state->pfont = new QFont(font.d, d_func()->pdev);
#define d d_func()
}

void QWin32PaintEngine::updateRenderHints(QPainter::RenderHints hints)
{
#ifdef QT_DEBUG_DRAW
    printf(" - QWin32PaintEngine::updateRenderHints(), %p\n", hints);
#else
    Q_UNUSED(hints);
#endif
}

QPainter::RenderHints QWin32PaintEngine::supportedRenderHints() const
{
    return 0;
}

HDC QWin32PaintEngine::getDC() const
{
    return d->hdc;
}

void QWin32PaintEngine::releaseDC(HDC) const
{
}

#define COLSCALE(x) uint( (x) * 0xff00 / 255.0 )

inline TRIVERTEX createVertex(int x, int y, const QColor &col)
{
    TRIVERTEX v;
    v.x = x;
    v.y = y;
    v.Alpha = COLSCALE(col.alpha());
    double premultiply = col.alpha() / 255.0;
    v.Red   = COLSCALE(col.red()*premultiply);
    v.Green = COLSCALE(col.green()*premultiply);
    v.Blue  = COLSCALE(col.blue()*premultiply);
    return v;
}

template <class T> void qt_swap(T &a, T &b) { T tmp=a; a=b; b=tmp; }

void QWin32PaintEnginePrivate::fillGradient(const QRect &rect)
{
#ifdef QT_NO_NATIVE_GRADIENT
    Q_ASSERT(!"QWin32PaintEnginePrivate::fillGradient()\n");
#endif
    Q_ASSERT(qGradientFill);

    const QLinearGradient *linGrad = static_cast<const QLinearGradient *>(brush.gradient());

    QColor gcol1 = linGrad->stops().first().second;
    QColor gcol2 = linGrad->stops().last().second;

    bool useMemDC = !(rect.x() == 0
                     && rect.y() == 0
                     && rect.width() == pdev->width()
                     && rect.height() == pdev->height()
                     && gcol1.alpha() == 255
                     && gcol2.alpha() == 255);
    HDC memdc = hdc;
    HBITMAP bitmap;
    if (useMemDC) {
        memdc = CreateCompatibleDC(hdc);
        bitmap = CreateCompatibleBitmap(hdc, rect.width(), rect.height());
        SelectObject(memdc, bitmap);
    }

    Q_ASSERT(brush.style() == Qt::LinearGradientPattern);

    QPointF gstart = linGrad->start();
    QPointF gstop  = linGrad->finalStop();

    gstart -= rect.topLeft();
    gstop -= rect.topLeft();

    int dx = qRound(gstop.x() - gstart.x());
    int dy = qRound(gstop.y() - gstart.y());

    int rw = rect.width();
    int rh = rect.height();

    if (qAbs(dx) > qAbs(dy)) { // Fill horizontally
        // Make sure we fill left to right.
        if (gstop.x() < gstart.x()) {
            qt_swap(gcol1, gcol2);
            qt_swap(gstart, gstop);
        }
        // Find the location where the lines covering the gradient intersect
        // the lines making up the top and bottom of the target rectangle.
        // Note: This might be outside the target rect, but that is ok.
        int xtop1, xtop2, xbot1, xbot2;
        if (dy == 0) {
            xtop1 = xbot1 = qRound(gstart.x());
            xtop2 = xbot2 = qRound(gstop.x());
        } else {
            double gamma = double(dx) / double(-dy);
            xtop1 = qRound((-gstart.y() + gamma * gstart.x() ) / gamma);
            xtop2 = qRound((-gstop.y()  + gamma * gstop.x()  ) / gamma);
            xbot1 = qRound((rh - gstart.y() + gamma * gstart.x() ) / gamma);
            xbot2 = qRound((rh - gstop.y()  + gamma * gstop.x()  ) / gamma);
            Q_ASSERT(xtop2 > xtop1);
        }

        // Fill the area to the left of the gradient
        TRIVERTEX polygon[4];
        int polyCount = 0;
	if (xtop1 > 0)
            polygon[polyCount++] = createVertex(0, 0, gcol1);
        polygon[polyCount++] = createVertex(xtop1+1, 0, gcol1);
        polygon[polyCount++] = createVertex(xbot1+1, rh, gcol1);
        if (xbot1 > 0)
            polygon[polyCount++] = createVertex(0, rh, gcol1);
        GRADIENT_TRIANGLE gtr[] = { { 0, 1, 2 }, { 2, 3, 0 } };
        int gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        // Fill the area to the right of the gradient
        polyCount = 0;
	polygon[polyCount++] = createVertex(xtop2-1, 0, gcol2);
	if (xtop2 < rw)
	    polygon[polyCount++] = createVertex(rw, 0, gcol2);
	if (xbot2 < rw)
	    polygon[polyCount++] = createVertex(rw, rh, gcol2);
	polygon[polyCount++] = createVertex(xbot2-1, rh, gcol2);
        gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polygon[0] = createVertex(xtop1, 0, gcol1);
        polygon[1] = createVertex(xbot1, rh, gcol1);
        polygon[2] = createVertex(xbot2, rh, gcol2);
        polygon[3] = createVertex(xtop2, 0, gcol2);
        qGradientFill(memdc, polygon, 4, gtr, 2, GRADIENT_FILL_TRIANGLE);

    } else {
        // Fill Verticallty
        // Code below is a conceptually equal to the one above except that all
        // coords are swapped x <-> y.
        // Make sure we fill top to bottom...
        if (gstop.y() < gstart.y()) {
            qt_swap(gstart, gstop);
            qt_swap(gcol1, gcol2);
        }
        int yleft1, yleft2, yright1, yright2;
        if (dx == 0) {
            yleft1 = yright1 = qRound(gstart.y());
            yleft2 = yright2 = qRound(gstop.y());
        } else {
            double gamma = double(dy) / double(-dx);
            yleft1 = qRound((-gstart.x() + gamma * gstart.y()) / gamma);
            yleft2 = qRound((-gstop.x() + gamma * gstop.y()) / gamma);
            yright1 = qRound((rw - gstart.x() + gamma*gstart.y()) / gamma);
            yright2 = qRound((rw - gstop.x() + gamma*gstop.y()) / gamma);
            Q_ASSERT(yleft2 > yleft1);
        }

        TRIVERTEX polygon[4];
        int polyCount = 0;
        polygon[polyCount++] = createVertex(0, yleft1+1, gcol1);
	if (yleft1 > 0)
	    polygon[polyCount++] = createVertex(0, 0, gcol1);
	if (yright1 > 0)
	    polygon[polyCount++] = createVertex(rw, 0, gcol1);
	polygon[polyCount++] = createVertex(rw, yright1+1, gcol1);
        GRADIENT_TRIANGLE gtr[] = { { 0, 1, 2 }, { 2, 3, 0 } };
        int gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polyCount = 0;
	polygon[polyCount++] = createVertex(0, yleft2-1, gcol2);
	if (yleft2 < rh)
	    polygon[polyCount++] = createVertex(0, rh, gcol2);
	if (yright2 < rh)
	    polygon[polyCount++] = createVertex(rw, rh, gcol2);
	polygon[polyCount++] = createVertex(rw, yright2-1, gcol2);
        gtrCount = polyCount == 4 ? 2 : 1;
        qGradientFill(memdc, polygon, polyCount, gtr, gtrCount, GRADIENT_FILL_TRIANGLE);

        polygon[0] = createVertex(0, yleft1, gcol1);
        polygon[1] = createVertex(rw, yright1, gcol1);
        polygon[2] = createVertex(rw, yright2, gcol2);
        polygon[3] = createVertex(0, yleft2, gcol2);
        qGradientFill(memdc, polygon, 4, gtr, 2, GRADIENT_FILL_TRIANGLE);
    }

    if (useMemDC) {
        if (gcol1.alpha() != 255 || gcol2.alpha() != 255) {
            BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
            qAlphaBlend(hdc, rect.x(), rect.y(), rw, rh, memdc, 0, 0, rw, rh, bf);
        } else {
            BitBlt(hdc, rect.x(), rect.y(), rw, rh, memdc, 0, 0, SRCCOPY);
        }

        DeleteDC(memdc);
        DeleteObject(bitmap);
    }
}

void QWin32PaintEnginePrivate::fillAlpha(const QRect &r, const QColor &color)
{
#ifdef QT_NO_NATIVE_ALPHA
    Q_ASSERT(!"QWin32PaintEnginePrivate::fillAlpha()\n");
#endif
    Q_ASSERT(qAlphaBlend);

    if (r.isEmpty())
        return;

    HDC memdc = CreateCompatibleDC(hdc);
    HBITMAP bitmap = CreateCompatibleBitmap(hdc, r.width(), r.height());
    SelectObject(memdc, bitmap);

    SelectObject(memdc, CreateSolidBrush(RGB(color.red(), color.green(), color.blue())));
    SelectObject(memdc, stock_nullPen);

    Rectangle(memdc, 0, 0, r.width() + 1, r.height() + 1);

    BLENDFUNCTION bf = { AC_SRC_OVER, 0, color.alpha(), 0 };
    if (!qAlphaBlend(hdc, r.x(), r.y(), r.width(), r.height(), memdc, 0, 0, r.width(), r.height(), bf))
        qErrnoWarning("QWin32PaintEngine::fillAlpha: AlphaBlend failed");

    DeleteObject(SelectObject(memdc, stock_nullBrush));
    DeleteDC(memdc);
    DeleteObject(bitmap);
}

void QWin32PaintEnginePrivate::composeGdiPath(const QPainterPath &path)
{
#ifdef QT_NO_NATIVE_PATH
    Q_ASSERT(!"QWin32PaintEnginePrivate::composeGdiPath()\n");
#endif
    if (!BeginPath(hdc))
        qErrnoWarning("QWin32PaintEngine::drawPath: BeginPath failed");

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
}


void QWin32PaintEnginePrivate::setNativeMatrix(const QMatrix &mtx)
{
    QT_WA( {
        XFORM m;
        if (d->txop > QPainterPrivate::TxNone) {
            m.eM11 = mtx.m11();
            m.eM12 = mtx.m12();
            m.eM21 = mtx.m21();
            m.eM22 = mtx.m22();
            m.eDx  = mtx.dx();
            m.eDy  = mtx.dy();
            if (!qSetGraphicsMode(d->hdc, GM_ADVANCED))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
            if (!SetWorldTransform(d->hdc, &m))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetWorldTransformation failed");
            d->advancedMode = true;
        } else {
            m.eM11 = m.eM22 = 1.0;
            m.eM12 = m.eM21 = m.eDx = m.eDy = 0.0;
            if (!qSetGraphicsMode(d->hdc, GM_ADVANCED))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
            qModifyWorldTransform(d->hdc, &m, MWT_IDENTITY);
            if (!qSetGraphicsMode(d->hdc, GM_COMPATIBLE))
                qErrnoWarning("QWin32PaintEngine::setNativeMatrix(), SetGraphicsMode failed");
            d->advancedMode = false;
        }
    }, {
        // ### How about 9x??
    } );
}


static QPaintEngine::PaintEngineFeatures qt_decide_paintengine_features()
{
    QPaintEngine::PaintEngineFeatures commonFeatures =
        QPaintEngine::UsesFontEngine
        | QPaintEngine::AlphaPixmap
        | QPaintEngine::PixmapScale
#ifndef QT_NO_NATIVE_XFORM
        | QPaintEngine::CoordTransform
        | QPaintEngine::PenWidthTransform
        | QPaintEngine::PixmapTransform
        | QPaintEngine::ClipTransform
#endif

#ifndef QT_NO_NATIVE_PATH
        | QPaintEngine::PainterPaths
#endif
        ;

#ifndef QT_NO_NATIVE_GRADIENT
    if (qGradientFill)
        commonFeatures |= QPaintEngine::LinearGradientFill;
#endif

#ifndef QT_NO_NATIVE_ALPHA
    if (qAlphaBlend)
        commonFeatures |= QPaintEngine::AlphaFill;
#endif

    return commonFeatures;
}

static void qMaskedBlt(HDC hdcDest, int x, int y, int w, int h,
                       HDC hdcSrc, int sx, int sy,
                       HDC hdcMask)
{
    COLORREF oldBgColor = SetBkColor(hdcDest, RGB(255, 255, 255));
    COLORREF oldFgColor = SetTextColor(hdcDest, RGB(0, 0, 0));

    BitBlt(hdcDest, x, y, w, h, hdcSrc, sx, sx, SRCINVERT);
    BitBlt(hdcDest, x, y, w, h, hdcMask, sx, sy, SRCAND);
    BitBlt(hdcDest, x, y, w, h, hdcSrc, sx, sy, SRCINVERT);

    SetBkColor(hdcDest, oldBgColor);
    SetTextColor(hdcDest, oldFgColor);
}
