/****************************************************************************
**
** Implementation of QPaintDevice class for Mac.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREEPROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qt_mac.h"
#include "qgc_mac.h"

QPaintDevice *g_cur_paintdev = 0;

QPaintDevice::QPaintDevice(uint devflags)
    : paintEngine(0)
{
    if(!qApp) {				// global constructor
	qFatal("QPaintDevice: Must construct a QApplication before a "
		"QPaintDevice");
	return;
    }
    devFlags = devflags;
    painters = 0;
    hd = 0;
    cg_hd = 0;
#if defined( USE_CORE_GRAPHICS )
    paintEngine = new QCoreGraphicsPaintEngine(this);
#else
    paintEngine = new QQuickDrawPaintEngine(this);
#endif
}

QPaintDevice::~QPaintDevice()
{
    if(paintingActive())
	qWarning("Qt: QPaintDevice: Cannot destroy paint device that is being "
		 "painted.  Be sure to QPainter::end() painters!");
}

int QPaintDevice::metric(int) const
{
    return 0;
}

int QPaintDevice::fontMet(QFont *, int, const char *, int) const
{
    return 0;
}

int QPaintDevice::fontInf(QFont *, int) const
{
    return 0;
}

QPoint posInWindow(QWidget *w);

void unclippedScaledBitBlt(QPaintDevice *dst, int dx, int dy, int dw, int dh,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     Qt::RasterOp rop, bool imask, bool set_fore_colour)
{
    if(!dst || !src) {
	qDebug("Qt: internal: Asked to paint to or from a null paintdevice, something is awry.");
	return;
    }

    if(rop == Qt::NotROP) { //this is the only way we can get a NotROP
	sx = dx;
	sy = dy;
	sw = dw;
	sh = dh;
	src = dst;
	rop = Qt::NotCopyROP;
	imask = TRUE;
    }

    QMacSavedPortInfo savedInfo;
    const bool scalew = (dw != sw), scaleh = (dh != sh);
    if(sx+sw>src->metric(QPaintDeviceMetrics::PdmWidth))
	sw=src->metric(QPaintDeviceMetrics::PdmWidth)-sx;
    if(sy+sh>src->metric(QPaintDeviceMetrics::PdmHeight))
	sh=src->metric(QPaintDeviceMetrics::PdmHeight)-sy;
    if(!sw || !sh)
	return;

    switch (src->devType()) {
    case QInternal::Widget: // OK, can blt from these
    case QInternal::Pixmap:
	break;
    default:
	qWarning("Qt: bitBlt: Cannot bitBlt from device type %x", src->devType());
	return;
    }
    int srcoffx = 0, srcoffy = 0, srcdepth = 0;
    const BitMap *srcbitmap=NULL;
    const QPixmap *srcmask=NULL;
    if(src->devType() == QInternal::Widget) {
	QWidget *w = (QWidget *)src;
	srcdepth = 32; //well, not 0 anyway :)
	if(w->isDesktop()) {
	    GDHandle gdh;
#if 0
	    if(GetWindowGreatestAreaDevice((WindowPtr)w->handle(), kWindowStructureRgn, &gdh, NULL) || !gdh)
		qDebug("Qt: internal: Unexpected condition reached: %s:%d", __FILE__, __LINE__);
#else
	    if(!(gdh=GetMainDevice()))
		qDebug("Qt: internal: Unexpected condition reached: %s:%d", __FILE__, __LINE__);
#endif
	    srcbitmap = (BitMap*)(*(*gdh)->gdPMap);
	} else {
	    srcbitmap = GetPortBitMapForCopyBits(GetWindowPort((WindowPtr)w->handle()));
	    QMacSavedPortInfo::setPaintDevice(w); //wtf?
	    QPoint p(posInWindow(w));
	    srcoffx = p.x();
	    srcoffy = p.y();
	}

	if(sw < 0)
	    sw = w->width();
	else if(sw > sx + w->width())
	    sw = w->width() - sx;
	if(sh < 0)
	    sh = w->height();
	else if(sh > sy + w->height())
	    sh = w->height() - sy;
    } else if(src->devType() == QInternal::Pixmap) {
	QPixmap *pm = (QPixmap *)src;
	srcbitmap = GetPortBitMapForCopyBits((GWorldPtr)pm->handle());
	if(pm->data->alphapm)
	    srcmask = pm->data->alphapm;
	else
	    srcmask = pm->mask();
	srcdepth = pm->depth();

	if(sw < 0)
	    sw = pm->width();
	else if(sw > sx + pm->width())
	    sw = pm->width() - sx;
	if(sh < 0)
	    sh = pm->height();
	else if(sh > sy + pm->height())
	    sh = pm->height() - sy;
    }

    switch (dst->devType()) {
    case QInternal::Printer: // OK, can blt to these
    case QInternal::Widget:
    case QInternal::Pixmap:
	break;
    default:
	qWarning("Qt: bitBlt: Cannot bitBlt to device type %x", dst->devType());
	return;
    }
    //if we are not scaling and we've fixed number we should fix the destination
    if(dw < 0 || (!scalew && sw != dw))
	dw = sw;
    if(dh < 0 || (!scaleh && sh != dh))
	dh = sh;
    int dstoffx=0, dstoffy=0;
    const BitMap *dstbitmap=NULL;
    if(dst->devType() == QInternal::Widget) {
	/* special case when you widget->widget blt */
	if(src != dst && src->devType() == QInternal::Widget) {
	    qDebug("Qt: internal: Need to find a test case FIXME! %s:%d",
		   __FILE__, __LINE__);
	    QPixmap tmppix(dw, dh, 32);
	    unclippedScaledBitBlt(&tmppix, 0, 0, dw, dh, src, sx, sy, sw, sh, rop, imask, TRUE);
	    unclippedScaledBitBlt(dst, dx, dy, dw, dh, &tmppix, 0, 0, dw, dh, rop, imask, TRUE);
	    return;
	}

	QWidget *w = (QWidget *)dst;
	dstbitmap = GetPortBitMapForCopyBits(GetWindowPort((WindowPtr)w->handle()));
	QMacSavedPortInfo::setPaintDevice(w); //wtf?
	if(src == dst) {
	    dstoffx = srcoffx;
	    dstoffy = srcoffy;
	} else {
	    QPoint p(posInWindow(w));
	    dstoffx = p.x();
	    dstoffy = p.y();
	}
	if(!scalew && dw > dx + w->width())
	    dw = w->width() - dx;
	if(!scaleh && dh > dy + w->height())
	    dh = w->height() - dy;
    } else if(dst->devType() == QInternal::Pixmap) {
	QPixmap *pm = (QPixmap *)dst;
	pm->detach(); //must detach when we blt
	dstbitmap = GetPortBitMapForCopyBits((GWorldPtr)pm->handle());
	if(!scalew && dw > dx + pm->width())
	    dw = pm->width() - dx;
	if(!scaleh && dh > dy + pm->height())
	    dh = pm->height() - dy;
    } else if(dst->devType() == QInternal::Printer) {
	dstbitmap = GetPortBitMapForCopyBits((GWorldPtr)dst->handle());
    }


