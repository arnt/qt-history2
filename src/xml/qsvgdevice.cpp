/****************************************************************************
** $Id: $
**
** Implementation of the QSvgDevice class
**
** Created : 20001024
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include <private/qsvgdevice_p.h>

#ifndef QT_NO_SVG

#include "qpainter.h"
#include "qpaintdevicemetrics.h"
#include "qfile.h"
#include "qmap.h"
#include "qregexp.h"
#include "qvaluelist.h"
#include "qtextstream.h"
#include "qimage.h"
#include "qpixmap.h"

#include <math.h>

const double deg2rad = 0.017453292519943295769;	// pi/180
const char piData[] = "version=\"1.0\" standalone=\"no\"";
const char publicId[] = "-//W3C//DTD SVG 20001102//EN";
const char systemId[] = "http://www.w3.org/TR/2000/CR-SVG-20001102/DTD/svg-20001102.dtd";

struct QM_EXPORT_SVG ImgElement {
    QDomElement element;
    QImage image;
};

struct QM_EXPORT_SVG PixElement {
    QDomElement element;
    QPixmap pixmap;
};

typedef QValueList<ImgElement> ImageList;
typedef QValueList<PixElement> PixmapList;

class QSvgDevicePrivate {
public:
    ImageList images;
    PixmapList pixmaps;
    int talign;				// text alignment
};

enum ElementType {
    InvalidElement = 0,
    CircleElement,
    CommentElement,
    DescElement,
    EllipseElement,
    GroupElement,
    ImageElement,
    LineElement,
    PolylineElement,
    PolygonElement,
    PathElement,
    RectElement,
    TextElement,
    TitleElement
};

typedef QMap<QString,ElementType> QSvgTypeMap;
static QSvgTypeMap *qSvgTypeMap=0; // element types
static QMap<QString,QString> *qSvgColMap=0; // recognized color keyword names

/*!
  \class QSvgDevice qsvgdevice.h
  \brief The QSvgDevice class provides a paint device for SVG vector graphics.

  \ingroup xml-tools
  \module XML
  \internal

  SVG is an XML vector graphics format. This class supports the
  loading and saving of SVG files with load() and save(), and the
  rendering of an SVG onto a QPainter using play(). Use
  toString() to put the SVG into a string.

  \sa QPaintDevice QPainter
*/

/*!
  Creates a QSvgDevice object.
 */

QSvgDevice::QSvgDevice()
    : QPaintDevice( QInternal::ExternalDevice ),
      pt( 0 )
{
    d = new QSvgDevicePrivate;
}

/*!
  Destroys the QSvgDevice object and frees the resources it used.
*/

QSvgDevice::~QSvgDevice()
{
    delete qSvgTypeMap; qSvgTypeMap = 0;	// static
    delete qSvgColMap; qSvgColMap = 0;
    delete d;
}

/*!
  Loads and parses \a fileName into the device. Returns TRUE on
  success, or FALSE if errors were encountered.
*/

bool QSvgDevice::load( const QString &fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_ReadOnly ) ) {
	qWarning( "QSvgDevice::load: Could not open input file" );
	return FALSE;
    }
    return doc.setContent( &f );
}

/*!
  Renders (replays) the graphic on the \a painter and returns TRUE if
  successful, or FALSE if the document format is not valid.
*/

