/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "uic.h"
#include "../shared/parser.h"
#include <qsizepolicy.h>
#include <qfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <qstringlist.h>
#include <qstrlist.h>
#include <qdatetime.h>
#include <widgetdatabase.h>
#include <domtool.h>
#include <globaldefs.h>
#include <qregexp.h>
#include <zlib.h>

#undef main

bool Uic::isMainWindow = FALSE;

QString Uic::mkBool( bool b )
{
    return b? "TRUE" : "FALSE";
}

QString Uic::mkBool( const QString& s )
{
    return mkBool( s == "true" || s == "1" );
}

bool Uic::toBool( const QString& s )
{
    return s == "true" || s.toInt() != 0;
}

// fixString is only used in conjunction with tr(). We need to write out the
// string in utf8 and make sure it's converted from utf8 when created.
QString Uic::fixString( const QString &str )
{
    QString s( str );
    s = s.replace( QRegExp( "\\\\" ), "\\\\" );
    s = s.replace( QRegExp( "\"" ), "\\\"" );
    s = s.replace( QRegExp( "\n" ), "\\n" );
    s = s.replace( QRegExp( "\r" ), "\\r" );

    bool onlyAscii = TRUE;
    unsigned int i;
    for ( i = 0; i < s.length(); i++ ) {
	if ( s.at(i).unicode() >= 0x80 ) {
	    onlyAscii = FALSE;
	    break;
	}
    }
    if ( onlyAscii )
	s = "\"" + s + "\"";
    else
	s = "QString::fromUtf8( \"" + s + "\" )";
    return s;
}

QString Uic::mkStdSet( const QString& prop )
{
    return QString( "set" ) + prop[0].upper() + prop.mid(1);
}



/*!
  \class Uic uic.h
  \brief User Interface Compiler

  The class Uic encapsulates the user interface compiler (uic).
 */
Uic::Uic( QTextStream &outStream, QDomDocument doc, bool decl, bool subcl, const QString &trm, const QString& subClass  )
    : out( outStream ), trmacro( trm )
{
    externPixmaps = FALSE;
    indent = "    "; // default indent

    item_used = cg_used = pal_used = 0;

    layouts << "hbox" << "vbox" << "grid";
    tags = layouts;
    tags << "widget";

    pixmapLoaderFunction = getPixmapLoaderFunction( doc.firstChild().toElement() );
    nameOfClass = getFormClassName( doc.firstChild().toElement() );

    stdsetdef = toBool( doc.firstChild().toElement().attribute("stdsetdef") );

    QDomElement firstWidget = doc.firstChild().firstChild().toElement();
    while ( firstWidget.tagName() != "widget" ) {
	if ( firstWidget.tagName() == "pixmapinproject" )
	    externPixmaps = TRUE;
	firstWidget = firstWidget.nextSibling().toElement();
    }

    if ( nameOfClass.isEmpty() )
	nameOfClass = getObjectName( firstWidget );

    if ( subcl ) {
	if ( decl )
	    createSubDecl( firstWidget, subClass );
	else
	    createSubImpl( firstWidget, subClass );
    } else {
	if ( decl )
	    createFormDecl( firstWidget );
	else
	    createFormImpl( firstWidget );
    }

}

/*! Extracts a pixmap loader function from \a e
 */
QString Uic::getPixmapLoaderFunction( const QDomElement& e )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "pixmapfunction" )
	    return n.firstChild().toText().data();
    }
    return QString::null;
}


/*! Extracts the forms class name from \a e
 */
QString Uic::getFormClassName( const QDomElement& e )
{
    QDomElement n;
    QString cn;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "class" ) {
	    QString s = n.firstChild().toText().data();
	    int i;
	    while ( ( i = s.find(' ' )) != -1  )
		s[i] = '_';
	    cn = s;
	}
    }
    return cn;
}

/*! Extracts a class name from \a e
 */
QString Uic::getClassName( const QDomElement& e )
{
    return e.attribute( "class" );
}

/*! Returns TRUE if database framework code is generated, else FALSE.
*/

bool Uic::isFrameworkCodeGenerated( const QDomElement& e )
{
    QDomElement n = getObjectProperty( e, "frameworkCode" );
    if ( n.attribute("name") == "frameworkCode" &&
	 !DomTool::elementToVariant( n.firstChild().toElement(), QVariant( TRUE, 0 ) ).toBool() )
	return FALSE;
    return TRUE;
}

/*! Extracts an object name from \a e. It's stored in the 'name'
 property.
 */
QString Uic::getObjectName( const QDomElement& e )
{
    QDomElement n = getObjectProperty( e, "name" );
    if ( n.firstChild().toElement().tagName() == "cstring" )
	return n.firstChild().toElement().firstChild().toText().data();
    return QString::null;
}

