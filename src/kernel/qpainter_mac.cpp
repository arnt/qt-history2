/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpainter_mac.cpp#316 $
**
** Implementation of QPainter class for Mac
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11/FIXME may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpainter.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qpixmapcache.h"
#include "qlist.h"
#include "qintdict.h"
#include "qfontdata_p.h"
#include "qtextcodec.h"
#include <ctype.h>
#include <stdlib.h>
#include "qpaintdevicemetrics.h"
#include "qpaintdevice.h"
#include "qt_mac.h"

const int TxNone=0;
const int TxTranslate=1;
const int TxScale=2;
const int TxRotShear=3;

/*****************************************************************************
  Trigonometric function for QPainter

  We have implemented simple sine and cosine function that are called from
  QPainter::drawPie() and QPainter::drawChord() when drawing the outline of
  pies and chords.
  These functions are slower and less accurate than math.h sin() and cos(),
  but with still around 1/70000th sec. execution time (on a 486DX2-66) and
  8 digits accuracy, it should not be the bottleneck in drawing these shapes.
  The advantage is that you don't have to link in the math library.
 *****************************************************************************/


/*****************************************************************************
  QPainter internal GC (Graphics Context) cache for solid pens and
  brushes.

  The GC cache makes a significant contribution to speeding up
  drawing.  Setting new pen and brush colors will make the painter
  look for another GC with the same color instead of changing the
  color value of the GC currently in use. The cache structure is
  optimized for fast lookup.  Only solid line pens with line width 0
  and solid brushes are cached.

  In addition, stored GCs may have an implicit clipping region
  set. This prevents any drawing outside paint events. Both
  updatePen() and updateBrush() keep track of the validity of this
  clipping region by storing the clip_serial number in the cache.

*****************************************************************************/


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/


/*!
  Internal function that initializes the painter.
*/

void QPainter::initialize()
{
    qDebug( "QPainter::initialize" );
}

/*!
  Internal function that cleans up the painter.
*/

void QPainter::cleanup()
{
    qDebug( "QPainter::cleanup" );
}


typedef QIntDict<QPaintDevice> QPaintDeviceDict;
static QPaintDeviceDict *pdev_dict = 0;

/*!
  Redirects all paint command for a paint device \a pdev to another
  paint device \a replacement, unless \a replacement is 0.  If \a
  replacement is 0, the redirection for \a pdev is removed.

  Mostly, you can get better results with less work by calling
  QPixmap::grabWidget() or QPixmap::grapWindow().
*/

void QPainter::redirect( QPaintDevice *, QPaintDevice * )
{
    qDebug( "QPainer::redirect" );
}


void QPainter::init()
{
    qDebug( "QPainter::init implemented" );
    flags = IsStartingUp;
    bg_col = white;                             // default background color
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
    hd = 0;
}


/*!
  \fn const QFont &QPainter::font() const

  Returns the currently set painter font.
  \sa setFont(), QFont
*/

/*!
  Sets a new painter font.

  This font is used by subsequent drawText() functions.  The text
  color is the same as the pen color.

  \sa font(), drawText()
*/

void QPainter::setFont( const QFont &font )
{
#if defined(CHECK_STATE)
    if ( !isActive() )
        warning( "QPainter::setFont: Will be reset by begin()" );
#endif
    if ( cfont.d != font.d ) {
        cfont = font;
        setf(DirtyFont);
    }
}


void QPainter::updateFont()
{
    clearf(DirtyFont);
    if ( testf(ExtDev) ) {
        QPDevCmdParam param[1];
        param[0].font = &cfont;
        if ( !pdev->cmd(QPaintDevice::PdcSetFont,this,param) || !hd )
            return;
    }
    setf(NoCache);
    if ( penRef )
        updatePen();                            // force a non-cached GC
    //    Qt::HANDLE h = cfont.handle();
    cfont.macSetFont(pdev);
}


void QPainter::updatePen()
{
    qDebug( "QPainter::updatePen implemented" );
    if ( !pdev || !pdev->handle() )
	return;
    WindowPtr p = (WindowPtr)pdev->handle();
    SetPortWindowPort( p );

    ::RGBColor f;
    f.red = cpen.color().red()*256;
    f.green = cpen.color().green()*256;
    f.blue = cpen.color().blue()*256;
    RGBForeColor( &f );
    int s = cpen.width();
    if ( s < 1 )
	s = 1;
    PenSize( s, s );
}


void QPainter::updateBrush()
{
    qDebug( "QPainter::updateBrush implemented" );
    if( !pdev || !pdev->handle() )
	return;

    if ( testf( ExtDev ) ) {
        QPDevCmdParam param[1];
        param[0].brush = &cbrush;
        if ( !pdev->cmd( QPaintDevice::PdcSetBrush,this,param) || !hd )
            return;
    }

    int  bs = cbrush.style();
    uchar *pat = 0;                             // pattern

    if( !pdev )
	return;

    if ( bs == CustomPattern || pat ) {
        QPixmap *pm;
        pm = cbrush.data->pixmap;
    }

    ::RGBColor f;
    f.red = cbrush.color().red()*256;
    f.green = cbrush.color().green()*256;
    f.blue = cbrush.color().blue()*256;
    RGBForeColor( &f );
    if ( pdev->devType() == QInternal::Widget ) {
	bg_col = ((QWidget *)pdev)->backgroundColor();
    }
}