bool QSvgDevice::play( QPainter *painter )
{
    if ( !painter ) {
#if defined(QT_CHECK_RANGE)
	Q_ASSERT( painter );
#endif
	return FALSE;
    }
    pt = painter;
    if ( doc.isNull() ) {
	qWarning( "QSvgDevice::play: No SVG data set." );
	return FALSE;
    }

    QDomNode svg = doc.namedItem( "svg" );
    if ( svg.isNull() || !svg.isElement() ) {
	qWarning( "QSvgDevice::play: Couldn't find any svg element." );
	return FALSE;
    }

    QDomNamedNodeMap attr = svg.attributes();
    QString wstr = attr.contains( "width" )
		   ? attr.namedItem( "width" ).nodeValue() : QString( "100%" );
    QString hstr = attr.contains( "height" )
		   ? attr.namedItem( "height" ).nodeValue() : QString( "100%" );
    if ( attr.contains( "viewBox" ) ) {
	QRegExp re( "\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*,?"
		    "\\s*(\\S+)\\s*,?\\s*(\\S+)\\s*" );
	if ( re.search( attr.namedItem( "viewBox" ).nodeValue() ) < 0 ) {
	    qWarning( "QSvgDevice::play: Invalid viewBox attribute.");
	    return FALSE;
	} else {
	    int x = int(re.cap( 1 ).toDouble());
	    int y = int(re.cap( 2 ).toDouble());
	    int w = int(re.cap( 3 ).toDouble());
	    int h = int(re.cap( 4 ).toDouble());
	    if ( w < 0 || h < 0 ) {
		qWarning( "QSvgDevice::play: Invalid viewBox dimension.");
		return FALSE;
	    } else if ( w == 0 || h == 0 ) {
		return TRUE;
	    }
	    painter->setWindow( x, y, w, h );
	}
    }
    int w = int(parseLen( wstr, 0, TRUE ));
    int h = int(parseLen( hstr, 0, FALSE ));
    brect.setWidth( w );
    brect.setHeight( h );
    painter->setClipRect( 0, 0, w, h, QPainter::CoordPainter );
    if ( attr.contains( "transform" ) )
	setTransform( attr.namedItem( "transform" ).nodeValue() );

    const struct ElementTable {
	const char *name;
	ElementType type;
    } etab[] = {
	{ "#comment", CommentElement  },
        { "circle",   CircleElement   },
	{ "desc",     DescElement     },
        { "ellipse",  EllipseElement  },
	{ "g",        GroupElement    },
        { "image",    ImageElement    },
        { "line",     LineElement     },
        { "polyline", PolylineElement },
        { "polygon",  PolygonElement  },
        { "path",     PathElement     },
        { "rect",     RectElement     },
        { "text",     TextElement     },
	{ "title",    TitleElement    },
	{ 0,          InvalidElement  }
    };
    // initialize only once
    if ( !qSvgTypeMap ) {
	qSvgTypeMap = new QSvgTypeMap;
	const ElementTable *t = etab;
	while ( t->name ) {
	    qSvgTypeMap->insert( t->name, t->type );
	    t++;
	}
    }

    // 'play' all elements recursively starting with 'svg' as root
    d->talign = Qt::AlignLeft;
    return play( svg );
}

/*!
    Returns the SVG as a single string of XML.
*/
QString QSvgDevice::toString() const
{
    if ( doc.isNull() )
	return QString();

    return doc.toString();
}

/*!
  Saves the SVG vector graphics to \a fileName.
 */

bool QSvgDevice::save( const QString &fileName )
{
    // guess svg id from fileName
    QString svgName = fileName.endsWith( ".svg" ) ?
		      fileName.left( fileName.length()-4 ) : fileName;

    // now we have the info about name and dimensions available
    QDomElement root = doc.documentElement();
    root.setAttribute( "id", svgName );
    root.setAttribute( "x", brect.x() );
    root.setAttribute( "y", brect.y() );
    root.setAttribute( "width", brect.width() );
    root.setAttribute( "height", brect.height() );

    // ... and know how to name any image files to be written out
    int icount = 0;
    ImageList::Iterator iit = d->images.begin();
    for ( ; iit != d->images.end(); iit++ ) {
	QString href = QString( "%1_%2.png" ).arg( svgName ).arg( icount );
	(*iit).image.save( href, "PNG" );
	(*iit).element.setAttribute( "xlink:href", href );
	icount++;
    }
    PixmapList::Iterator pit = d->pixmaps.begin();
    for ( ; pit != d->pixmaps.end(); pit++ ) {
	QString href = QString( "%1_%2.png" ).arg( svgName ).arg( icount );
	(*pit).pixmap.save( href, "PNG" );
	(*pit).element.setAttribute( "xlink:href", href );
	icount++;
    }

    QFile f( fileName );
    if ( !f.open ( IO_WriteOnly ) )
	return FALSE;
    QTextStream s( &f );
    s.setEncoding( QTextStream::UnicodeUTF8 );
    s << doc;

    return TRUE;
}

/*!
  \fn QRect QSvgDevice::boundingRect() const

  Returns the bounding rectangle of the vector graphic.
 */

/*!
  Sets the bounding rectangle of the vector graphic to rectangle \a r.
 */

void QSvgDevice::setBoundingRect( const QRect &r )
{
    brect = r;
}

/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  A QSvgDevice has the following hard coded values:
  dpi = 72, numcolors=16777216 and depth=24.
  \a m is the metric to get.
*/

int QSvgDevice::metric( int m ) const
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
	qWarning( "QSvgDevice::metric: Invalid metric command" );
#endif
    }
    return val;
}

/*!
  \internal
  Records painter commands and stores them in the QDomDocument doc.
 */