/*! Extracts an layout name from \a e. It's stored in the 'name'
 property of the preceeding sibling (the first child of a QLayoutWidget).
 */
QString Uic::getLayoutName( const QDomElement& e )
{
    QDomElement p = e.parentNode().toElement();
    QString tail = QString::null;

    if (getClassName(p) != "QLayoutWidget")
	tail = "Layout";

    QDomElement n = getObjectProperty( p, "name" );
    if ( n.firstChild().toElement().tagName() == "cstring" )
	return n.firstChild().toElement().firstChild().toText().data() + tail;
    return e.tagName();
}


QString Uic::getDatabaseInfo( const QDomElement& e, const QString& tag )
{
    QDomElement n;
    QDomElement n1;
    int child = 0;
    // database info is a stringlist stored in this order
    if ( tag == "connection" )
	child = 0;
    else if ( tag == "table" )
	child = 1;
    else if ( tag == "field" )
	child = 2;
    else
	return QString::null;
    n = getObjectProperty( e, "database" );
    if ( n.firstChild().toElement().tagName() == "stringlist" ) {
	    // find correct stringlist entry
	    QDomElement n1 = n.firstChild().firstChild().toElement();
	    for ( int i = 0; i < child && !n1.isNull(); ++i )
		n1 = n1.nextSibling().toElement();
	    if ( n1.isNull() )
		return QString::null;
	    return n1.firstChild().toText().data();
    }
    return QString::null;
}


void Uic::registerLayouts( const QDomElement &e )
{
    if (layouts.contains(e.tagName()))
	createObjectDecl(e);

    QDomNodeList nl = e.childNodes();
    for (int i = 0; i < (int) nl.length(); ++i)
	registerLayouts(nl.item(i).toElement());
}


/*!
  Returns include file for class \a className or a null string.
 */
QString Uic::getInclude( const QString& className )
{
    int wid = WidgetDatabase::idFromClassName( className );
    if ( wid != -1 )
	return WidgetDatabase::includeFile( wid );
    return QString::null;
}


void Uic::createActionDecl( const QDomElement& e )
{
    QString objClass = e.tagName() == "action" ? "QAction" : "QActionGroup";
    QString objName = getObjectName( e );
    if ( objName.isEmpty() )
	return;
    out << "    " << objClass << "* " << objName << ";" << endl;
    if ( e.tagName() == "actiongroup" ) {
	for ( QDomElement n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "action" || n.tagName() == "actiongroup" )
		createActionDecl( n );
	}
    }
}

void Uic::createToolbarDecl( const QDomElement &e )
{
    if ( e.tagName() == "toolbar" )
	out << "    " << "QToolBar *" << e.attribute( "name" ) << ";" << endl;
}

void Uic::createMenuBarDecl( const QDomElement &e )
{
    if ( e.tagName() == "item" )
	out << "    " << "QPopupMenu *" << e.attribute( "name" ) << ";" << endl;
}

void Uic::createActionImpl( const QDomElement &n, const QString &parent )
{
    for ( QDomElement ae = n; !ae.isNull(); ae = ae.nextSibling().toElement() ) {
	QString objName = registerObject( getObjectName( ae ) );
	if ( ae.tagName() == "action" )
	    out << indent << objName << " = new QAction( " << parent << ", \"" << objName << "\" );" << endl;
	else if ( ae.tagName() == "actiongroup" )
	    out << indent << objName << " = new QActionGroup( " << parent << ", \"" << objName << "\" );" << endl;
	else
	    continue;
	bool subActionsDone = FALSE;
	for ( QDomElement n2 = ae.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
	    if ( n2.tagName() == "property" ) {
		bool stdset = stdsetdef;
		if ( n2.hasAttribute( "stdset" ) )
		    stdset = toBool( n2.attribute( "stdset" ) );
		QString prop = n2.attribute("name");
		if ( prop == "name" )
		    continue;
		QString value = setObjectProperty( "QAction", objName, prop, n2.firstChild().toElement(), stdset );
		if ( value.isEmpty() )
		    continue;
		if ( stdset )
		    out << indent << objName << "->" << mkStdSet(prop ) << "( " << value << " );" << endl;
		else
		    out << indent << objName << "->setProperty( \"" << prop << "\", " << value << " );" << endl;
	    } else if ( !subActionsDone && ( n2.tagName() == "actiongroup" || n2.tagName() == "action" ) ) {
		createActionImpl( n2, objName );
		subActionsDone = TRUE;
	    }
	}
    }
}

QString get_dock( const QString &d )
{
    if ( d == "0" )
	return "Unmanaged";
    if ( d == "1" )
	return "TornOff";
    if ( d == "2" )
	return "Top";
    if ( d == "3" )
	return "Bottom";
    if ( d == "4" )
	return "Right";
    if ( d == "5" )
	return "Left";
    if ( d == "6" )
	return "Minimized";
    return "";
}

