#include "qfontengine_p.h"
#include <qglobal.h>
#include "qapplication_p.h"
#include "qcomplextext_p.h"

//antialiasing
#ifdef Q_WS_MACX
# define QMAC_FONT_ANTIALIAS
#endif
#ifdef QMAC_FONT_ANTIALIAS
# include "qpixmap.h"
# include "qpaintdevicemetrics.h"
# include <ApplicationServices/ApplicationServices.h>
#endif

//Externals
QCString p2qstring(const unsigned char *c); //qglobal.cpp
unsigned char * p_str(const QString &); //qglobal.cpp

//Generic engine
QFontEngine::~QFontEngine()
{

}

//Mac (ATSUI) engine
QFontEngine::Error 
QFontEngineMac::stringToCMap(const QChar *str, int len, glyph_t *glyphs, advance_t *advances, int *nglyphs) const
{
    if(*nglyphs < len) {
	*nglyphs = len;
	return OutOfMemory;
    }
    for(int i = 0; i < len; i++ )
	glyphs[i] = str[i].unicode();
    *nglyphs = len;
    if(advances) {
	for(int i = 0; i < len; i++) 
	    advances[i] = doTextTask(str+i, 0, 1, 1, WIDTH);
    }
    return NoError;
}

void 
QFontEngineMac::draw(QPainter *p, int x, int y, const glyph_t *glyphs,
		      const advance_t *advances, const offset_t *offsets, int numGlyphs, bool reverse)
{
    uchar task = DRAW;
    if(fontDef.underline || fontDef.strikeOut) 
	task |= WIDTH; //I need the width for these..
    int w = 0;
    const QRegion &rgn = qt_mac_update_painter(p, FALSE);
    if(reverse) {
	offsets += numGlyphs;
	advances += numGlyphs;
	glyphs += numGlyphs;
	for(int i = 0; i < numGlyphs; i++) {
	    glyphs--;
	    offsets--;
	    advances--;
	    MoveTo(x, y);
	    w += doTextTask((QChar*)glyphs, 0, 1, 1, task, x, y, p->device(), &rgn);
	    x += *advances;
	}
    } else {
	MoveTo(x, y);
	w = doTextTask((QChar*)glyphs, 0, numGlyphs, numGlyphs, task, x, y, p->device(), &rgn);
    }
    if(w && (fontDef.underline || fontDef.strikeOut)) { 
	int lineWidth = p->fontMetrics().lineWidth();
	if(fontDef.underline) {
	    Rect r;
	    SetRect(&r, x, (y + 2) - (lineWidth / 2), 
		    x + w, (y + 2) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
	if(fontDef.strikeOut) {
	    int spos = ascent() / 3;
	    if(!spos)
		spos = 1;
	    Rect r;
	    SetRect(&r, x, (y - spos) - (lineWidth / 2), 
		    x + w, (y - spos) + (lineWidth / 2));
	    if(!(r.bottom - r.top))
		r.bottom++;
	    PaintRect(&r);
	}
    } 
    }

glyph_metrics_t 
QFontEngineMac::boundingBox(const glyph_t *, const advance_t *advances, const offset_t *, int numGlyphs)
{
    int w = 0;
    const advance_t *end = advances + numGlyphs;
    while(end > advances)
	w += *(--end);
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0);
}

glyph_metrics_t 
QFontEngineMac::boundingBox(glyph_t glyph)
{
    int w = doTextTask((QChar*)&glyph, 0, 1, 1, WIDTH);
    return glyph_metrics_t(0, -(ascent()), w, ascent()+descent(), w, 0 );
}

bool 
QFontEngineMac::canRender(const QChar *string,  int len)
{
    return doTextTask(string, 0, len, len, EXISTS);
}

int 
QFontEngineMac::doTextTask(const QChar *s, int pos, int use_len, int len, uchar task, int, int y,
			   QPaintDevice *dev, const QRegion *rgn) const
{
#if !defined(QMAC_FONT_ANTIALIAS)
    Q_UNUSED(dev);
    Q_UNUSED(rgn);
#endif
    if(task & EXISTS) {
	if(task != EXISTS)
	    qWarning("Qt: EXISTS must appear by itself!");
	qWarning("Qt: need to implement exists()");
	return 1;
    }

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

#ifdef MACOSX_102
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
#if defined(QMAC_FONT_ANTIALIAS)
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
    if(rgn && !rgn->isNull() && !rgn->isEmpty()) 
	rgnh = rgn->handle(TRUE);
    if(QDBeginCGContext(port, &ctx)) {
	qDebug("Qt: internal: WH0A, QDBeginCGContext failed. %s:%d", __FILE__, __LINE__);
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
#if defined(QMAC_FONT_ANTIALIAS)
	QDEndCGContext(port, &ctx);
#endif
	return 0;
    }
    ATSUSetTransientFontMatching(alayout, true);

    //do required task now
    if(task & WIDTH) {
	ATSUTextMeasurement left, right, bottom, top;
#if defined(MACOSX_102)
	if(qMacVersion() >= Qt::MV_10_DOT_2) 
	    ATSUGetUnjustifiedBounds(alayout, kATSUFromTextBeginning, kATSUToTextEnd,
				     &left, &right, &bottom, &top);
	else
#endif
	    ATSUMeasureText(alayout, kATSUFromTextBeginning, kATSUToTextEnd,
			    &left, &right, &bottom, &top);
#if 0
	qDebug("(%s) (%s): %p %d %d %d (%d %d == %d)", 
	       QString(s+pos, use_len).latin1(), QString(s, len).latin1(),
	       s, pos, use_len, len, FixRound(left), FixRound(right), 
	       FixRound(right) - FixRound(left));
#endif
	ret = FixRound(right-left);
    }
    if(task & DRAW) {
	ATSUDrawText(alayout, kATSUFromTextBeginning, kATSUToTextEnd, 
#if defined(QMAC_FONT_ANTIALIAS)
		     kATSUUseGrafPortPenLoc, FixRatio((clipr.bottom-clipr.top)-y, 1)
#else
		     kATSUUseGrafPortPenLoc, kATSUUseGrafPortPenLoc
#endif
	    );
    }
    //cleanup
    ATSUDisposeTextLayout(alayout);
#if defined(QMAC_FONT_ANTIALIAS)
    QDEndCGContext(port, &ctx);
#endif
    return ret;
}



