/****************************************************************************
**
** Implementation of QPainter class for Mac.
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

#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qptrlist.h"
#include "qfontdata_p.h"
#include "qtextcodec.h"
#include "qpaintdevicemetrics.h"
#include "qpaintdevice.h"
#include "qt_mac.h"
#include <qptrstack.h>
#include <qtextcodec.h>
#include <qprinter.h>
#include <private/qtextengine_p.h>
#include <private/qfontengine_p.h>
#include <private/qtextlayout_p.h>
#include <string.h>

class paintevent_item;
class QPainterPrivate
{
public:
    //shared details
    int offx, offy;
    QPixmap *brush_style_pix;
    uint unclipped : 1, locked : 1;

    //implementation details of the port information (for QuickDraw)
    struct {
	QMacSavedPortInfo *saved;
	paintevent_item *paintevent;
	QRegion clippedreg, paintreg;
	uint crgn_dirty : 1,  clip_serial : 15;
    } qd_info;
    //implementation details of the port information (for CoreGraphics)
    struct {
	CGPatternRef fill_pattern;
	CGColorSpaceRef fill_colorspace;
    } cg_info;
    inline void cg_mac_point(const int &inx, const int &iny, float *outx, float *outy, bool global=false) {
	if(outx)
	    *outx = inx;
	if(outy)
	    *outy = iny;
	if(!global) {
	    if(outx)
		*outx += offx;
	    if(outy)
		*outy += offy;
	}
    }
    inline void cg_mac_point(const int &inx, const int &iny, CGPoint *p, bool global=false) {
	cg_mac_point(inx, iny, &p->x, &p->y, global);
    }
    inline void cg_mac_rect(const int &inx, const int &iny, const int &inw, const int &inh, CGRect *rct, bool global=false) {
	*rct = CGRectMake(0, 0, inw, inh);
	cg_mac_point(inx, iny, &rct->origin, global);
    }
};


/*****************************************************************************
  Current "active" QPainter
 *****************************************************************************/
QPainter *qt_mac_current_painter = 0;

/*****************************************************************************
  QPainter member functions
 *****************************************************************************/
bool qt_recreate_root_win(); //qwidget_mac.cpp
static void drawTile(QPainter *, int, int, int, int, const QPixmap &, int, int);
QPoint posInWindow(QWidget *w);
QRegion make_region(RgnHandle handle);
void qt_mac_clip_cg_handle(CGContextRef, const QRegion &, const QRect &, bool); //qpaintdevice_mac.cpp
void unclippedBitBlt(QPaintDevice *dst, int dx, int dy,
		     const QPaintDevice *src, int sx, int sy, int sw, int sh,
		     Qt::RasterOp rop, bool imask, bool set_fore_colour); //qpaintdevice_mac.cpp
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp

/* paintevent magic to provide Windows semantics on Qt/Mac */
class paintevent_item
{
    QWidget *clippedTo;
    QPaintDevice* dev;
    QRegion clipRegion;
public:
    paintevent_item(QPaintDevice *d, QRegion r, QWidget *c) : clippedTo(c), dev(d), clipRegion(r) { }
    inline bool operator==(QPaintDevice *rhs) const { return rhs == dev; }
    inline bool operator!=(QPaintDevice *rhs) const { return !(this->operator==(rhs)); }
    inline QWidget *clip() const { return clippedTo; }
    inline QPaintDevice *device() const { return dev; }
    inline QRegion region() const { return clipRegion; }
};
QPtrStack<paintevent_item> paintevents;

void qt_set_paintevent_clipping(QPaintDevice* dev, const QRegion& region, QWidget *clip)
{
    QRegion r = region;
    if(dev && dev->devType() == QInternal::Widget) {
	QWidget *w = (QWidget *)dev;
	QPoint mp(posInWindow(w));
	r.translate(mp.x(), mp.y());
    }
    if(paintevent_item *curr = paintevents.current()) {
	if(curr->device() == dev || curr->clip() == dev || clip == curr->clip()
	   || curr->device() == clip)
	    r &= curr->region();
    }
    paintevents.push(new paintevent_item(dev, r, clip));
}

void qt_clear_paintevent_clipping(QPaintDevice *dev)
{
    if(paintevents.isEmpty() || !((*paintevents.current()) == dev)) {
	qDebug("Qt: internal: WH0A, qt_clear_paintevent_clipping mismatch.");
	return;
    }
    delete paintevents.pop();
}




static inline CGContextRef qt_mac_get_cg(QPaintDevice *pdev, QPainterPrivate *paint_d)
{
    CGContextRef ret = 0;
    if(pdev->devType() == QInternal::Widget) 
	ret = (CGContextRef)((QWidget*)pdev)->macCGHandle(!paint_d->unclipped);
    else
	ret = (CGContextRef)pdev->macCGHandle();
    //apply paint event region (in global coords)
    if(paintevent_item *pevent = paintevents.current()) {
	if((*pevent) == pdev) {
	    qt_mac_clip_cg_handle(ret, pevent->region(), QRect(0, 0, 0, 0), true);
	}
    }
    return ret;
}

void QPainter::initialize()
{
}

void QPainter::cleanup()
{
}

void QPainter::destroy()
{
    delete d;
}

void QPainter::init()
{
    flags = IsStartingUp;
    bg_brush = white;                             // default background color
    bg_mode = TransparentMode;                  // default background mode
    rop = CopyROP;                              // default ROP
    tabstops = 0;                               // default tabbing
    tabarray = 0;
    tabarraylen = 0;
    ps_stack = 0;
    wm_stack = 0;
    pdev = 0;
    txop = txinv = 0;
    penRef = brushRef = 0;
    pfont = 0;
    block_ext = false;

    d = new QPainterPrivate;
    d->qd_info.saved = 0;
    d->qd_info.paintevent = 0;
    d->qd_info.clip_serial = 0;
    d->brush_style_pix = 0;
    d->qd_info.crgn_dirty = d->locked = d->unclipped = false;
    d->qd_info.clippedreg = d->qd_info.paintreg = QRegion();
    d->offx = d->offy = 0;
    d->cg_info.fill_pattern = 0;
    d->cg_info.fill_colorspace = 0;
}

void QPainter::setFont(const QFont &font)
{
    if(!isActive())
	qWarning("QPainter::setFont: Will be reset by begin()");
    if(cfont.d != font.d) {
	cfont = font;
	setf(DirtyFont);
    }
}

void QPainter::updateFont()
{
    clearf(DirtyFont);
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	delete pfont;
	pfont = new QFont(cfont.d, pdev);
	param[0].font = &cfont;
	if(!pdev->cmd(QPaintDevice::PdcSetFont,this,param) || !hd)
	    return;
    }
    if(penRef)
	updatePen();                            // force a non-cached GC
    cfont.macSetFont(pdev);
}

void QPainter::updateXForm()
{

}

static int ropCodes[] = {			// ROP translation table
    patCopy, patOr, patXor, patBic, notPatCopy,
    notPatOr, notPatXor, notPatBic,
    666, 666, 666, 666, 666, 666, 666, 666, 666
};
inline static float qt_mac_convert_color_to_cg(int c) { return ((float)c * 1000 / 255) / 1000; }

void QPainter::updatePen()
{
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].pen = &cpen;
	if(!pdev->cmd(QPaintDevice::PdcSetPen, this, param) || !hd)
	    return;
    }

    //***********
    //QuickDraw
    //***********

    //pen color
    ::RGBColor f;
    f.red = cpen.color().red()*256;
    f.green = cpen.color().green()*256;
    f.blue = cpen.color().blue()*256;
    RGBForeColor(&f);

    //pen size
    int dot = cpen.width();
    if(dot < 1)
	dot = 1;
    PenSize(dot, dot);

    int	ps = cpen.style();
    Pattern pat;
    switch(ps) {
	case DotLine:
	case DashDotLine:
	case DashDotDotLine:
//	    qDebug("Penstyle not implemented %s - %d", __FILE__, __LINE__);
	case DashLine:
	    GetQDGlobalsGray(&pat);
	    break;
	default:
	    GetQDGlobalsBlack(&pat);
	    break;
    }
    PenPat(&pat);

    //penmodes
    //Throw away a desktop when you paint into it non copy mode (xor?) I do this because
    //xor doesn't really work on an overlay widget FIXME
    if(rop != CopyROP && pdev->devType() == QInternal::Widget && ((QWidget *)pdev)->isDesktop())
	qt_recreate_root_win();
    PenMode(ropCodes[rop]);