void Uic::createToolbarImpl( const QDomElement &n )
{
    QDomNodeList nl = n.elementsByTagName( "toolbar" );
    for ( int i = 0; i < (int) nl.length(); i++ ) {
	QDomElement ae = nl.item( i ).toElement();
	QString objName = ae.attribute( "name" );
	QString label = ae.attribute( "label" );
	QString dock = get_dock( ae.attribute( "dock" ) );
	out << indent << objName << " = new QToolBar( \"" << label << "\", this, " << dock << " ); " << endl;
	for ( QDomElement n2 = ae.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
	    if ( n2.tagName() == "action" )
		out << indent << n2.attribute( "name" ) << "->addTo( " << objName << " );" << endl;
	    else if ( n2.tagName() == "separator" )
		out << indent << objName << "->addSeparator();" << endl;
	    else if ( n2.tagName() == "widget" )
		createObjectImpl( n2, "QToolBar", objName );
	}
    }
}

void Uic::createMenuBarImpl( const QDomElement &n )
{
    QDomNodeList nl = n.elementsByTagName( "item" );
    for ( int i = 0; i < (int) nl.length(); i++ ) {
	QDomElement ae = nl.item( i ).toElement();
	QString objName = ae.attribute( "name" );
	out << indent << objName << " = new QPopupMenu( this ); " << endl;
	for ( QDomElement n2 = ae.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
	    if ( n2.tagName() == "action" )
		out << indent << n2.attribute( "name" ) << "->addTo( " << objName << " );" << endl;
	    else if ( n2.tagName() == "separator" )
		out << indent << objName << "->insertSeparator();" << endl;
	}
	out << indent << "menuBar()->insertItem( " << trmacro << "(" << fixString( ae.attribute( "text" ) ) << "), " << objName << " );" << endl;
	out << endl;
    }
}

/*!
  Creates implementation of an listbox item tag.
*/

QString Uic::createListBoxItemImpl( const QDomElement &e, const QString &parent )
{
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString pix;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = v.toString();
	    else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append(  QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( pix.isEmpty() )
	return parent + "->insertItem( " + trmacro + "( " + fixString( txt ) + " ) );";
    else
	return parent + "->insertItem( " + pix + ", " + trmacro + "( " + fixString( txt ) + " ) );";

    return QString::null;
}

/*!
  Creates implementation of an iconview item tag.
*/

QString Uic::createIconViewItemImpl( const QDomElement &e, const QString &parent )
{
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString pix;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = v.toString();
	    else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( pix.isEmpty() )
	return "(void) new QIconViewItem( " + parent + ", " + trmacro + "( " + fixString( txt ) + " ) );";
    return "(void) new QIconViewItem( " + parent + ", " + trmacro + "( " + fixString( txt ) + " ), " + pix + " );";

}

/*!
  Creates implementation of an listview item tag.
*/

QString Uic::createListViewItemImpl( const QDomElement &e, const QString &parent,
				     const QString &parentItem )
{
    QString s;

    QDomElement n = e.firstChild().toElement();

    bool hasChildren = e.elementsByTagName( "item" ).count() > 0;
    QString item;

    if ( hasChildren ) {
	item = registerObject( "item" );
	s = indent + "QListViewItem * " + item + " = ";
    } else {
	item = "item";
	if ( item_used )
	    s = indent + item + " = ";
	else
	    s = indent + "QListViewItem * " + item + " = ";
	item_used = TRUE;
    }

    if ( !parentItem.isEmpty() )
	s += "new QListViewItem( " + parentItem + ", " + lastItem + " );\n";
    else
	s += "new QListViewItem( " + parent + ", " + lastItem + " );\n";

    QStringList textes;
    QStringList pixmaps;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		textes << v.toString();
	    else if ( attrib == "pixmap" ) {
		QString pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
		pixmaps << pix;
	    }
	} else if ( n.tagName() == "item" ) {
	    s += indent + item + "->setOpen( TRUE );\n";
	    s += createListViewItemImpl( n, parent, item );
	}
	n = n.nextSibling().toElement();
    }

    for ( int i = 0; i < (int)textes.count(); ++i ) {
	if ( !textes[ i ].isEmpty() )
	    s += indent + item + "->setText( " + QString::number( i ) + ", " + trmacro + "( " + fixString( textes[ i ] ) + " ) );\n";
	if ( !pixmaps[ i ].isEmpty() )
	    s += indent + item + "->setPixmap( " + QString::number( i ) + ", " + pixmaps[ i ] + " );\n";
    }

    lastItem = item;
    return s;
}

/*!
  Creates implementation of an listview column tag.
*/

