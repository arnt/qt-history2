/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap_mac.cpp
**
** Implementation of QPixmap class for mac
**
** Created : 001019
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

#include "qglobal.h"

// NOT REVISED

// Uncomment the next line to enable the MIT Shared Memory extension
//
// WARNING:  This has some problems:
//
//    1. Consumes a 800x600 pixmap
//    2. Qt does not handle the ShmCompletion message, so you will
//        get strange effects if you xForm() repeatedly.
//
// #define MITSHM

#if defined(_OS_WIN32_) && defined(MITSHM)
#undef MITSHM
#endif

#include "qbitmap.h"
#include "qpaintdevicemetrics.h"
#include "qimage.h"
#include "qwmatrix.h"
#include "qapplication.h"
#include <stdlib.h>




void QPixmap::init( int, int, int, bool, Optimization )
{
    qDebug( "QPixmap::init" );

    data = new QPixmapData;
    CHECK_PTR( data );

    memset( data, 0, sizeof(QPixmapData) );

}


void QPixmap::deref()
{
    qDebug( "QPixmap::deref" );
}


/*!
  Constructs a monochrome pixmap which is initialized with the data in \e bits.
  This constructor is protected and used by the QBitmap class.
*/

QPixmap::QPixmap( int, int, const uchar *, bool )
    : QPaintDevice( QInternal::Pixmap )
{						// for bitmaps only
    qDebug( "QPixmap::QPixmap" );
}


/*!
  Special-purpose function that detaches the pixmap from shared pixmap data.

  A pixmap is automatically detached by Qt whenever its contents is about
  to change.  This is done in all QPixmap member functions that modify the
  pixmap (fill(), resize(), convertFromImage(), load() etc.), in bitBlt()
  for the destination pixmap and in QPainter::begin() on a pixmap.

  It is possible to modify a pixmap without letting Qt know.
  You can first obtain the \link handle() system-dependent handle\endlink
  and then call system-specific functions (for instance BitBlt under Windows)
  that modifies the pixmap contents.  In this case, you can call detach()
  to cut the pixmap loose from other pixmaps that share data with this one.

  detach() returns immediately if there is just a single reference or if
  the pixmap has not been initialized yet.
*/

void QPixmap::detach()
{
    qDebug( "QPainter::begin" );
}


/*!
  Returns the default pixmap depth, i.e. the depth a pixmap gets
  if -1 is specified.
  \sa depth()
*/

int QPixmap::defaultDepth()
{
    qDebug( "QPixmap::defaultDepth" );
    return 24;
}


/*!
  \fn QPixmap::Optimization QPixmap::optimization() const

  Returns the optimization setting for this pixmap.

  The default optimization setting is \c QPixmap::NormalOptim. You may
  change this settings in two ways:
  <ul>
  <li> Call setDefaultOptimization() to set the default optimization
  for all new pixmaps.
  <li> Call setOptimization() to set a the optimization for individual
  pixmaps.
  </ul>

  \sa setOptimization(), setDefaultOptimization(), defaultOptimization()
*/

/*!
  Sets pixmap drawing optimization for this pixmap.

  The optimization setting affects pixmap operations, in particular
  drawing of transparent pixmaps (bitBlt() a pixmap with a mask set) and
  pixmap transformations (the xForm() function).

  Pixmap optimization involves keeping intermediate results in a cache
  buffer and use the data in the cache to speed up bitBlt() and xForm().
  The cost is more memory consumption, up to twice as much as an
  unoptimized pixmap.

  Use the setDefaultOptimization() to change the default optimization
  for all new pixmaps.

  \sa optimization(), setDefaultOptimization(), defaultOptimization()
*/

void QPixmap::setOptimization( Optimization )
{
    qDebug( "QPixmap::Optimization" );
}


/*!
  Fills the pixmap with the color \e fillColor.
*/

