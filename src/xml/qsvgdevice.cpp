/****************************************************************************
** $Id: //depot/qt/main/src/xml/qsvgdevice.cpp#8 $
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

#include "qpainter.h"
#include "qpaintdevicemetrics.h"
#include "qfile.h"
#include "qmap.h"
#include "qregexp.h"

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
    : QPaintDevice( QInternal::ExternalDevice ),
      pt( 0 )
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
    if ( !painter ) {
#if defined(QT_CHECK_RANGE)
	Q_ASSERT( painter );
#endif
	return FALSE;
    }
    if ( doc.isNull() ) {
	qWarning( "QSVGDevice::play: No SVG data set." );
	return FALSE;
    }

    QDomNode svg = doc.namedItem( "svg" );
    if ( svg.isNull() || !svg.isElement() ) {
	qWarning( "QSVGDevice::play: Couldn't find any svg element." );
	return FALSE;
    }

    const struct ElementTable {
	const char *name;
	ElementType type;
    } etab[] = {
        { "line",     LineElement     },
        { "circle",   CircleElement   },
        { "ellipse",  EllipseElement  },
        { "rect",     RectElement     },
        { "polyline", PolylineElement },
        { "polygon",  PolygonElement  },
        { "path",     PathElement     },
        { "text",     TextElement     },
        { "image",    ImageElement    },
	{ "g",        GroupElement    },
	{ 0,          InvalidElement  }
    };
    // initialize only once
    if ( typeMap.isEmpty() ) {
	const ElementTable *t = etab;
	while ( t->name ) {
	    typeMap.insert( t->name, t->type );
	    t++;
	}
    }

    // 'play' all elements recursively starting with 'svg' as root
    pt = painter;
    return play( svg );
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

bool QSVGDevice::play( const QDomNode &node )
{
    QDomNode child = node.firstChild();

    while ( !child.isNull() ) {
	pt->save();

	ElementType t = typeMap[ child.nodeName() ];
	switch ( t ) {
	case RectElement:
	case CircleElement:
	case EllipseElement:
	case LineElement:
	case PolylineElement:
	case PolygonElement:
	case GroupElement:
	case PathElement:
	case TextElement:
	case ImageElement:
	case InvalidElement:
	    qWarning( "QSVGDevice::play: unknown element type " +
		      child.nodeName() );
	    break;
	};

	pt->restore();

	// move on to the next node
	child = child.nextSibling();
    }

    return TRUE;
}

/*!
  \internal
  Parses a CSS2-compatible color specification. Either a keyword or a numerical
  RGB specification like #ff00ff or rgb(255,0,50%).
 */

QColor QSVGDevice::parseColor( const QString &col )
{
    static const struct ColorTable {
	const char *name;
	const char *rgb;
    } coltab[] = {
	{ "black",   "#000000" },
	{ "silver",  "#c0c0c0" },
	{ "gray",    "#808080" },
	{ "white",   "#ffffff" },
	{ "maroon",  "#800000" },
	{ "red",     "#ff0000" },
	{ "purple",  "#800080" },
	{ "fuchsia", "#ff00ff" },
	{ "green",   "#008000" },
	{ "lime",    "#00ff00" },
	{ "olive",   "#808000" },
	{ "yellow",  "#ffff00" },
	{ "navy",    "#000080" },
	{ "blue",    "#0000ff" },
	{ "teal",    "#008080" },
	{ "aqua",    "#00ffff" },
	// ### the latest spec has more
	{ 0,         0         }
    };

    // initialize color map on first use
    if ( colMap.isEmpty() ) {
	const struct ColorTable *t = coltab;
	while ( t->name ) {
	    colMap.insert( t->name, t->rgb );
	    t++;
	}
    }

    // a keyword ?
    if ( colMap.contains ( col ) )
	return QColor( colMap[ col ] );
    // in rgb(r,g,b) form ?
    QString c = col;
    c.replace( QRegExp( "\\s*" ), "" );
    QRegExp reg( "^rgb\\((\\d+)(%?),(\\d+)(%?),(\\d+)(%?)\\)$" );
    if ( reg.search( c ) >= 0 ) {
	int comp[3];
	for ( int i = 0; i < 3; i++ ) {
	    comp[ i ] = reg.cap( 2*i+1 ).toInt();
	    if ( !reg.cap( 2*i+2 ).isEmpty() )		// percentage ?
		comp[ i ] = int((double(255*comp[ i ])/100.0));
	}
	return QColor( comp[ 0 ], comp[ 1 ], comp[ 2 ] );
    }

    // check for predefined Qt color objects, #RRGGBB and #RGB
    return QColor( col );
}

#endif // QT_NO_SVG