#ifdef USE_CORE_GRAPHICS
    //***********
    //CoreGraphics
    //***********
    float *lengths = NULL;
    int count = 0;
    if(cpen.style() == DashLine) {
	static float inner_lengths[] = { 3, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(cpen.style() == DotLine) {
	static float inner_lengths[] = { 1, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(cpen.style() == DashDotLine) {
	static float inner_lengths[] = { 3, 1, 1, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    } else if(cpen.style() == DashDotDotLine) {
	static float inner_lengths[] = { 3, 1, 1, 1, 1, 1 };
	lengths = inner_lengths;
	count = sizeof(sizeof(inner_lengths) / sizeof(inner_lengths[0]));
    }
    CGContextSetLineDash((CGContextRef)hd, 0, lengths, count);

    CGLineCap cglinecap = kCGLineCapButt;
    if(cpen.capStyle() == SquareCap)
	cglinecap = kCGLineCapSquare;
    else if(cpen.capStyle() == RoundCap)
	cglinecap = kCGLineCapRound;
    CGContextSetLineCap((CGContextRef)hd, cglinecap);
    
    CGContextSetLineWidth((CGContextRef)hd, (float)(cpen.width() < 1 ? 1 : cpen.width()));

    const QColor &col = cpen.color();
    CGContextSetRGBStrokeColor((CGContextRef)hd, qt_mac_convert_color_to_cg(col.red()),
			       qt_mac_convert_color_to_cg(col.green()), qt_mac_convert_color_to_cg(col.blue()), 1.0);

    CGLineJoin cglinejoin = kCGLineJoinMiter;
    if(cpen.joinStyle() == BevelJoin)
	cglinejoin = kCGLineJoinBevel;
    else if(cpen.joinStyle() == RoundJoin)
	cglinejoin = kCGLineJoinRound;
    CGContextSetLineJoin((CGContextRef)hd, cglinejoin);
#endif
}

#ifdef USE_CORE_GRAPHICS
struct QMacPattern {
    bool as_mask;
    union {
	QPixmap *pixmap;
	struct {
	    uchar *bytes;
	    uint byte_per_line;
	} mask;
    };
    struct ImageConv {
	CGImageRef image;
	CGColorSpaceRef colorspace;
	CGDataProviderRef provider;
    } *im;
};
static void qt_mac_draw_pattern(void *info, CGContextRef c)
{
    QMacPattern *pat = (QMacPattern*)info;
    int w = 0, h = 0;
    if(pat->as_mask) {
	qDebug("Completely untested!!"); //need to test patterns
	w = h = pat->mask.byte_per_line;
	if(!pat->im) {
	    pat->im = new QMacPattern::ImageConv;
	    pat->im->colorspace = CGColorSpaceCreateDeviceRGB();
	    pat->im->provider = CGDataProviderCreateWithData(0, pat->mask.bytes, w, 0);
	    pat->im->image = CGImageCreate(w, h, 8, 1, w, pat->im->colorspace, kCGImageAlphaOnly, 
					   pat->im->provider, 0, 0, kCGRenderingIntentDefault);
	}
    } else {
	w = pat->pixmap->width();
	h = pat->pixmap->height();
	if(!pat->im) {
	    pat->im = new QMacPattern::ImageConv;
	    pat->im->colorspace = CGColorSpaceCreateDeviceRGB();
	    uint bpl = GetPixRowBytes(GetGWorldPixMap((GWorldPtr)pat->pixmap->handle()));
	    pat->im->provider = CGDataProviderCreateWithData(0, GetPixBaseAddr(GetGWorldPixMap((GWorldPtr)pat->pixmap->handle())), bpl, 0);
	    pat->im->image = CGImageCreate(w, h, 8, pat->pixmap->depth(), bpl, pat->im->colorspace, kCGImageAlphaNoneSkipFirst, 
					   pat->im->provider, 0, 0, kCGRenderingIntentDefault);
	}
    }
    CGRect rect = CGRectMake(0, 0, w, h);
#if 1
    /* For whatever reason HIViews are top, left - so we'll just use this convenience function to
       actually render the CGImageRef. If this proves not to be an efficent funciton call (I doubt
       it), we'll just flip the image in the conversion above. */
    HIViewDrawCGImage(c, &rect, pat->im->image);
#else
    CGContextDrawImage(c, rect, pat->im->image);
#endif
}

static void qt_mac_dispose_pattern(void *info)
{
    QMacPattern *pat = (QMacPattern*)info;
    if(pat->im) {
	if(pat->im->image)
	    CGImageRelease(pat->im->image);
	if(pat->im->colorspace)
	    CGColorSpaceRelease(pat->im->colorspace);
	if(pat->im->provider)
	    CGDataProviderRelease(pat->im->provider);
    }
    if(!pat->as_mask)
	delete pat->pixmap;
    delete pat;
    pat = NULL;
}
#endif

void QPainter::updateBrush()
{
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].brush = &cbrush;
	if(!pdev->cmd(QPaintDevice::PdcSetBrush,this,param) || !hd)
	    return;
    }

    static uchar dense1_pat[] = { 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff };
    static uchar dense2_pat[] = { 0x77, 0xff, 0xdd, 0xff, 0x77, 0xff, 0xdd, 0xff };
    static uchar dense3_pat[] = { 0x55, 0xbb, 0x55, 0xee, 0x55, 0xbb, 0x55, 0xee };
    static uchar dense4_pat[] = { 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa };
    static uchar dense5_pat[] = { 0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11 };
    static uchar dense6_pat[] = { 0x88, 0x00, 0x22, 0x00, 0x88, 0x00, 0x22, 0x00 };
    static uchar dense7_pat[] = { 0x00, 0x44, 0x00, 0x00, 0x00, 0x44, 0x00, 0x00 };
    static uchar hor_pat[] = {			// horizontal pattern
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    static uchar ver_pat[] = {			// vertical pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar cross_pat[] = {			// cross pattern
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0xff, 0xff, 0xff,
	0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20,
	0x08, 0x82, 0x20, 0xff, 0xff, 0xff, 0x08, 0x82, 0x20, 0x08, 0x82, 0x20 };
    static uchar bdiag_pat[] = {			// backward diagonal pattern
	0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, 0x01, 0x01,
	0x80, 0x80, 0x40, 0x40, 0x20, 0x20, 0x10, 0x10, 0x08, 0x08, 0x04, 0x04,
	0x02, 0x02, 0x01, 0x01, 0x80, 0x80, 0x40, 0x40 };
    static uchar fdiag_pat[] = {			// forward diagonal pattern
	0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x20, 0x20, 0x40, 0x40,
	0x80, 0x80, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10,
	0x20, 0x20, 0x40, 0x40, 0x80, 0x80, 0x01, 0x01 };
    static uchar dcross_pat[] = {			// diagonal cross pattern
	0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14, 0x22, 0x22, 0x41, 0x41,
	0x80, 0x80, 0x41, 0x41, 0x22, 0x22, 0x14, 0x14, 0x08, 0x08, 0x14, 0x14,
	0x22, 0x22, 0x41, 0x41, 0x80, 0x80, 0x41, 0x41 };
    static uchar *pat_tbl[] = {
	dense1_pat, dense2_pat, dense3_pat, dense4_pat, dense5_pat,
	dense6_pat, dense7_pat,
	hor_pat, ver_pat, cross_pat, bdiag_pat, fdiag_pat, dcross_pat };

    //Throw away a desktop when you paint into it non copy mode (xor?) I do this because
    //xor doesn't really work on an overlay widget FIXME
    if(rop != CopyROP && pdev->devType() == QInternal::Widget && ((QWidget *)pdev)->isDesktop())
	qt_recreate_root_win();

    //***********
    //QuickDraw
    //***********

    //color
    ::RGBColor f;
    f.red = cbrush.color().red()*256;
    f.green = cbrush.color().green()*256;
    f.blue = cbrush.color().blue()*256;
    RGBForeColor(&f);

    d->brush_style_pix = 0;
    int bs = cbrush.style();
    if(bs >= Dense1Pattern && bs <= DiagCrossPattern) {
	QString key;
	key.sprintf("$qt-brush$%d", bs);
	d->brush_style_pix = QPixmapCache::find(key);
	if(!d->brush_style_pix) {                        // not already in pm dict
	    uchar *pat=pat_tbl[bs-Dense1Pattern];
	    int size = 16;
	    if(bs<=Dense7Pattern)
		size=8;
	    else if(bs<=CrossPattern)
		size=24;
	    d->brush_style_pix = new QPixmap(size, size);
	    d->brush_style_pix->setMask(QBitmap(size, size, pat, false));
	    QPixmapCache::insert(key, d->brush_style_pix);
	}
	d->brush_style_pix->fill(cbrush.color());
    }

    //penmodes
    PenMode(ropCodes[rop]);

#ifdef USE_CORE_GRAPHICS
    //***********
    //CoreGraphics
    //***********

    const QColor &col = cbrush.color();
    CGContextSetRGBFillColor((CGContextRef)hd, qt_mac_convert_color_to_cg(col.red()),
			       qt_mac_convert_color_to_cg(col.green()), qt_mac_convert_color_to_cg(col.blue()), 1.0);

    if(bs != SolidPattern && bs != NoBrush) {
	if(d->cg_info.fill_pattern)
	    CGPatternRelease(d->cg_info.fill_pattern);
	if(d->cg_info.fill_colorspace)
	    CGColorSpaceRelease(d->cg_info.fill_colorspace);

	int width = 0, height = 0;
	CGColorSpaceRef cs_base = 0;
	QMacPattern *qpattern = new QMacPattern;
	qpattern->im = 0;
	if(bs == CustomPattern) {
	    qpattern->as_mask = false;
	    qpattern->pixmap = new QPixmap(*cbrush.pixmap());
	    width = qpattern->pixmap->width();
	    height = qpattern->pixmap->height();
	} else {
	    qpattern->as_mask = true;
	    qpattern->mask.bytes = pat_tbl[bs-Dense1Pattern];
	    if(bs<=Dense7Pattern)
		qpattern->mask.byte_per_line = 8;
	    else if(bs<=CrossPattern)
		qpattern->mask.byte_per_line = 24;
	    else
		qpattern->mask.byte_per_line = 16;
	    width = height = qpattern->mask.byte_per_line;
	    cs_base = CGColorSpaceCreateDeviceRGB();
	}
	d->cg_info.fill_colorspace = CGColorSpaceCreatePattern(cs_base);

	CGPatternCallbacks callbks;
	callbks.version = 0;
	callbks.drawPattern = qt_mac_draw_pattern;
	callbks.releaseInfo = qt_mac_dispose_pattern;
	d->cg_info.fill_pattern = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height), CGContextGetCTM((CGContextRef)hd), width, height,
						  kCGPatternTilingNoDistortion, !qpattern->as_mask, &callbks);
	CGContextSetFillColorSpace((CGContextRef)hd, d->cg_info.fill_colorspace); 
	const float tmp_float = 1; //wtf?? --SAM (this seems to be necessary, but why!?!) ###
	CGContextSetFillPattern((CGContextRef)hd, d->cg_info.fill_pattern, &tmp_float);
    }
#endif
}

bool QPainter::begin(const QPaintDevice *pd, bool unclipped)
{
    if(isActive()) {                         // already active painting
	qWarning("QPainter::begin: Painter is already active."
		 "\n\tYou must end() the painter before a second begin()");
	return false;
    }
    if(!pd) {
	qWarning("QPainter::begin: Paint device cannot be null");
	return false;
    }
    if(pd->devType() == QInternal::Widget && 
       !static_cast<const QWidget*>(pd)->testWState(WState_InPaintEvent)) {
	qWarning("QPainter::begin: Widget painting can only begin as a "
		 "result of a paintEvent");
	return false;
    }

    //save the gworld now, we'll reset it in end()
    d->qd_info.saved = new QMacSavedPortInfo;

    const QWidget *copyMe = 0;
    if((pdev = const_cast<QPaintDevice*>(redirected(pd, &redirection_offset)))) {
	if(pd->devType() == QInternal::Widget)
	    copyMe = static_cast<const QWidget *>(pd); // copy widget settings
    } else {
	pdev = const_cast<QPaintDevice*>(pd);
    }

    if(pdev->isExtDev() && pdev->paintingActive()) {
	// somebody else is already painting
	qWarning("QPainter::begin: Another QPainter is already painting "
		 "this device;\n\tAn extended paint device can only be painted "
		 "by one QPainter at a time.");
	return false;
    }

    int dt = pdev->devType();                   // get the device type
    bool reinit = flags != IsStartingUp;        // 2nd or 3rd etc. time called
    flags = IsActive | DirtyFont;               // init flags

    if((pdev->devFlags & QInternal::ExternalDevice) != 0) // this is an extended device
	setf(ExtDev);
    else if(dt == QInternal::Pixmap)         // device is a pixmap
	((QPixmap*)pdev)->detach();             // will modify it

    if(testf(ExtDev)) {                      // external device
	if(!pdev->cmd(QPaintDevice::PdcBegin, this, 0)) {   // could not begin painting
	    clearf(IsActive);
	    pdev = 0;
	    return false;
	}
	if(tabstops)                         // update tabstops for device
	    setTabStops(tabstops);
	if(tabarray)                         // update tabarray for device
	    setTabArray(tabarray);
    }

    pdev->painters++;                           // also tell paint device
    bro = QPoint(0, 0);
    if(reinit) {
	bg_mode = TransparentMode;              // default background mode
	rop = CopyROP;                          // default ROP
	wxmat.reset();                          // reset world xform matrix
	xmat.reset();
	ixmat.reset();
	txop = txinv = 0;
	if(dt != QInternal::Widget) {
	    QFont  defaultFont;                 // default drawing tools
	    QPen   defaultPen;
	    QBrush defaultBrush;
	    cfont  = defaultFont;               // set these drawing tools
	    cpen   = defaultPen;
	    cbrush = defaultBrush;
	    bg_brush = white;                     // default background color
	    // was white
	}
    }
    d->qd_info.clip_serial = 0;
    d->qd_info.paintevent = 0;
    d->qd_info.crgn_dirty = false;
    d->offx = d->offy = 0;
    d->cg_info.fill_pattern = 0;
    wx = wy = vx = vy = 0;                      // default view origins

    if(pdev->devType() == QInternal::Widget) {                    // device is a widget
	QWidget *w = (QWidget*)pdev;
	copyFrom(w);
	if(reinit) {
	    QBrush defaultBrush;
	    cbrush = defaultBrush;
	}
	
	{ //offset painting in widget relative the tld
	    QPoint wp(posInWindow(w));
	    d->offx = wp.x();
	    d->offy = wp.y();
	}
	ww = vw = w->width();                   // default view size
	wh = vh = w->height();
	if(!unclipped)
	    unclipped = (bool)w->testWFlags(WPaintUnclipped);

	if(!d->locked) {
	    LockPortBits(GetWindowPort((WindowPtr)w->handle()));
	    d->locked = true;
	}

	if(w->isDesktop()) {
	    if(!unclipped)
		qWarning("QPainter::begin: Does not support clipped desktop on MacOSX");
	    ShowWindow((WindowPtr)w->handle());
	}
    } else {
	if(pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
	    QPixmap *pm = (QPixmap*)pdev;
	    if(pm->isNull()) {
		qWarning("QPainter::begin: Cannot paint null pixmap");
		end();
		return false;
	    }
#ifndef QMAC_ONE_PIXEL_LOCK
	    if(!d->locked) {
		Q_ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)pm->handle())));
		d->locked = true;
	    }