QString Uic::createListViewColumnImpl( const QDomElement &e, const QString &parent )
{
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString pix;
    bool clickable = FALSE, resizeable = FALSE;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = v.toString();
	    else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    } else if ( attrib == "clickable" )
		clickable = v.toBool();
	    else if ( attrib == "resizeable" )
		resizeable = v.toBool();
	}
	n = n.nextSibling().toElement();
    }

    QString s;
    s = indent + parent + "->addColumn( " + trmacro + "( " + fixString( txt ) + " ) );\n";
    if ( !pix.isEmpty() )
	s += indent + parent + "->header()->setLabel( " + parent + "->header()->count() - 1, " + pix + ", " + trmacro + "( " + fixString( txt ) + " ) );\n";
    if ( !clickable )
	s += indent + parent + "->header()->setClickEnabled( FALSE, " + parent + "->header()->count() - 1 );\n";
    if ( !resizeable )
	s += indent + parent + "->header()->setResizeEnabled( FALSE, " + parent + "->header()->count() - 1 );\n";

    return s;
}

QString Uic::createTableRowColumnImpl( const QDomElement &e, const QString &parent )
{
    QString objClass = getClassName( e.parentNode().toElement() );
    QDomElement n = e.firstChild().toElement();
    QString txt;
    QString pix;
    QString field;
    bool isRow = e.tagName() == "row";
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute("name");
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = v.toString();
	    else if ( attrib == "pixmap" ) {
		pix = v.toString();
		if ( !pix.isEmpty() && !pixmapLoaderFunction.isEmpty() ) {
		    pix.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		    pix.append( QString( externPixmaps ? "\"" : "" ) + " )" );
		}
	    } else if ( attrib == "field" )
		field = v.toString();
	}
	n = n.nextSibling().toElement();
    }

    // ### This generated code sucks! We have to set the number of
    // rows/cols before and then only do setLabel/()
    // ### careful, though, since QDataTable has an API which makes this code pretty good

    QString s;
    if ( isRow ) {
	s = indent + parent + "->setNumRows( " + parent + "->numRows() + 1 );";
	if ( pix.isEmpty() )
	    s += indent + parent + "->verticalHeader()->setLabel( " + parent + "->numRows() - 1, "
		 + trmacro + "( " + fixString( txt ) + " ) );\n";
	else
	    s += indent + parent + "->verticalHeader()->setLabel( " + parent + "->numRows() - 1, "
		 + pix + ", " + trmacro + "( " + fixString( txt ) + " ) );\n";
    } else {
	if ( objClass == "QTable" ) {
	    s = indent + parent + "->setNumCols( " + parent + "->numCols() + 1 );";
	    if ( pix.isEmpty() )
		s += indent + parent + "->horizontalHeader()->setLabel( " + parent + "->numCols() - 1, "
		     + trmacro + "( " + fixString( txt ) + " ) );\n";
	    else
		s += indent + parent + "->horizontalHeader()->setLabel( " + parent + "->numCols() - 1, "
		     + pix + ", " + trmacro + "( " + fixString( txt ) + " ) );\n";
	} else if ( objClass == "QDataTable" ) {
	    if ( !txt.isEmpty() && !field.isEmpty() ) {
		if ( pix.isEmpty() )
		    out << indent << parent << "->addColumn( " << fixString( field ) << ", " << fixString( txt ) << " );" << endl;
		else
		    out << indent << parent << "->addColumn( " << fixString( field ) << ", " << fixString( txt ) << ", " << pix << " );" << endl;
	    }
	}
    }
    return s;
}

/*!
  Creates the implementation of a layout tag. Called from createObjectImpl().
 */