/*!
  Begins painting the paint device \a pd and returns TRUE if successful,
  or FALSE if an error occurs.

  The errors that can occur are serious problems, such as these:

  \code
    p->begin( 0 ); // impossible - paint device cannot be 0

    QPixmap pm( 0, 0 );
    p->begin( pm ); // impossible - pm.isNull();

    p->begin( myWidget );
    p2->begin( myWidget ); // impossible - only one painter at a time
  \endcode

  Note that most of the time, you can use one of the constructors
  instead of begin(), and that end() is automatically done at
  destruction.

  \warning A paint device can only be painted by one painter at a time.

  \sa end(), flush()
*/

typedef QIntDict<QPaintDevice> QPaintDeviceDict;

bool QPainter::begin( const QPaintDevice *pd )
{
    qDebug( "QPainter::begin implemented x11 like" );

    if ( isActive() ) {                         // already active painting
#if defined(CHECK_STATE)
        warning( "QPainter::begin: Painter is already active."
                 "\n\tYou must end() the painter before a second begin()" );
#endif
        return FALSE;
    }
    if ( pd == 0 ) {
#if defined(CHECK_NULL)
        warning( "QPainter::begin: Paint device cannot be null" );
#endif
        return FALSE;
    }

    QWidget *copyFrom = 0;
    if ( pdev_dict ) {                          // redirected paint device?
        pdev = pdev_dict->find( (long)pd );
        if ( pdev ) {
            if ( pd->devType() == QInternal::Widget )
                copyFrom = (QWidget *)pd;       // copy widget settings
        } else {
            pdev = (QPaintDevice *)pd;
        }
    } else {
        pdev = (QPaintDevice *)pd;
    }

    if ( pdev->isExtDev() && pdev->paintingActive() ) {
	// somebody else is already painting
#if defined(CHECK_STATE)
        warning( "QPainter::begin: Another QPainter is already painting "
                 "this device;\n\tAn extended paint device can only be painted "
                 "by one QPainter at a time." );
#endif
        return FALSE;
    }

    bool reinit = flags != IsStartingUp;        // 2nd or 3rd etc. time called
    flags = IsActive | DirtyFont;               // init flags
    int dt = pdev->devType();                   // get the device type

    if ( (pdev->devFlags & QInternal::ExternalDevice) != 0 )
	// this is an extended device
        setf(ExtDev);
    else if ( dt == QInternal::Pixmap )         // device is a pixmap
        ((QPixmap*)pdev)->detach();             // will modify it

    if ( testf( ExtDev ) ) {                      // external device
        if ( !pdev->cmd( QPaintDevice::PdcBegin, this, 0) ) {   // could not begin painting
            pdev = 0;
            return FALSE;
        }
        if ( tabstops )                         // update tabstops for device
            setTabStops( tabstops );
        if ( tabarray )                         // update tabarray for device
            setTabArray( tabarray );
    }

    pdev->painters++;                           // also tell paint device
    hd = pdev->hd;
    bro = QPoint( 0, 0 );
    if ( reinit ) {
        bg_mode = TransparentMode;              // default background mode
        rop = CopyROP;                          // default ROP
        wxmat.reset();                          // reset world xform matrix
        txop = txinv = 0;
        if ( dt != QInternal::Widget ) {
            QFont  defaultFont;                 // default drawing tools
            QPen   defaultPen;
            QBrush defaultBrush;
            cfont  = defaultFont;               // set these drawing tools
            cpen   = defaultPen;
            cbrush = defaultBrush;
            bg_col = white;                     // default background color
	    // was white
        }
    }
    wx = wy = vx = vy = 0;                      // default view origins

    if ( dt == QInternal::Widget ) {                    // device is a widget
        QWidget *w = (QWidget*)pdev;
        cfont = w->font();                      // use widget font
        cpen = QPen( w->foregroundColor() );    // use widget fg color
        if ( reinit ) {
            QBrush defaultBrush;
            cbrush = defaultBrush;
        }
        bg_col = w->backgroundColor();          // use widget bg color
        ww = vw = w->width();                   // default view size
        wh = vh = w->height();
        if ( w->testWFlags(WPaintUnclipped) ) { // paint direct on device
            setf( NoCache );
            updatePen();
            updateBrush();
        }
    } else if ( dt == QInternal::Pixmap ) {             // device is a pixmap
        QPixmap *pm = (QPixmap*)pdev;
        if ( pm->isNull() ) {
#if defined(CHECK_NULL)
            warning( "QPainter::begin: Cannot paint null pixmap" );
#endif
            end();
            return FALSE;
        }
        ww = vw = pm->width();                  // default view size
        wh = vh = pm->height();
    } else if ( testf(ExtDev) ) {               // external device
        // FIXME: Untested modification
        ww = vw = pdev->metric( QPaintDeviceMetrics::PdmWidth ); // sanders
        wh = vh = pdev->metric( QPaintDeviceMetrics::PdmHeight ); // sanders
    }
    if ( ww == 0 )
        ww = wh = vw = vh = 1024;
    if ( copyFrom ) {                           // copy redirected widget
        cfont = copyFrom->font();
        cpen = QPen( copyFrom->foregroundColor() );
        bg_col = copyFrom->backgroundColor();
    }
    if ( testf(ExtDev) ) {                      // external device
        setBackgroundColor( bg_col );           // default background color
        setBackgroundMode( TransparentMode );   // default background mode
        setRasterOp( CopyROP );                 // default raster operation
    }
    updateBrush();
    updatePen();
    return TRUE;
}

