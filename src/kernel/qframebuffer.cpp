/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qframebuffer.cpp#2 $
**
** Implementation of QFrameBuffer class
**
** Created : 990103
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qframebuffer.h"


/*!
  \class QFrameBuffer qframebuffer.h
  \brief The QFrameBuffer class is similar to QImage, only faster and more low-level.

  \ingroup kernel

  A frame buffer is very similar to an \link QImage image\endlink, but
  optimized for drawing speed and not ease of programming.  A frame buffer
  can be thought of as a \link QPixmap pixmap\endlink where you can access
  the pixel data directly.  QFrameBuffer is ideal for high-performance
  graphics, for example action games and real-time image rendering where
  QImage turns out to be too slow.

  The low-level nature of QFrameBuffer makes it more cumbersome to use for
  the programmer than QImage.  While QImage offers 3 bit depths only (1, 8
  and 32 bpp), QFrameBuffer gives you whatever the underlying hardware
  offers.

  To draw the contents of a frame buffer on a \link QWidget widget\endlink
  or a pixmap, simply use the \l bitBlt function.
*/


/*!
  Not yet implemented.
*/

QFrameBuffer::QFrameBuffer( int, int )
{
}


/*!
  Not yet implemented.
*/

QFrameBuffer::~QFrameBuffer()
{
}