QString Uic::createLayoutImpl( const QDomElement &e, const QString& parentClass, const QString& parent, const QString& layout )
{
    QDomElement n;
    QString objClass, objName;
    objClass = e.tagName();

    QString qlayout = "QVBoxLayout";
    if ( objClass == "hbox" )
	qlayout = "QHBoxLayout";
    else if ( objClass == "grid" )
	qlayout = "QGridLayout";

    bool isGrid = e.tagName() == "grid" ;
    objName = registerObject( getLayoutName( e ) );
    layoutObjects += objName;
    int margin = DomTool::readProperty( e, "margin", BOXLAYOUT_DEFAULT_MARGIN ).toInt();
    int spacing = DomTool::readProperty( e, "spacing", BOXLAYOUT_DEFAULT_SPACING ).toInt();

    if ( (parentClass == "QGroupBox" || parentClass == "QButtonGroup") && layout.isEmpty() ) {
	// special case for group box
	out << indent << parent << "->setColumnLayout(0, Qt::Vertical );" << endl;
	out << indent << parent << "->layout()->setSpacing( 0 );" << endl;
	out << indent << parent << "->layout()->setMargin( 0 );" << endl;
	out << indent << objName << " = new " << qlayout << "( " << parent << "->layout() );" << endl;
	out << indent << objName << "->setAlignment( Qt::AlignTop );" << endl;
    } else {
	if ( layout.isEmpty() )
	    out << indent << objName << " = new " << qlayout << "( " << parent << " ); " << endl;
	else
	    out << indent << objName << " = new " << qlayout << "; " << endl;
    }

    out << indent << objName << "->setSpacing( " << spacing << " );" << endl;
    out << indent << objName << "->setMargin( " << margin << " );" << endl;

    if ( !isGrid ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName() == "spacer" ) {
		QString child = createSpacerImpl( n, parentClass, parent, objName );
		out << indent << objName << "->addItem( " << child << " );" << endl;
	    } else if ( tags.contains( n.tagName() ) ) {
		QString child = createObjectImpl( n, parentClass, parent, objName );
		if ( isLayout( child ) )
		    out << indent << objName << "->addLayout( " << child << " );" << endl;
		else
		    out << indent << objName << "->addWidget( " << child << " );" << endl;
	    }
	}
    } else {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    QDomElement ae = n;
	    int row = ae.attribute( "row" ).toInt();
	    int col = ae.attribute( "column" ).toInt();
	    int rowspan = ae.attribute( "rowspan" ).toInt();
	    int colspan = ae.attribute( "colspan" ).toInt();
	    if ( rowspan < 1 )
		rowspan = 1;
	    if ( colspan < 1 )
		colspan = 1;
	    if ( n.tagName() == "spacer" ) {
		QString child = createSpacerImpl( n, parentClass, parent, objName );
		if ( rowspan * colspan != 1 )
		    out << indent << objName << "->addMultiCell( " << child << ", "
			<< row << ", " << row + rowspan - 1 << ", " << col << ", " << col  + colspan - 1 << " );" << endl;
		else
		    out << indent << objName << "->addItem( " << child << ", "
			<< row << ", " << col << " );" << endl;
	    } else if ( tags.contains( n.tagName() ) ) {
		QString child = createObjectImpl( n, parentClass, parent, objName );
		out << endl;
		QString o = "Widget";
		if ( isLayout( child ) )
		    o = "Layout";
		if ( rowspan * colspan != 1 )
		    out << indent << objName << "->addMultiCell" << o << "( " << child << ", "
			<< row << ", " << row + rowspan - 1 << ", " << col << ", " << col  + colspan - 1 << " );" << endl;
		else
		    out << indent << objName << "->add" << o << "( " << child << ", "
			<< row << ", " << col << " );" << endl;
	    }
	}
    }

    return objName;
}



QString Uic::createSpacerImpl( const QDomElement &e, const QString& /*parentClass*/, const QString& /*parent*/, const QString& /*layout*/)
{
    QDomElement n;
    QString objClass, objName;
    objClass = e.tagName();
    objName = registerObject( "spacer" );

    QSize size = DomTool::readProperty( e, "sizeHint", QSize( 0, 0 ) ).toSize();
    QString sizeType = DomTool::readProperty( e, "sizeType", "Expanding" ).toString();
    bool isVspacer = DomTool::readProperty( e, "orientation", "Horizontal" ) == "Vertical";

    if ( sizeType != "Expanding" && sizeType != "MinimumExpanding" &&
	 DomTool::hasProperty( e, "geometry" ) ) { // compatibility Qt 2.2
	QRect geom = DomTool::readProperty( e, "geometry", QRect(0,0,0,0) ).toRect();
	size = geom.size();
    }

    if ( isVspacer )
	out << "    QSpacerItem* " << objName << " = new QSpacerItem( "
	    << size.width() << ", " << size.height()
	    << ", QSizePolicy::Minimum, QSizePolicy::" << sizeType << " );" << endl;
    else
	out << "    QSpacerItem* " << objName << " = new QSpacerItem( "
	    << size.width() << ", " << size.height()
	    << ", QSizePolicy::" << sizeType << ", QSizePolicy::Minimum );" << endl;

    return objName;
}

static const char* const ColorRole[] = {
    "Foreground", "Button", "Light", "Midlight", "Dark", "Mid",
    "Text", "BrightText", "ButtonText", "Base", "Background", "Shadow",
    "Highlight", "HighlightedText", "Link", "LinkVisited", 0
};


/*!
  Creates a colorgroup with name \a name from the color group \a cg
 */