#if 0 // ### port
    if(dst->paintingActive() && dst->isExtDev()) {
	QPixmap *pm;				// output to picture/printer
	bool	 tmp_pm = FALSE;;
	if(src->devType() == QInternal::Pixmap) {
	    pm = (QPixmap*)src;
	    if(sx != 0 || sy != 0 || sw != pm->width() || sh != pm->height() || imask) {
		tmp_pm = TRUE;
		QPixmap *tmp = new QPixmap(sw, sh, pm->depth());
		bitBlt(tmp, 0, 0, pm, sx, sy, sw, sh, Qt::CopyROP, TRUE);
		if(pm->mask() && !imask) {
		    QBitmap mask(sw, sh);
		    bitBlt(&mask, 0, 0, pm->mask(), sx, sy, sw, sh,
			    Qt::CopyROP, TRUE);
		    tmp->setMask(mask);
		}
		pm = tmp;
	    }
	} else if(src->devType() == QInternal::Widget) {// bitBlt to temp pixmap
	    tmp_pm = TRUE;
	    pm = new QPixmap(sw, sh);
	    bitBlt(pm, 0, 0, src, sx, sy, sw, sh);
	} else {
	    qWarning("Qt: bitBlt: Cannot bitBlt from device");
	    return;
	}
	QPDevCmdParam param[3];
	QPoint p(dx,dy);
	param[0].point	= &p;
	param[1].pixmap = pm;
	bool ret = dst->cmd(QPaintDevice::PdcDrawPixmap, 0, param);
	if(tmp_pm)
	    delete pm;
	if(!ret || !dstbitmap)
	    return;
    }
