/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qpixmap.cpp#7 $
**
** Implementation of QPixmap class
**
** Author  : Haavard Nord
** Created : 950301
**
** Copyright (C) 1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qpixmap.h"

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/kernel/qpixmap.cpp#7 $";
#endif


void QPixmap::detach()				// detach shared pixmap
{
    if ( data->optim )				// detach is called before
	data->dirty = TRUE;			//   pixmap is changed
    if ( data->virgin || data->count == 1 ) {
	data->virgin = FALSE;
	return;
    }
    data->deref();
    *this = copy();
}

QPixmap QPixmap::copy() const			// deep copy
{
    QPixmap tmp( data->w, data->h, data->d );
    bitBlt( &tmp, 0,0, this, 0,0, data->w, data->h );
    return tmp;
}


/*!
Resizes the pixmap to \e w x \e h.
*/

void QPixmap::resize( int w, int h )
{
    if ( !data->virgin ) {			// has existing pixmap
	QPixmap pm( w, h, depth() );
	bitBlt( &pm, 0, 0, this, 0, 0,		// copy old pixmap
		QMIN(width(), w),
		QMIN(height(),h) );
	*this = pm;
    }
    else					// create new pixmap
	*this = QPixmap( w, h, isBitmap() ? 1 : -1 );
}



bool QPixmap::isBitmap() const			// reimplemented in QBitmap
{
    return FALSE;
}