void Uic::createColorGroupImpl( const QString& name, const QDomElement& e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QString color;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    QColor col = DomTool::readColor( n );
	    color = "QColor( %1, %2, %3)";
	    color = color.arg( col.red() ).arg( col.green() ).arg( col.blue() );
	    if ( col == white )
		color = "white";
	    else if ( col == black )
	    color = "black";
	    if ( n.nextSibling().toElement().tagName() != "pixmap" ) {
		out << indent << name << ".setColor( QColorGroup::" << ColorRole[r] << ", " << color << " );" << endl;
	    }
	} else if ( n.tagName() == "pixmap" ) {
	    QString pixmap = n.firstChild().toText().data();
	    if ( !pixmapLoaderFunction.isEmpty() ) {
		pixmap.prepend( pixmapLoaderFunction + "( " + QString( externPixmaps ? "\"" : "" ) );
		pixmap.append( QString( externPixmaps ? "\"" : "" ) + " )" );
	    }
	    out << indent << name << ".setBrush( QColorGroup::"
		<< ColorRole[r] << ", QBrush( " << color << ", " << pixmap << " ) );" << endl;
	}
	n = n.nextSibling().toElement();
    }
}

/*!
  Auxiliary function to load a color group. The colorgroup must not
  contain pixmaps.
 */
QColorGroup Uic::loadColorGroup( const QDomElement &e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QColor col;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    cg.setColor( (QColorGroup::ColorRole)r, (col = DomTool::readColor( n ) ) );
	}
	n = n.nextSibling().toElement();
    }
    return cg;
}

/*!  Returns TRUE if the widget properties specify that it belongs to
  the database \a connection and \a table.
*/

bool Uic::isWidgetInTable( const QDomElement& e, const QString& connection, const QString& table )
{
    QString conn = getDatabaseInfo( e, "connection" );
    QString tab = getDatabaseInfo( e, "table" );
    if ( conn == connection && tab == table )
	return TRUE;
    return FALSE;
}

/*!
  Registers all database connections, cursors and forms.
*/

void Uic::registerDatabases( const QDomElement& e )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    nl = e.parentNode().toElement().elementsByTagName( "widget" );
    for ( i = 0; i < (int) nl.length(); ++i ) {
	n = nl.item(i).toElement();
	QString conn = getDatabaseInfo( n, "connection"  );
	QString tab = getDatabaseInfo( n, "table"  );
	QString fld = getDatabaseInfo( n, "field"  );
	if ( !conn.isNull() ) {
	    dbConnections += conn;
	    if ( !tab.isNull() ) {
		dbCursors[conn] += tab;
		if ( !fld.isNull() )
		    dbForms[conn] += tab;
	    }
	}
    }
}

/*!
  Registers an object with name \a name.

  The returned name is a valid variable identifier, as similar to \a
  name as possible and guaranteed to be unique within the form.

  \sa registeredName(), isObjectRegistered()
 */
QString Uic::registerObject( const QString& name )
{
    if ( objectNames.isEmpty() ) {
	// some temporary variables we need
	objectNames += "img";
	objectNames += "item";
	objectNames += "cg";
	objectNames += "pal";
    }

    QString result = name;
    int i;
    while ( ( i = result.find(' ' )) != -1  ) {
	result[i] = '_';
    }

    if ( objectNames.contains( result ) ) {
	int i = 2;
	while ( objectNames.contains( result + "_" + QString::number(i) ) )
		i++;
	result += "_";
	result += QString::number(i);
    }
    objectNames += result;
    objectMapper.insert( name, result );
    return result;
}

/*!
  Returns the registered name for the original name \a name
  or \a name if \a name  wasn't registered.

  \sa registerObject(), isObjectRegistered()
 */
QString Uic::registeredName( const QString& name )
{
    if ( !objectMapper.contains( name ) )
	return name;
    return objectMapper[name];
}

/*!
  Returns whether the object \a name was registered yet or not.
 */
bool Uic::isObjectRegistered( const QString& name )
{
    return objectMapper.contains( name );
}


/*!
  Unifies the entries in stringlist \a list. Should really be a QStringList feature.
 */
QStringList Uic::unique( const QStringList& list )
{
    QStringList result;
    if (list.isEmpty() )
	return result;
    QStringList l = list;
    l.sort();
    result += l.first();
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it ) {
	if ( *it != result.last() )
	    result += *it;
    }
    return result;
}



/*!
  Creates an instance of class \a objClass, with parent \a parent and name \a objName
 */
QString Uic::createObjectInstance( const QString& objClass, const QString& parent, const QString& objName )
{

    if ( objClass.mid( 1 ) == "ComboBox" ) {
	return objClass + "( FALSE, " + parent + ", \"" + objName + "\" )";
    }
    return objClass + "( " + parent + ", \"" + objName + "\" )";
}

bool Uic::isLayout( const QString& name ) const
{
    return layoutObjects.contains( name );
}


/*!
  Creates a declaration ( headerfile ) for a subclass \a subClass
  of the form given in \a e

  \sa createSubImpl()
 */
