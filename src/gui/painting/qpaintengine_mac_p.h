/****************************************************************************
**
** Definition of QPaintEngine(for Mac) private data.
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

#ifndef __QPAINTENGINE_MAC_P_H__
#define __QPAINTENGINE_MAC_P_H__

/*****************************************************************************
  QuickDraw Private data
 *****************************************************************************/
static int ropCodes[] = {			// ROP translation table
    patCopy, patOr, patXor, patBic, notPatCopy,
    notPatOr, notPatXor, notPatBic,
    666, 666, 666, 666, 666, 666, 666, 666, 666
};

class paintevent_item;
class QQuickDrawPaintEnginePrivate
{
public:
    QQuickDrawPaintEnginePrivate() {
	saved = 0;
	paintevent = 0;
	clip.serial = 0;
	clip.dirty = locked = unclipped = false;
	clip.pdev = clip.paintable = QRegion();
	brush_style_pix = 0;
	offx = offy = 0;
    }

    QPaintDevice *pdev;
    struct {
	QPen pen;
	QBrush brush;
	QFont font;
	Qt::RasterOp rop;
	QRegion clip;
	struct {
	    QPoint origin;
	    Qt::BGMode mode;
	    QColor color;
	} bg;
    } current;

    int offx, offy;
    QPixmap *brush_style_pix;
    uint unclipped : 1, locked : 1;
    QMacSavedPortInfo *saved;
    paintevent_item *paintevent;

    struct {
	QRegion pdev, paintable;
	uint dirty : 1, serial : 15;
    } clip;

};

/*****************************************************************************
  Private data
 *****************************************************************************/
class QCoreGraphicsPaintEnginePrivate
{
public:
    QCoreGraphicsPaintEnginePrivate() {
	hd = 0;
	pdev = 0;
	unclipped = 0;
	brush_style_pix = 0;
	offx = offy = 0;
	fill_pattern = 0;
	fill_colorspace = 0;
    }

    struct {
	QPen pen;
	QBrush brush;
	struct {
	    QPoint origin;
	    Qt::BGMode mode;
	    QColor color;
	} bg;
    } current;

    CGContextRef hd;
    int offx, offy;
    uint unclipped : 1;
    QPaintDevice *pdev;
    QPixmap *brush_style_pix;
    CGPatternRef fill_pattern;
    CGColorSpaceRef fill_colorspace;

    inline void mac_point(const int &inx, const int &iny, float *outx, float *outy, bool global=false) {
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
    inline void mac_point(const int &inx, const int &iny, CGPoint *p, bool global=false) {
	mac_point(inx, iny, &p->x, &p->y, global);
    }
    inline void mac_rect(const int &inx, const int &iny, const int &inw, const int &inh, CGRect *rct, bool global=false) {
	*rct = CGRectMake(0, 0, inw, inh);
	mac_point(inx, iny, &rct->origin, global);
    }
};

#endif /* __QPAINTENGINE_MAC_P_H__ */