/*!
  Ends painting.  Any resources used while painting are released.

  Note that while you mostly don't need to call end(), the destructor
  will do it, there is at least one common case, namely double
  buffering.

  \code
    QPainter p( myPixmap, this )
    // ...
    p.end(); // stops drawing on myPixmap
    p.begin( this );
    p.drawPixmap( myPixmap );
  \endcode

  Since you can't draw a QPixmap while it is being painted, it is
  necessary to close the active painter.

  \sa begin(), isActive()
*/

bool QPainter::end()				// end painting
{
    qDebug( "QPainter::end implemented x11 like" );
    if ( !isActive() ) {
#if defined(CHECK_STATE)
        warning( "QPainter::end: Missing begin() or begin() failed" );
#endif
        return FALSE;
    }
    if ( testf( FontMet ) )                       // remove references to this
        QFontMetrics::reset( this );
    if ( testf( FontInf ) )                       // remove references to this
        QFontInfo::reset( this );


    if ( testf(ExtDev) )
	pdev->cmd( QPaintDevice::PdcEnd, this, 0 );

    flags = 0;
    pdev->painters--;
    pdev = 0;
    return TRUE;
}


/*!
  Flushes any buffered drawing operations.
*/

void QPainter::flush()
{
    qDebug( "QPainter::flush" );
}


/*!
  Sets the background color of the painter to \a c.

  The background color is the color that is filled in when drawing
  opaque text, stippled lines and bitmaps.  The background color has
  no effect in transparent background mode (which is the default).

  \sa backgroundColor() setBackgroundMode() BackgroundMode
*/

void QPainter::setBackgroundColor( const QColor & )
{
    qDebug( "QPainter::setBackgroundColor" );
}

/*!
  Sets the background mode of the painter to \a m, which must be one
  of \c TransparentMode (the default) and \c OpaqueMode.

  Transparent mode draws stippled lines and text without setting the
  background pixels. Opaque mode fills these space with the current
  background color.

  Note that in order to draw a bitmap or pixmap transparently, you must use
  QPixmap::setMask().

  \sa backgroundMode(), setBackgroundColor() */

void QPainter::setBackgroundMode( BGMode )
{
    qDebug( "QPixmap::setMask" );
}

/*!
  Sets the raster operation to \a r.  The default is \c CopyROP.
  \sa rasterOp()
*/

void QPainter::setRasterOp( RasterOp )
{
    qDebug( "QPainter::setRasterOp" );
}

// ### matthias - true?

/*!
  Sets the brush origin to \a (x,y).

  The brush origin specifies the (0,0) coordinate of the painter's
  brush.  This setting only applies to pattern brushes and pixmap
  brushes.

  \sa brushOrigin()
*/

void QPainter::setBrushOrigin( int, int )
{
    qDebug( "QPainter::setBrushOrigin" );
}


/*!
  Enables clipping if \a enable is TRUE, or disables clipping if \a enable
  is FALSE.
  \sa hasClipping(), setClipRect(), setClipRegion()
*/

void QPainter::setClipping( bool )
{
    qDebug( "QPainter::setClipping" );
}


/*!
  \overload void QPainter::setClipRect( const QRect &r )
*/

void QPainter::setClipRect( const QRect & )
{
    qDebug( "QPainter::setClipRect" );
}

/*!
  Sets the clip region to \a rgn and enables clipping.

  Note that the clip region is given in physical device coordinates and
  \e not subject to any \link coordsys.html coordinate
  transformation.\endlink

  \sa setClipRect(), clipRegion(), setClipping()
*/

void QPainter::setClipRegion( const QRegion & )
{
    qDebug( "QPainter::setClipRegion" );
}


/*!
  Internal function for drawing a polygon.
*/

void QPainter::drawPolyInternal( const QPointArray &a, bool close )
{
    qDebug( "QPainter::drawPolyInternal implemented tested" );
    pdev->lockPort();
    RgnHandle polyRegion = NewRgn();
    OpenRgn();
    uint loopc;
    MoveTo( a[0].x(), a[0].y() );
    for ( loopc = 1; loopc < a.size(); loopc++ ) {
	LineTo( a[loopc].x(), a[loopc].y() );
	MoveTo( a[loopc].x(), a[loopc].y() );
    }
    LineTo( a[0].x(), a[0].y() );
    CloseRgn( polyRegion );
    if ( close ) {
	updateBrush();
	PaintRgn( polyRegion );
    }
    if ( cpen.style() != NoPen ) {
	updatePen();
	FrameRgn( polyRegion );
    }
    DisposeRgn( polyRegion );
    pdev->unlockPort();
}


/*!
  Draws/plots a single point at \a (x,y) using the current pen.

  \sa QPen
*/

void QPainter::drawPoint( int x, int y )
{
    qDebug( "QPainter::drawPoint implemented tested" );
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QPoint p( x, y );
            param[0].point = &p;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPoint, this, param ) /*|| !hdc*/ )
                return;
        }
        map( x, y, &x, &y );
    }
    pdev->lockPort();
    // FIXME: This is a bit silly
    updatePen();
    MoveTo(x,y);
    LineTo(x,y);
    pdev->unlockPort();
}


/*!
  Draws/plots an array of points using the current pen.

  If \a index is non-zero (the default is zero) only points from \a
  index are drawn.  If \a npoints is negative (the default) the rest
  of the points from \a index are drawn.  If is is zero or greater, \a
  npoints points are drawn.
*/

