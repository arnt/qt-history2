/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "domtool.h"

#include <qsizepolicy.h>
#include <qcolor.h>
#include <qcursor.h>
#include <qdatetime.h>
#include <qrect.h>
#include <qsize.h>
#include <qfont.h>
#include <qdom.h>
#include <qbytearray.h>

/*!
  \class DomTool domtool.h
  \brief Tools for the dom

  A collection of static functions used by Resource (part of the
  designer) and Uic.

*/

/*!
  Returns the contents of property \a name of object \a e as
  variant or the variant passed as \a defValue if the property does
  not exist.

  \sa hasProperty()
*/
QVariant DomTool::readProperty( const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    if ( n.attribute( "name" ) != name )
		continue;
	    return elementToVariant( n.firstChild().toElement(), defValue, comment );
	}
    }
    return defValue;
}


/*!
  \overload
 */
QVariant DomTool::readProperty( const QDomElement& e, const QString& name, const QVariant& defValue )
{
    QString comment;
    return readProperty( e, name, defValue, comment );
}

/*!
  Returns wheter object \a e defines property \a name or not.

  \sa readProperty()
 */
bool DomTool::hasProperty( const QDomElement& e, const QString& name )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    if ( n.attribute( "name" ) != name )
		continue;
	    return TRUE;
	}
    }
    return FALSE;
}

QStringList DomTool::propertiesOfType( const QDomElement& e, const QString& type )
{
    QStringList result;
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    if ( n2.tagName() == type )
		result += n.attribute( "name" );
	}
    }
    return result;
}


QVariant DomTool::elementToVariant( const QDomElement& e, const QVariant& defValue )
{
    QString dummy;
    return elementToVariant( e, defValue, dummy );
}

/*!
  Interprets element \a e as variant and returns the result of the interpretation.
 */
QVariant DomTool::elementToVariant( const QDomElement& e, const QVariant& defValue, QString &comment )
{
    QVariant v;
    if ( e.tagName() == "rect" ) {
	QDomElement n3 = e.firstChild().toElement();
	int x = 0, y = 0, w = 0, h = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "x" )
		x = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "y" )
		y = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "width" )
		w = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "height" )
		h = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( QRect( x, y, w, h ) );
    } else if ( e.tagName() == "point" ) {
	QDomElement n3 = e.firstChild().toElement();
	int x = 0, y = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "x" )
		x = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "y" )
		y = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( QPoint( x, y ) );
    } else if ( e.tagName() == "size" ) {
	QDomElement n3 = e.firstChild().toElement();
	int w = 0, h = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "width" )
		w = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "height" )
		h = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( QSize( w, h ) );
    } else if ( e.tagName() == "color" ) {
	v = QVariant( readColor( e ) );
    } else if ( e.tagName() == "font" ) {
	QDomElement n3 = e.firstChild().toElement();
	QFont f( defValue.toFont()  );
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "family" )
		f.setFamily( n3.firstChild().toText().data() );
	    else if ( n3.tagName() == "pointsize" )
		f.setPointSize( n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "bold" )
		f.setBold( n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "italic" )
		f.setItalic( n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "underline" )
		f.setUnderline( n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "strikeout" )
		f.setStrikeOut( n3.firstChild().toText().data().toInt() );
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( f );
    } else if ( e.tagName() == "string" ) {
	v = QVariant( e.firstChild().toText().data() );
	QDomElement n = e;
	n = n.nextSibling().toElement();
	if ( n.tagName() == "comment" )
	    comment = n.firstChild().toText().data();
    } else if ( e.tagName() == "cstring" ) {
	v = QVariant( e.firstChild().toText().data().toAscii() );
    } else if ( e.tagName() == "number" ) {
	bool ok = TRUE;
	v = QVariant( e.firstChild().toText().data().toInt( &ok ) );
	if ( !ok )
	    v = QVariant( e.firstChild().toText().data().toDouble() );
    } else if ( e.tagName() == "bool" ) {
	QString t = e.firstChild().toText().data();
	v = QVariant(t == "true" || t == "1");
    } else if ( e.tagName() == "pixmap" ) {
	v = QVariant( e.firstChild().toText().data() );
    } else if ( e.tagName() == "iconset" ) {
	v = QVariant( e.firstChild().toText().data() );
    } else if ( e.tagName() == "image" ) {
	v = QVariant( e.firstChild().toText().data() );
    } else if ( e.tagName() == "enum" ) {
	v = QVariant( e.firstChild().toText().data() );
    } else if ( e.tagName() == "set" ) {
	v = QVariant( e.firstChild().toText().data() );
    } else if ( e.tagName() == "sizepolicy" ) {
	QDomElement n3 = e.firstChild().toElement();
	QSizePolicy sp;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hsizetype" )
		sp.setHorData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "vsizetype" )
		sp.setVerData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "horstretch" )
		sp.setHorStretch( n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "verstretch" )
		sp.setVerStretch( n3.firstChild().toText().data().toInt() );
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( sp );
    } else if ( e.tagName() == "cursor" ) {
	v = QVariant( QCursor( (Qt::CursorShape)e.firstChild().toText().data().toInt() ) );
    } else if ( e.tagName() == "stringlist" ) {
	QStringList lst;
	QDomElement n;
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() )
	    lst << n.firstChild().toText().data();
	v = QVariant( lst );
    } else if ( e.tagName() == "date" ) {
	QDomElement n3 = e.firstChild().toElement();
	int y, m, d;
	y = m = d = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "year" )
		y = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "month" )
		m = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "day" )
		d = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( QDate( y, m, d ) );
    } else if ( e.tagName() == "time" ) {
	QDomElement n3 = e.firstChild().toElement();
	int h, m, s;
	h = m = s = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hour" )
		h = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "minute" )
		m = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "second" )
		s = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( QTime( h, m, s ) );
    } else if ( e.tagName() == "datetime" ) {
	QDomElement n3 = e.firstChild().toElement();
	int h, mi, s, y, mo, d ;
	h = mi = s = y = mo = d = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hour" )
		h = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "minute" )
		mi = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "second" )
		s = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "year" )
		y = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "month" )
		mo = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "day" )
		d = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = QVariant( QDateTime( QDate( y, mo, d ), QTime( h, mi, s ) ) );
    }
    return v;
}


