/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.h#8 $
**
** Definition of QPainter class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QPAINTER_H
#define QPAINTER_H

#include "qpaintd.h"
#include "qcolor.h"
#include "qfontmet.h"
#include "qregion.h"
#include "qpen.h"
#include "qbrush.h"


enum BGMode					// background mode
    { TransparentMode, OpaqueMode };

enum PaintUnit					// paint unit
    { PixelUnit, LoMetricUnit, HiMetricUnit, LoEnglishUnit, HiEnglishUnit,
      TwipsUnit };


class QPainter					// painter class
{
friend class QFont;
friend class QPen;
friend class QBrush;
public:
    QPainter();
   ~QPainter();

    bool	begin( const QPaintDevice * );	// begin painting in device
    bool	end();				// end painting
    QPaintDevice *device() const { return pdev; }

    static bool redirect( const QPaintDevice * );

    bool	isActive() const { return testf(IsActive); }

    void	save();				// save (push) current state
    void	restore();			// restore (pop) state

  // Drawing tools

    QFont      &font() { return cfont; }	// get/set font
    QFont	getFont() const { return cfont; }
    void	setFont( const QFont & );
    QPen       &pen() { return cpen; }		// get/set pen
    QPen	getPen() const { return cpen; }
    void	setPen( const QPen & );
    QBrush     &brush() { return cbrush; }	// get/set brush
    QBrush	getBrush() const { return cbrush; }
    void	setBrush( const QBrush & );

  // Drawing attributes/modes

    QColor	backgroundColor() const;
    void	setBackgroundColor( const QColor & );
    BGMode	backgroundMode() const;
    void	setBackgroundMode( BGMode );
    RasterOp	rasterOp() const;
    void	setRasterOp( RasterOp );
    QPoint	brushOrigin() const;
    void	setBrushOrigin( int x, int y );
    void	setBrushOrigin( const QPoint & );

  // Scaling and transformations

    PaintUnit	unit()	       const;		// get set painter unit
    void	setUnit( PaintUnit );
    void	setViewXForm( bool );		// set xform on/off
    bool	hasViewXForm() const { return testf(VxF); }
    QRect	sourceView()   const;		// get source view
    void	setSourceView( const QRect & );	// set source view (virtual)
    QRect	targetView()   const;		// get target view
    void	setTargetView( const QRect & );	// set target view (device)

    void	setWorldXForm( bool );		// set world xform on/off
    bool	hasWorldXForm() const { return testf(WxF); }
    QWXFMatrix *wxfMatrix()	const;		// get/set world xform matrix
    void	setWxfMatrix( const QWXFMatrix &, bool concat=FALSE );

    QPoint	xForm( const QPoint & ) const;	// map virtual -> device
    QRect	xForm( const QRect & )	const;
    QPointArray xForm( const QPointArray & ) const;
    QPoint	xFormDev( const QPoint & ) const; // map device -> virtual
    QRect	xFormDev( const QRect & )  const;
    QPointArray xFormDev( const QPointArray & ) const;

  // Clipping

    void	setClipping( bool );		// set clipping on/off
    bool	hasClipping() const { return testf(ClipOn); }
    void	setClipRect( const QRect & );	// set clip rectangle
    void	setClipRegion( const QRegion &);// set clip region

  // Graphics drawing functions

    void	drawPoint( int x, int y );
    void	drawPoint( const QPoint & );
    void	moveTo( int x, int y );
    void	moveTo( const QPoint & );
    void	lineTo( int x, int y );
    void	lineTo( const QPoint & );
    void	drawLine( int x1, int y1, int x2, int y2 );
    void	drawLine( const QPoint &, const QPoint & );
    void	drawRect( int x, int y, int w, int h );
    void	drawRect( const QRect & );
    void	drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd);
    void	drawRoundRect( const QRect &, int, int );
    void	drawEllipse( int x, int y, int w, int h );
    void	drawEllipse( const QRect & );
    void	drawArc( int x, int y, int w, int h, int a1, int a2 );
    void	drawArc( const QRect &, int a1, int a2 );
    void	drawPie( int x, int y, int w, int h, int a1, int a2 );
    void	drawPie( const QRect &, int a1, int a2 );
    void	drawChord( int x, int y, int w, int h, int a1, int a2 );
    void	drawChord( const QRect &, int a1, int a2 );
    void	drawLineSegments( const QPointArray &,
				  int index=0, int nlines=-1 );
    void	drawPolyline( const QPointArray &,
			      int index=0, int npoints=-1 );
    void	drawPolygon( const QPointArray &, bool winding=FALSE,
			     int index=0, int npoints=-1 );
    void	drawPixMap( int x, int y, const QPixMap & );
    void	drawMetaFile( const QMetaFile & );

    void	fillRect( int x, int y, int w, int h, const QColor & );
    void	fillRect( const QRect &, const QColor & );
    void	eraseRect( int x, int y, int w, int h );
    void	eraseRect( const QRect & );

  // Shade drawing functions

    void	drawShadeLine( int x1, int y1, int x2, int y2,
			       const QColor &tColor, const QColor &bColor );
    void	drawShadeLine( const QPoint &, const QPoint &,
			       const QColor &tColor, const QColor &bColor );
    void	drawShadeRect( int x, int y, int w, int h,
			       const QColor &tColor, const QColor &bColor,
			       bool fill=FALSE );
    void	drawShadeRect( const QRect &,
			       const QColor &tColor, const QColor &bColor,
			       bool fill=FALSE );
    void	drawShadePanel( int x, int y, int w, int h,
				const QColor &tColor, const QColor &bColor,
				int tWidth=2, int bWidth=2,
				bool fill=FALSE );
    void	drawShadePanel( const QRect &,
				const QColor &tColor, const QColor &bColor,
				int tWidth=2, int bWidth=2,
				bool fill=FALSE );

  // Text drawing functions

    void	drawText( int x, int y, const char *str, int len = -1 );
    void	drawText( const QPoint &, const char *str, int len = -1 );
    void	drawText( int x, int y, int w, int h, TextAlignment,
			  const char *str, int len = -1 );
    void	drawText( const QRect &, TextAlignment,
			  const char *str, int len = -1 );