void QPainter::drawPoints( const QPointArray&, int, int )
{
    qDebug( "QPainter::drawPoints" );
}


/*!
  Sets the current pen position to \a (x,y)
  \sa lineTo(), pos()
*/

void QPainter::moveTo( int x, int y )
{
    qDebug( "QPainter::moveTo implemented untested" );
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QPoint p( x, y );
            param[0].point = &p;
            if ( !pdev->cmd(QPaintDevice::PdcMoveTo,this,param) /*|| !hdc*/ )
                return;
        }
        map( x, y, &x, &y );
    }
    penx = x;
    peny = y;
}

/*!
  Draws a line from the current pen position to \a (x,y) and sets \a
  (x,y) to be the new current pen position.

  \sa QPen moveTo(), drawLine(), pos()
*/

void QPainter::lineTo( int x, int y )
{
    qDebug( "QPainter::lineTo implemented untested" );
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QPoint p( x, y );
            param[0].point = &p;
            if ( !pdev->cmd( QPaintDevice::PdcLineTo, this, param ) /*|| !hdc*/ )
                return;
        }
        map( x, y, &x, &y );
    }
    pdev->lockPort();
    updateBrush();
    updatePen();
    MoveTo(penx,peny);
    LineTo(x,y);
    penx = x;
    peny = y;
    pdev->unlockPort();
}

/*!
  Draws a line from \a (x1,y2) to \a (x2,y2) and sets \a (x2,y2) to be
  the new current pen position.

  \sa QPen
*/

void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{
    qDebug( "QPainter::drawLine implemented untested" );
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[2];
            QPoint p1(x1, y1), p2(x2, y2);
            param[0].point = &p1;
            param[1].point = &p2;
            if ( !pdev->cmd( QPaintDevice::PdcDrawLine, this, param ) /*|| !hdc*/ )
                return;
        }
        map( x1, y1, &x1, &y1 );
        map( x2, y2, &x2, &y2 );
    }
    pdev->lockPort();
    updatePen();
    MoveTo(x1,y1);
    LineTo(x2,y2);
    pdev->unlockPort();
}



/*!
  Draws a rectangle with upper left corner at \a (x,y) and with
  width \a w and height \a h.

  \sa QPen, drawRoundRect()
*/

void QPainter::drawRect( int x, int y, int w, int h )
{
    qDebug( "QPainter::drawRect implemented tested" );
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            if ( !pdev->cmd( QPaintDevice::PdcSetFont, this, param ) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
            QPointArray a( QRect(x,y,w,h) );
            drawPolyInternal( xForm(a) );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }

    pdev->lockPort();

    Rect rect;
    SetRect( &rect, x, y, x + w, y + h );
    if( this->brush().style() == SolidPattern ) {
	updateBrush();
	PaintRect( &rect );
    }
    if( cpen.style() != NoPen ) {
	updatePen();
	FrameRect(&rect);
    }

    pdev->unlockPort();
}

/*!
  Draws a Windows focus rectangle with upper left corner at \a (x,y) and with
  width \a w and height \a h.

  This function draws a stippled XOR rectangle that is used to indicate
  keyboard focus (when QApplication::style() is \c WindowStyle).

  \warning This function draws nothing if the coordinate system has been
  \link rotate() rotated\endlink or \link shear() sheared\endlink.

  \sa drawRect(), QApplication::style()
*/

void QPainter::drawWinFocusRect( int x, int y, int w, int h )
{
    qDebug( "QApplication::style implemented untested" );
    drawWinFocusRect( x, y, w, h, TRUE, color0 );
}

/*!
  Draws a Windows focus rectangle with upper left corner at \a (x,y) and with
  width \a w and height \a h using a pen color that contrasts with \a bgColor.

  This function draws a stippled rectangle (XOR is not used) that is
  used to indicate keyboard focus (when the QApplication::style() is
  \c WindowStyle).

  The pen color used to draw the rectangle is either white or black
  depending on the color of \a bgColor (see QColor::gray()).

  \warning This function draws nothing if the coordinate system has been
  \link rotate() rotated\endlink or \link shear() sheared\endlink.

  \sa drawRect(), QApplication::style()
*/

void QPainter::drawWinFocusRect( int x, int y, int w, int h,
				 const QColor &bgColor )
{
    qDebug( "QApplication::style implemented untested" );
    drawWinFocusRect( x, y, w, h, FALSE, bgColor);
}


/*!
  \internal
*/

#ifdef _WS_X11_
void QPainter::drawWinFocusRect( int, int, int, int,
				 bool, const QColor & )
{
    qDebug( "QPainter::drawWinFocusRect" );
}
#endif

/*! \overload void QPainter::drawRoundRect( int x, int y, int w, int h )

  As the main version of the function, but with the roundness
  arguments fixed at 25.
*/


/*! \overload void QPainter::drawRoundRect( const QRect & )

  As the main version of the function, but with the roundness
  arguments fixed at 25.
*/


/*!
  Draws a rectangle with round corners at \a (x,y), with width \a w
  and height \a h.

  The \a xRnd and \a yRnd arguments specify how rounded the corners
  should be.  0 is angled corners, 99 is maximum roundedness.

  The width and height include all of the drawn lines.

  \sa drawRect(), QPen
*/