void Uic::createSubDecl( const QDomElement &e, const QString& subClass )
{
    QDomElement n;
    QStringList::Iterator it;

    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;

    out << "class " << subClass << " : public " << nameOfClass << endl;
    out << "{ " << endl;

/* tmake ignore Q_OBJECT */
    out << "    Q_OBJECT" << endl;
    out << endl;
    out << "public:" << endl;

    // constructor
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "    " << subClass << "( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );" << endl;
    } else { // standard QWidget
	out << "    " << subClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );" << endl;
    }

    // destructor
    out << "    ~" << subClass << "();" << endl;
    out << endl;

    // find additional slots
    QStringList publicSlots, protectedSlots;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "connections" ) {
	    for ( QDomElement n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		if ( n2.tagName() == "slot" ) {
		    QString access = n2.attribute( "access" );
		    if ( access == "protected" )
			protectedSlots += n2.firstChild().toText().data();
		    else
			publicSlots += n2.firstChild().toText().data();
		}
	    }
	}
    }

    // create public additional slots as pure-virtual functions
    if ( !publicSlots.isEmpty() ) {
	out << "public slots:" << endl;
	for ( it = publicSlots.begin(); it != publicSlots.end(); ++it )
	    out << "    void " << (*it) << ";" << endl;
	out << endl;
    }

    // create protected additional slots as pure-virtual functions
    if ( !protectedSlots.isEmpty() ) {
	out << "protected slots:" << endl;
	for ( it = protectedSlots.begin(); it != protectedSlots.end(); ++it )
	    out << "    void " << (*it) << ";" << endl;
	out << endl;
    }

    out << "};" << endl;
}

/*!
  Creates an implementation for a subclass \a subClass of the form
  given in \a e

  \sa createSubDecl()
 */
void Uic::createSubImpl( const QDomElement &e, const QString& subClass )
{
    QDomElement n;
    QStringList::Iterator it;

    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;

    // constructor
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "/* " << endl;
	out << " *  Constructs a " << subClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f' " << endl;
	out << " *" << endl;
	out << " *  The " << objClass.mid(1).lower() << " will by default be modeless, unless you set 'modal' to" << endl;
	out << " *  TRUE to construct a modal " << objClass.mid(1).lower() << "." << endl;
	out << " */" << endl;
	out << subClass << "::" << subClass << "( QWidget* parent,  const char* name, bool modal, WFlags fl )" << endl;
	out << "    : " << nameOfClass << "( parent, name, modal, fl )" << endl;
    } else { // standard QWidget
	out << "/* " << endl;
	out << " *  Constructs a " << subClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f' " << endl;
	out << " */" << endl;
	out << subClass << "::" << subClass << "( QWidget* parent,  const char* name, WFlags fl )" << endl;
	out << "    : " << nameOfClass << "( parent, name, fl )" << endl;
    }
    out << "{" << endl;
    out << "}" << endl;
    out << endl;

    // destructor
    out << "/*  " << endl;
    out << " *  Destroys the object and frees any allocated resources" << endl;
    out << " */" << endl;
    out << subClass << "::~" << subClass << "()" << endl;
    out << "{" << endl;
    out << "    // no need to delete child widgets, Qt does it all for us" << endl;
    out << "}" << endl;
    out << endl;

    // find additional slots
    QStringList publicSlots, protectedSlots;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "connections" ) {
	    for ( QDomElement n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		if ( n2.tagName() == "slot" ) {
		    QString access = n2.attribute( "access" );
		    if ( access == "protected" )
			protectedSlots += n2.firstChild().toText().data();
		    else
			publicSlots += n2.firstChild().toText().data();
		}
	    }
	}
    }

    // create stubs for public additional slots
    if ( !publicSlots.isEmpty() ) {
	for ( it = publicSlots.begin(); it != publicSlots.end(); ++it ) {
	    out << "/* " << endl;
	    out << " * public slot" << endl;
	    out << " */" << endl;
	    out << "void " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl;
	}
	out << endl;
    }

    // create stubs for protected additional slots
    if ( !protectedSlots.isEmpty() ) {
	for ( it = protectedSlots.begin(); it != protectedSlots.end(); ++it ) {
	    out << "/* " << endl;
	    out << " * protected slot" << endl;
	    out << " */" << endl;
	    out << "void " << subClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << subClass << "::" << (*it) << " not yet implemented!\" ); " << endl;
	    out << "}" << endl;
	}
	out << endl;
    }

}