#endif
	    ww = vw = pm->width();                  // default view size
	    wh = vh = pm->height();
	} 
    }
    d->unclipped = unclipped;
    hd = qt_mac_get_cg(pdev, d); // get handle to drawable
    if(hd)
	CGContextRetain((CGContextRef)hd);
    initPaintDevice(true); //force setting paint device, this does unclipped fu

    if(testf(ExtDev)) {               // external device
	ww = vw = pdev->metric(QPaintDeviceMetrics::PdmWidth);
	wh = vh = pdev->metric(QPaintDeviceMetrics::PdmHeight);
    }

    if(ww == 0)
	ww = wh = vw = vh = 1024;
    if(copyMe)
	copyFrom(copyMe);
    if(testf(ExtDev)) {                      // external device
	setBackgroundColor(bg_brush);           // default background color
	setBackgroundMode(TransparentMode);   // default background mode
	setRasterOp(CopyROP);                 // default raster operation
    }

    updateXForm();
    updateBrush();
    updatePen();
    if(!redirection_offset.isNull())
	translate(-redirection_offset.x(), -redirection_offset.y());
    return true;
}

bool QPainter::end()				// end painting
{
    if(!isActive()) {
	qWarning("QPainter::end: Missing begin() or begin() failed");
	return false;
    }
    killPStack();

    if(testf(ExtDev))
	pdev->cmd(QPaintDevice::PdcEnd, this, 0);

    if(d->locked) {
	if(pdev->devType() == QInternal::Widget)
	    UnlockPortBits(GetWindowPort((WindowPtr)pdev->handle()));
#ifndef QMAC_ONE_PIXEL_LOCK
	else
	    UnlockPixels(GetGWorldPixMap((GWorldPtr)pdev->handle()));
#endif
	d->locked = false;
    }

    //Quickdraw cleanup
    delete d->qd_info.saved;
    d->qd_info.saved = 0;
    if(qt_mac_current_painter == this)
	qt_mac_current_painter = 0;

    //CoreGraphics cleanup
    if(d->cg_info.fill_pattern) {
        CGPatternRelease(d->cg_info.fill_pattern);
	d->cg_info.fill_pattern = 0;
    }
    if(d->cg_info.fill_colorspace) {
	CGColorSpaceRelease(d->cg_info.fill_colorspace);
	d->cg_info.fill_colorspace = 0;
    }

    if(pdev->painters == 1 &&
       pdev->devType() == QInternal::Widget && ((QWidget*)pdev)->isDesktop())
	HideWindow((WindowPtr)pdev->handle());
    if(pfont) {
	delete pfont;
	pfont = 0;
    }

    flags = 0;
    pdev->painters--;
    if(hd) {
	if(CFGetRetainCount(hd) == 1)
	    CGContextFlush((CGContextRef)hd);
	CGContextRelease((CGContextRef)hd);
	hd = 0;
    }
    pdev = 0;
    return true;
}

void QPainter::flush(const QRegion &rgn, CoordinateMode m)
{
    if(!isActive())
	return;

    //QuickDraw
    initPaintDevice();
    QRegion b;
    if(m == CoordDevice)
	b = rgn;
    else
	b = xmat * rgn;
    b.translate(d->offx, d->offy);
    QMacSavedPortInfo::flush(pdev, b & d->qd_info.paintreg, true);

    //CoreGraphics
    CGContextFlush((CGContextRef)hd);
}

void QPainter::flush()
{
    if(!isActive())
	return;

    //QuickDraw
    initPaintDevice();
    QMacSavedPortInfo::flush(pdev, d->qd_info.paintreg, true);

    //CoreGraphics
    CGContextFlush((CGContextRef)hd);

}

void QPainter::setBackgroundColor(const QColor &c)
{
    if(!isActive()) {
	qWarning("Qt: QPainter::setBackgroundColor: Call begin() first");
	return;
    }
    bg_brush = c;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].color = &bg_brush.color();
	if(!pdev->cmd(QPaintDevice::PdcSetBkColor, this, param) || !hd)
	    return;
    }
    if(!penRef)
	updatePen();				// update pen setting
    if(!brushRef)
	updateBrush();				// update brush setting
}

void QPainter::setBackgroundMode(BGMode m)
{
    if(!isActive()) {
#if defined(CHECK_STATE)
	qWarning("Qt: QPainter::setBackgroundMode: Call begin() first");
#endif
	return;
    }
    if(m != TransparentMode && m != OpaqueMode) {
#if defined(CHECK_RANGE)
	qWarning("Qt: QPainter::setBackgroundMode: Invalid mode");
#endif
	return;
    }
    bg_mode = m;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].ival = m;
	if(!pdev->cmd(QPaintDevice::PdcSetBkMode, this, param) || !hd)
	    return;
    }
    if(!penRef)
	updatePen();				// update pen setting
    if(!brushRef)
	updateBrush();				// update brush setting
}