void QPainter::drawRoundRect( int x, int y, int w, int h, int xRnd, int yRnd)
{
    qDebug( "QPainter::drawRoundRect implemented tested" );
    if ( !isActive() )
        return;
    if ( xRnd <= 0 || yRnd <= 0 ) {
        drawRect( x, y, w, h );                 // draw normal rectangle
        return;
    }
    if ( xRnd >= 100 )                          // fix ranges
        xRnd = 99;
    if ( yRnd >= 100 )
        yRnd = 99;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = xRnd;
            param[2].ival = yRnd;
            if ( !pdev->cmd( QPaintDevice::PdcDrawRoundRect, this, param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
            QPointArray a;
            if ( w <= 0 || h <= 0 )
                fix_neg_rect( &x, &y, &w, &h );
            w--;
            h--;
            int rxx = w*xRnd/200;
            int ryy = h*yRnd/200;
            int rxx2 = 2*rxx;
            int ryy2 = 2*ryy;
            int xx, yy;

            // ###### WWA: this should use the new makeArc (with xmat)
            // FIXME: look into above statement

            a.makeEllipse( x, y, rxx2, ryy2 );
            int s = a.size()/4;
            int i = 0;
            while ( i < s ) {
                a.point( i, &xx, &yy );
                xx += w - rxx2;
                a.setPoint( i++, xx, yy );
            }
            i = 2*s;
            while ( i < 3*s ) {
                a.point( i, &xx, &yy );
                yy += h - ryy2;
                a.setPoint( i++, xx, yy );
            }
            while ( i < 4*s ) {
                a.point( i, &xx, &yy );
                xx += w - rxx2;
                yy += h - ryy2;
                a.setPoint( i++, xx, yy );
            }
            drawPolyInternal( xForm(a) );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }

    pdev->lockPort();
    Rect rect;
    SetRect( &rect, x, y, x + w, y + h );
    if( this->brush().style() == SolidPattern ) {
	updateBrush();
	PaintRoundRect( &rect, w*xRnd/100, h*yRnd/100 );
    }
    if( cpen.style() != NoPen ) {
	updatePen();
	FrameRoundRect( &rect, w*xRnd/100, h*yRnd/100 );
    }

    pdev->unlockPort();
}

/*!
  Draws an ellipse with center at \a (x+w/2,y+h/2) and size \a (w,h).
*/

void QPainter::drawEllipse( int x, int y, int w, int h )
{
    qDebug( "QPainter::drawEllipse implemented tested" );
    if ( !isActive() ) {
        return;
    }
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[1];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            if ( !pdev->cmd( QPaintDevice::PdcDrawEllipse, this, param ) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear polygon
            QPointArray a;
            a.makeArc( x, y, w, h, 0, 360*16, xmat );
            drawPolyInternal( a );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }
    pdev->lockPort();
    Rect r;
    SetRect( &r, x, y, x + w, y + h );
    if( this->brush().style() == SolidPattern ) {
	updateBrush();
	PaintOval( &r );
    }
    updatePen();
    FrameOval( &r );
    pdev->unlockPort();
}


/*!
  Draws an arc defined by the rectangle \a (x,y,w,h), the start
  angle \a a and the arc length \a alen.

  The angles \a a and \a alen are 1/16th of a degree, i.e. a full
  circle equals 5760 (16*360). Positive values of \a a and \a alen mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.

  Example:
  \code
    QPainter p( myWidget );
    p.drawArc( 10,10, 70,100, 100*16, 160*16 ); // draws a "(" arc
  \endcode

  \sa drawPie(), drawChord()
*/

void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
    // FIXME transformation is broken
    qDebug( "QPainter::drawArc transformation is broken" );
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawArc, this, param ) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat );    // arc polyline
            drawPolyInternal( pa, TRUE );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    pdev->lockPort();
    Rect bounds;
    SetRect(&bounds,x,y,x+w,y+h);
    updatePen();
    FrameArc(&bounds,a/16,alen/16);
    pdev->unlockPort();
}


/*!
  Draws a pie defined by the rectangle \a (x,y,w,h), the start
  angle \a a and the arc length \a alen.

  The pie is filled with the current brush().

  The angles \a a and \a alen are 1/16th of a degree, i.e. a full
  circle equals 5760 (16*360). Positive values of \a a and \a alen mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.

  \sa drawArc(), drawChord()
*/

void QPainter::drawPie( int x, int y, int w, int h, int a, int alen )
{
    qDebug( "QPainter::drawPie" );
    if ( a > (360*16) ) {
      a = a % (360*16);
    } else if ( a < 0 ) {
      a = a % (360*16);
      if ( a < 0 ) a += (360*16);
    }
    if ( !isActive() )
        return;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            QPDevCmdParam param[3];
            QRect r( x, y, w, h );
            param[0].rect = &r;
            param[1].ival = a;
            param[2].ival = alen;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPie, this, param) /*|| !hdc*/ )
                return;
        }
        if ( txop == TxRotShear ) {             // rotate/shear
            QPointArray pa;
            pa.makeArc( x, y, w, h, a, alen, xmat ); // arc polyline
            int n = pa.size();
            int cx, cy;
            xmat.map(x+w/2, y+h/2, &cx, &cy);
            pa.resize( n+2 );
            pa.setPoint( n, cx, cy );   // add legs
            pa.setPoint( n+1, pa.at(0) );
            drawPolyInternal( pa );
            return;
        }
        map( x, y, w, h, &x, &y, &w, &h );
    }
    if ( w <= 0 || h <= 0 ) {
        if ( w == 0 || h == 0 )
            return;
        fix_neg_rect( &x, &y, &w, &h );
    }
    if ( cpen.style() == NoPen ) {
        w++;
        h++;
    }
    pdev->lockPort();
    Rect bounds;
    SetRect(&bounds,x,y,x+w,y+h);
    //PaintArc(&bounds,a*16,alen*16);
    int aa,bb;
    if(!a) {
	aa=0;
    } else {
	aa=a/16;
    }
    if(!alen) {
	bb=0;
    } else {
	bb=alen/16;
    }
    if(aa<0)
	aa=0;
    if(bb<1)
	bb=1;
    if(this->brush().style()==SolidPattern) {
	updateBrush();
	PaintArc(&bounds,aa,bb);
    }
    if(cpen.style()!=NoPen) {
	updatePen();
	FrameArc(&bounds,aa,bb);
    }
    pdev->unlockPort();
}