bool QSvgDevice::cmd ( int c, QPainter *painter, QPDevCmdParam *p )
{
    pt = painter;

    if ( c == PdcBegin ) {
	QDomImplementation domImpl;
	QDomDocumentType docType = domImpl.createDocumentType( "svg",
							       publicId,
							       systemId );
	doc = domImpl.createDocument( "http://www.w3.org/2000/svg",
				      "svg", docType );
	doc.insertBefore( doc.createProcessingInstruction( "xml", piData ),
			  doc.firstChild() );
	current = doc.documentElement();
	d->images.clear();
	d->pixmaps.clear();
	dirtyTransform = dirtyStyle = FALSE; // ###
	return TRUE;
    } else if ( c == PdcEnd ) {
	return TRUE;
    }

    QDomElement e;
    QString str;
    QRect rect;
    QPointArray a;
    int i;
    switch ( c ) {
    case PdcNOP:
	break;
    case PdcMoveTo:
	curPt = *p[0].point;
	break;
    case PdcLineTo:
	e = doc.createElement( "line" );
	e.setAttribute( "x1", curPt.x() );
	e.setAttribute( "y1", curPt.y() );
	e.setAttribute( "x2", p[0].point->x() );
	e.setAttribute( "y2", p[0].point->y() );
	break;
    case PdcDrawPoint:
    case PdcDrawLine:
	e = doc.createElement( "line" );
	e.setAttribute( "x1", p[0].point->x() );
	e.setAttribute( "y1", p[0].point->y() );
	i = ( c == PdcDrawLine ) ? 1 : 0;
	e.setAttribute( "x2", p[i].point->x() );
	e.setAttribute( "y2", p[i].point->y() );
	break;
    case PdcDrawRect:
    case PdcDrawRoundRect:
	e = doc.createElement( "rect" );
	e.setAttribute( "x", p[0].rect->x() );
	e.setAttribute( "y", p[0].rect->y() );
	e.setAttribute( "width", p[0].rect->width() );
	e.setAttribute( "height", p[0].rect->height() );
	if ( c == PdcDrawRoundRect ) {
	    e.setAttribute( "rx", (p[1].ival*p[0].rect->width())/200 );
	    e.setAttribute( "ry", (p[2].ival*p[0].rect->height())/200 );
	}
	break;
    case PdcDrawEllipse:
	rect = *p[0].rect;
	if ( rect.width() == rect.height() ) {
	    e = doc.createElement( "circle" );
	    e.setAttribute( "cx", rect.center().x() );
	    e.setAttribute( "cy", rect.center().y() );
	    e.setAttribute( "r", rect.width() / 2 );
	} else {
	    e = doc.createElement( "ellipse" );
	    e.setAttribute( "cx", rect.center().x() );
	    e.setAttribute( "cy", rect.center().y() );
	    e.setAttribute( "rx", rect.width() / 2 );
	    e.setAttribute( "ry", rect.height() / 2 );
	}
	break;
    case PdcDrawArc:
    case PdcDrawPie:
    case PdcDrawChord: {
	rect = *p[0].rect;
	double a = (double)p[1].ival / 16.0 * deg2rad;
	double al = (double)p[2].ival / 16.0 * deg2rad;
	double rx = rect.width() / 2.0;
	double ry = rect.height() / 2.0;
	double x0 = (double)rect.x() + rx;
	double y0 = (double)rect.y() + ry;
	double x1 = x0 + rx*(1+cos(a));
	double y1 = y0 + ry*(1-sin(a));
	double x2 = x0 + rx*(1+cos(a+al));
	double y2 = y0 + ry*(1-sin(a+al));
	int large = al > 180.0 ? 1 : 0;
	int sweep = al > 0.0 ? 1 : 0;
	if ( c == PdcDrawPie ) {
	    str = QString( "M %1 %2 " ).arg( x0 ).arg( y0 );
	    str += QString( "L %1 %2 " ).arg( x1 ).arg( y1 );
	} else
	    str = QString( "M %1 %2 " ).arg( x1 ).arg( y1 );
	str += QString( "A %1 %2 0 %3 %4 %5 %6" )
	       .arg( rx ).arg( ry ).arg( large ).arg( sweep )
	       .arg( x2 ).arg( y2 );
	if ( c != PdcDrawArc )
	    str += "z";
	e = doc.createElement( "path" );
	e.setAttribute( "d", str );
    }
	break;
    case PdcDrawLineSegments:
	{
	    a = *p[0].ptarr;
	    for (uint i = 0; i < a.size() / 2; i++) {
		e = doc.createElement( "line" );
		e.setAttribute( "x1", a[int(2*i)].x() );
		e.setAttribute( "y1", a[int(2*i)].y() );
		e.setAttribute( "x2", a[int(2*i+1)].x() );
		e.setAttribute( "y2", a[int(2*i+1)].y() );
	    }
	}
	break;
    case PdcDrawPolyline:
    case PdcDrawPolygon:
	{
	    a = *p[0].ptarr;
	    e = doc.createElement( ( c == PdcDrawPolyline ) ?
				   "polyline" : "polygon" );
	    for (uint i = 0; i < a.size(); i++) {
		QString tmp;
		tmp.sprintf( "%d %d ", a[ (int)i ].x(), a[ (int)i ].y() );
		str += tmp;
	    }
	    e.setAttribute( "points", str.stripWhiteSpace() );
	}
	break;
    case PdcDrawCubicBezier:
	a = *p[0].ptarr;
	e = doc.createElement( "path" );
	str.sprintf( "M %d %d C %d %d %d %d %d %d", a[0].x(), a[0].y(),
		     a[1].x(), a[1].y(), a[2].x(), a[2].y(),
		     a[3].x(), a[3].y() );
	e.setAttribute( "d", str );
	break;
    case PdcDrawText2:
	e = doc.createElement( "text" );
	if ( p[0].point->x() )
	    e.setAttribute( "x", p[0].point->x() );
	if ( p[0].point->y() )
	    e.setAttribute( "y", p[0].point->y() );
	e.appendChild( doc.createTextNode( *p[1].str ) );
	break;
    case PdcDrawText2Formatted:
	e = doc.createElement( "text" );
	if ( p[0].rect->x() )
	    e.setAttribute( "x", p[0].rect->x() );
	if ( p[0].point->y() )
	    e.setAttribute( "y", p[0].rect->y()+painter->fontMetrics().ascent() );
	// ### int tf = p[1].ival;
	e.appendChild( doc.createTextNode( *p[2].str ) );
	break;
    case PdcDrawPixmap:
    case PdcDrawImage:
	e = doc.createElement( "image" );
	e.setAttribute( "x", p[0].rect->x() );
	e.setAttribute( "y", p[0].rect->y() );
	e.setAttribute( "width", p[0].rect->width() );
	e.setAttribute( "height", p[0].rect->height() );
	if ( c == PdcDrawImage ) {
	    ImgElement ie;
	    ie.element = e;
	    ie.image = *p[1].image;
	    d->images.append( ie );
	} else {
	    PixElement pe;
	    pe.element = e;
	    pe.pixmap = *p[1].pixmap;
	    d->pixmaps.append( pe );
	}
	// saving to disk and setting the xlink:href attribute will be
	// done later in save() once we now the svg document name.
	break;
    case PdcSave:
	e = doc.createElement( "g" );
	break;
    case PdcRestore:
	current = current.parentNode();
	// ### reset dirty flags
	break;
    case PdcSetBkColor:
    case PdcSetBkMode:
    case PdcSetROP:
    case PdcSetBrushOrigin:
    case PdcSetFont:
    case PdcSetPen:
    case PdcSetBrush:
	dirtyStyle = TRUE;
	break;
    case PdcSetTabStops:
	// ###
	break;
    case PdcSetTabArray:
	// ###
	break;
    case PdcSetVXform:
    case PdcSetWindow:
    case PdcSetViewport:
    case PdcSetWXform:
    case PdcSetWMatrix:
    case PdcSaveWMatrix:
    case PdcRestoreWMatrix:
	dirtyTransform = TRUE;
	break;
    case PdcSetClip:
	// ###
	break;
    case PdcSetClipRegion:
	// ###
	break;
    default:
#if defined(CHECK_RANGE)
	qWarning( "QSVDevice::cmd: Invalid command %d", c );
#endif
	break;
    }

    if ( !e.isNull() ) {
	current.appendChild( e );
	if ( c == PdcSave )
	    current = e;
	// ### optimize application of attributes utilizing <g>
	if ( dirtyStyle )		// only reset when entering
	    applyStyle( &e, c );	// or leaving a <g> tag
	if ( dirtyTransform )		// same as above
	    applyTransform( &e );
    }

    return TRUE;
}

