/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter.h#3 $
**
** Definition of QPainter class
**
** Author  : Haavard Nord
** Created : 940112
**
** Copyright (C) 1994 by Troll Tech as.  All rights reserved.
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


class QPainter					// painter class
{
friend class QFont;
friend class QPen;
friend class QBrush;
public:
    QPainter();
   ~QPainter();

    bool      begin( const QPaintDevice * );	// begin painting in device
    bool      end();				// end painting

    static bool redirect( const QPaintDevice * );

  // Drawing tools

    QFont     font()  const { return cfont; }	// get/set font
    void      setFont( const QFont & );
    QPen      pen()   const { return cpen; }	// get/set pen
    void      setPen( const QPen & );
    QBrush    brush() const { return cbrush; }	// get/set brush
    void      setBrush( const QBrush & );

  // Drawing attributes/modes

    QColor    backgroundColor() const;		// get/set background color
    void      setBackgroundColor( const QColor & );
    BGMode    backgroundMode() const;		// get/set background mode
    void      setBackgroundMode( BGMode );
    RasterOp  rasterOp() const { return rop; }	// get/set raster operation
    void      setRasterOp( RasterOp );

  // Scaling and transformations

    void      setXForm( bool );			// set xform on/off
    bool      hasXForm() const { return doXForm; }
    QRect     sourceView() const;		// get source view
    void      setSourceView( const QRect & );	// set source view (virtual)
    QRect     targetView() const;		// get target view
    void      setTargetView( const QRect & );	// set target view (device)
    QPoint    xForm( const QPoint & ) const;	// map virtual -> device
    QRect     xForm( const QRect & ) const;
    QPointArray xForm( const QPointArray & ) const;
    QPoint    xFormDev( const QPoint & ) const; // map device -> virtual
    QRect     xFormDev( const QRect & ) const;
    QPointArray xFormDev( const QPointArray & ) const;

  // Clipping

    void      setClipping( bool );		// set clipping on/off
    bool      hasClipping() const { return doClip; }
    void      setRegion( const QRegion & );	// set clip region

  // Graphics drawing functions

    void      drawPoint( int x, int y );
    void      drawPoint( const QPoint & );
    void      moveTo( int x, int y );
    void      moveTo( const QPoint & );
    void      lineTo( int x, int y );
    void      lineTo( const QPoint & );
    void      drawLine( int x1, int y1, int x2, int y2 );
    void      drawLine( const QPoint &, const QPoint & );
    void      drawRect( int x, int y, int w, int h );
    void      drawRect( const QRect & );
    void      drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd );
    void      drawRoundRect( const QRect &, int, int );
    void      drawEllipse( int x, int y, int w, int h );
    void      drawEllipse( const QRect & );
    void      drawArc( int x, int y, int w, int h, int a1, int a2 );
    void      drawArc( const QRect &, int a1, int a2 );
    void      drawPie( int x, int y, int w, int h, int a1, int a2 );
    void      drawPie( const QRect &, int a1, int a2 );
    void      drawChord( int x, int y, int w, int h, int a1, int a2 );
    void      drawChord( const QRect &, int a1, int a2 );
    void      drawLineSegments( const QPointArray & );
    void      drawPolyline( const QPointArray & );
    void      drawPolygon( const QPointArray &, bool winding=FALSE );
    void      drawPixMap( int x, int y, const QPixMap & );

    void      fillRect( int x, int y, int w, int h, const QColor & );
    void      fillRect( const QRect &, const QColor & );
    void      eraseRect( int x, int y, int w, int h );
    void      eraseRect( const QRect & );

  // Shade drawing functions

    void      drawShadeLine( int x1, int y1, int x2, int y2,
			     const QColor &tColor, const QColor &bColor );
    void      drawShadeLine( const QPoint &, const QPoint &,
			     const QColor &tColor, const QColor &bColor );
    void      drawShadeRect( int x, int y, int w, int h,
			     const QColor &tColor, const QColor &bColor,
			     bool fill=FALSE );
    void      drawShadeRect( const QRect &,
			     const QColor &tColor, const QColor &bColor,
			     bool fill=FALSE );
    void      drawShadePanel( int x, int y, int w, int h,
			     const QColor &tColor, const QColor &bColor,
			     int tWidth=2, int bWidth=2,
			     bool fill=FALSE );
    void      drawShadePanel( const QRect &,
			     const QColor &tColor, const QColor &bColor,
			     int tWidth=2, int bWidth=2,
			     bool fill=FALSE );

  // Text drawing functions

    void      drawText( int x, int y, const char *str, int len = -1 );
    void      drawText( const QPoint &, const char *str, int len = -1 );
    void      drawText( int x, int y, int w, int h, TextAlignment,
			const char *str, int len = -1 );
    void      drawText( const QRect &, TextAlignment,
			const char *str, int len = -1 );