/*!
  Draws a chord defined by the rectangle \a (x,y,w,h), the start
  angle \a a and the arc length \a alen.

  The chord is filled with the current brush().

  The angles \a a and \a alen are 1/16th of a degree, i.e. a full
  circle equals 5760 (16*360). Positive values of \a a and \a alen mean
  counter-clockwise while negative values mean clockwise direction.
  Zero degrees is at the 3'o clock position.

  \sa drawArc(), drawPie()
*/

void QPainter::drawChord( int, int, int, int, int, int )
{
    qDebug( "QPainter::drawChord" );
}


/*!
  Draws \a nlines separate lines from points defined in \a a, starting
  at a[\a index] (\a index defaults to 0). If \a nlines is -1 (the
  defauls) all points until the end of the array are used
  (i.e. (a.size()-index)/2 lines are drawn).

  Draws the 1st line from \a a[index] to \a a[index+1].
  Draws the 2nd line from \a a[index+2] to \a a[index+3] etc.

  \sa drawPolyline(), drawPolygon(), QPen
*/

void QPainter::drawLineSegments( const QPointArray &, int, int )
{
    qDebug( "QPainter::drawLineSegments" );
}


/*!
  Draws the polyline defined by the \a npoints points in \a a starting
  at \a a[index].  (\a index defaults to 0.)

  If \a npoints is -1 (the default) all points until the end of the
  array are used (i.e. a.size()-index-1 line segments are drawn).

  \sa drawLineSegments(), drawPolygon(), QPen
*/

void QPainter::drawPolyline( const QPointArray &, int, int )
{
    qDebug( "QPainter::drawPolyline" );
}


/*!
  Draws the polygon defined by the \a npoints points in \a a starting at
  \a a[index].  (\a index defaults to 0.)

  If \a npoints is -1 (the default) all points until the end of the
  array are used (i.e. a.size()-index line segments define the
  polygon).

  The first point is always connected to the last point.

  The polygon is filled with the current brush().
  If \a winding is TRUE, the polygon is filled using the winding
  fill algorithm. If \a winding is FALSE, the polygon is filled using the
  even-odd (alternative) fill algorithm.

  \sa drawLineSegments(), drawPolyline(), QPen
*/

void QPainter::drawPolygon( const QPointArray &a, bool winding,
                            int index, int npoints )
{
    qDebug( "QPainter::drawPolygon" );
    if ( npoints < 0 )
        npoints = a.size() - index;
    if ( index + npoints > (int)a.size() )
        npoints = a.size() - index;
    if ( !isActive() || npoints < 2 || index < 0 )
        return;
    QPointArray pa = a;
    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) ) {
            if ( npoints != (int)a.size() ) {
                pa = QPointArray( npoints );
                for ( int i=0; i<npoints; i++ )
                    pa.setPoint( i, a.point(index+i) );
            }
            QPDevCmdParam param[2];
            param[0].ptarr = (QPointArray*)&pa;
            param[1].ival = winding;
            if ( !pdev->cmd( QPaintDevice::PdcDrawPolygon, this, param ) /*|!hdc*/ )
                return;
        }
        if ( txop != TxNone ) {
            pa = xForm( a, index, npoints );
            if ( pa.size() != a.size() ) {
                index   = 0;
                npoints = pa.size();
            }
        }
    }
    drawPolyInternal(pa,true);
}


/*!
  Draws a cubic Bezier curve defined by the control points in \a a,
  starting at \a a[index].  (\a index defaults to 0.)

  Control points after \a a[index+3] are ignored.  Nothing happens if
  there aren't enough control points.
*/

void QPainter::drawQuadBezier( const QPointArray &, int )
{
    qDebug( "QPainter::drawQuadBezier" );
}