#endif

    //if we are not scaling and we've fixed number we should fix the source
    if(!scalew && sw != dw)
	sw = dw;
    if(!scaleh && sh != dh)
	sh = dh;

    if(!dstbitmap || !srcbitmap) {
	qDebug("Qt: internal: bitBlt: Unexpected condition reached %d", __LINE__);
	return;
    }

    ::RGBColor f;
    if(set_fore_colour || srcdepth > 1) {
	f.red = f.green = f.blue = 0;
	RGBForeColor(&f);
    }
    f.red = f.green = f.blue = ~0;
    RGBBackColor(&f);

    short copymode;
    switch(rop) {
    default:
    case Qt::CopyROP:   copymode = srcCopy; break;
    case Qt::OrROP:     copymode = notSrcBic; break;
    case Qt::XorROP:    copymode = srcXor; break;
    case Qt::NotAndROP: copymode = srcBic; break;
    case Qt::NotCopyROP:copymode = notSrcCopy; break;
    case Qt::NotOrROP:  copymode = notSrcOr; break;
    case Qt::NotXorROP: copymode = notSrcXor; break;
    case Qt::AndROP:     copymode = notSrcBic; break;
/*
  case NotROP:      dst = NOT dst
  case ClearROP:    dst = 0
  case SetROP:      dst = 1
  case NopROP:      dst = dst
  case AndNotROP:   dst = src AND (NOT dst)
  case OrNotROP:    dst = src OR (NOT dst)
  case NandROP:     dst = NOT (src AND dst)
  case NorROP:      dst = NOT (src OR dst)
*/
    }

    Rect srcr;
    SetRect(&srcr,sx+srcoffx,sy+srcoffy,sx+sw+srcoffx+1,sy+sh+srcoffy+1);
    Rect dstr;
    SetRect(&dstr,dx+dstoffx,dy+dstoffy,dx+dw+dstoffx+1,dy+dh+dstoffy+1);
    if(srcmask && !imask) {
	const BitMap *maskbits = GetPortBitMapForCopyBits((GWorldPtr)srcmask->handle());
	if(copymode == srcCopy && srcmask->depth() > 1)
	    copymode = ditherCopy;
	if(dst->devType() == QInternal::Printer) { //can't use CopyDeepMask on a printer
	    QPixmap tmppix(dw, dh, srcdepth);
	    Rect pixr;
	    SetRect(&pixr, 0, 0, dw+1, dh+1);
	    const BitMap *pixbits = GetPortBitMapForCopyBits((GWorldPtr)tmppix.handle());
	    {
		QMacSavedPortInfo pi(&tmppix);
		EraseRect(&pixr);
		CopyDeepMask(srcbitmap, maskbits, pixbits, &srcr, &srcr, &pixr, copymode, 0);
	    }
	    CopyBits(pixbits, dstbitmap, &pixr, &dstr, srcOr, 0); //use srcOr transfer, to "emulate" the mask
	} else {
	    CopyDeepMask(srcbitmap, maskbits, dstbitmap, &srcr, &srcr, &dstr, copymode, 0);
	}
    } else {
	CopyBits(srcbitmap, dstbitmap, &srcr, &dstr, copymode, 0);
    }

#ifndef QMAC_ONE_PIXEL_LOCK
    if(dst->devType() == QInternal::Pixmap) {
	QPixmap *pm = (QPixmap *)dst;
	UnlockPixels(GetGWorldPixMap((GWorldPtr)pm->handle()));
    }
#endif
}

void unclippedBitBlt(QPaintDevice *dst, int dx, int dy,
		      const QPaintDevice *src, int sx, int sy, int sw, int sh,
		      Qt::RasterOp rop, bool imask, bool set_fore_colour)
{
    unclippedScaledBitBlt(dst, dx, dy, sw, sh, src, sx, sy, sw, sh, rop, imask, set_fore_colour);
}

void scaledBitBlt(QPaintDevice *dst, int dx, int dy, int dw, int dh,
		   const QPaintDevice *src, int sx, int sy, int sw, int sh,
		   Qt::RasterOp rop, bool imask)
{
  //at the end of this function this will go out of scope and the destructor will restore the state
  QMacSavedPortInfo saveportstate(dst);

  if(dst && dst->devType() == QInternal::Widget) {
      QMacSavedPortInfo::setClipRegion(((QWidget *)dst)->clippedRegion());
  } else if(dst && dst->devType() == QInternal::Pixmap) {
      QPixmap *pm = (QPixmap *)dst;
      QMacSavedPortInfo::setClipRegion(QRect(0, 0, pm->width(), pm->height()));
  }
  unclippedScaledBitBlt(dst, dx, dy, dw, dh, src, sx, sy, sw, sh, rop, imask, TRUE);
}