/*!  Returns the color which is returned in the dom element \a e.
 */

QColor DomTool::readColor( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    int r= 0, g = 0, b = 0;
    while ( !n.isNull() ) {
	if ( n.tagName() == "red" )
	    r = n.firstChild().toText().data().toInt();
	else if ( n.tagName() == "green" )
	    g = n.firstChild().toText().data().toInt();
	else if ( n.tagName() == "blue" )
	    b = n.firstChild().toText().data().toInt();
	n = n.nextSibling().toElement();
    }

    return QColor( r, g, b );
}

/*!
  Returns the contents of attribute \a name of object \a e as
  variant or the variant passed as \a defValue if the attribute does
  not exist.

  \sa hasAttribute()
 */
QVariant DomTool::readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue, QString& comment )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "attribute" ) {
	    if ( n.attribute( "name" ) != name )
		continue;
	    return elementToVariant( n.firstChild().toElement(), defValue, comment );
	}
    }
    return defValue;
}

/*!
  \overload
*/
QVariant DomTool::readAttribute( const QDomElement& e, const QString& name, const QVariant& defValue )
{
    QString comment;
    return readAttribute( e, name, defValue, comment );
}

/*!
  Returns wheter object \a e defines attribute \a name or not.

  \sa readAttribute()
 */
bool DomTool::hasAttribute( const QDomElement& e, const QString& name )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "attribute" ) {
	    if ( n.attribute( "name" ) != name )
		continue;
	    return TRUE;
	}
    }
    return FALSE;
}

static bool toBool( const QString& s )
{
    return s == "true" || s.toInt() != 0;
}

