/****************************************************************************
** $Id: //depot/qt/main/src/xml/qsvgdevice.cpp#10 $
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

    QDomNamedNodeMap attr = svg.attributes();
    int w = lenToInt( attr, "width" );
    int h = lenToInt( attr, "height" );
    brect.setWidth( w );
    brect.setHeight( h );

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
    pt->setPen( Qt::NoPen );
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

	QDomNamedNodeMap attr = child.attributes();
	if ( attr.contains( "style" ) )
	    setStyle( attr.namedItem( "style" ).nodeValue() );

	int x1, y1, x2, y2, rx, ry;
	ElementType t = typeMap[ child.nodeName() ];
	switch ( t ) {
	case RectElement:
	    break;
	case CircleElement:
	    x1 = lenToInt( attr, "cx" );
	    y1 = lenToInt( attr, "cy" );
	    rx = lenToInt( attr, "r" );
	    pt->drawEllipse( x1-rx, y1-rx, 2*rx, 2*rx );
	    break;
	case EllipseElement:
	    x1 = lenToInt( attr, "cx" );
	    y1 = lenToInt( attr, "cy" );
	    rx = lenToInt( attr, "rx" );
	    ry = lenToInt( attr, "ry" );
	    pt->drawEllipse( x1-rx, y1-ry, 2*rx, 2*ry );
	    break;
	case LineElement:
	    x1 = lenToInt( attr, "x1" );
	    x2 = lenToInt( attr, "x2" );
	    y1 = lenToInt( attr, "y1" );
	    y2 = lenToInt( attr, "y2" );
	    pt->drawLine( x1, y1, x2, y2 );
	    break;
	case PolylineElement:
	case PolygonElement:
	    break;
	case GroupElement:
	    play( child );
	    break;
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

/*!
  \internal
  Parse a <length> datatype consisting of a number followed by an optional
  unit specifier. Can be used for type <coordinate> as well.
*/

double QSVGDevice::parseLen( const QString &str, bool *ok ) const
{
    QRegExp reg( "([+-]*\\d*\\.*\\d*[Ee]?[+-]?\\d*)(em|ex|px|pt|pc|cm|mm|)" );
    if ( reg.search( str ) == -1 ) {
	qWarning( "QSVGDevice::parseLen: couldn't parse " + str );
	if ( ok )
	    *ok = FALSE;
	return 0.0;
    }

    double d = reg.cap( 1 ).toDouble();
    // ### respect unit identifier
    if ( ok )
	*ok = TRUE;
    return d;
}

/*!
  \internal
  Returns the length specified in attribute \a attr in \a map. If the
  specified attribute doesn't exist or can't be parsed \a def is returned.
*/

int QSVGDevice::lenToInt( const QDomNamedNodeMap &map, const QString &attr,
			  int def ) const
{
    if ( map.contains( attr ) ) {
	bool ok;
	double d = parseLen( map.namedItem( attr ).nodeValue(), &ok );
	if ( ok )
	    return int(d);
    }
    return def;
}

void QSVGDevice::setStyle( const QString &s )
{
    QStringList rules = QStringList::split( QRegExp( ";" ), s );

    QPen pen = pt->pen();
    QFont font = pt->font();

    QStringList::ConstIterator it = rules.begin();
    for ( ; it != rules.end(); it++ ) {
	int col = (*it).find( ':' );
	if ( col > 0 ) {
	    QString prop = (*it).left( col ).simplifyWhiteSpace();
	    QString val = (*it).right( (*it).length() - col - 1 );
	    val = val.lower().stripWhiteSpace();
	    if ( prop == "stroke" ) {
		if ( val == "none" )
		    pen.setStyle( Qt::NoPen );
		else {
		    pen.setColor( parseColor( val ));
		    if ( pen.style() == Qt::NoPen )
			pen.setStyle( Qt::SolidLine );
		}
	    } else if ( prop == "stroke-width" ) {
		pen.setWidth( int(parseLen( val )) );
		if ( pen.style() == Qt::NoPen )
		    pen.setStyle( Qt::SolidLine );
	    } else if ( prop == "stroke-linecap" ) {
		if ( val == "butt" )
		    pen.setCapStyle( Qt::FlatCap );
		else if ( val == "round" )
		    pen.setCapStyle( Qt::RoundCap );
		else if ( val == "square" )
		    pen.setCapStyle( Qt::SquareCap );
	    } else if ( prop == "stroke-linejoin" ) {
		if ( val == "miter" )
		    pen.setJoinStyle( Qt::MiterJoin );
		else if ( val == "round" )
		    pen.setJoinStyle( Qt::RoundJoin );
		else if ( val == "bevel" )
		    pen.setJoinStyle( Qt::BevelJoin );
	    } else if ( prop == "fill" ) {
		if ( val == "none" )
		    pt->setBrush( Qt::NoBrush );
		else
		    pt->setBrush( parseColor( val ));
	    } else if ( prop == "font-size" ) {
		font.setPixelSizeFloat( float(parseLen( val )) );
	    } else if ( prop == "font-family" ) {
		font.setFamily( val );
	    }
	}
    }

    pt->setPen( pen );
    pt->setFont( font );
}

#endif // QT_NO_SVG