/*!
  \internal
  Evaluate \a node, drawing on \a p. Allows recursive calls.
*/

bool QSvgDevice::play( const QDomNode &node )
{
    QDomNode child = node.firstChild();

    while ( !child.isNull() ) {
	pt->save();

	QDomNamedNodeMap attr = child.attributes();
	if ( attr.contains( "style" ) )
	    setStyle( attr.namedItem( "style" ).nodeValue() );
	if ( attr.contains( "transform" ) )
	    setTransform( attr.namedItem( "transform" ).nodeValue() );

	int x1, y1, x2, y2, rx, ry, w, h;
	ElementType t = (*qSvgTypeMap)[ child.nodeName() ];
	switch ( t ) {
	case CommentElement:
	    // ignore
	    break;
	case RectElement:
	    rx = ry = 0;
	    x1 = lenToInt( attr, "x" );
	    y1 = lenToInt( attr, "y" );
	    w = lenToInt( attr, "width" );
	    h = lenToInt( attr, "height" );
	    if ( w == 0 || h == 0 )	// prevent div by zero below
		break;
	    x2 = (int)attr.contains( "rx" ); // tiny abuse of x2 and y2
	    y2 = (int)attr.contains( "ry" );
	    if ( x2 )
		rx = lenToInt( attr, "rx" );
	    if ( y2 )
		ry = lenToInt( attr, "ry" );
	    if ( x2 && !y2 )
		ry = rx;
	    else if ( !x2 && y2 )
		rx = ry;
	    rx = int(200.0*double(rx)/double(w));
	    ry = int(200.0*double(ry)/double(h));
	    pt->drawRoundRect( x1, y1, w, h, rx, ry );
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
	    {
		QString pts = attr.namedItem( "points" ).nodeValue();
		pts = pts.simplifyWhiteSpace();
		QStringList sl = QStringList::split( QRegExp( "[ ,]" ), pts );
		QPointArray ptarr( (uint)sl.count() / 2);
		for ( int i = 0; i < (int)sl.count() / 2; i++ ) {
		    double dx = sl[2*i].toDouble();
		    double dy = sl[2*i+1].toDouble();
		    ptarr.setPoint( i, int(dx), int(dy) );
		}
  		if ( t == PolylineElement ) {
		    if ( pt->brush().style() != Qt::NoBrush ) {
			QPen pn = pt->pen();
			pt->setPen( Qt::NoPen );
			pt->drawPolygon( ptarr );
			pt->setPen( pn );
		    }
		    pt->drawPolyline( ptarr ); // ### closes when filled. bug ?
  		} else {
		    pt->drawPolygon( ptarr );
		}
	    }
	    break;
	case GroupElement:
	    {
		// ### have a real stack for save and restore
		int talign = d->talign;
		play( child );
		d->talign = talign;
	    }
	    break;
	case PathElement:
	    drawPath( attr.namedItem( "d" ).nodeValue() );
	    break;
	case TextElement:
	    {
		x1 = lenToInt( attr, "x" );
		y1 = lenToInt( attr, "y" );
		// backup old colors
		QPen pn = pt->pen();
		QColor pcolor = pn.color();
		QColor bcolor = pt->brush().color();
		QDomNode c = child.firstChild();
		while ( !c.isNull() ) {
		    if ( c.isText() ) {
			// we have pen and brush reversed for text drawing
			pn.setColor( bcolor );
			pt->setPen( pn );
			QString text = c.toText().nodeValue();
			text = text.simplifyWhiteSpace(); // ### 'preserve'
			w = pt->fontMetrics().width( text );
			if ( d->talign == Qt::AlignHCenter )
			    x1 -= w / 2;
			else if ( d->talign == Qt::AlignRight )
			    x1 -= w;
			pt->drawText( x1, y1, text );
			// restore pen
			pn.setColor( pcolor );
			pt->setPen( pn );
			x1 += w;
		    }
		    c = c.nextSibling();
		}
	    }
	    break;
	case ImageElement:
	    {
		x1 = lenToInt( attr, "x" );
		y1 = lenToInt( attr, "y" );
		w = lenToInt( attr, "width" );
		h = lenToInt( attr, "height" );
		QString href = attr.namedItem( "xlink:href" ).nodeValue();
		// ### catch references to embedded .svg files
		QPixmap pix;
		if ( !pix.load( href ) ) {
		    qWarning( "QSvgDevice::play: Couldn't load image "+href );
		    break;
		}
		pt->drawPixmap( QRect( x1, y1, w, h ), pix );
	    }
	    break;
	case DescElement:
	case TitleElement:
	    // ignored for now
	    break;
	case InvalidElement:
	    qWarning( "QSvgDevice::play: unknown element type " +
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

QColor QSvgDevice::parseColor( const QString &col )
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
    if ( !qSvgColMap ) {
	qSvgColMap = new QMap<QString, QString>;
	const struct ColorTable *t = coltab;
	while ( t->name ) {
	    qSvgColMap->insert( t->name, t->rgb );
	    t++;
	}
    }

    // a keyword ?
    if ( qSvgColMap->contains ( col ) )
	return QColor( (*qSvgColMap)[ col ] );
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
  unit specifier. Can be used for type <coordinate> as well. For relative
  units the value of \a horiz will determine whether the horizontal or
  vertical dimension will be used.
*/

double QSvgDevice::parseLen( const QString &str, bool *ok, bool horiz ) const
{
    QRegExp reg( "([+-]?\\d*\\.*\\d*[Ee]?[+-]?\\d*)(em|ex|px|%|pt|pc|cm|mm|in|)$" );
    if ( reg.search( str ) == -1 ) {
	qWarning( "QSvgDevice::parseLen: couldn't parse " + str );
	if ( ok )
	    *ok = FALSE;
	return 0.0;
    }

    double d = reg.cap( 1 ).toDouble();
    QString u = reg.cap( 2 );
    if ( !u.isEmpty() && u != "px" ) {
	QPaintDeviceMetrics m( pt->device() );
	if ( u == "em" )
	    d *= pt->font().pointSizeFloat(); // ### pixel size
	else if ( u == "ex" )
	    d *= 0.5 * pt->font().pointSizeFloat(); // ### not precise
	else if ( u == "%" )
	    d *= (horiz ? pt->window().width() : pt->window().height())/100.0;
	else if ( u == "cm" )
	    d *= m.logicalDpiX() / 2.54;
	else if ( u == "mm" )
	    d *= m.logicalDpiX() / 25.4;
	else if ( u == "in" )
	    d *= m.logicalDpiX();
	else if ( u == "pt" )
	    d *= m.logicalDpiX() / 72.0;
	else if ( u == "pc" )
	    d *= m.logicalDpiX() / 6.0;
	else
	    qWarning( "QSvgDevice::parseLen: Unknown unit " + u );
    }
    if ( ok )
	*ok = TRUE;
    return d;
}

/*!
  \internal
  Returns the length specified in attribute \a attr in \a map. If the
  specified attribute doesn't exist or can't be parsed \a def is returned.
*/

int QSvgDevice::lenToInt( const QDomNamedNodeMap &map, const QString &attr,
			  int def ) const
{
    if ( map.contains( attr ) ) {
	bool ok;
	double d = parseLen( map.namedItem( attr ).nodeValue(), &ok );
	if ( ok )
	    return qRound( d );
    }
    return def;
}

void QSvgDevice::setStyle( const QString &s )
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
		if ( val == "none" ) {
		    pen.setStyle( Qt::NoPen );
		} else {
		    pen.setColor( parseColor( val ));
 		    if ( pen.style() == Qt::NoPen )
 			pen.setStyle( Qt::SolidLine );
		    if ( pen.width() == 0 )
 			pen.setWidth( 1 );
		}
	    } else if ( prop == "stroke-width" ) {
		pen.setWidth( int(parseLen( val )) );
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
		// ### pixel size seems more logical but produces bad results
		font.setPointSizeFloat( float(parseLen( val )) );
	    } else if ( prop == "font-family" ) {
		font.setFamily( val );
	    } else if ( prop == "text-anchor" ) {
		if ( val == "middle" )
		    d->talign = Qt::AlignHCenter;
		else if ( val == "end" )
		    d->talign = Qt::AlignRight;
		else
		    d->talign = Qt::AlignLeft;
	    }
	}
    }

    pt->setPen( pen );
    pt->setFont( font );
}

