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

QPaintDevice::QPaintDevice( uint devflags )
{
    if ( !qApp ) {				// global constructor
#if defined(QT_CHECK_STATE)
	qFatal( "QPaintDevice: Must construct a QApplication before a "
		"QPaintDevice" );
#endif
	return;
    }
    devFlags = devflags;
    painters = 0;
    hd=0;
}

QPaintDevice::~QPaintDevice()
{
#if defined(QT_CHECK_STATE)
    if ( paintingActive() )
	qWarning( "QPaintDevice: Cannot destroy paint device that is being "
		  "painted.  Be sure to QPainter::end() painters!" );
#endif
}

bool QPaintDevice::cmd( int, QPainter *, QPDevCmdParam * )
{
    return FALSE;
}

int QPaintDevice::metric( int ) const
{
    return 0;
}

int QPaintDevice::fontMet( QFont *, int, const char *, int ) const
{
    return 0;
}

int QPaintDevice::fontInf( QFont *, int ) const
{
    return 0;
}


QPoint posInWindow(QWidget *w);

void unclippedBitBlt( QPaintDevice *dst, int dx, int dy, 
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

  if(!dst || !src) {
      qDebug("Asked to paint to or from a null paintdevice, something is hosed.");
      return;
  }

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

  ::RGBColor f;
  f.red = f.green = f.blue = 0;
  RGBForeColor( &f );

  int dstoffx=0, dstoffy=0;
  const BitMap *dstbitmap=NULL;
  if(dst->devType() == QInternal::Widget) {

      QWidget *w = (QWidget *)dst;
      dstbitmap = (BitMap *)*GetPortPixMap(GetWindowPort((WindowPtr)w->handle()));
      SetPortWindowPort((WindowPtr)w->handle()); //wtf?

      QPoint p(posInWindow(w));
      dstoffx = p.x();
      dstoffy = p.y();

  } else if(dst->devType() == QInternal::Pixmap) {

      QPixmap *pm = (QPixmap *)dst;
      SetGWorld((GWorldPtr)pm->handle(),0);
      ASSERT(LockPixels(GetGWorldPixMap((GWorldPtr)pm->handle())));
      dstbitmap = (BitMap *)*GetGWorldPixMap((GWorldPtr)pm->handle());

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


void bitBlt( QPaintDevice *dst, int dx, int dy, 
	     const QPaintDevice *src, int sx, int sy, int sw, int sh, 
	     Qt::RasterOp rop, bool imask)
{
  //at the end of this function this will go out of scope and the destructor will restore the state
  QMacSavedPortInfo saveportstate; 
    
  if(dst && dst->devType() == QInternal::Widget) {
      SetClip((RgnHandle)((QWidget *)dst)->clippedRegion().handle()); //probably shouldn't do this?
  } else if(dst && dst->devType() == QInternal::Pixmap) {
      QPixmap *pm = (QPixmap *)dst;
      //I'm paranoid..
      QRegion rgn(0,0,pm->width(),pm->height());
      SetClip((RgnHandle)rgn.handle());
  }
  unclippedBitBlt(dst, dx, dy, src, sx, sy, sw, sh, rop, imask);
}

Qt::HANDLE QPaintDevice::handle() const
{
    return hd;
}

