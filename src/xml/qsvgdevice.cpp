/****************************************************************************
** $Id: //depot/qt/main/src/xml/qsvgdevice.cpp#6 $
**
** Implementation of the QSVGDevice class
**
** Created : 20001024
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the XML module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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
*****************************************************************************/

#include "qsvgdevice.h"

#ifndef QT_NO_SVG

#include "qpaintdevicemetrics.h"
#include "qfile.h"

class QSVGDevicePrivate {
};

/*!
  \class QSVGDevice qsvgdevice.h

  \brief The QSVGDevice provides a paint device for SVG vector graphics.

  \module XML

  \sa QPaintDevice QPainter
*/

/*!
  Creates a QSVGDevice object.
 */

QSVGDevice::QSVGDevice()
    : QPaintDevice( QInternal::ExternalDevice )
{
    d = new QSVGDevicePrivate;
}

/*!
  Destructor.
*/

QSVGDevice::~QSVGDevice()
{
    delete d;
}

/*!
  Loads and parses \a file into the device. Returns TRUE on success,
  FALSE if errors were encountered.
 */

bool QSVGDevice::load( const QString &file )
{
    QFile f( file );
    if ( !f.open( IO_ReadOnly ) ) {
	qWarning( "QSVGDevice::load: Could not open input file" );
	return FALSE;
    }
    return doc.setContent( &f );
}

/*!
  Replays the graphic using \a painter and returns TRUE if successful, or
  FALSE if the document format is not valid.
 */

bool QSVGDevice::play( QPainter *painter )
{
    if ( doc.isNull() ) {
	qWarning( "QSVGDevice::play: No SVG data set." );
	return FALSE;
    }

    QDomNode svg = doc.namedItem( "svg" );
    if ( svg.isNull() || !svg.isElement() ) {
	qWarning( "QSVGDevice::play: Couldn't find any svg element." );
	return FALSE;
    }

    // 'play' all elements recursively starting with 'svg' as root
    return play( svg, painter );
}

/*!  \fn QRect QSVGDevice::boundingRect() const
  Returns the bounding rectangle of the vector graphic.
 */

/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  A QSVGDevice has the following hard coded values:
  dpi = 72, numcolors=16777216 and depth=24.
*/

int QSVGDevice::metric( int m ) const
{
    int val;
    switch ( m ) {
    case QPaintDeviceMetrics::PdmWidth:
	val = brect.width();
	break;
    case QPaintDeviceMetrics::PdmHeight:
	val = brect.height();
	break;
    case QPaintDeviceMetrics::PdmWidthMM:
	val = int(25.4/72.0*brect.width());
	break;
    case QPaintDeviceMetrics::PdmHeightMM:
	val = int(25.4/72.0*brect.height());
	break;
    case QPaintDeviceMetrics::PdmDpiX:
	val = 72;
	break;
    case QPaintDeviceMetrics::PdmDpiY:
	val = 72;
	break;
    case QPaintDeviceMetrics::PdmNumColors:
	val = 16777216;
	break;
    case QPaintDeviceMetrics::PdmDepth:
	val = 24;
	break;
    default:
	val = 0;
#if defined(QT_CHECK_RANGE)
	qWarning( "QSVGDevice::metric: Invalid metric command" );
#endif
    }
    return val;
}

/*!
  \internal
  Records painter commands and stores them in the QDomDocument doc.
 */

bool QSVGDevice::cmd ( int, QPainter*, QPDevCmdParam * )
{
    return FALSE;
}

/*!
  \internal
  Evaluate \a node, drawing on \a p. Allows recursive calls.
*/

bool QSVGDevice::play( const QDomNode & /*node*/, QPainter * /*p*/ )
{
    return FALSE; //###
}

#endif // QT_NO_SVG