void QSvgDevice::setTransform( const QString &tr )
{
    QString t = tr.simplifyWhiteSpace();

    QRegExp reg( "\\s*([\\w]+)\\s*\\(([^\\(]*)\\)" );
    int index = 0;
    while ( (index = reg.search(t, index)) >= 0 ) {
	QString command = reg.cap( 1 );
	QString params = reg.cap( 2 );
	QStringList plist = QStringList::split( QRegExp("[,\\s]"), params );
	if ( command == "translate" ) {
	    double tx = 0, ty = 0;
	    tx = plist[0].toDouble();
	    if ( plist.count() >= 2 )
		ty = plist[1].toDouble();
	    pt->translate( tx, ty );
	} else if ( command == "rotate" ) {
	    pt->rotate( plist[0].toDouble() );
	} else if ( command == "scale" ) {
	    double sx, sy;
	    sx = sy = plist[0].toDouble();
	    if ( plist.count() >= 2 )
		sy = plist[1].toDouble();
	    pt->scale( sx, sy );
	} else if ( command == "matrix" && plist.count() >= 6 ) {
	    double m[ 6 ];
	    for (int i = 0; i < 6; i++)
		m[ i ] = plist[ i ].toDouble();
	    QWMatrix wm( m[ 0 ], m[ 1 ], m[ 2 ],
			 m[ 3 ], m[ 4 ], m[ 5 ] );
	    pt->setWorldMatrix( wm, TRUE );
	} else if ( command == "skewX" ) {
	    pt->shear( 0.0, tan( plist[0].toDouble() * deg2rad ) );
	} else if ( command == "skewY" ) {
	    pt->shear( tan( plist[0].toDouble() * deg2rad ), 0.0 );
	}

	// move on to next command
	index += reg.matchedLength();
    }
}