/*!
  Draws a pixmap at \a (x,y) by copying a part of \a pixmap into the
  paint device.

  \a (x,y) specify the top-left point in the paint device that is to
  be drawn onto.  \a (sx,sy) specify the top-left point in \a pixmap
  that is to be drawn (the default is (0,0).  \a (sw,sh) specify the
  size of the pixmap that is to be drawn (the default, (-1,-1), means
  all the way to the right/bottom of the pixmap).

  \sa bitBlt(), QPixmap::setMask()
*/

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap, int sx, int sy, int sw, int sh )
{
    if ( !isActive() || pixmap.isNull() ) {
        return;
    }
    if ( sw < 0 )
        sw = pixmap.width() - sx;
    if ( sh < 0 )
        sh = pixmap.height() - sy;

    // Sanity-check clipping
    if ( sx < 0 ) {
        x -= sx;
        sw += sx;
        sx = 0;
    }
    if ( sw + sx > pixmap.width() )
        sw = pixmap.width() - sx;
    if ( sy < 0 ) {
        y -= sy;
        sh += sy;
        sy = 0;
    }
    if ( sh + sy > pixmap.height() )
        sh = pixmap.height() - sy;
    if ( sw <= 0 || sh <= 0 ) {
        return;
    }

    if ( testf(ExtDev|VxF|WxF) ) {
        if ( testf(ExtDev) || (txop == TxScale && pixmap.mask()) ||
             txop == TxRotShear ) {
            if ( sx != 0 || sy != 0 ||
                 sw != pixmap.width() || sh != pixmap.height() ) {
                QPixmap tmp( sw, sh, pixmap.depth() );
                bitBlt( &tmp, 0, 0, &pixmap, sx, sy, sw, sh, CopyROP, TRUE );
                if ( pixmap.mask() ) {
                    QBitmap mask( sw, sh );
                    bitBlt( &mask, 0, 0, pixmap.mask(), sx, sy, sw, sh,
                            CopyROP, TRUE );
                    tmp.setMask( mask );
                }
                drawPixmap( x, y, tmp );
                return;
            }
            if ( testf(ExtDev) ) {
                QPDevCmdParam param[2];
                QPoint p(x,y);
                param[0].point  = &p;
                param[1].pixmap = &pixmap;
                if ( !pdev->cmd(QPaintDevice::PdcDrawPixmap,this,param) /* || !hdc */ ) {
                    return;
		}
            }
        }
        if ( txop == TxTranslate )
            map( x, y, &x, &y );
    }

    bitBlt( pdev, x, y, &pixmap, sx, sy, sw, sh, (RasterOp)rop );
}


static void drawTile( QPainter *p, int x, int y, int w, int h,
                      const QPixmap &pixmap, int xOffset, int yOffset )
{
    int yPos, xPos, drawH, drawW, yOff, xOff;
    yPos = y;
    yOff = yOffset;
    while( yPos < y + h ) {
        drawH = pixmap.height() - yOff;    // Cropping first row
        if ( yPos + drawH > y + h )        // Cropping last row
            drawH = y + h - yPos;
        xPos = x;
        xOff = xOffset;
        while( xPos < x + w ) {
            drawW = pixmap.width() - xOff; // Cropping first column
            if ( xPos + drawW > x + w )    // Cropping last column
                drawW = x + w - xPos;
            p->drawPixmap( xPos, yPos, pixmap, xOff, yOff, drawW, drawH );
            xPos += drawW;
            xOff = 0;
        }
        yPos += drawH;
        yOff = 0;
    }
}

/*!
  Draws a tiled \a pixmap in the specified rectangle.

  \a (x,y) specify the top-left point in the paint device that is to
  be drawn onto.  \a (sx,sy) specify the top-left point in \a pixmap
  that is to be drawn (the default is (0,0).

  Calling drawTiledPixmap() is similar to calling drawPixmap() several
  times to fill (tile) an area with a pixmap, but can be much more
  efficient.

  \sa drawPixmap()
*/

void QPainter::drawTiledPixmap( int x, int y, int w, int h,
                                const QPixmap &pixmap, int sx, int sy )
{
    int sw = pixmap.width();
    int sh = pixmap.height();
    if (!sw || !sh )
	return;
    if ( sx < 0 )
        sx = sw - -sx % sw;
    else
        sx = sx % sw;
    if ( sy < 0 )
        sy = sh - -sy % sh;
    else
        sy = sy % sh;
    /*
      Requirements for optimizing tiled pixmaps:
      - not an external device
      - not scale or rotshear
      - no mask
    */
    QBitmap *mask = (QBitmap *)pixmap.mask();
    if ( !testf(ExtDev) && txop <= TxTranslate && mask == 0 ) {
        if ( txop == TxTranslate )
            map( x, y, &x, &y );
        return;
    }
    if ( sw*sh < 8192 && sw*sh < 16*w*h ) {
        int tw = sw, th = sh;
        while ( tw*th < 32678 && tw < w/2 )
            tw *= 2;
        while ( tw*th < 32678 && th < h/2 )
            th *= 2;
        QPixmap tile( tw, th, pixmap.depth() );
        if ( mask ) {
            QBitmap tilemask( tw, th );
            tile.setMask( tilemask );
        }
        drawTile( this, x, y, w, h, tile, sx, sy );
    } else {
        drawTile( this, x, y, w, h, pixmap, sx, sy );
    }
}


/*!
  Draws at most \a len characters from \a str at position \a (x,y).

  \a (x,y) is the base line position.  Note that the meaning of \a y
  is not the same for the two drawText() varieties.
*/
extern const unsigned char * p_str(const char * c);

