/**********************************************************************
**   Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
**   This file is part of Qt GUI Designer.
**
**   This file may be distributed under the terms of the GNU General
**   Public License version 2 as published by the Free Software
**   Foundation and appearing in the file COPYING included in the
**   packaging of this file. If you did not get the file, send email
**   to info@trolltech.com
**
**   The file is provided AS IS with NO WARRANTY OF ANY KIND,
**   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR
**   A PARTICULAR PURPOSE.
**
**********************************************************************/

#include "splashloader.h"
#include "../pics/splash.h"

#include <qimage.h>

QPixmap splashScreen()
{
    Embed2 *e = &embed_vec2[ 0 ];
    while ( e->name ) {
	if ( QString( e->name ) == "splash.png" ) {
	    QImage img;
	    img.loadFromData( (const uchar*)e->data, e->size );
	    QPixmap pix;
	    pix.convertFromImage( img );
	    return pix;
	}
	e++;
    }
    return QPixmap();
}