void QPainter::setRasterOp(RasterOp r)
{
    if(!isActive()) {
#if defined(CHECK_STATE)
	qWarning("Qt: QPainter::setRasterOp: Call begin() first");
#endif
	return;
    }
    if((uint)r > LastROP) {
#if defined(CHECK_RANGE)
	qWarning("Qt: QPainter::setRasterOp: Invalid ROP code");
#endif
	return;
    }
    if(ropCodes[r] == 666) {
	//qWarning("Woops, we don't have that rasterop, FIXME!!");
	r = XorROP;
    }
    rop = r;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].ival = r;
	if(!pdev->cmd(QPaintDevice::PdcSetROP, this, param) || !hd)
	    return;
    }
    if(penRef)
	updatePen();				// get non-cached pen GC
    if(brushRef)
	updateBrush();				// get non-cached brush GC
}

void QPainter::setBrushOrigin(int x, int y)
{
    if(!isActive()) {
#if defined(CHECK_STATE)
	qWarning("Qt: QPainter::setBrushOrigin: Call begin() first");
#endif
	return;
    }
    bro = QPoint(x,y);
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].point = &bro;
	if(!pdev->cmd(QPaintDevice::PdcSetBrushOrigin, this, param) || !hd)
	    return;
    }
    if(brushRef)
	updateBrush();				// get non-cached brush GC
}

void QPainter::setClipping(bool b)
{
    if(!isActive()) {
#if defined(CHECK_STATE)
	qWarning("Qt: QPainter::setClipping: Call begin() first");
#endif
	return;
    }

    bool old_clipon = testf(ClipOn);
    if(b)
	setf(ClipOn);
    else
	clearf(ClipOn);

    //QuickDraw
    d->qd_info.crgn_dirty = true;

#ifdef USE_CORE_GRAPHICS
    //CoreGraphics
    if(hd) {
	if(b || b != old_clipon) { //reset the clip
	    CGContextRelease((CGContextRef)hd);
	    hd = qt_mac_get_cg(pdev, d);
	    if(hd)
		CGContextRetain((CGContextRef)hd);
	}
	if(hd && b) 
	    qt_mac_clip_cg_handle((CGContextRef)hd, crgn, QRect(d->offx, d->offy, 0, 0), true);
    }
#endif
}


void QPainter::setClipRect(const QRect &r, CoordinateMode m)
{
    setClipRegion(QRegion(r), m);
}

void QPainter::setClipRegion(const QRegion &rgn, CoordinateMode m)
{
    if(!isActive()) {
#if defined(CHECK_STATE)
	qWarning("Qt: QPainter::setClipRegion: Call begin() first");
#endif
	return;
    }

    if(m == CoordDevice) {
	crgn = rgn;
	if(!redirection_offset.isNull())
	    crgn.translate(-redirection_offset);
    } else {
	crgn = xmat * rgn;
    }
    setClipping(true);
}

void QPainter::drawPolyInternal(const QPointArray &a, bool close, bool inset)
{
    if(a.isEmpty())
	return;

#ifdef USE_CORE_GRAPHICS
    Q_UNUSED(inset);

    float cg_x, cg_y;
    d->cg_mac_point(a[0].x(), a[0].y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
    for(int x = 1; x < a.size(); x++) {
	d->cg_mac_point(a[x].x(), a[x].y(), &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)hd, cg_x, cg_y);
    }
    if(close) {
	d->cg_mac_point(a[0].x(), a[0].y(), &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)hd, cg_x, cg_y);
    }
    if(cbrush.style() != NoBrush)
	CGContextFillPath((CGContextRef)hd);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
#else
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    RgnHandle polyRegion = qt_mac_get_rgn();
    OpenRgn();
    MoveTo(a[0].x()+d->offx, a[0].y()+d->offy);
    for(int x = 1; x < a.size(); x++)
	LineTo(a[x].x()+d->offx, a[x].y()+d->offy);
    if(close)
	LineTo(a[0].x()+d->offx, a[0].y()+d->offy);
    CloseRgn(polyRegion);

    if(close && this->brush().style() != NoBrush) {
	updateBrush();
	if(inset && cpen.style() == NoPen)  // Inset all points, no frame will be painted.
	    InsetRgn(polyRegion, 1, 1);
	if(this->brush().style() == SolidPattern) {
	    PaintRgn(polyRegion);
	} else {
	    QPixmap *pm = 0;
	    if(cbrush.style() == QBrush::CustomPattern) {
		pm = cbrush.pixmap();
	    } else {
		pm = d->brush_style_pix;
		if(bg_mode == OpaqueMode) {
		    ::RGBColor f;
		    f.red = bg_brush.color().red()*256;
		    f.green = bg_brush.color().green()*256;
		    f.blue = bg_brush.color().blue()*256;
		    RGBForeColor(&f);
		    PaintRgn(polyRegion);
		}
	    }

	    if(pm && !pm->isNull()) {
		//save the clip
		bool clipon = testf(ClipOn);
		QRegion clip = crgn;

		//create the region
		QRegion newclip(a);
		if(clipon)
		    newclip &= clip;
		setClipRegion(newclip);

		//turn off translation flags
		uint save_flags = flags;
		flags = IsActive | ClipOn;

		//draw the brush
		QRect r(a.boundingRect());
		drawTiledPixmap(r.x(), r.y(), r.width(), r.height(), *pm,
				r.x() - bro.x(), r.y() - bro.y());

		//restore translation flags
		flags = save_flags;

		//restore the clip
		if(clipon)
		    setClipRegion(clip);
		else
		    setClipping(false);
	    }
	}
    }
    if(cpen.style() != NoPen) {
	updatePen();
	FrameRgn(polyRegion);
    }
    qt_mac_dispose_rgn(polyRegion);
#endif
}

void QPainter::drawPoint(int x, int y)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	QPoint p(x, y);
	param[0].point = &p;
	if(!pdev->cmd(QPaintDevice::PdcDrawPoint, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF))
	map(x, y, &x, &y);

    if(cpen.style() != NoPen) {
#ifdef USE_CORE_GRAPHICS
	float cg_x, cg_y;
	d->cg_mac_point(x, y, &cg_x, &cg_y);
	CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
	CGContextAddLineToPoint((CGContextRef)hd, cg_x+1, cg_y);  
	CGContextStrokePath((CGContextRef)hd);
#else
	initPaintDevice();
	if(d->qd_info.paintreg.isEmpty())
	    return;
	updatePen();
	MoveTo(x + d->offx, y + d->offy);
	Line(0,1);
#endif
    }
}

void QPainter::drawPoints(const QPointArray& a, int index, int npoints)
{
    if(npoints < 0)
	npoints = a.size() - index;
    if(index + npoints > (int)a.size())
	npoints = a.size() - index;
    if(!isActive() || npoints < 1 || index < 0)
	return;
    QPointArray pa = a;
    if(testf(ExtDev|VxF|WxF)) {
	if(testf(ExtDev)) {
	    QPDevCmdParam param[1];
	    for(int i=0; i<npoints; i++) {
		QPoint p(pa[index+i].x(), pa[index+i].y());
		param[0].point = &p;
		if(!pdev->cmd(QPaintDevice::PdcDrawPoint,this,param) || !hd)
		    return;
	    }
	}
	if(txop != TxNone) {
	    pa = xForm(a, index, npoints);
	    if(pa.size() != a.size()) {
		index = 0;
		npoints = pa.size();
	    }
	}
    }

    if(cpen.style() != NoPen) {
#ifdef USE_CORE_GRAPHICS
	float cg_x, cg_y;
	for(int i=0; i<npoints; i++) {
	    d->cg_mac_point(pa[index+i].x(), pa[index+i].y(), &cg_x, &cg_y);
	    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
	    CGContextAddLineToPoint((CGContextRef)hd, cg_x+1, cg_y);  
	    CGContextStrokePath((CGContextRef)hd);
	}
#else
	initPaintDevice();
	if(d->qd_info.paintreg.isEmpty())
	    return;
	updatePen();
	for(int i=0; i<npoints; i++) {
	    MoveTo(pa[index+i].x()+d->offx, pa[index+i].y()+d->offy);
	    Line(0,1);
	}
#endif
    }
}

void QPainter::moveTo(int x, int y)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	QPoint p(x, y);
	param[0].point = &p;
	if(!pdev->cmd(QPaintDevice::PdcMoveTo,this,param) || !hd)
	    return;
    }
    if(testf(VxF|WxF))
	map(x, y, &x, &y);

#ifdef USE_CORE_GRAPHICS
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
#else
    initPaintDevice();
    MoveTo(x+d->offx, y+d->offy);
#endif
}

void QPainter::lineTo(int x, int y)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	QPoint p(x, y);
	param[0].point = &p;
	if(!pdev->cmd(QPaintDevice::PdcLineTo, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF))
	map(x, y, &x, &y);

#ifdef USE_CORE_GRAPHICS
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);
    CGContextAddLineToPoint((CGContextRef)hd, cg_x, cg_y);  
    CGContextStrokePath((CGContextRef)hd);
    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
#else
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    updatePen();
    LineTo(x+d->offx,y+d->offy);
#endif
}