private:
    static void changedFont( const QFont *, bool );
    static void changedPen( const QPen *, bool );
    static void changedBrush( const QBrush *, bool );

    void      updateFont();			// update font data
    void      updatePen();			// update pen data
    void      updateBrush();			// update brush data

    static QPaintDevice *pdev_ov;		// overriding paint device
    QPaintDevice *pdev;				// paint device
    QColor    bg_col;				// background color
    BGMode    bg_mode;				// background mode
    RasterOp  rop;				// raster op/transfer mode
    QFont     cfont;				// current font
    QPen      cpen;				// current pen
    QBrush    cbrush;				// current brush
    QRegion   crgn;				// current region

    uint      isActive	 : 1;			// active painting flag
    uint      dirtyFont	 : 1;			// update font flag
    uint      dirtyPen	 : 1;			// update pen flag
    uint      dirtyBrush : 1;			// update brush flag
    uint      doXForm	 : 1;			// xform flag
    uint      doClip	 : 1;			// clipping flag
    uint      extPDev	 : 1;			// external paint device
    int	      sx, sy, sw, sh;			// source rect
    int	      tx, ty, tw, th;			// target rect
#if defined(_WS_WIN_)
    HDC	      hdc;				// device context
#elif defined(_WS_PM_)
    HPS	      hps;				// presentation space
    int	      dh;				// device height
    long      lctrl;				// outline/fill control
    void      updateCtrl();			// update lctrl
#elif defined(_WS_X11_)
    Display  *dpy;				// current display
    WId	      hd;				// handle to drawable
    GC	      gc;				// graphics context (standard)
    GC	      gc_brush;				// graphics contect for brush
    QPoint    curPt;				// current point
#endif
    static QPnList *list;
};


#if !(defined(QPAINTER_C) || defined(DEBUG))

// --------------------------------------------------------------------------
// QPainter member functions
//

inline QColor QPainter::backgroundColor() const
{
    return bg_col;
}

inline BGMode QPainter::backgroundMode() const
{
    return bg_mode;
}

inline void QPainter::drawPoint( const QPoint &p )
{
    drawPoint( p.getX(), p.getY() );
}

inline void QPainter::moveTo( const QPoint &p )
{
    moveTo( p.getX(), p.getY() );
}

inline void QPainter::lineTo( const QPoint &p )
{
    lineTo( p.getX(), p.getY() );
}

inline void QPainter::drawLine( const QPoint &p1, const QPoint &p2 )
{
    drawLine( p1.getX(), p1.getY(), p2.getX(), p2.getY() );
}

inline void QPainter::drawRect( const QRect &r )
{
    drawRect( r.left(), r.top(), r.width(), r.height() );
}

inline void QPainter::drawRoundRect( const QRect &r, int xRnd, int yRnd )
{
    drawRoundRect( r.left(), r.top(), r.width(), r.height(), xRnd, yRnd );
}

inline void QPainter::drawEllipse( const QRect &r )
{
    drawEllipse( r.left(), r.top(), r.width(), r.height() );
}

inline void QPainter::drawArc( const QRect &r, int a1, int a2 )
{
    drawArc( r.left(), r.top(), r.width(), r.height(), a1, a2 );
}

inline void QPainter::drawPie( const QRect &r, int a1, int a2 )
{
    drawPie( r.left(), r.top(), r.width(), r.height(), a1, a2 );
}

inline void QPainter::drawChord( const QRect &r, int a1, int a2 )
{
    drawChord( r.left(), r.top(), r.width(), r.height(), a1, a2 );
}

inline void QPainter::fillRect( const QRect &r, const QColor &c )
{
    fillRect( r.left(), r.top(), r.width(), r.height(), c );
}

inline void QPainter::eraseRect( int x, int y, int w, int h )
{
    fillRect( x, y, w, h, backgroundColor() );
}

inline void QPainter::eraseRect( const QRect &r )
{
    fillRect( r.left(), r.top(), r.width(), r.height(), backgroundColor() );
}

inline void QPainter::drawShadeLine( const QPoint &p1, const QPoint &p2,
				     const QColor &tC, const QColor &bC )
{
    drawShadeLine( p1.getX(), p1.getY(), p2.getX(), p2.getY(), tC, bC );
}

inline void QPainter::drawShadeRect( const QRect &r,
				     const QColor &tC, const QColor &bC,
				     bool fill )
{
    drawShadeRect( r.left(), r.top(), r.width(), r.height(), tC, bC, fill );
}

inline void QPainter::drawShadePanel( const QRect &r,
				      const QColor &tC, const QColor &bC,
				      int tWidth, int bWidth,
				      bool fill )
{
    drawShadePanel( r.left(), r.top(), r.width(), r.height(), tC, bC,
		    tWidth, bWidth, fill );
}

inline void QPainter::drawText( const QPoint &p, const char *s, int len )
{
    drawText( p.getX(), p.getY(), s, len );
}

inline void QPainter::drawText( const QRect &r, TextAlign ta, const char *s,
				int len )
{
    drawText( r.left(), r.top(), r.width(), r.height(), ta, s, len );
}

#endif // inline functions


#endif // QPAINTER_H