/*!
  Convert Qt 2.x format to Qt 3.x format if necessary
*/
void DomTool::fixDocument( QDomDocument& doc )
{
    QDomElement e;
    QDomNode n;
    QDomNodeList nl;
    int i = 0;

    e = doc.firstChild().toElement();
    if ( e.tagName() != "UI" )
	return;

    // rename classes and properties
    double version = ( e.hasAttribute("version") ? e.attribute("version").toDouble() : 0.0 );
    nl = e.childNodes();
    fixAttributes( nl, version );

    // 3.x don't do anything more
    if ( e.hasAttribute("version") && e.attribute("version").toDouble() >= 3.0 )
	return;

    // in versions smaller than 3.0 we need to change more

    e.setAttribute( "version", 3.0 );
    e.setAttribute("stdsetdef", 1 );
    nl = e.elementsByTagName( "property" );
    for ( i = 0; i <  (int) nl.length(); i++ ) {
	e = nl.item(i).toElement();
	QString name;
	QDomElement n2 = e.firstChild().toElement();
	if ( n2.tagName() == "name" ) {
	    name = n2.firstChild().toText().data();
	    if ( name == "resizeable" )
		e.setAttribute( "name", "resizable" );
	    else
		e.setAttribute( "name", name );
	    e.removeChild( n2 );
	}
	bool stdset = toBool( e.attribute( "stdset" ) );
	if ( stdset || name == "toolTip" || name == "whatsThis" ||
	     name == "buddy" ||
	     e.parentNode().toElement().tagName() == "item" ||
	     e.parentNode().toElement().tagName() == "spacer" ||
	     e.parentNode().toElement().tagName() == "column"
	     )
	    e.removeAttribute( "stdset" );
	else
	    e.setAttribute( "stdset", 0 );
    }

    nl = doc.elementsByTagName( "attribute" );
    for ( i = 0; i <  (int) nl.length(); i++ ) {
	e = nl.item(i).toElement();
	QString name;
	QDomElement n2 = e.firstChild().toElement();
	if ( n2.tagName() == "name" ) {
	    name = n2.firstChild().toText().data();
	    e.setAttribute( "name", name );
	    e.removeChild( n2 );
	}
    }

    nl = doc.elementsByTagName( "image" );
    for ( i = 0; i <  (int) nl.length(); i++ ) {
	e = nl.item(i).toElement();
	QString name;
	QDomElement n2 = e.firstChild().toElement();
	if ( n2.tagName() == "name" ) {
	    name = n2.firstChild().toText().data();
	    e.setAttribute( "name", name );
	    e.removeChild( n2 );
	}
    }

    nl = doc.elementsByTagName( "widget" );
    for ( i = 0; i <  (int) nl.length(); i++ ) {
	e = nl.item(i).toElement();
	QString name;
	QDomElement n2 = e.firstChild().toElement();
	if ( n2.tagName() == "class" ) {
	    name = n2.firstChild().toText().data();
	    e.setAttribute( "class", name );
	    e.removeChild( n2 );
	}
    }

}

struct widgetName {
    widgetName( double v, QString b, QString a )
	: version( v ), before( b ), after( a ) {}
    double version;
    QString before;
    QString after;
};

struct propertyName : public widgetName {
    propertyName( double v, QString b, QString a, QString c = QString::null )
	: widgetName( v, b, a ), clss( c ) {}
    QString clss;
};

const int widgs = 1;
widgetName widgetTable[1] = {
    widgetName( 3.3, "before", "after" ),
};

const int props = 1;
propertyName propertyTable[1] = {
    propertyName( 3.0, "resizeable", "resizable" ), // we need to fix a spelling error in 3.0
};

void DomTool::fixAttributes( QDomNodeList &nodes, double version )
{
    QDomNode n;
    QDomNodeList nl;
    for ( int i = 0; i < (int) nodes.count(); ++i ) {
	n = nodes.item( i );
	fixAttribute( n, version );
	nl = n.childNodes();
	fixAttributes( nl, version );
    }
}

void DomTool::fixAttribute( QDomNode &node, double version )
{
    QString tagName =  node.toElement().tagName();
    if ( tagName == "widget" ) {
	QString clss = node.toElement().attribute( "class" );
	for ( int i = 0; i < widgs; ++i )
	    if ( ( version < widgetTable[i].version )
		 && ( clss == widgetTable[i].before ) ) {
		node.toElement().setAttribute( "class", propertyTable[i].after );
		return;
	    }
	return;
    }
    if ( tagName == "property" ) {
	QDomElement e = node.parentNode().toElement();
	QString clss = e.attribute( "class" );
	QString name = node.toElement().attribute( "name", "" );
	for ( int i = 0; i < props; ++i )
	    if ( ( version < propertyTable[i].version )
		 && ( clss == propertyTable[i].clss )
		 && ( propertyTable[i].before == QString::null
		      || name == propertyTable[i].before ) ) {
		node.toElement().setAttribute( "name", propertyTable[i].after );
		return;
	    }
    }
}