void QSvgDevice::drawPath( const QString &data )
{
    int x0 = 0, y0 = 0;			// starting point
    int x = 0, y = 0;			// current point
    int controlX = 0, controlY = 0;	// last control point for curves
    QPointArray path( 500 );		// resulting path
    QValueList<int> subIndex;		// start indices for subpaths
    QPointArray quad( 4 ), bezier;	// for curve calculations
    int pcount = 0;			// current point array index
    uint idx = 0;			// current data position
    int mode = 0, lastMode = 0;		// parser state
    bool relative = FALSE;		// e.g. 'h' vs. 'H'
    QString commands( "MZLHVCSQTA" );	// recognized commands
    int cmdArgs[] = { 2, 0, 2, 1, 1, 6, 4, 4, 2, 7 };	// no of arguments
    QRegExp reg( "\\s*,?\\s*([+-]?\\d*\\.?\\d*)" );	// floating point

    subIndex.append( 0 );
    // detect next command
    while ( idx < data.length() ) {
	QChar ch = data[ (int)idx++ ];
	if ( ch.isSpace() )
	    continue;
	QChar chUp = ch.upper();
	int cmd = commands.find( chUp );
	if ( cmd >= 0 ) {
	    // switch to new command mode
	    mode = cmd;
	    relative = ( ch != chUp );		// e.g. 'm' instead of 'M'
	} else {
	    if ( mode && !ch.isLetter() ) {
		cmd = mode;			// continue in previous mode
		idx--;
	    } else {
		qWarning( "QSvgDevice::drawPath: Unknown command" );
		return;
	    }
	}

	// read in the required number of arguments
	const int maxArgs = 7;
	double arg[ maxArgs ];
	int numArgs = cmdArgs[ cmd ];
	for ( int i = 0; i < numArgs; i++ ) {
	    int pos = reg.search( data, idx );
	    if ( pos == -1 ) {
		qWarning( "QSvgDevice::drawPath: Error parsing arguments" );
		return;
	    }
	    arg[ i ] = reg.cap( 1 ).toDouble();
	    idx = pos + reg.matchedLength();
	};

	// process command
	int offsetX = relative ? x : 0;		// correction offsets
	int offsetY = relative ? y : 0;		// for relative commands
	switch ( mode ) {
	case 0:					// 'M' move to
	    if ( x != x0 || y != y0 )
		path.setPoint( pcount++, x0, y0 );
	    x = x0 = int(arg[ 0 ]) + offsetX;
	    y = y0 = int(arg[ 1 ]) + offsetY;
	    subIndex.append( pcount );
	    path.setPoint( pcount++, x0, y0 );
	    mode = 2;				// -> 'L'
	    break;
	case 1:					// 'Z' close path
	    path.setPoint( pcount++, x0, y0 );
	    x = x0;
	    y = y0;
	    mode = 0;
	    break;
	case 2:					// 'L' line to
	    x = int(arg[ 0 ]) + offsetX;
	    y = int(arg[ 1 ]) + offsetY;
	    path.setPoint( pcount++, x, y );
	    break;
	case 3:					// 'H' horizontal line
	    x = int(arg[ 0 ]) + offsetX;
	    path.setPoint( pcount++, x, y );
	    break;
	case 4:					// 'V' vertical line
	    y = int(arg[ 0 ]) + offsetY;
	    path.setPoint( pcount++, x, y );
	    break;
	case 5:					// 'C' cubic bezier curveto
	case 6:					// 'S' smooth shorthand
	case 7:					// 'Q' quadratic bezier curves
	case 8: {				// 'T' smooth shorthand
	    quad.setPoint( 0, x, y );
	    // if possible, reflect last control point if smooth shorthand
	    if ( mode == 'S' || mode == 'T' ) {
		bool cont = mode == lastMode ||
		     mode == 'S' && lastMode == 'C' ||
		     mode == 'T' && lastMode == 'Q';
		x = cont ? 2*x-controlX : x;
		y = cont ? 2*y-controlY : y;
		quad.setPoint( 1, x, y );
		quad.setPoint( 2, x, y );
	    }
	    for ( int j = 0; j < numArgs/2; j++ ) {
		x = int(arg[ 2*j   ]) + offsetX;
		y = int(arg[ 2*j+1 ]) + offsetY;
		quad.setPoint( j+4-numArgs/2, x, y );
	    }
	    // remember last control point for next shorthand
	    controlX = quad[ 2 ].x();
	    controlY = quad[ 2 ].y();
	    // transform quadratic into cubic Bezier
	    if ( mode == 'Q' || mode == 'T' ) {
		int x31 = quad[0].x()+int(2.0*(quad[2].x()-quad[0].x())/3.0);
		int y31 = quad[0].y()+int(2.0*(quad[2].y()-quad[0].y())/3.0);
		int x32 = quad[2].x()+int(2.0*(quad[3].x()-quad[2].x())/3.0);
		int y32 = quad[2].y()+int(2.0*(quad[3].y()-quad[2].y())/3.0);
		quad.setPoint( 1, x31, y31 );
		quad.setPoint( 2, x32, y32 );
	    }
	    // calculate points on curve
	    bezier = quad.cubicBezier();
	    for ( int k = 0; k < (int)bezier.size(); k ++ )
		path.setPoint( pcount++, bezier[ k ] );
	    break;
	}
	case 9:					// 'A' elliptical arc curve
	    // ### just a straight line
	    x = int(arg[ 5 ]) + offsetX;
	    y = int(arg[ 6 ]) + offsetY;
	    path.setPoint( pcount++, x, y );
	    break;
	};
	lastMode = mode;
    }

    subIndex.append( pcount );			// dummy marking the end
    if ( pt->brush().style() != Qt::NoBrush ) {
	// fill the area without stroke first
	if ( x != x0 || y != y0 )
	    path.setPoint( pcount++, x0, y0 );
	QPen pen = pt->pen();
	pt->setPen( Qt::NoPen );
	pt->drawPolygon( path, FALSE, 0, pcount );
	pt->setPen( pen );
    }
    // draw each subpath stroke seperately
    QValueListConstIterator<int> it = subIndex.begin();
    int start = 0;
    while ( it != subIndex.fromLast() ) {
	int next = *++it;
	// ### always joins ends if first and last point coincide.
	// ### 'Z' can't have the desired effect
	pt->drawPolyline( path, start, next-start );
	start = next;
    }
}

