/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpaintdevice_mac.cpp
**
** Implementation of QPaintDevice class for Mac
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

#include "qpaintdevice.h"
#include "qpaintdevicemetrics.h"
#include "qwidget.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qt_mac.h"

// NOT REVISED
/*!
  \class QPaintDevice qpaintdevice.h
  \brief The base class of objects that can be painted.

  \ingroup drawing

  A paint device is an abstraction of a two-dimensional space that can be
  drawn using a QPainter.
  The drawing capabilities are implemented by the subclasses: QWidget,
  QPixmap, QPicture and QPrinter.

  The default coordinate system of a paint device has its origin
  located at the top left position. X increases to the right and Y
  increases downwards. The unit is one pixel.  There are several ways
  to set up a user-defined coordinate system using the painter, for
  example by QPainter::setWorldMatrix().

  Example (draw on a paint device):
  \code
    void MyWidget::paintEvent( QPaintEvent * )
    {
	QPainter p;				// our painter
	p.begin( this );			// start painting widget
	p.setPen( red );			// blue outline
	p.setBrush( yellow );			// yellow fill
	p.drawEllipse( 10,20, 100,100 );	// 100x100 ellipse at 10,20
	p.end();				// painting done
    }
  \endcode

  The bit block transfer is an extremely useful operation for copying pixels
  from one paint device to another (or to itself).
  It is implemented as the global function bitBlt().

  Example (scroll widget contents 10 pixels to the right):
  \code
    bitBlt( myWidget, 10,0, myWidget );
  \endcode

  \warning Qt requires that a QApplication object must exist before any paint
  devices can be created.  Paint devices access window system resources, and
  these resources are not initialized before an application object is created.
*/


/*!
  Constructs a paint device with internal flags \e devflags.
  This constructor can only be invoked from subclasses of QPaintDevice.
*/

QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(CHECK_STATE)
	qFatal( "QPaintDevice: Must construct a QApplication before a "
		"QPaintDevice" );
#endif
	return;
    }
    devFlags = devflags;
    painters = 0;
    hd=0;
}

/*!
  Destructs the paint device and frees window system resources.
*/

QPaintDevice::~QPaintDevice()
{
#if defined(CHECK_STATE)
    if ( paintingActive() )
	qWarning( "QPaintDevice: Cannot destroy paint device that is being "
		  "painted.  Be sure to QPainter::end() painters!" );
#endif
}

/*!
  \fn int QPaintDevice::devType() const

  Returns the device type identifier: \c QInternal::Widget, \c
  QInternal::Pixmap, \c QInternal::Printer, \c QInternal::Picture or
  \c QInternal::UndefinedDevice.
*/

/*!
  \fn bool QPaintDevice::isExtDev() const
  Returns TRUE if the device is a so-called external paint device.

  External paint devices cannot be bitBlt()'ed from.
  QPicture and QPrinter are external paint devices.
*/

/*!
  \fn HANDLE QPaintDevice::handle() const

  Returns the window system handle of the paint device, for low-level
  access.  <em>Using this function is not portable.</em>

  The HANDLE type varies with platform; see qpaintdevice.h and qwindowdefs.h
  for details.

  \sa x11Display()
*/

/*!
  \fn HDC QPaintDevice::handle() const

  Returns the window system handle of the paint device, for low-level
  access.  <em>Using this function is not portable.</em>

  The HDC type varies with platform; see qpaintdevice.h and qwindowdefs.h
  for details.
*/


/*!
  \fn bool QPaintDevice::paintingActive() const
  Returns TRUE if the device is being painted, i.e. someone has called
  QPainter::begin() and not yet QPainter::end() for this device.
  \sa QPainter::isActive()
*/

/*!
  Internal virtual function that interprets drawing commands from
  the painter.

  Implemented by subclasses that have no direct support for drawing
  graphics (external paint devices, for example QPicture).
*/

bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
{
    return FALSE;
}

/*!
  Internal virtual function that returns paint device metrics.

  Please use the QPaintDeviceMetrics class instead.
*/