int main( int argc, char * argv[] )
{
    bool impl = FALSE;
    bool subcl = FALSE;
    const char *error = 0;
    const char* fileName = 0;
    const char* className = 0;
    const char* headerFile = 0;
    const char* outputFile = 0;
    const char* trmacro = 0;
    bool fix = FALSE;
    for ( int n = 1; n < argc && error == 0; n++ ) {
	QCString arg = argv[n];
	if ( arg[0] == '-' ) {			// option
	    QCString opt = &arg[1];
	    if ( opt[0] == 'o' ) {		// output redirection
		if ( opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing output-file name";
			break;
		    }
		    outputFile = argv[++n];
		} else
		    outputFile = &opt[1];
	    } else if ( opt[0] == 'i' || opt == "impl" ) {
		impl = TRUE;
		if ( opt == "impl" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing name of header file.";
			break;
		    }
		    headerFile = argv[++n];
		} else
		    headerFile = &opt[1];
	    } else if ( opt == "subdecl" ) {
		subcl = TRUE;
		if ( !(n < argc-2) ) {
		    error = "Missing arguments.";
		    break;
		}
		className = argv[++n];
		headerFile = argv[++n];
	    } else if ( opt == "subimpl" ) {
		subcl = TRUE;
		impl = TRUE;
		if ( !(n < argc-2) ) {
		    error = "Missing arguments.";
		    break;
		}
		className = argv[++n];
		headerFile = argv[++n];
	    } else if ( opt == "tr" ) {
		if ( opt == "tr" || opt[1] == '\0' ) {
		    if ( !(n < argc-1) ) {
			error = "Missing tr macro.";
			break;
		    }
		    trmacro = argv[++n];
		} else {
		    trmacro = &opt[1];
		}
	    } else if ( opt == "fix" ) {
		fix = TRUE;
	    }
	} else {
	    if ( fileName)		// can handle only one file
		error	 = "Too many input files specified";
	    else
		fileName = argv[n];
	}
    }

    if ( argc < 2 || error || !fileName ) {
	fprintf( stderr, "Qt user interface compiler\n" );
	if ( error )
	    fprintf( stderr, "uic: %s\n", error );

	fprintf( stderr, "Usage: %s  [options] [mode] <uifile>\n"
		 "\nGenerate declaration:\n"
		 "   %s  [options]  <uifile>\n"
		 "Generate implementation:\n"
		 "   %s  [options] -impl <headerfile> <uifile>\n"
		 "\t<headerfile>	name of the declaration file\n"
		 "Generate subclass declaration:\n"
		 "   %s  [options] -subdecl <classname> <headerfile> <uifile>\n"
		 "\t<classname>	name of the subclass to generate\n"
		 "\t<headerfile>	declaration file of the baseclass\n"
		 "Generate subclass implementation:\n"
		 "   %s  [options] -subimpl <classname> <headerfile> <uifile>\n"
		 "\t<classname>	name of the subclass to generate\n"
		 "\t<headerfile>	declaration file of the subclass\n"
		 "Options:\n"
		 "\t-o file	Write output to file rather than stdout\n"
		 "\t-tr func	Use func(...) rather than tr(...) for i18n\n"
		 , argv[0], argv[0], argv[0], argv[0], argv[0]);
	exit( 1 );
    }

    QFile file( fileName );
    if ( !file.open( IO_ReadOnly ) )
	qFatal( "uic: Could not open file '%s' ", fileName );

    QFile fileOut;
    if (outputFile ) {
	fileOut.setName( outputFile );
	if (!fileOut.open( IO_WriteOnly ) )
	    qFatal( "uic: Could not open output file '%s'", outputFile );
    } else {
	fileOut.open( IO_WriteOnly, stdout );
    }
    QTextStream out( &fileOut );
    out.setEncoding( QTextStream::UnicodeUTF8 );

    QDomDocument doc;
    if ( !doc.setContent( &file ) )
	qFatal( "uic: Failed to parse %s\n", fileName );

    DomTool::fixDocument( doc );

    if ( fix ) {
	out << doc.toString();
	return 0;
    }

    if ( !subcl ) {
	out << "/****************************************************************************" << endl;
	out << "** Form "<< (impl? "implementation" : "interface") << " generated from reading ui file '" << fileName << "'" << endl;
	out << "**" << endl;
	out << "** Created: " << QDateTime::currentDateTime().toString() << endl;
	out << "**      by:  The User Interface Compiler (uic)" << endl;
	out << "**" << endl;
	out << "** WARNING! All changes made in this file will be lost!" << endl;
	out << "****************************************************************************/" << endl;
    }

    QString protector;
    if ( subcl && className && !impl )
	protector = QString::fromLocal8Bit( className ).upper() + "_H";

    if ( !protector.isEmpty() ) {
	out << "#ifndef " << protector << endl;
	out << "#define " << protector << endl;
    }

    if ( headerFile ) {
	out << "#include \"" << headerFile << "\"" << endl << endl;
    }

    Uic( out, doc, !impl, subcl, trmacro ? trmacro : "tr", className );

    if ( !protector.isEmpty() ) {
	out << endl;
	out << "#endif // " << protector << endl;
    }
    return 0;
}