void QSvgDevice::applyStyle( QDomElement *e, int c ) const
{
    // ### do not write every attribute each time
    QColor pcol = pt->pen().color();
    QColor bcol = pt->brush().color();
    QString s;
    if ( c == PdcDrawText2 || c == PdcDrawText2Formatted ) {
	// QPainter has a reversed understanding of pen/stroke vs.
	// brush/fill for text
	s += QString( "fill:rgb(%1,%2,%3);" )
	     .arg( pcol.red() ).arg( pcol.green() ).arg( pcol.blue() );
	s += QString( "font-size:%1;" ).arg( pt->font().pointSize() );
	s += QString( "stroke-width:0;" );
    } else {
	s += QString( "stroke:rgb(%1,%2,%3);" )
	     .arg( pcol.red() ).arg( pcol.green() ).arg( pcol.blue() );
	s += QString( "stroke-width:%1;" ).arg( pt->pen().width() );
	if ( pt->brush().style() != Qt::NoBrush ) {
	    s += QString( "fill:rgb(%1,%2,%3);" )
		 .arg( bcol.red() ).arg( bcol.green() ).arg( bcol.blue() );
	}
    }
    e->setAttribute( "style", s );
}

void QSvgDevice::applyTransform( QDomElement *e ) const
{
    QWMatrix m = pt->worldMatrix();

    QString s;
    bool rot = ( m.m11() != 1.0 || m.m12() != 0.0 ||
		 m.m21() != 0.0 && m.m22() != 1.0 );
    if ( !rot && ( m.dx() != 0.0 || m.dy() != 0.0 ) )
	s = QString( "translate(%1 %2)" ).arg( m.dx() ).arg( m.dy() );
    else if ( rot ) {
	if ( m.m12() == 0.0 && m.m21() == 0.0 &&
	     m.dx() == 0.0 && m.dy() == 0 )
	    s = QString( "scale(%1 %2)" ).arg( m.m11() ).arg( m.m22() );
	else
	    s = QString( "matrix(%1 %2 %3 %4 %5 %6)" )
		.arg( m.m11() ).arg( m.m12() )
		.arg( m.m21() ).arg( m.m22() )
		.arg( m.dx() ).arg( m.dy() );
    }
    else
	return;

    e->setAttribute( "transform", s );
}

#endif // QT_NO_SVG