int QPaintDevice::metric( int ) const
{
    return 0;
}

/*!
  Internal virtual function. Reserved for future use.

  \internal
  Please use the QFontMetrics class instead.
*/

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
    return 0;
}

/*!
  Internal virtual function. Reserved for future use.

  \internal
  Please use the QFontInfo class instead.
*/

int QPaintDevice::fontInf( QFont *, int ) const
{
    return 0;
}


//
// Internal functions for simple GC caching for blt'ing masked pixmaps.
// This cache is used when the pixmap optimization is set to Normal
// and the pixmap size doesn't exceed 128x128.
//



/*!
  \relates QPaintDevice
  This function copies a block of pixels from one paint device to another
  (bitBlt means bit block transfer).

  \arg \e dst is the paint device to copy to.
  \arg \e dx and \e dy is the position to copy to.
  \arg \e src is the paint device to copy from.
  \arg \e sx and \e sy is the position to copy from.
  \arg \e sw and \e sh is the width and height of the block to be copied.
  \arg \e rop defines the raster operation to be used when copying.

  If \e sw is 0 or \e sh is 0, then bitBlt will do nothing.

  If \e sw is negative, then bitBlt calculates <code>sw = src->width -
  sx.</code> If \e sh is negative, then bitBlt calculates <code>sh =
  src->height - sy.</code>

  The \e rop argument can be one of:
  <ul>
  <li> \c CopyROP:     dst = src.
  <li> \c OrROP:       dst = src OR dst.
  <li> \c XorROP:      dst = src XOR dst.
  <li> \c NotAndROP:   dst = (NOT src) AND dst
  <li> \c NotCopyROP:  dst = NOT src
  <li> \c NotOrROP:    dst = (NOT src) OR dst
  <li> \c NotXorROP:   dst = (NOT src) XOR dst
  <li> \c AndROP       dst = src AND dst
  <li> \c NotROP:      dst = NOT dst
  <li> \c ClearROP:    dst = 0
  <li> \c SetROP:      dst = 1
  <li> \c NopROP:      dst = dst
  <li> \c AndNotROP:   dst = src AND (NOT dst)
  <li> \c OrNotROP:    dst = src OR (NOT dst)
  <li> \c NandROP:     dst = NOT (src AND dst)
  <li> \c NorROP:      dst = NOT (src OR dst)
  </ul>

  The \e ignoreMask argument (default FALSE) applies where \e src is
  a QPixmap with a \link QPixmap::setMask() mask\endlink.
  If \e ignoreMask is TRUE, bitBlt ignores the pixmap's mask.

  BitBlt has two restrictions:
  <ol>
  <li> The \e src device must be QWidget or QPixmap.  You cannot copy pixels
  from a picture or a printer (external device).
  <li> The \e src device may not have pixel depth greater than \e dst.
  You cannot copy from an 8 bit pixmap to a 1 bit pixmap.
  </ol>
*/

QPoint posInWindow(QWidget *w);

