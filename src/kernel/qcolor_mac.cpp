/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qcolor_mac.cpp
**
** Implementation of QColor class for mac
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

#include "qcolor.h"
#include "qcolor_p.h"
#include "string.h"
#include "qpaintdevice.h"
#include "qapplication.h"
#include "qapplication_p.h"

// NOT REVISED

#include "qintdict.h"

struct QColorData {
    uint pix;					// allocated pixel value
    int	 context;				// allocation context
};

typedef QIntDict<QColorData> QColorDict;
typedef QIntDictIterator<QColorData> QColorDictIt;


/*
  This function is called from the event loop. It resets the colors_avail
  flag so that the application can retry to allocate read-only colors
  that other applications may have deallocated lately.

  The g_our_alloc and g_carr are global arrays that optimize color
  approximation when there are no more colors left to allocate.
*/

void qt_reset_color_avail()
{
}

/*****************************************************************************
  QColor static member functions
 *****************************************************************************/

int QColor::maxColors()
{
    return 2^24;
}

int QColor::numBitPlanes()
{
    return 32;
}


void QColor::initialize()
{
    if ( color_init )
	return;

    color_init = TRUE;
}

void QColor::cleanup()
{
}


/*****************************************************************************
  QColor member functions
 *****************************************************************************/

uint QColor::alloc()
{
    if ( (rgbVal & RGB_INVALID) || !color_init ) {
	rgbVal = 0;				// invalid color or state
	pix = 0;
    } else {
	rgbVal &= RGB_MASK;
	pix = qRed( rgbVal ) << 16 | qGreen( rgbVal ) << 8 | qBlue( rgbVal );
    }

    return pix;
}


void QColor::setSystemNamedColor( const QString& name )
{
    if ( !color_init ) {
#if defined(QT_CHECK_STATE)
	qWarning( "QColor::setSystemNamedColor: Cannot perform this operation "
		 "because QApplication does not exist" );
#endif
	alloc();				// makes the color black
	return;
    }
    rgbVal = qt_get_rgb_val( name.latin1() );
    if ( lazy_alloc ) {
	rgbVal |= RGB_DIRTY;			// alloc later
	pix = 0;
    } else {
	alloc();				// alloc now
    }
}


int QColor::enterAllocContext()
{
    return 1;
}


void QColor::leaveAllocContext()
{
}


int QColor::currentAllocContext()
{
    return 0;
}


void QColor::destroyAllocContext( int )
{
}