private:
    static void changedFont( const QFont *, bool );
    static void changedPen( const QPen *, bool );
    static void changedBrush( const QBrush *, bool );

    void	updateFont();			// update font data
    void	updatePen();			// update pen data
    void	updateBrush();			// update brush data
    void	updateXForm();			// update internal xform params

    enum { IsActive=0x01, DirtyFont=0x02, DirtyPen=0x04, DirtyBrush=0x08,
	   VxF=0x10, WxF=0x20, ClipOn=0x40, ExtDev=0x80, SafePolygon=0x100 };
    ushort	flags;				// painter flags
    bool	testf( ushort b ) const	{ return (flags&b)!=0; }
    void	setf( ushort b )	{ flags |= b; }
    void	setf( ushort b, bool v );
    void	clearf( ushort b )	{ flags &= ~b; }

    static QPaintDevice *pdev_ov;		// overriding paint device
    QPaintDevice *pdev;				// paint device
    QColor	bg_col;				// background color
    uchar	bg_mode;			// background mode
    uchar	rop;				// raster op/transfer mode
    uchar	pu;				// coordinate unit
    QPoint	bro;				// brush origin
    QFont	cfont;				// current font
    QPen	cpen;				// current pen
    QBrush	cbrush;				// current brush
    QRegion	crgn;				// current region
    QCOOT	sx, sy, sw, sh;			// source rect
    QCOOT	tx, ty, tw, th;			// target rect
    QWXFMatrix *wxfmat;				// world xform matrix
    QWXFMatrix *wxfimat;			// inverse xform matrix
#if defined(_WS_MAC_) || defined(_WS_WIN16_) || defined(_WS_X11_)
    long	wm11, wm12, wm21, wm22, wdx, wdy;
    long	im11, im12, im21, im22, idx, idy;
#endif
    void       *ps_stack;			// painter save/restore stack
    void	killPStack();

protected:
#if defined(_WS_WIN_)
    HDC		hdc;				// device context
#elif defined(_WS_PM_)
    HPS		hps;				// presentation space
    int		dh;				// device height
    long	lctrl;				// outline/fill control
    void	updateCtrl();			// update lctrl
#elif defined(_WS_X11_)
    Display    *dpy;				// current display
    WId		hd;				// handle to drawable
    GC		gc;				// graphics context (standard)
    GC		gc_brush;			// graphics contect for brush
    QPoint	curPt;				// current point
#endif
    static QPnList *list;
};


// --------------------------------------------------------------------------
// QPainter member functions
//

inline QColor QPainter::backgroundColor() const
{
    return bg_col;
}

inline BGMode QPainter::backgroundMode() const
{
    return (BGMode)bg_mode;
}

inline RasterOp QPainter::rasterOp() const
{
    return (RasterOp)rop;
}

inline QPoint QPainter::brushOrigin() const
{
    return bro;
}

inline PaintUnit QPainter::unit() const
{
    return (PaintUnit)pu;
}


#if !(defined(QPAINTER_C) || defined(DEBUG))

inline void QPainter::setBrushOrigin( const QPoint &p )
{
    setBrushOrigin( p.x(), p.y() );
}

inline void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.x(), p.y() );
}

inline void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.x(), p.y() );
}

inline void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.x(), p.y() );
}

inline void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.x(), p1.y(), p2.x(), p2.y() );
}

inline void QPainter::drawRect( const QRect &r )
{
    drawRect( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.x(), r.y(), r.width(), r.height(), xRnd, yRnd );
}

inline void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.x(), r.y(), r.width(), r.height() );
}

inline void QPainter::drawArc( const QRect &r, int a1, int a2 )
{
    drawArc( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

inline void QPainter::drawPie( const QRect &r, int a1, int a2 )
{
    drawPie( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

inline void QPainter::drawChord( const QRect &r, int a1, int a2 )
{
    drawChord( r.x(), r.y(), r.width(), r.height(), a1, a2 );
}

inline void QPainter::fillRect( const QRect &r, const QColor &c )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), c );
}

inline void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

inline void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.x(), r.y(), r.width(), r.height(), backgroundColor() );
}

inline void QPainter::drawShadeLine( const QPoint &p1, const QPoint &p2,
				     const QColor &tC, const QColor &bC )
{
    drawShadeLine( p1.x(), p1.y(), p2.x(), p2.y(), tC, bC );
}

inline void QPainter::drawShadeRect( const QRect &r,
				     const QColor &tC, const QColor &bC,
				     bool fill )
{
    drawShadeRect( r.x(), r.y(), r.width(), r.height(), tC, bC, fill );
}

inline void QPainter::drawShadePanel( const QRect &r,
				      const QColor &tC, const QColor &bC,
				      int tWidth, int bWidth,
				      bool fill )
{
    drawShadePanel( r.x(), r.y(), r.width(), r.height(), tC, bC,
		    tWidth, bWidth, fill );
}

inline void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.x(), p.y(), s, len );
}

inline void QPainter::drawText( const QRect &r, TextAlignment ta,
				const char *str, int len )
{
    drawText( r.x(), r.y(), r.width(), r.height(), ta, str, len );
}

#endif // inline functions


#endif // QPAINTER_H
