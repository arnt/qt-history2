/****************************************************************************
** $Id: $
**
** Implementation of QColor class for mac
**
** Created : 001019
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcolor.h"
#include "private/qcolor_p.h"
#include "string.h"
#include "qpaintdevice.h"
#include "qapplication.h"
#include "private/qapplication_p.h"

// NOT REVISED

#include "qintdict.h"


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
    return 1<<24;
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
    d.d32.pix = qRed( d.argb ) << 16 | qGreen( d.argb ) << 8 | qBlue( d.argb );
    return d.d32.pix;
}


void QColor::setSystemNamedColor( const QString& name )
{
#if defined(QT_CHECK_STATE)
    if ( !color_init ) {
	qWarning( "QColor::setSystemNamedColor: Cannot perform this operation "
		 "because QApplication does not exist" );
    } else
#endif
    {
	QRgb rgb;
	if ( qt_get_named_rgb( name.latin1(), rgb ) ) {
	    d.argb = rgb;
	    alloc();
	}
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