void bitBlt( QPaintDevice *dst, int dx, int dy, 
	     const QPaintDevice *src, int sx, int sy, int sw, int sh, 
	     Qt::RasterOp rop, bool imask)
{
  if(dx+sw>dst->metric(QPaintDeviceMetrics::PdmWidth)) {
    sw=dst->metric(QPaintDeviceMetrics::PdmWidth)-dx;
  }
  if(dy+sh>dst->metric(QPaintDeviceMetrics::PdmHeight)) {
    sh=dst->metric(QPaintDeviceMetrics::PdmHeight)-dy;
  }

  if(!sw || !sh)
      return;


  //at the end of this function this will go out of scope and the destructor will restore the state
  QMacSavedPortInfo saveportstate; 
    
  int srcoffx = 0, srcoffy = 0;
  BitMap *srcbitmap=NULL;
  const QBitmap *srcbitmask=NULL;
  if(src->devType() == QInternal::Widget) {
      QWidget *w = (QWidget *)src;
      srcbitmap = (BitMap *)*GetPortPixMap(GetWindowPort((WindowPtr)w->handle()));

      QPoint p(posInWindow(w));
      srcoffx = p.x();
      srcoffy = p.y();

      if(sw < 0)
	  sw = w->width();
      if(sh < 0)
	  sh = w->height();
  } else if(src->devType() == QInternal::Pixmap) {
      QPixmap *pm = (QPixmap *)src;
      srcbitmap = (BitMap *)*GetGWorldPixMap((GWorldPtr)pm->handle());
      srcbitmask = pm->mask();

      if(sw < 0)
	  sw = pm->width();
      if(sh < 0)
	  sh = pm->height();
  }

  int dstoffx=0, dstoffy=0;
  const BitMap *dstbitmap=NULL;
  if(dst->devType() == QInternal::Widget) {
      QWidget *w = (QWidget *)dst;
      dstbitmap = (BitMap *)*GetPortPixMap(GetWindowPort((WindowPtr)w->handle()));
      SetPortWindowPort((WindowPtr)w->handle()); //wtf?

      QPoint p(posInWindow(w));
      dstoffx = p.x();
      dstoffy = p.y();

      SetClip((RgnHandle)w->clippedRegion().handle()); //probably shouldn't do this?

  } else if(dst->devType() == QInternal::Pixmap) {

      QPixmap *pm = (QPixmap *)dst;
      SetGWorld((GWorldPtr)pm->handle(),0);
      LockPixels(GetGWorldPixMap((GWorldPtr)pm->handle()));
      dstbitmap = (BitMap *)*GetGWorldPixMap((GWorldPtr)pm->handle());

      //I'm paranoid..
      QRegion rgn(0,0,pm->width(),pm->height());
      SetClip((RgnHandle)rgn.handle());
  }

  if(!dstbitmap || !srcbitmap) {  //FIXME, need to handle ExtDevice!!!!!!
      qWarning("This shouldn't have happened yet, but fix me! %s:%d", __FILE__, __LINE__);
      return;
  }

  short copymode;
  switch(rop) {
  default:
  case Qt::CopyROP:   copymode = srcCopy; break;
  case Qt::OrROP:     copymode = srcOr; break;
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

  Rect r;
  SetRect(&r,sx+srcoffx,sy+srcoffy,sx+sw+srcoffx,sy+sh+srcoffy);
  Rect r2;
  SetRect(&r2,dx+dstoffx,dy+dstoffy,dx+sw+dstoffx,dy+sh+dstoffy);
  if(srcbitmask && !imask) {
      BitMap *maskbits = (BitMap *)*GetGWorldPixMap((GWorldPtr)srcbitmask->handle());
      Rect maskr;
      SetRect(&maskr, 0, 0, srcbitmask->width(), srcbitmask->height());
      CopyDeepMask(srcbitmap, maskbits, dstbitmap, &r, &maskr, &r2, copymode, 0);
  }
  else {
      CopyBits(srcbitmap, dstbitmap, &r,&r2,copymode, 0);
  }

  if(dst->devType() == QInternal::Pixmap) {
      QPixmap *pm = (QPixmap *)dst;
      UnlockPixels(GetGWorldPixMap((GWorldPtr)pm->handle()));    
  } 
#if 0 //this is good for debugging and not much else, do not leave this in production
else if(dst->devType() == QInternal::Widget) {
      GWorldPtr cgworld;
      GDHandle cghandle;
      GetGWorld(&cgworld, &cghandle);
      QRegion rup(dx+dstoffx,dy+dstoffy,dx+sw+dstoffx,dy+sh+dstoffy);
      QDFlushPortBuffer(cgworld, (RgnHandle)rup.handle());
  }
#endif
}


/*!
  \fn void bitBlt( QPaintDevice *dst, const QPoint &dp, const QPaintDevice *src, const QRect &sr, RasterOp rop )

  Overloaded bitBlt() with the destination point \e dp and source rectangle
  \e sr.

  \relates QPaintDevice
*/


Qt::HANDLE QPaintDevice::handle() const
{
    return hd;
}