void bitBlt(QPaintDevice *dst, int dx, int dy,
	     const QPaintDevice *src, int sx, int sy, int sw, int sh,
	     Qt::RasterOp rop, bool imask)
{
    QPoint redirection_offset;
    const QPaintDevice *redirected = QPainter::redirected(dst, &redirection_offset);
    if (redirected) {
	dst = const_cast<QPaintDevice*>(redirected);
 	dx -= redirection_offset.x();
 	dy -= redirection_offset.y();
    }

    scaledBitBlt(dst, dx, dy, sw, sh, src, sx, sy, sw, sh, rop, imask);
}

Q_GUI_EXPORT void copyBlt( QPixmap *dst, int dx, int dy,
		       const QPixmap *src, int sx, int sy, int sw, int sh )
{
    if ( ! dst || ! src || sw == 0 || sh == 0 || dst->depth() != src->depth() ) {
#ifdef QT_CHECK_NULL
	Q_ASSERT( dst != 0 );
	Q_ASSERT( src != 0 );
#endif
	return;
    }

    // copy pixel data
    bitBlt( dst, dx, dy, src, sx, sy, sw, sh, Qt::CopyROP, TRUE );

    // copy mask data
    if ( src->data->mask ) {
	if ( ! dst->data->mask ) {
	    dst->data->mask = new QBitmap( dst->width(), dst->height() );

	    // new masks are fully opaque by default
	    dst->data->mask->fill( Qt::color1 );
	}

	bitBlt( dst->data->mask, dx, dy,
		src->data->mask, sx, sy, sw, sh, Qt::CopyROP, TRUE );
    }

#ifdef QMAC_PIXMAP_ALPHA
    // copy alpha data
    if ( ! src->data->alphapm )
	return;

    if ( sw < 0 )
	sw = src->width() - sx;
    else
	sw = qMin( src->width()-sx, sw );
    sw = qMin( dst->width()-dx, sw );

    if ( sh < 0 )
	sh = src->height() - sy ;
    else
	sh = qMin( src->height()-sy, sh );
    sh = qMin( dst->height()-dy, sh );

    if ( sw <= 0 || sh <= 0 )
	return;

    if ( ! dst->data->alphapm ) {
	dst->data->alphapm = new QPixmap( dst->data->w, dst->data->h, 32 );

	// new alpha pixmaps are fully opaque by default
	dst->data->alphapm->fill( Qt::black );
    }

    bitBlt( dst->data->alphapm, dx, dy,
	    src->data->alphapm, sx, sy, sw, sh, Qt::CopyROP, TRUE );
#endif // QMAC_PIXMAP_ALPHA
}

void qt_mac_clip_cg_handle(CGContextRef hd, const QRegion &rgn, const QPoint &offp, bool combine)
{
    if(rgn.isEmpty()) {
	CGContextBeginPath(hd);
	CGContextAddRect(hd, CGRectMake(0, 0, 0, 0));
	CGContextClip(hd);
    } else {
	if(!combine) {
#if 1
	    QRect qrect = QRect(0, 0, 99999, 999999);
#else
	    /* I have no idea why this doesn't work, something about the translation applied to the CGContextRef
	       I suspect, I'll have to experiment, but for now just reset it as I do above!! FIXME!! ## --Sam */
	    QRect qrect = rgn.boundingRect();
	    qrect.moveBy(offp);
#endif
	    Rect qdr; SetRect(&qdr, qrect.left(), qrect.top(), qrect.right()+1, qrect.bottom()+1);
	    ClipCGContextToRegion(hd, &qdr, QRegion(qrect).handle(true));
	}
	QVector<QRect> rects = rgn.rects();
	const int count = rects.size();
	CGRect *cg_rects = (CGRect *)malloc(sizeof(CGRect)*count);
	for(int i = 0; i < count; i++) {
	    const QRect &r = rects[i];
	    cg_rects[i] = CGRectMake(r.x()+offp.x(), r.y()+offp.y(), r.width(), r.height());
	}
	CGContextClipToRects(hd, cg_rects, count);
	free(cg_rects);
    }
}

/*!
    Returns the window system handle of the paint device for
    CoreGraphics support. Use of this function is not portable. This
    function will return 0 if the handle could not be created.
*/
Qt::HANDLE QPaintDevice::macCGHandle() const
{
    return cg_hd;
}

Qt::HANDLE QPaintDevice::handle() const
{
    return hd;
}

void QPaintDevice::setResolution(int)
{
}

int QPaintDevice::resolution() const
{
    return metric(QPaintDeviceMetrics::PdmDpiY);
}