void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[2];
	QPoint p1(x1, y1), p2(x2, y2);
	param[0].point = &p1;
	param[1].point = &p2;
	if(!pdev->cmd(QPaintDevice::PdcDrawLine, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	map(x1, y1, &x1, &y1);
	map(x2, y2, &x2, &y2);
    }

#ifdef USE_CORE_GRAPHICS
    float cg_x, cg_y;
    d->cg_mac_point(x1, y1, &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
    d->cg_mac_point(x2, y2, &cg_x, &cg_y);
    CGContextAddLineToPoint((CGContextRef)hd, cg_x, cg_y);  
    CGContextStrokePath((CGContextRef)hd);
#else
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;
    updatePen();
    MoveTo(x1+d->offx,y1+d->offy);
    LineTo(x2+d->offx,y2+d->offy);
#endif
}

void QPainter::drawRect(int x, int y, int w, int h)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	QRect r(x, y, w, h);
	param[0].rect = &r;
	if(!pdev->cmd(QPaintDevice::PdcDrawRect, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	if(txop == TxRotShear) {             // rotate/shear polygon
	    QPointArray a(QRect(x,y,w,h));
	    drawPolyInternal(xForm(a));
	    return;
	}
	map(x, y, w, h, &x, &y, &w, &h);
    }
    if(w <= 0 || h <= 0) {
	if(w == 0 || h == 0)
	    return;
	fix_neg_rect(&x, &y, &w, &h);
    }

#ifdef USE_CORE_GRAPHICS
    CGRect mac_rect;
    d->cg_mac_rect(x, y, w, h, &mac_rect);
    if(cbrush.style() != NoBrush)
	CGContextFillRect((CGContextRef)hd, mac_rect);
    if(cpen.style() != NoPen) 
	CGContextStrokeRect((CGContextRef)hd, mac_rect);
#else
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    Rect rect;
    SetRect(&rect, x+d->offx, y+d->offy, x + w+d->offx, y + h+d->offy);
    if(this->brush().style() != NoBrush) {
	updateBrush();
	if(this->brush().style() == SolidPattern) {
	    PaintRect(&rect);
	} else {
	    QPixmap *pm = 0;
	    if(cbrush.style() == QBrush::CustomPattern) {
		pm = cbrush.pixmap();
	    } else {
		pm = d->brush_style_pix;
		if(bg_mode == OpaqueMode) {
		    ::RGBColor f;
		    f.red = bg_brush.color().red()*256;
		    f.green = bg_brush.color().green()*256;
		    f.blue = bg_brush.color().blue()*256;
		    RGBForeColor(&f);
		    PaintRect(&rect);
		}
	    }
	    if(pm && !pm->isNull()) {
		//save the clip
		bool clipon = testf(ClipOn);
		QRegion clip = crgn;

		//create the region
		QRegion newclip(QRect(x, y, w, h));
		if(clipon)
		    newclip &= clip;
		setClipRegion(newclip);

		//turn off translation flags
		uint save_flags = flags;
		flags = IsActive | ClipOn;

		//draw the brush
		drawTiledPixmap(x, y, w, h, *pm, x - bro.x(), y - bro.y());

		//restore translation flags
		flags = save_flags;

		//restore the clip
		if(clipon)
		    setClipRegion(clip);
		else
		    setClipping(false);
	    }
	}
    }
    if(cpen.style() != NoPen) {
	updatePen();
	FrameRect(&rect);
    }
#endif
}

void QPainter::drawWinFocusRect(int x, int y, int w, int h)
{
    drawWinFocusRect(x, y, w, h, true, color0);
}

void QPainter::drawWinFocusRect(int x, int y, int w, int h,
				 const QColor &bgColor)
{
    drawWinFocusRect(x, y, w, h, false, bgColor);
}

void QPainter::drawWinFocusRect(int x, int y, int w, int h, bool xorPaint, const QColor &bgColor)
{
    if(!isActive())
	return;
    if(txop == TxRotShear)
	return;
    QPen    old_pen = cpen;
    QBrush  old_brush = cbrush;
    RasterOp old_rop = (RasterOp)rop;

    setBrush(QBrush());

    if(xorPaint) {
	if(QColor::numBitPlanes() <= 8)
	    setPen(color1);
	else
	    setPen(white);
	setRasterOp(XorROP);
    } else {
	if(qGray(bgColor.rgb()) < 128)
	    setPen(white);
	else
	    setPen(black);
    }
    if(w <= 0 || h <= 0) {
	if(w == 0 || h == 0)
	    return;
	fix_neg_rect(&x, &y, &w, &h);
    }

    cpen.setStyle(DashLine);
    updatePen();
    if(cpen.style() != NoPen)
	drawRect(x, y, w, h);
    setRasterOp(old_rop);
    setPen(old_pen);
    setBrush(old_brush);
}

void QPainter::drawRoundRect(int x, int y, int w, int h, int xRnd, int yRnd)
{
    if(!isActive())
	return;
    if(xRnd <= 0 || yRnd <= 0) {
	drawRect(x, y, w, h);                 // draw normal rectangle
	return;
    }
    if(xRnd >= 100)                          // fix ranges
	xRnd = 99;
    if(yRnd >= 100)
	yRnd = 99;
    if(testf(ExtDev)) {
	QPDevCmdParam param[3];
	QRect r(x, y, w, h);
	param[0].rect = &r;
	param[1].ival = xRnd;
	param[2].ival = yRnd;
	if(!pdev->cmd(QPaintDevice::PdcDrawRoundRect, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	if(txop == TxRotShear) {             // rotate/shear polygon
	    if(w <= 0 || h <= 0)
		fix_neg_rect(&x, &y, &w, &h);
	    w--;
	    h--;
	    int rxx = w*xRnd/200;
	    int ryy = h*yRnd/200;
	    // were there overflows?
	    if(rxx < 0)
		rxx = w/200*xRnd;
	    if(ryy < 0)
		ryy = h/200*yRnd;
	    int rxx2 = 2*rxx;
	    int ryy2 = 2*ryy;
	    QPointArray a[4];
	    a[0].makeArc(x, y, rxx2, ryy2, 1*16*90, 16*90, xmat);
	    a[1].makeArc(x, y+h-ryy2, rxx2, ryy2, 2*16*90, 16*90, xmat);
	    a[2].makeArc(x+w-rxx2, y+h-ryy2, rxx2, ryy2, 3*16*90, 16*90, xmat);
	    a[3].makeArc(x+w-rxx2, y, rxx2, ryy2, 0*16*90, 16*90, xmat);
	    // ### is there a better way to join QPointArrays?
	    QPointArray aa;
	    aa.resize(a[0].size() + a[1].size() + a[2].size() + a[3].size());
	    uint j = 0;
	    for(int k=0; k<4; k++) {
		for(int i=0; i<a[k].size(); i++) {
		    aa.setPoint(j, a[k].point(i));
		    j++;
		}
	    }
	    drawPolyInternal(aa);
	    return;
	}
	map(x, y, w, h, &x, &y, &w, &h);
    }
    if(w <= 0 || h <= 0) {
	if(w == 0 || h == 0)
	    return;
	fix_neg_rect(&x, &y, &w, &h);
    }


#ifdef USE_CORE_GRAPHICS
    // I need to test how close this is to other platforms, I only rolled this without testing --Sam
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);

    CGMutablePathRef path = CGPathCreateMutable();
    CGPathMoveToPoint(path, 0, cg_x, cg_y+yRnd);                             //start
    CGPathAddQuadCurveToPoint(path, 0, cg_x, cg_y, cg_x+xRnd, cg_y);         //top left
    CGPathAddLineToPoint(path, 0, cg_x+(w-xRnd), cg_y);                        //top
    CGPathAddQuadCurveToPoint(path, 0, cg_x+w, cg_y, cg_x+w, cg_y+yRnd);       //top right
    CGPathAddLineToPoint(path, 0, cg_x+w, cg_y+(h-yRnd));                      //right
    CGPathAddQuadCurveToPoint(path, 0, cg_x+w, cg_y+h, cg_x+(w-xRnd), cg_y+h); //bottom right
    CGPathAddLineToPoint(path, 0, cg_x+xRnd, cg_y+h);                          //bottom
    CGPathAddQuadCurveToPoint(path, 0, cg_x, cg_y+h, cg_x, cg_y+(h-yRnd));     //bottom left
    CGPathAddLineToPoint(path, 0, cg_x, cg_y+yRnd);                            //left
    CGContextBeginPath((CGContextRef)hd);
    CGContextAddPath((CGContextRef)hd, path);
    if(cbrush.style() != NoBrush)
	CGContextFillPath((CGContextRef)hd);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
    CGPathRelease(path);
#else
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    Rect rect;
    SetRect(&rect, x+d->offx, y+d->offy, x + w+d->offx, y + h+d->offy);
    if(this->brush().style() == SolidPattern) {
	updateBrush();
	PaintRoundRect(&rect, w*xRnd/100, h*yRnd/100);
    }
    if(cpen.style() != NoPen) {
	updatePen();
	FrameRoundRect(&rect, w*xRnd/100, h*yRnd/100);
    }
#endif
}

void QPainter::drawEllipse(int x, int y, int w, int h)
{
    if(!isActive()) {
	return;
    }
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	QRect r(x, y, w, h);
	param[0].rect = &r;
	if(!pdev->cmd(QPaintDevice::PdcDrawEllipse, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	if(txop == TxRotShear) {             // rotate/shear polygon
	    QPointArray a;
	    a.makeArc(x, y, w, h, 0, 360*16, xmat);
	    drawPolyInternal(a);
	    return;
	}
	map(x, y, w, h, &x, &y, &w, &h);
    }
    if(w <= 0 || h <= 0) {
	if(w == 0 || h == 0)
	    return;
	fix_neg_rect(&x, &y, &w, &h);
    }

#ifdef USE_CORE_GRAPHICS
    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(w != h) 
	transform = CGAffineTransformMakeScale(((float)w)/h, 1);
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);
    CGPathAddArc(path, w == h ? 0 : &transform, (cg_x+(w/2))/((float)w/h), cg_y + (h/2), h/2, 0, 360, false);
    CGContextBeginPath((CGContextRef)hd);
    CGContextAddPath((CGContextRef)hd, path);
    if(cbrush.style() != NoBrush)
	CGContextFillPath((CGContextRef)hd);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
    CGPathRelease(path);
#else
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    Rect r;
    SetRect(&r, x+d->offx, y+d->offy, x + w+d->offx, y + h+d->offy);
    if(this->brush().style() != NoBrush) {
	updateBrush();
	if(this->brush().style() == SolidPattern) {
	    PaintOval(&r);
	} else {
	    QPixmap *pm = 0;
	    if(cbrush.style() == QBrush::CustomPattern) {
		pm = cbrush.pixmap();
	    } else {
		pm = d->brush_style_pix;
		if(bg_mode == OpaqueMode) {
		    ::RGBColor f;
		    f.red = bg_brush.color().red()*256;
		    f.green = bg_brush.color().green()*256;
		    f.blue = bg_brush.color().blue()*256;
		    RGBForeColor(&f);
		    PaintOval(&r);
		}
	    }
	    if(pm && !pm->isNull()) {
		//save the clip
		bool clipon = testf(ClipOn);
		QRegion clip = crgn;

		//create the region
		QRegion newclip(QRect(x, y, w, h), QRegion::Ellipse);
		if(clipon)
		    newclip &= clip;
		setClipRegion(newclip);

		//turn off translation flags
		uint save_flags = flags;
		flags = IsActive | ClipOn;

		//draw the brush
		drawTiledPixmap(x, y, w, h, *pm, x - bro.x(), y - bro.y());

		//restore translation flags
		flags = save_flags;

		//restore the clip
		if(clipon)
		    setClipRegion(clip);
		else
		    setClipping(false);
	    }
	}
    }

    if(cpen.style() != NoPen) {
	updatePen();
	FrameOval(&r);
    }
#endif
}

void QPainter::drawArc(int x, int y, int w, int h, int a, int alen)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[3];
	QRect r(x, y, w, h);
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	if(!pdev->cmd(QPaintDevice::PdcDrawArc, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
        if(txop == TxRotShear) {             // rotate/shear
            QPointArray pa;
            pa.makeArc(x, y, w, h, a, alen, xmat); // arc polyline
            drawPolyInternal(pa, false);
            return;
        }
        map(x, y, w, h, &x, &y, &w, &h);
    }
    if(w <= 0 || h <= 0) {
	if(w == 0 || h == 0)
	    return;
	fix_neg_rect(&x, &y, &w, &h);
    }
#ifdef USE_CORE_GRAPHICS
    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(w != h) 
	transform = CGAffineTransformMakeScale(((float)w)/h, 1);
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);
    float begin_radians = ((float)a/16) * (M_PI/180), end_radians = ((float)(a+alen)/16) * (M_PI/180);
    CGPathAddArc(path, w == h ? 0 : &transform, (cg_x+(w/2))/((float)w/h), cg_y + (h/2), h/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGContextBeginPath((CGContextRef)hd);
    CGContextAddPath((CGContextRef)hd, path);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
    CGPathRelease(path);
#else
    QPointArray pa;
    pa.makeArc(x, y, w, h, a, alen); // arc polyline
    drawPolyline(pa);
#endif
}

void QPainter::drawPie(int x, int y, int w, int h, int a, int alen)
{
    if(!isActive())
	return;

    if(a > (360*16)) {
	a = a % (360*16);
    } else if(a < 0) {
	a = a % (360*16);
	if(a < 0)
	    a += (360*16);
    }
    if(testf(ExtDev)) {
	QPDevCmdParam param[3];
	QRect r(x, y, w, h);
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	if(!pdev->cmd(QPaintDevice::PdcDrawPie, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
        if(txop == TxRotShear) {             // rotate/shear
            QPointArray pa;
            pa.makeArc(x, y, w, h, a, alen, xmat); // arc polyline
            int n = pa.size();
            int cx, cy;
            xmat.map(x+w/2, y+h/2, &cx, &cy);
            pa.resize(n+2);
            pa.setPoint(n, cx, cy);   // add legs
            pa.setPoint(n+1, pa.at(0));
            drawPolyInternal(pa);
            return;
        }
        map(x, y, w, h, &x, &y, &w, &h);
    }
#ifdef USE_CORE_GRAPHICS
    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(w != h) 
	transform = CGAffineTransformMakeScale(((float)w)/h, 1);
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);
    float begin_radians = ((float)a/16) * (M_PI/180), end_radians = ((float)(a+alen)/16) * (M_PI/180);
    CGPathMoveToPoint(path, 0, cg_x + (w/2), cg_y + (h/2));
    CGPathAddArc(path, w == h ? 0 : &transform, (cg_x+(w/2))/((float)w/h), cg_y + (h/2), h/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGPathAddLineToPoint(path, 0, cg_x + (w/2), cg_y + (h/2));
    CGContextBeginPath((CGContextRef)hd);
    CGContextAddPath((CGContextRef)hd, path);
    if(cbrush.style() != NoBrush) 
	CGContextFillPath((CGContextRef)hd);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
    CGPathRelease(path);
#else
    QPointArray pa;
    pa.makeArc(x, y, w, h, a, alen, xmat); // arc polyline
    int n = pa.size();
    int cx, cy;
    xmat.map(x+w/2, y+h/2, &cx, &cy);
    pa.resize(n+2);
    pa.setPoint(n, cx, cy);	// add legs
    pa.setPoint(n+1, pa.at(0));
    drawPolyInternal(pa, true, false);
#endif
}

void QPainter::drawChord(int x, int y, int w, int h, int a, int alen)
{
    if(!isActive())
	return;
    if(testf(ExtDev)) {
	QPDevCmdParam param[3];
	QRect r(x, y, w, h);
	param[0].rect = &r;
	param[1].ival = a;
	param[2].ival = alen;
	if(!pdev->cmd(QPaintDevice::PdcDrawChord, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
        if(txop == TxRotShear) {             // rotate/shear
            QPointArray pa;
            pa.makeArc(x, y, w-1, h-1, a, alen, xmat); // arc polygon
            int n = pa.size();
            pa.resize(n+1);
            pa.setPoint(n, pa.at(0));         // connect endpoints
            drawPolyInternal(pa);
            return;
        }
        map(x, y, w, h, &x, &y, &w, &h);
    }
#ifdef USE_CORE_GRAPHICS
    CGMutablePathRef path = CGPathCreateMutable();
    CGAffineTransform transform;
    if(w != h) 
	transform = CGAffineTransformMakeScale(((float)w)/h, 1);
    float cg_x, cg_y;
    d->cg_mac_point(x, y, &cg_x, &cg_y);
    float begin_radians = ((float)a/16) * (M_PI/180), end_radians = ((float)(a+alen)/16) * (M_PI/180);
    //We draw twice because the first draw will set the point to the end of arc, and the second pass will draw the line to the first point
    CGPathAddArc(path, w == h ? 0 : &transform, (cg_x+(w/2))/((float)w/h), cg_y+(h/2), h/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGPathAddArc(path, w == h ? 0 : &transform, (cg_x+(w/2))/((float)w/h), cg_y+(h/2), h/2, begin_radians, end_radians, a < 0 || alen < 0);
    CGContextBeginPath((CGContextRef)hd);
    CGContextAddPath((CGContextRef)hd, path);
    if(cbrush.style() != NoBrush) 
	CGContextFillPath((CGContextRef)hd);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
    CGPathRelease(path);
#else
    QPointArray pa;
    pa.makeArc(x, y, w-1, h-1, a, alen);
    int n = pa.size();
    pa.resize(n + 1);
    pa.setPoint(n, pa.at(0));
    drawPolyInternal(pa);
#endif
}

void QPainter::drawLineSegments(const QPointArray &a, int index, int nlines)
{
    if(nlines < 0)
	nlines = a.size()/2 - index/2;
    if(index + nlines*2 > (int)a.size())
	nlines = (a.size() - index)/2;
    if(!isActive() || nlines < 1 || index < 0)
	return;
    QPointArray pa = a;
    if(testf(ExtDev)) {
	if(2*nlines != (int)pa.size()) {
	    pa = QPointArray(nlines*2);
	    for(int i=0; i<nlines*2; i++)
		pa.setPoint(i, a.point(index+i));
	    index = 0;
	}
	QPDevCmdParam param[1];
	param[0].ptarr = (QPointArray*)&pa;
	if(!pdev->cmd(QPaintDevice::PdcDrawLineSegments,this,param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	if(txop != TxNone) {
	    pa = xForm(a, index, nlines*2);
	    if(pa.size() != a.size()) {
		index  = 0;
		nlines = pa.size()/2;
	    }
	}
    }

#ifdef USE_CORE_GRAPHICS
    float cg_x, cg_y;
    int  x1, y1, x2, y2;
    uint i = index;
    while(nlines--) {
	pa.point(i++, &x1, &y1);
	pa.point(i++, &x2, &y2);
	d->cg_mac_point(x1, y1, &cg_x, &cg_y);
	CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
	d->cg_mac_point(x2, y2, &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)hd, cg_x, cg_y);  
	CGContextStrokePath((CGContextRef)hd);
    }
#else
    int  x1, y1, x2, y2;
    uint i = index;

    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    updatePen();
    while(nlines--) {
	pa.point(i++, &x1, &y1);
	pa.point(i++, &x2, &y2);
	MoveTo(x1 + d->offx, y1 + d->offy);
	LineTo(x2 + d->offx, y2 + d->offy);
    }
#endif
}

void QPainter::drawPolyline(const QPointArray &a, int index, int npoints)
{
    if(npoints < 0)
	npoints = a.size() - index;
    if(index + npoints > (int)a.size())
	npoints = a.size() - index;
    if(!isActive() || npoints < 2 || index < 0)
	return;
    QPointArray pa = a;
    if(testf(ExtDev)) {
	if(npoints != (int)pa.size()) {
	    pa = QPointArray(npoints);
	    for(int i=0; i<npoints; i++)
		pa.setPoint(i, a.point(index+i));
	    index = 0;
	}
	QPDevCmdParam param[1];
	param[0].ptarr = (QPointArray*)&pa;
	if(!pdev->cmd(QPaintDevice::PdcDrawPolyline,this,param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	if(txop != TxNone) {
	    pa = xForm(pa, index, npoints);
	    if(pa.size() != a.size()) {
		index   = 0;
		npoints = pa.size();
	    }
	}
    }
#ifdef USE_CORE_GRAPHICS
    float cg_x, cg_y;
    d->cg_mac_point(a[0].x(), a[0].y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
    for(int x = 1; x < a.size(); x++) {
	d->cg_mac_point(a[x].x(), a[x].y(), &cg_x, &cg_y);
	CGContextAddLineToPoint((CGContextRef)hd, cg_x, cg_y);
    }
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
#else
    int x1, y1, x2, y2, xsave, ysave;
    pa.point(index+npoints-2, &x1, &y1);      // last line segment
    pa.point(index+npoints-1, &x2, &y2);
    xsave = x2; ysave = y2;
    bool plot_pixel = false;
    if(x1 == x2) {                           // vertical
	if(y1 < y2)
	    y2++;
	else
	    y2--;
    } else if(y1 == y2) {                    // horizontal
	if(x1 < x2)
	    x2++;
	else
	    x2--;
    } else {
	plot_pixel = cpen.style() == SolidLine; // plot last pixel
    }
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;
    //make a region of it
    PolyHandle poly = OpenPoly();
    MoveTo(pa[0].x()+d->offx, pa[0].y()+d->offy);
    for(int x = 1; x < pa.size(); x++)
	LineTo(pa[x].x()+d->offx, pa[x].y()+d->offy);
    ClosePoly();
    //now draw it
    updatePen();
    FramePoly(poly);
    KillPoly(poly);
#endif
}

void QPainter::drawConvexPolygon(const QPointArray &pa,
			     int index, int npoints)
{
    // Implemented identically as drawPolygon() [no optimization]
    drawPolygon(pa,false,index,npoints);
}

void QPainter::drawPolygon(const QPointArray &a, bool winding,
			    int index, int npoints)
{
    if(npoints < 0)
	npoints = a.size() - index;
    if(index + npoints > (int)a.size())
	npoints = a.size() - index;
    if(!isActive() || npoints < 2 || index < 0)
	return;
    QPointArray pa;
    if(npoints != (int)a.size()) {
	pa = QPointArray(npoints);
	for(int i=0; i<npoints; i++)
	    pa.setPoint(i, a.point(index+i));
	index = 0;
    } else {
	pa = a;
    }
    if(testf(ExtDev)) {
	QPDevCmdParam param[2];
	param[0].ptarr = (QPointArray*)&pa;
	param[1].ival = winding;
	if(!pdev->cmd(QPaintDevice::PdcDrawPolygon, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
	if(txop != TxNone) {
	    pa = xForm(a, index, npoints);
	    if(pa.size() != a.size()) {
		index   = 0;
		npoints = pa.size();
	    }
	}
    }
    drawPolyInternal(pa,true);
}

void QPainter::drawCubicBezier(const QPointArray &a, int index)
{
    if(!isActive())
	return;
    if(a.size() - index < 4) {
	qWarning("Qt: QPainter::drawCubicBezier: Cubic Bezier "
		 "needs 4 control points");
	return;
    }
    QPointArray pa(a);
    if(index != 0 || a.size() > 4) {
        pa = QPointArray(4);
        for(int i=0; i<4; i++)
            pa.setPoint(i, a.point(index+i));
    }
    if(testf(ExtDev)) {
	QPDevCmdParam param[1];
	param[0].ptarr = (QPointArray*)&pa;
	if(!pdev->cmd(QPaintDevice::PdcDrawCubicBezier, this, param) || !hd)
	    return;
    }
    if(testf(VxF|WxF)) {
        if(txop != TxNone) {
            pa = xForm(pa);
	    pa.translate(-redirection_offset);
	}
    }
#ifdef USE_CORE_GRAPHICS
    float cg_x, cg_y;
    d->cg_mac_point(a[0].x(), a[0].y(), &cg_x, &cg_y);
    CGContextMoveToPoint((CGContextRef)hd, cg_x, cg_y);
    float c1_x, c1_y, c2_x, c2_y;
    d->cg_mac_point(a[1].x(), a[1].y(), &c1_x, &c1_y);
    d->cg_mac_point(a[2].x(), a[2].y(), &c2_x, &c2_y);
    d->cg_mac_point(a[3].x(), a[3].y(), &cg_x, &cg_y);
    CGContextAddCurveToPoint((CGContextRef)hd, c1_x, c1_y, c2_x, c2_y, cg_x, cg_y);
    if(cpen.style() != NoPen) 
	CGContextStrokePath((CGContextRef)hd);
#else
    drawPolyline(pa.cubicBezier());
#endif
}

void QPainter::drawPixmap(int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh)
{
    if(!isActive() || pixmap.isNull()) {
	return;
    }
    if(sw < 0)
	sw = pixmap.width() - sx;
    if(sh < 0)
	sh = pixmap.height() - sy;

    // Sanity-check clipping
    if(sx < 0) {
	x -= sx;
	sw += sx;
	sx = 0;
    }
    if(sw + sx > pixmap.width())
	sw = pixmap.width() - sx;
    if(sy < 0) {
	y -= sy;
	sh += sy;
	sy = 0;
    }
    if(sh + sy > pixmap.height())
	sh = pixmap.height() - sy;
    if(sw <= 0 || sh <= 0) {
	return;
    }

    if(testf(ExtDev|VxF|WxF)) {
	if(txop == TxScale) {
	    // Plain scaling, then unclippedScaledBitBlt is fastest
	    int w, h;
	    map(x, y, sw, sh, &x, &y, &w, &h);
	    initPaintDevice();
	    if(d->qd_info.paintreg.isEmpty())
		return;
	    updatePen();
	    unclippedScaledBitBlt(pdev, x, y, w, h, &pixmap, sx, sy, sw, sh, (RasterOp)rop,
				   false, false);
	    return;
	} else if(testf(ExtDev) || txop == TxRotShear) {
	    if(sx != 0 || sy != 0 ||
		 sw != pixmap.width() || sh != pixmap.height()) {
		QPixmap tmp(sw, sh, pixmap.depth());
		updatePen();
		unclippedBitBlt(&tmp, 0, 0, &pixmap, sx, sy, sw, sh, CopyROP, true, false);
		if(pixmap.mask()) {
		    QBitmap mask(sw, sh);
		    unclippedBitBlt(&mask, 0, 0, pixmap.mask(), sx, sy, sw, sh,
				     CopyROP, true, false);
		    tmp.setMask(mask);
		}
		drawPixmap(x, y, tmp);
		return;
	    }
	    if(testf(ExtDev)) {
		QPDevCmdParam param[2];
		QRect r(x, y, pixmap.width(), pixmap.height());
		param[0].rect  = &r;
		param[1].pixmap = &pixmap;
		if(!pdev->cmd(QPaintDevice::PdcDrawPixmap,this,param) || !hd) {
		    return;
		}
	    }
	    if(txop == TxScale || txop == TxRotShear) {
		QWMatrix mat(m11(), m12(),
			      m21(), m22(),
			      dx(),  dy());
		mat = QPixmap::trueMatrix(mat, sw, sh);
		QPixmap pm = pixmap.xForm(mat);
		if(!pm.data->alphapm && !pm.mask() && txop == TxRotShear) {
		    QBitmap bm_clip(sw, sh, true);
		    bm_clip.fill(color1);
		    pm.setMask(bm_clip.xForm(mat));
		}
		map(x, y, &x, &y);		// compute position of pixmap
		int dx, dy;
		mat.map(0, 0, &dx, &dy);
		initPaintDevice();
		if(d->qd_info.paintreg.isEmpty())
		    return;
		updatePen();
		unclippedBitBlt(pdev, x-dx, y-dy, &pm, 0, 0, pm.width(),
				 pm.height(), (RasterOp)rop, false, false);
		return;
	    }
	}

	if(txop == TxTranslate)
	    map(x, y, &x, &y);
    }
    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;
    updatePen();
    unclippedBitBlt(pdev, x, y, &pixmap, sx, sy, sw, sh, (RasterOp)rop, false, false);
}

#ifndef USE_CORE_GRAPHICS
static void drawTile(QPainter *p, int x, int y, int w, int h,
		      const QPixmap &pixmap, int xOffset, int yOffset)
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while(yPos < y + h) {
	drawH = pixmap.height() - yOff;    // Cropping first row
	if(yPos + drawH > y + h)        // Cropping last row
	    drawH = y + h - yPos;
	xPos = x;
	xOff = xOffset;
	while(xPos < x + w) {
	    drawW = pixmap.width() - xOff; // Cropping first column
	    if(xPos + drawW > x + w)    // Cropping last column
		drawW = x + w - xPos;
	    p->drawPixmap(xPos, yPos, pixmap, xOff, yOff, drawW, drawH);
	    xPos += drawW;
	    xOff = 0;
	}
	yPos += drawH;
	yOff = 0;
    }
}
#endif

void QPainter::drawTiledPixmap(int x, int y, int w, int h,
				const QPixmap &pixmap, int sx, int sy)
{
    int sw = pixmap.width();
    int sh = pixmap.height();
    if(!sw || !sh)
	return;

    if(sx < 0)
	sx = sw - -sx % sw;
    else
	sx = sx % sw;
    if(sy < 0)
	sy = sh - -sy % sh;
    else
	sy = sy % sh;
#if defined(USE_CORE_GRAPHICS)
    //save the old state
    CGContextSaveGState((CGContextRef)hd);
    //setup the pattern
    QMacPattern *qpattern = new QMacPattern;
    qpattern->im = 0;
    qpattern->as_mask = false;
    qpattern->pixmap = new QPixmap(*cbrush.pixmap());
    CGPatternCallbacks callbks;
    callbks.version = 0;
    callbks.drawPattern = qt_mac_draw_pattern;
    callbks.releaseInfo = qt_mac_dispose_pattern;
    const int width = pixmap.width(), height = pixmap.height();
    CGPatternRef pat = CGPatternCreate(qpattern, CGRectMake(0, 0, width, height), CGContextGetCTM((CGContextRef)hd), width, height,
					      kCGPatternTilingNoDistortion, true, &callbks);
    CGColorSpaceRef cs = CGColorSpaceCreatePattern(NULL);
    CGContextSetFillColorSpace((CGContextRef)hd, cs); 
    const float tmp_float = 1; //wtf?? --SAM (this seems to be necessary, but why!?!) ###
    CGContextSetFillPattern((CGContextRef)hd, pat, &tmp_float);
    CGContextSetPatternPhase((CGContextRef)hd, CGSizeMake(-sx, -sy));
    //fill the rectangle
    CGRect mac_rect;
    d->cg_mac_rect(x, y, w, h, &mac_rect);
    CGContextFillRect((CGContextRef)hd, mac_rect);
    //restore the state
    CGContextRestoreGState((CGContextRef)hd);
    //cleanup
    CGColorSpaceRelease(cs);
    CGPatternRelease(pat);
#else
    drawTile(this, x, y, w, h, pixmap, sx, sy);
#endif
}

void QPainter::drawText(int x, int y, const QString &str, int len, QPainter::TextDirection dir)
{
    drawText(x, y, str, 0, len, dir);
}

void QPainter::drawText(int x, int y, const QString &str, int pos, int len, QPainter::TextDirection dir)
{
    if(!isActive())
	return;
    if(len < 0 || pos + len > (int)str.length())
	len = str.length() - pos;
    if(len == 0 || pos >= (int)str.length())   // empty string
	return;

    if(testf(ExtDev)) {
	QPDevCmdParam param[2];
	QPoint p(x, y);
	QString newstr = str.mid(pos, len);
	param[0].point = &p;
	param[1].str = &newstr;
	if(!pdev->cmd(QPaintDevice::PdcDrawText2,this,param) || !hd)
	    return;
    }

    initPaintDevice();
    if(d->qd_info.paintreg.isEmpty())
	return;

    QTextLayout layout(str, this);
    layout.beginLayout();
    layout.setBoundary(pos);
    layout.setBoundary(pos + len);

    QTextEngine *engine = layout.d;
    if(dir != Auto) {
	int level = (dir == RTL) ? 1 : 0;
	for(int i = engine->items.size(); i >= 0; i--)
	    engine->items[i].analysis.bidiLevel = level;
    }
    // small hack to force skipping of unneeded items
    int start = 0;
    while(engine->items[start].position < pos)
	++start;
    engine->currentItem = start;
    layout.beginLine(0xfffffff);
    int end = start;
    while(!layout.atEnd() && layout.currentItem().from() < pos + len) {
	layout.addCurrentItem();
	end++;
    }
    int ascent = fontMetrics().ascent();
    layout.endLine(0, 0, Qt::AlignLeft, &ascent, 0);
    // do _not_ call endLayout() here, as it would clean up the shaped items and we would do shaping another time
    // for painting.
    for(int i = start; i < end; i++) {
	QScriptItem *si = &engine->items[i];
	QFontEngine *fe = si->fontEngine;
	Q_ASSERT(fe);

	int textFlags = 0;
	if(cfont.d->underline)
	    textFlags |= Qt::Underline;
	if(cfont.d->overline)
	    textFlags |= Qt::Overline;
	if(cfont.d->strikeOut)
	    textFlags |= Qt::StrikeOut;
	fe->draw(this, x + si->x,  y + si->y - ascent, engine, si, textFlags);
    }
}

void QPainter::drawTextItem(int x, int y, const QTextItem &ti, int textFlags)
{
    if(testf(ExtDev)) {
        QPDevCmdParam param[2];
        QPoint p(x, y);
        param[0].point = &p;
        param[1].textItem = &ti;
        if(!pdev->cmd(QPaintDevice::PdcDrawTextItem, this, param) || !hd)
            return;
    }

    QTextEngine *engine = ti.engine;
    QScriptItem &si = engine->items[ti.item];
    engine->shape(ti.item);
    QFontEngine *fe = si.fontEngine;

    Q_ASSERT(fe);
    x += si.x;
    y += si.y;

    fe->draw(this, x,  y, engine, &si, textFlags);
}

QPoint QPainter::pos() const
{
    QPoint ret;
#ifdef USE_CORE_GRAPHICS
    CGPoint pt = CGContextGetPathCurrentPoint((CGContextRef)hd);
    ret = QPoint((int)(pt.x - d->offx), (int)(pt.y - d->offy));
#else
    ((QPainter *)this)->initPaintDevice();
    Point pt;
    GetPen(&pt);
    ret = QPoint(pt.h - d->offx, pt.v - d->offy);
#endif
    return xFormDev(ret);
}

/*!
    \internal
*/
void QPainter::initPaintDevice(bool force, QPoint *off, QRegion *rgn) {
    bool remade_clip = false;
    if(pdev->devType() == QInternal::Printer) {
	if(force && pdev->handle()) {
	    remade_clip = true;
	    d->qd_info.clippedreg = QRegion(0, 0, pdev->metric(QPaintDeviceMetrics::PdmWidth),
					  pdev->metric(QPaintDeviceMetrics::PdmHeight));
	}
    } else if(pdev->devType() == QInternal::Widget) {                    // device is a widget
	paintevent_item *pevent = paintevents.current();
	if(pevent && (*pevent) != pdev)
	    pevent = 0;
	QWidget *w = (QWidget*)pdev, *clip = w;
	if(pevent && pevent->clip())
	    clip = pevent->clip();
	if(!(remade_clip = force)) {
	    if(pevent != d->qd_info.paintevent)
		remade_clip = true;
	    else if(!clip->isVisible())
		remade_clip = d->qd_info.clip_serial;
	    else
		remade_clip = (d->qd_info.clip_serial != clip->clippedSerial(!d->unclipped));
	}
	if(remade_clip) {
	    //offset painting in widget relative the tld
	    QPoint wp(posInWindow(w));
	    d->offx = wp.x();
	    d->offy = wp.y();

	    if(!clip->isVisible()) {
		d->qd_info.clippedreg = QRegion(0, 0, 0, 0); //make the clipped reg empty if not visible!!!
		d->qd_info.clip_serial = 0;
	    } else {
		d->qd_info.clippedreg = clip->clippedRegion(!d->unclipped);
		d->qd_info.clip_serial = clip->clippedSerial(!d->unclipped);
	    }
	    if(pevent) 
		d->qd_info.clippedreg &= pevent->region();
	    d->qd_info.paintevent = pevent;
	}
    } else if(pdev->devType() == QInternal::Pixmap) {             // device is a pixmap
	QPixmap *pm = (QPixmap*)pdev;
	if(force) {//clip out my bounding rect
	    remade_clip = true;
	    d->qd_info.clippedreg = QRegion(0, 0, pm->width(), pm->height());
	}
    }
    if(remade_clip || d->qd_info.crgn_dirty) { 	//update clipped region
	remade_clip = true;
	if(!d->qd_info.clippedreg.isEmpty() && testf(ClipOn)) {
	    d->qd_info.paintreg = crgn;
	    d->qd_info.paintreg.translate(d->offx, d->offy);
	    d->qd_info.paintreg &= d->qd_info.clippedreg;
	} else {
	    d->qd_info.paintreg = d->qd_info.clippedreg;
	}

	CGrafPtr ptr;
	if(pdev->devType() == QInternal::Widget)
	    ptr = GetWindowPort((WindowPtr)pdev->handle());
	else
	    ptr = (GWorldPtr)pdev->handle();
	if(RgnHandle rgn = d->qd_info.paintreg.handle()) {
	    QDAddRegionToDirtyRegion(ptr, rgn);
	} else {
	    QRect qr = d->qd_info.paintreg.boundingRect();
	    Rect mr; SetRect(&mr, qr.x(), qr.y(), qr.right(), qr.bottom());
	    QDAddRectToDirtyRegion(ptr, &mr);
	}
	d->qd_info.crgn_dirty = false;
    }
    if(remade_clip || qt_mac_current_painter != this) {
	QMacSavedPortInfo::setPaintDevice(pdev);
	QMacSavedPortInfo::setClipRegion(d->qd_info.paintreg);
	qt_mac_current_painter = this;
    }
    if(off)
	*off = QPoint(d->offx, d->offy);
    if(rgn)
	*rgn = d->qd_info.paintreg;
}

