/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qglobal.h>
#include <qpixmapcache.h>
#include <qbitmap.h>
#include <qgc_mac.h>
#include <private/qpainter_p.h>
#include <private/qapplication_p.h>
#include <private/qfontengine_p.h>
#include <private/qpainter_p.h>

#ifndef QMAC_FONT_NO_ANTIALIAS
# include "qpixmap.h"
# include "qpaintdevicemetrics.h"
# include <ApplicationServices/ApplicationServices.h>
#endif

//Externals
QByteArray p2qstring(const unsigned char *c); //qglobal.cpp
unsigned char * p_str(const QString &); //qglobal.cpp
void unclippedBitBlt(QPaintDevice *dst, int dx, int dy,
		     const QPaintDevice *src, int sx, int sy, int sw, int sh,
		     Qt::RasterOp rop, bool imask, bool set_fore_colour); //qpaintdevice_mac.cpp

//Generic engine
QFontEngine::~QFontEngine()
{
}

int QFontEngine::lineThickness() const
{
  // ad hoc algorithm
  int score = fontDef.pixelSize * fontDef.weight;
  int lth = score / 700;

  // looks better with thicker line for small pointsizes
  if(lth < 2 && score >= 1050)
      lth = 2;
  else if(lth == 0)
      lth = 1;
  return lth;
}

int QFontEngine::underlinePosition() const
{
  int pos = ((lineThickness() * 2) + 3) / 6;
  return pos ? pos : 1;
}

//Mac (ATSUI) engine
QFontEngineMac::QFontEngineMac() : QFontEngine(), info(NULL), fnum(-1), internal_fi(NULL)
{
    memset(widthCache, '\0', widthCacheSize);
}

QFontEngine::Error
QFontEngineMac::stringToCMap(const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs,
			     bool /*mirrored*/) const
{
    if(*nglyphs < len) {
	*nglyphs = len;
	return OutOfMemory;
    }
     memcpy(glyphs, str, len*sizeof(QChar));
    *nglyphs = len;
    if(advances) {
	for(int i = 0; i < len; i++) {
	    if(str[i].unicode() < widthCacheSize && widthCache[str[i].unicode()])
		advances[i] = widthCache[str[i].unicode()];
	    else
		advances[i] = doTextTask(str+i, 0, 1, 1, WIDTH);
	}
    }
    return NoError;
}