void QPainter::drawText( int x, int y, const QString &str, int len)
{
  qDebug( "QPainter::drawText" );
  if ( !isActive() )
    return;
  if ( len < 0 )
    len = str.length();
  if ( len == 0 )                             // empty string
    return;

  pdev->lockPort();
  updateBrush();
  updatePen();

  if ( testf(DirtyFont|ExtDev|VxF|WxF) ) {
    if ( testf(DirtyFont) )
      updateFont();

    if ( testf(ExtDev) ) {
      QPDevCmdParam param[2];
      QPoint p( x, y );
      QString newstr = str.left(len);
      param[0].point = &p;
      param[1].str = &newstr;
      if ( !pdev->cmd(QPaintDevice::PdcDrawText2,this,param) || !hd ) {
	pdev->unlockPort();
	return;
      }
    }

    if ( txop >= TxScale ) {
      const QFontMetrics & fm = fontMetrics();
      QFontInfo    fi = fontInfo();
      QRect bbox = fm.boundingRect( str, len );
      int w=bbox.width(), h=bbox.height();
      int aw, ah;
      int tx=-bbox.x(),  ty=-bbox.y();    // text position
      QWMatrix mat1( m11(), m12(), m21(), m22(), dx(),  dy() );
      QFont dfont( cfont );
      QWMatrix mat2;
      if ( txop == TxScale ) {
	int newSize = qRound( m22() * (double)cfont.pointSize() ) - 1;
	newSize = QMAX( 6, QMIN( newSize, 72 ) ); // empirical values
	dfont.setPointSize( newSize );
	QFontMetrics fm2( dfont );
	QRect abbox = fm2.boundingRect( str, len );
	aw = abbox.width();
	ah = abbox.height();
	tx = -abbox.x();
	ty = -abbox.y();        // text position - off-by-one?
	if ( aw == 0 || ah == 0 )
	  return;
	double rx = (double)bbox.width() * mat1.m11() / (double)aw;
	double ry = (double)bbox.height() * mat1.m22() /(double)ah;
	mat2 = QWMatrix( rx, 0, 0, ry, 0, 0 );
      }
      else {
	mat2 = QPixmap::trueMatrix( mat1, w, h );
	aw = w;
	ah = h;
      }
      bool empty = aw == 0 || ah == 0;
      QBitmap * wx_bm=0;
      bool create_new_bm = wx_bm == 0;
      if ( create_new_bm && !empty ) {    // no such cached bitmap
	QBitmap bm( aw, ah );           // create bitmap
	bm.fill( color0 );
	QPainter paint;
	paint.begin( &bm );             // draw text in bitmap
	paint.setFont( dfont );
	paint.drawText( tx, ty, str, len );
	paint.end();
	wx_bm = new QBitmap( bm.xForm(mat2) ); // transform bitmap
	if ( wx_bm->isNull() ) {
	  pdev->unlockPort();
	  delete wx_bm;               // nothing to draw
	  return;
	}
      }
      if ( bg_mode == OpaqueMode ) {      // opaque fill
	int fx = x;
	int fy = y - fm.ascent();
	int fw = fm.width(str,len);
	int fh = fm.ascent() + fm.descent();
	int m, n;
	QPointArray a(5);
	mat1.map( fx,    fy,    &m, &n );  a.setPoint( 0, m, n );
	a.setPoint( 4, m, n );
	mat1.map( fx+fw, fy,    &m, &n );  a.setPoint( 1, m, n );
	mat1.map( fx+fw, fy+fh, &m, &n );  a.setPoint( 2, m, n );
	mat1.map( fx,    fy+fh, &m, &n );  a.setPoint( 3, m, n );
	QBrush oldBrush = cbrush;
	setBrush( backgroundColor() );
	updateBrush();
	//XFillPolygon( dpy, hd, gc_brush, (XPoint*)a.data(), 4,
	//              Nonconvex, CoordModeOrigin );
	//XDrawLines( dpy, hd, gc_brush, (XPoint*)a.data(), 5,
	//            CoordModeOrigin );
	setBrush( oldBrush );
      }
      if ( empty ) {
	pdev->unlockPort();
	return;
      }
      double fx=x, fy=y, nfx, nfy;
      mat1.map( fx,fy, &nfx,&nfy );
      double tfx=tx, tfy=ty, dx, dy;
      mat2.map( tfx, tfy, &dx, &dy );     // compute position of bitmap
      x = qRound(nfx-dx);
      y = qRound(nfy-dy);
      pdev->unlockPort();
      return;
    }
    if ( txop == TxTranslate )
      map( x, y, &x, &y );
  }

  QCString mapped='?';

  const QTextCodec* mapper = cfont.d->mapper();
  if ( mapper ) {
    // translate from Unicode to font charset encoding here
    mapped = mapper->fromUnicode(str,len);
  }

  if(!mapped) {
    char * soo=new char[2];
    soo[0]='?';
    soo[1]='\0';
    mapped=soo;
  }

  if ( !cfont.handle() ) {
    if ( mapped.isNull() )
      warning("Fontsets only apply to mapped encodings");
    else {
      MoveTo(x,y);
      DrawString(p_str(mapped));
    }
  } else {
    if ( !mapped.isNull() ) {
      MoveTo(x,y);
      DrawString(p_str(mapped));
    } else {
      // Unicode font

      QString v = str;
#ifdef QT_BIDI
      v.compose();  // apply ligatures (for arabic, etc...)
      v = v.visual(); // visual ordering
      len = v.length();
#endif

      MoveTo(x,y);
      DrawString(p_str((char *)v.unicode()));
    }
  }

#if 0
  if ( cfont.underline() || cfont.strikeOut() ) {
    const QFontMetrics & fm = fontMetrics();
    int lw = fm.lineWidth();
    int tw = fm.width( str, len );
  }
#endif
  pdev->unlockPort();
}

/*!
  Returns the current position of the  pen.

  \sa moveTo()
 */
QPoint QPainter::pos() const
{
    qDebug( "QPainter::pos" );
    return QPoint();
}