void QPixmap::fill( const QColor & )
{
    qDebug( "QPixmap::fill" );
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.
*/

int QPixmap::metric( int ) const
{
    qDebug( "QPaintDevice::metric" );
    return 1;
}

/*!
  Converts the pixmap to an image. Returns a null image if the operation
  failed.

  If the pixmap has 1 bit depth, the returned image will also be 1
  bits deep.  If the pixmap has 2-8 bit depth, the returned image
  has 8 bit depth.  If the pixmap has greater than 8 bit depth, the
  returned image has 32 bit depth.

  \bug Does not support 2 or 4 bit display hardware.

  \bug Alpha masks on monochrome images are ignored.

  \sa convertFromImage()
*/

QImage QPixmap::convertToImage() const
{
    qDebug( "QPixmap::convertToImage" );
    return QImage();
}


/*!
  Converts an image and sets this pixmap. Returns TRUE if successful.

  The \a conversion_flags argument is a bitwise-OR from the following choices.
  The options marked \e (default) are the choice if no other choice from the
  list is included (they are zero):

  <dl>
   <dt>Color/Mono preference (ignored for QBitmap)
   <dd>
    <ul>
     <li> \c AutoColor (default) - If the \e image has \link
	       QImage::depth() depth\endlink 1 and contains only
	       black and white pixels, then the pixmap becomes monochrome.
     <li> \c ColorOnly - The pixmap is dithered/converted to the
	       \link defaultDepth() native display depth\endlink.
     <li> \c MonoOnly - The pixmap becomes monochrome.  If necessary,
	       it is dithered using the chosen dithering algorithm.
    </ul>
   <dt>Dithering mode preference, for RGB channels
   <dd>
    <ul>
     <li> \c DiffuseDither (default) - a high quality dither
     <li> \c OrderedDither - a faster more ordered dither
     <li> \c ThresholdDither - no dithering, closest color is used
    </ul>
   <dt>Dithering mode preference, for alpha channel
   <dd>
    <ul>
     <li> \c DiffuseAlphaDither - a high quality dither
     <li> \c OrderedAlphaDither - a faster more ordered dither
     <li> \c ThresholdAlphaDither (default) - no dithering
    </ul>
   <dt>Color matching versus dithering preference
   <dd>
    <ul>
     <li> \c PreferDither - always dither 32-bit images when
		the image
		is being converted to 8-bits.
		This is the default when converting to a pixmap.
     <li> \c AvoidDither - only dither 32-bit images if
		the image
		has more than 256 colors and it
		is being converted to 8-bits.
		This is the default when an image is converted
		for the purpose of saving to a file.
    </ul>
  </dl>

  Passing 0 for \a conversion_flags gives all the default options.

  Note that even though a QPixmap with depth 1 behaves much like a
  QBitmap, isQBitmap() returns FALSE.

  If a pixmap with depth 1 is painted with color0 and color1 and
  converted to an image, the pixels painted with color0 will produce
  pixel index 0 in the image and those painted with color1 will produce
  pixel index 1.

  \bug Does not support 2 or 4 bit display hardware.

  \sa convertToImage(), isQBitmap(), QImage::convertDepth(), defaultDepth(),
  QImage::hasAlphaBuffer()
*/

bool QPixmap::convertFromImage( const QImage &, int )
{
    qDebug( "QImage::depth" );
    return TRUE;
}


/*!
  Grabs the contents of a window and makes a pixmap out of it.
  Returns the pixmap.

  The arguments \e (x,y) specify the offset in the window, while
  \e (w,h) specify the width and height of the area to be copied.

  If \e w is negative, the function copies everything to the right
  border of the window.	 If \e h is negative, the function copies
  everything to the bottom of the window.

  Note that grabWindows() grabs pixels from the screen, not from the
  window.  This means that If there is another window partially or
  entirely over the one you grab, you get pixels from the overlying
  window too.

  Note also that the mouse cursor is generally not grabbed.

  The reason we use a window identifier and not a QWidget is to enable
  grabbing of windows that are not part of the application, window
  system frames, and so on.

  \warning Grabbing an area outside the screen is not safe in general.
  This depends on the underlying window system.

  \sa grabWidget()
*/

QPixmap QPixmap::grabWindow( WId, int, int, int w, int h )
{
    qDebug( "QPixmap::grabWindow" );
    QPixmap pm( w, h );				// create new pixmap
    return pm;
}


/*!
  Returns a copy of the pixmap that is transformed using \e matrix.

  Qt uses this function to implement rotated text on window systems
  that do not support such complex features.

  Example of how to manually draw a rotated text at (100,200) in a widget:
  \code
    char    *str = "Trolls R Qt";	// text to be drawn
    QFont    f( "Charter", 24 );	// use Charter 24pt font
    QPixmap  pm( 8, 8 );
    QPainter p;
    QRect    r;				// text bounding rectangle
    QPoint   bl;			// text baseline position

    p.begin( &pm );			// first get the bounding
    p.setFont( f );			//   text rectangle
    r = p.fontMetrics().boundingRect(str);
    bl = -r.topLeft();			// get baseline position
    p.end();

    pm.resize( r.size() );		// resize to fit the text
    pm.fill( white );			// fills pm with white
    p.begin( &pm );			// begin painting pm
    p.setFont( f );			// set the font
    p.setPen( blue );			// set blue text color
    p.drawText( bl, str );		// draw the text
    p.end();				// painting done

    QWMatrix m;				// transformation matrix
    m.rotate( -33.4 );			// rotate coordinate system
    QPixmap rp = pm.xForm( m );		// rp is rotated pixmap

    QWMatrix t = QPixmap::trueMatrix( m, pm.width(), pm.height() );
    int x, y;
    t.map( bl.x(),bl.y(), &x,&y );	// get pm's baseline pos in rp

    bitBlt( myWidget, 100-x, 200-y,	// blt rp into a widget
	    &rp, 0, 0, -1, -1 );
  \endcode

  This example outlines how Qt implements rotated text under X11.
  The font calculation is the most tedious part. The rotation itself is
  only 3 lines of code.

  If you want to draw rotated text, you do not have to implement all the
  code above. The code below does exactly the same thing as the example
  above, except that it uses a QPainter.

  \code
    char    *str = "Trolls R Qt";	// text to be drawn
    QFont    f( "Charter", 24 );	// use Charter 24pt font
    QPainter p;

    p.begin( myWidget );
    p.translate( 100, 200 );		// translates coord system
    p.rotate( -33.4 );			// rotates it counterclockwise
    p.setFont( f );
    p.drawText( 0, 0, str );
    p.end();
  \endcode

  \bug 2 and 4 bits pixmaps are not supported.

  \sa trueMatrix(), QWMatrix, QPainter::setWorldMatrix()
*/

QPixmap QPixmap::xForm( const QWMatrix & ) const
{
    qDebug( "QPixmap::trueMatrix" );
    return QPixmap();
}


/*!
  Returns the actual matrix used for transforming a pixmap with \e w
  width and \e h height.

  When transforming a pixmap with xForm(), the transformation matrix
  is internally adjusted to compensate for unwanted translation,
  i.e. xForm() returns the smallest pixmap containing all transformed
  points of the original pixmap.

  This function returns the modified matrix, which maps points
  correctly from the original pixmap into the new pixmap.

  \sa xForm(), QWMatrix
*/

QWMatrix QPixmap::trueMatrix( const QWMatrix &, int, int )
{
    qDebug( "QPixmap::trueMatrix" );
    QWMatrix mat( 1, 0, 0, 1, -1, -1 );	// true matrix
    return mat;
}