void
QFontEngineMac::draw(QPainter *p, int x, int y, const QTextEngine *engine,
		     const QScriptItem *si, int textFlags)
{
    int txop = p->d->txop;
    QWMatrix xmat = p->d->matrix;

    if(txop >= QPainter::TxScale) {
	int aw = si->width, ah = si->ascent + si->descent + 1;
	if(aw == 0 || ah == 0)
	    return;
	QWMatrix mat1 = xmat, mat2 = QPixmap::trueMatrix(mat1, aw, ah);
	QBitmap *wx_bm = 0;
	{
	    QBitmap bm(aw, ah, TRUE);	// create bitmap
	    QPainter paint;
	    paint.begin(&bm);		// draw text in bitmap
	    paint.setPen(Qt::color1);
	    draw(&paint, 0, si->ascent, engine, si, textFlags);
	    paint.end();
	    wx_bm = new QBitmap(bm.xForm(mat2)); // transform bitmap
	    if(wx_bm->isNull()) {
		delete wx_bm;		// nothing to draw
		return;
	    }
	}

	QPixmap pm(wx_bm->width(), wx_bm->height());
	if(p->backgroundMode() != QPainter::OpaqueMode) {
	    QPainter paint(&pm);
	    paint.fillRect(0, 0, pm.width(), pm.height(), p->pen().color());
	    pm.setMask(*wx_bm);
	} else { //This is untested code, I need to find a test case, FIXME --Sam
	    pm = *wx_bm;
	    QBitmap bm(pm.width(), pm.height(), TRUE);
	    bm.fill(Qt::color1);
	    bm = bm.xForm(mat2);
	    pm.setMask(bm);
	}
	double nfx, nfy;
	mat1.map(x, y - si->ascent, &nfx, &nfy);
	double dx, dy;
	mat2.map(0, 0, &dx, &dy);     // compute position of bitmap
	unclippedBitBlt(p->device(), qRound(nfx-dx), qRound(nfy-dy), &pm, 0, 0, -1, -1, Qt::CopyROP, FALSE, FALSE );
	delete wx_bm;
	return;
    } else if(txop == QPainter::TxTranslate) {
	p->map(x, y, &x, &y);
    }

    QPoint off;
    QRegion rgn;
    if(p->d->gc && (p->d->gc->type() == QAbstractGC::QuickDraw || p->d->gc->type() == QAbstractGC::CoreGraphics)) {
	QQuickDrawGC *mgc = (QQuickDrawGC*)p->d->gc;
	mgc->updateState(mgc->state);
	mgc->setupQDPort(false, &off, &rgn);
	if(rgn.isEmpty())
	    return;
#ifdef USE_CORE_GRAPHICS
	QMacSavedPortInfo::setClipRegion(rgn);
#endif
    }

    glyph_t *glyphs = engine->glyphs(si);
    advance_t *advances = engine->advances(si);
    qoffset_t *offsets = engine->offsets(si);
    if(p->backgroundMode() == Qt::OpaqueMode) {
	glyph_metrics_t br = boundingBox(glyphs, advances, offsets, si->num_glyphs);
	p->fillRect(x+br.x, y+br.y, br.width, br.height, p->backgroundColor());
    }
    if(p->d->gc && p->d->gc->type() == QAbstractGC::QuickDraw)
	((QQuickDrawGC*)p->d->gc)->setupQDFont();

    x += off.x();
    y += off.y();


    int w = 0;
    uchar task = DRAW;
    if(textFlags != 0)
	task |= WIDTH; //I need the width for these..
    if(si->analysis.bidiLevel % 2 ) {
	offsets += si->num_glyphs;
	advances += si->num_glyphs;
	glyphs += si->num_glyphs;
	for(int i = 0; i < si->num_glyphs; i++) {
	    glyphs--;
	    offsets--;
	    advances--;
	    MoveTo(x, y);
	    w += doTextTask((QChar*)glyphs, 0, 1, 1, task, x, y, p->device(), &rgn);
	    x += *advances;
	}
    } else {
	MoveTo(x, y);
	w = doTextTask((QChar*)glyphs, 0, si->num_glyphs, si->num_glyphs, task, x, y, p->device(), &rgn);
    }
    if(w && textFlags != 0) {
	int lineWidth = p->fontMetrics().lineWidth();
	if(textFlags & Qt::Underline) {
	    Rect r;
	    int start_x = x, end_x = x + w;
	    if(si->analysis.bidiLevel % 2) {
		start_x = x - w;
		end_x = x;
	    }
	    SetRect(&r, x, (y + 2) - (lineWidth / 2),
		    end_x, (y + 2) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
	if(textFlags & Qt::Overline) {
	    int spos = ascent() + 1;
	    Rect r;
	    int start_x = x, end_x = x + w;
	    if(si->analysis.bidiLevel % 2) {
		start_x = x - w;
		end_x = x;
	    }
	    SetRect(&r, start_x, (y - spos) - (lineWidth / 2),
		    end_x, (y - spos) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
	if(textFlags & Qt::StrikeOut) {
	    int spos = ascent() / 3;
	    if(!spos)
		spos = 1;
	    Rect r;
	    int start_x = x, end_x = x + w;
	    if(si->analysis.bidiLevel % 2) {
		start_x = x - w;
		end_x = x;
	    }
	    SetRect(&r, start_x, (y - spos) - (lineWidth / 2),
		    end_x, (y - spos) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
    }
    }

glyph_metrics_t
QFontEngineMac::boundingBox(const glyph_t *, const advance_t *advances, const qoffset_t *, int numGlyphs)
{
    int w = 0;
    const advance_t *end = advances + numGlyphs;
    while(end > advances)
	w += *(--end);
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent()+1, w, 0);
}

glyph_metrics_t
QFontEngineMac::boundingBox(glyph_t glyph)
{
    int w = doTextTask((QChar*)&glyph, 0, 1, 1, WIDTH);
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent()+1, w, 0 );
}

bool
QFontEngineMac::canRender(const QChar *string,  int len)
{
    return doTextTask(string, 0, len, len, EXISTS);
}

void
QFontEngineMac::calculateCost()
{
    // ### don't know how to get the number of glyphs from the font,
    // ### so default to 1024
    cache_cost = (ascent() + descent() + 1) * maxCharWidth() * 1024;
}

int
QFontEngineMac::doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, int, int y,
			   QPaintDevice *dev, const QRegion *rgn) const
{
#if defined(QMAC_FONT_NO_ANTIALIAS)
    Q_UNUSED(dev);
    Q_UNUSED(rgn);
#endif

    int ret = 0;
    QMacSetFontInfo fi(this, dev);
    QMacFontInfo::QATSUStyle *st = fi.atsuStyle();
    if(!st)
	return 0;

    if(task & DRAW) {
	RGBColor fcolor;
	GetForeColor(&fcolor);
	if(st->rgb.red != fcolor.red || st->rgb.green != fcolor.green ||
	   st->rgb.blue != fcolor.blue) {
	    st->rgb = fcolor;
	    const ATSUAttributeTag tag = kATSUColorTag;
	    ByteCount size = sizeof(fcolor);
	    ATSUAttributeValuePtr value = &fcolor;
	    if(OSStatus e = ATSUSetAttributes(st->style, 1, &tag, &size, &value)) {
		qDebug("Qt: internal: %ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
		return 0;
	    }
	}
    }
    if((task & WIDTH)) {
 	bool use_cached_width = TRUE;
 	for(int i = 0; i < use_len; i++) {
 	    if(s[i].unicode() >= widthCacheSize || !widthCache[s[i].unicode()]) {
 		use_cached_width = FALSE;
 		break;
 	    }
 	    ret += widthCache[s[i].unicode()];
 	}
 	if(use_cached_width && task == WIDTH)
	    return ret;
    }

    //create layout
    ATSUTextLayout alayout;
    const UniCharCount count = use_len;
#if 0
    if(OSStatus e = ATSUCreateTextLayoutWithTextPtr((UniChar *)(s), pos,
						    count, len, 1, &count,
						    &st->style, &alayout)) {
	qDebug("%ld: This shouldn't happen %s:%d", e, __FILE__, __LINE__);
	return 0;
    }
#else
    Q_UNUSED(len);
    if(OSStatus e = ATSUCreateTextLayoutWithTextPtr((UniChar *)(s)+pos, 0,
						    count, use_len, 1, &count,
						    &st->style, &alayout)) {
	qDebug("Qt: internal: %ld: Unexpected condition reached %s:%d", e, __FILE__, __LINE__);
	return 0;
    }
#endif
    const int arr_guess = 5;
    int arr = 0;
    ATSUAttributeTag tags[arr_guess];
    ByteCount valueSizes[arr_guess];
    ATSUAttributeValuePtr values[arr_guess];
    tags[arr] = kATSULineLayoutOptionsTag;
    ATSLineLayoutOptions layopts = kATSLineHasNoOpticalAlignment | kATSLineIgnoreFontLeading | kATSLineFractDisable;
    if(fontDef.styleStrategy & QFont::NoAntialias)
	layopts |= kATSLineNoAntiAliasing;

#if QT_MACOSX_VERSION >= 0x1020
    if(qMacVersion() == Qt::MV_10_DOT_1)
	layopts |= kATSLineIsDisplayOnly;
    else
	layopts |= kATSLineDisableAutoAdjustDisplayPos | kATSLineDisableAllLayoutOperations |
		   kATSLineUseDeviceMetrics;
#else
    layopts |= kATSLineIsDisplayOnly;
#endif
    valueSizes[arr] = sizeof(layopts);
    values[arr] = &layopts;
    arr++;
#if !defined(QMAC_FONT_NO_ANTIALIAS)
    tags[arr] = kATSUCGContextTag; //cgcontext
    CGrafPtr port = NULL;
    CGContextRef ctx = NULL;
    if(dev) {
	if(dev->devType() == QInternal::Widget)
	    port = GetWindowPort((WindowPtr)dev->handle());
	else
	    port = (CGrafPtr)dev->handle();
    } else {
	static QPixmap *p = NULL;
	if(!p)
	    p = new QPixmap(1, 1, 32);
	port = (CGrafPtr)p->handle();
    }
    RgnHandle rgnh = NULL;
    if(rgn && !rgn->isEmpty())
	rgnh = rgn->handle(TRUE);
    if(OSStatus err = QDBeginCGContext(port, &ctx)) {
	qDebug("Qt: internal: WH0A, QDBeginCGContext(%ld) failed. %s:%d", err, __FILE__, __LINE__);
	ATSUDisposeTextLayout(alayout);
	return 0;
    }
    Rect clipr;
    GetPortBounds(port, &clipr);
    if(rgnh)
	ClipCGContextToRegion(ctx, &clipr, rgnh);
    valueSizes[arr] = sizeof(ctx);
    values[arr] = &ctx;
    arr++;
#endif
    if(arr > arr_guess) //this won't really happen, just so I will not miss the case
	qDebug("Qt: internal: %d: WH0A, arr_guess underflow %d", __LINE__, arr);
    if(OSStatus e = ATSUSetLayoutControls(alayout, arr, tags, valueSizes, values)) {
	qDebug("Qt: internal: %ld: Unexpected condition reached %s:%d", e, __FILE__, __LINE__);
	ATSUDisposeTextLayout(alayout);
#if !defined(QMAC_FONT_NO_ANTIALIAS)
	QDEndCGContext(port, &ctx);
#endif
	return 0;
    }

    ATSUSetTransientFontMatching(alayout, true);
    //do required task now
    if(task & EXISTS) {
	if(task != EXISTS)
	    qWarning("Qt: EXISTS must appear by itself!");
	ATSUFontID fid;
	UniCharArrayOffset off;
	UniCharCount off_len;
	if(ATSUMatchFontsToText(alayout, kATSUFromTextBeginning, kATSUToTextEnd, &fid, &off, &off_len) != kATSUFontsNotMatched)
	    ret = 1;
    } else if((task & WIDTH) && !ret) {
	ATSUTextMeasurement left, right, bottom, top;
#if QT_MACOSX_VERSION >= 0x1020
	if(qMacVersion() >= Qt::MV_10_DOT_2)
	    ATSUGetUnjustifiedBounds(alayout, kATSUFromTextBeginning, kATSUToTextEnd,
				     &left, &right, &bottom, &top);
	else
#endif
	    ATSUMeasureText(alayout, kATSUFromTextBeginning, kATSUToTextEnd,
			    &left, &right, &bottom, &top);
	ret = FixRound(right-left);
 	if(use_len == 1 && s->unicode() < widthCacheSize)
 	    widthCache[s->unicode()] = ret;
    }
    if(task & DRAW) {
	ATSUDrawText(alayout, kATSUFromTextBeginning, kATSUToTextEnd,
#if !defined(QMAC_FONT_NO_ANTIALIAS)
		     kATSUUseGrafPortPenLoc, FixRatio((clipr.bottom-clipr.top)-y, 1)
#else
		     kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc
#endif
	    );
    }
    //cleanup
    ATSUDisposeTextLayout(alayout);
#if !defined(QMAC_FONT_NO_ANTIALIAS)
    QDEndCGContext(port, &ctx);
#endif
    return ret;
}



