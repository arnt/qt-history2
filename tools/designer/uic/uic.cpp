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
#include <qapplication.h>
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

static bool isMainWindow = FALSE;

static QString mkBool( bool b )
{
    return b? "TRUE" : "FALSE";
}

static QString mkBool( const QString& s )
{
    return mkBool( s == "true" || s == "1" );
}

static bool toBool( const QString& s )
{
    return s == "true" || s.toInt() != 0;
}

// fixString is only used in conjunction with tr(). We need to write out the
// string in utf8 and make sure it's converted from utf8 when created.
static QString fixString( const QString &str )
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

static QString mkStdSet( const QString& prop )
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

    indent = "    "; // default indent

    item_used = cg_used = pal_used = 0;

    layouts << "hbox" << "vbox" << "grid";
    tags = layouts;
    tags << "widget";

    pixmapLoaderFunction = getPixmapLoaderFunction( doc.firstChild().toElement() );
    nameOfClass = getFormClassName( doc.firstChild().toElement() );

    stdsetdef = toBool( doc.firstChild().toElement().attribute("stdsetdef") );

    QDomElement firstWidget = doc.firstChild().firstChild().toElement();
    while ( firstWidget.tagName() != "widget" )
	firstWidget = firstWidget.nextSibling().toElement();

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

/*! Extracts an object name from \a e. It's stored in the 'name'
 property.
 */
QString Uic::getObjectName( const QDomElement& e )
{
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property"  && n.toElement().attribute("name") == "name"
	     && n.firstChild().toElement().tagName() == "cstring" )
	    return n.firstChild().toElement().firstChild().toText().data();
    }
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

    QDomElement n;
    for ( n = p.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property"  && n.toElement().attribute("name") == "name"
	     && n.firstChild().toElement().tagName() == "cstring" )
	    return n.firstChild().toElement().firstChild().toText().data() + tail;
    }
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
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property"
	     && n.toElement().attribute("name") == "database"
	     && n.firstChild().toElement().tagName() == "stringlist" ) {
	    // find correct stringlist entry
	    QDomElement n1 = n.firstChild().firstChild().toElement();
	    for ( int i = 0; i < child && !n1.isNull(); ++i )
		n1 = n1.nextSibling().toElement();
	    if ( n1.isNull() ) {
		return QString::null;
	    }
	    return n1.firstChild().toText().data();
	}
    }
    return QString::null;
}

QByteArray unzipXPM( QString data, ulong& length )
{
    char *ba = new char[ data.length() / 2 ];
    for ( int i = 0; i < (int)data.length() / 2; ++i ) {
	char h = data[ 2 * i ].latin1();
	char l = data[ 2 * i  + 1 ].latin1();
	uchar r = 0;
	if ( h <= '9' )
	    r += h - '0';
	else
	    r += h - 'a' + 10;
	r = r << 4;
	if ( l <= '9' )
	    r += l - '0';
	else
	    r += l - 'a' + 10;
	ba[ i ] = r;
    }
    if ( length <  data.length() * 5 )
	length = data.length() * 5;
    QByteArray baunzip( length );
    ::uncompress( (uchar*) baunzip.data(), &length, (uchar*) ba, data.length()/2 );
    return baunzip;
}

/*!
  Creates a declaration ( headerfile ) for the form given in \a e

  \sa createFormImpl(), createObjectDecl()
 */
void Uic::createFormDecl( const QDomElement &e )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;
    QString objName = getObjectName( e );

    QStringList typeDefs;

    QMap<QString, CustomInclude> customWidgetIncludes;

    // database support
    bool dbAware = FALSE;
    if ( objClass == "QSqlDialog" || objClass == "QSqlWidget" ) {
	dbAware = TRUE;
    }

    // at first the images, we need to ensure the names are unique
    QStringList forwardDecl;
    QStringList forwardDecl2;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "images" ) {
	    nl = n.elementsByTagName( "image" );
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		registerObject( nl.item(i).firstChild().firstChild().toText().data() );
	    }
	} else if ( n.tagName() == "customwidgets" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "customwidget" ) {
		    QDomElement n3 = n2.firstChild().toElement();
		    QString cl;
		    while ( !n3.isNull() ) {
			if ( n3.tagName() == "class" ) {
			    forwardDecl << n3.firstChild().toText().data();
			    cl = n3.firstChild().toText().data();
			} else if ( n3.tagName() == "header" ) {
			    CustomInclude ci;
			    ci.header = n3.firstChild().toText().data();
			    ci.location = n3.attribute( "location", "global" );
			    customWidgetIncludes.insert( cl, ci );
			}
			n3 = n3.nextSibling().toElement();
		    }
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
    }

    // register the object and unify its name
    objName = registerObject( objName );
    QString protector = objName.upper() + "_H";
    out << "#ifndef " << protector << endl;
    out << "#define " << protector << endl;
    out << endl;
    out << "#include <qvariant.h>" << endl; // for broken HPUX compilers
    if ( dbAware )
	out << "#include <qvector.h>" << endl;

    QStringList globalIncludes, localIncludes;
    int wid = WidgetDatabase::idFromClassName( objClass );
    if ( wid != -1 ) {
	globalIncludes += WidgetDatabase::includeFile( wid );
    } else {
	QMap<QString, CustomInclude>::Iterator it = customWidgetIncludes.find( objClass );
	if ( it != customWidgetIncludes.end() ) {
	    if ( ( *it ).location == "global" )
		globalIncludes += (*it).header;
	    else
		localIncludes += (*it).header;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "include" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in declaration" &&
	     n2.attribute( "location" ) != "local" )
	    globalIncludes += s;
    }
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in declaration" &&
	     n2.attribute( "location" ) == "local" &&!globalIncludes.contains( s ) )
	    localIncludes += s;
    }

    QStringList::Iterator it;

    globalIncludes = unique( globalIncludes );
    for ( it = globalIncludes.begin(); it != globalIncludes.end(); ++it ) {
	if ( !(*it).isEmpty() )
	    out << "#include <" << *it << ">" << endl;
    }
    localIncludes = unique( localIncludes );
    for ( it = localIncludes.begin(); it != localIncludes.end(); ++it ) {
	if ( !(*it).isEmpty() )
	    out << "#include \"" << *it << "\"" << endl;
    }

    // forward declarations for child widgets and layouts
    out << "class QVBoxLayout; " << endl;
    out << "class QHBoxLayout; " << endl;
    out << "class QGridLayout; " << endl;
    if ( objClass == "QMainWindow" ) {
	out << "class QAction;" << endl;
	out << "class QActionGroup;" << endl;
	out << "class QToolBar;" << endl;
	out << "class QPopupMenu;" << endl;
    }


    bool dbForm = FALSE;
    if ( dbAware ) {
	registerDatabases( e );
	if ( dbConnections.count() )
	    forwardDecl += "QSqlDatabase";
	if ( dbCursors.count() )
	    forwardDecl += "QSqlCursor";
	if ( dbForms[ "(default)" ].count() ) {
	    dbForm = TRUE;
	    forwardDecl += "QSqlForm";
	}
    }

    for ( it = tags.begin(); it != tags.end(); ++it ) {
	nl = e.elementsByTagName( *it );
	for ( i = 0; i < (int) nl.length(); i++ ) {
	    QString s = getClassName( nl.item(i).toElement() );
	    if ( s == "QLayoutWidget" )
		continue; // hide qlayoutwidgets
	    if ( s == "Line" )
		s = "QFrame";
	    forwardDecl += s;
	    if ( s.mid( 1 ) == "ListBox" || s.mid( 1 ) == "ListView" || s.mid( 1 ) == "IconView" )
		forwardDecl += "Q" + s.mid( 1 ) + "Item";
	}
    }

    // some typedefs, maybe
    typeDefs = unique( typeDefs );
    for ( it = typeDefs.begin(); it != typeDefs.end(); ++it ) {
	if ( !(*it).isEmpty() )
	    out << "typedef " << *it << ";" << endl;
    }

    nl = e.parentNode().toElement().elementsByTagName( "forward" );
    for ( i = 0; i < (int) nl.length(); i++ )
	forwardDecl2 << nl.item(i).toElement().firstChild().toText().data();

    nl = e.parentNode().toElement().elementsByTagName( "include" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in declaration" &&
	     n2.attribute( "location" ) != "local" )
	    globalIncludes += s;
    }
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in declaration" &&
	     n2.attribute( "location" ) == "local" &&!globalIncludes.contains( s ) )
	    localIncludes += s;
    }

    forwardDecl = unique( forwardDecl );
    for ( it = forwardDecl.begin(); it != forwardDecl.end(); ++it ) {
	if ( !(*it).isEmpty() && (*it) != objClass )
	    out << "class " << *it << ";" << endl;
    }

    for ( it = forwardDecl2.begin(); it != forwardDecl2.end(); ++it ) {
	QString fd = *it;
	fd = fd.stripWhiteSpace();
	if ( fd[ (int)fd.length() - 1 ] != ';' )
	    fd += ";";
	out << fd << endl;
    }

    out << endl;
    out << "class " << nameOfClass << " : public " << objClass << endl;
    out << "{ " << endl;

/* tmake ignore Q_OBJECT */
    out << "    Q_OBJECT" << endl;
    out << endl;
    out << "public:" << endl;

    // constructor(s)
    if ( objClass == "QDialog" || objClass == "QWizard" || objClass == "QSqlDialog" ) {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );" << endl;
	if ( dbAware ) {
	    out << "    " << nameOfClass << "( QSqlCursor* cursor, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );" << endl;
	}
    } else if ( objClass == "QWidget" || objClass == "QSqlWidget" ) {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );" << endl;
	if ( dbAware ) {
	    out << "    " << nameOfClass << "( QSqlCursor* cursor, QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );" << endl;
	}
    } else if ( objClass == "QMainWindow" ) {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );" << endl;
	if ( dbAware ) {
	    out << "    " << nameOfClass << "( QSqlCursor* cursor, QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );" << endl;
	}
	isMainWindow = TRUE;
    } else {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0 );" << endl;
    }

    // destructor
    out << "    ~" << nameOfClass << "();" << endl;
    out << endl;

    // database support
    if ( dbAware ) {
	out << "    " << "virtual bool setCursor( QSqlCursor* cursor );" << endl;
	out << "    " << "virtual QSqlCursor* createCursor( const QString& name );" << endl;
	out << endl;
    }

    // children
    bool needEventHandler = FALSE;
    bool needPolish = dbAware;
    nl = e.elementsByTagName( "widget" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	n = nl.item(i).toElement();
	createObjectDecl( n );
	needEventHandler = needEventHandler ||
			   !DomTool::propertiesOfType( n, "font" ).isEmpty() ;
	QString s = getClassName( n );
	if ( s == "QSqlTable" )
	    needPolish = TRUE;
    }

    // actions, toolbars, menus
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "actions" ) {
	    for ( QDomElement a = n.firstChild().toElement(); !a.isNull(); a = a.nextSibling().toElement() )
		createActionDecl( a );
	} else if ( n.tagName() == "toolbars" ) {
	    for ( QDomElement a = n.firstChild().toElement(); !a.isNull(); a = a.nextSibling().toElement() )
		createToolbarDecl( a );
	} else if ( n.tagName() == "menubar" ) {
	    for ( QDomElement a = n.firstChild().toElement(); !a.isNull(); a = a.nextSibling().toElement() )
		createMenuBarDecl( a );
	}
    }

    out << endl;

    if ( dbAware ) {
	dbConnections = unique( dbConnections );
	for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	    if ( !(*it).isEmpty() ) {
		if ( (*it) == "(default)" )
		    out << "    QSqlDatabase* defaultConnection;" << endl;
		else if ( !(*it).isEmpty() )
		    out << "    QSqlDatabase* " << *it << "Connection;" << endl;
		QStringList::Iterator it2;
		dbCursors[ (*it) ] = unique( dbCursors[ (*it) ] );
		for( it2 = dbCursors[ (*it) ].begin();
		      it2 != dbCursors[ (*it) ].end();
		      ++it2 ) {
		    if ( !(*it2).isEmpty() )
			out << "    QSqlCursor* " << (*it2) << "Cursor;" << endl;
		}
		if ( dbForm ) {
		    dbForms[ (*it) ] = unique( dbForms[ (*it) ] );
		    for ( it2 = dbForms[ (*it) ].begin();
			  it2 != dbForms[ (*it) ].end();
			  ++it2 ) {
			if ( !(*it2).isEmpty() )
			    out << "    QSqlForm* " << (*it2) << "Form;" << endl;
		    }
		}
	    }
	}
	out << "    QVector< QSqlCursor > autoDeleteCursors;" << endl;;
    }

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
    if ( !publicSlots.isEmpty() || needPolish ) {
	out << "public slots:" << endl;
	for ( it = publicSlots.begin(); it != publicSlots.end(); ++it )
	    out << indent << "virtual void " << (*it) << ";" << endl;
	if ( needPolish )
	    out << indent << "void polish();" << endl;
	out << endl;
    }

    // create protected additional slots as pure-virtual functions
    if ( !protectedSlots.isEmpty() ) {
	out << "protected slots:" << endl;
	for ( it = protectedSlots.begin(); it != protectedSlots.end(); ++it )
	    out << "    virtual void " << (*it) << ";" << endl;
	out << endl;
    }


    bool needProtected = needEventHandler;
    for ( it = layouts.begin(); !needProtected && it != layouts.end(); ++it )
	needProtected = needProtected || e.elementsByTagName( *it ).count() > 0 ;
    if ( needProtected )
	out << "protected:" << endl;

    // child layouts
    registerLayouts(e);

    // handle application events
    if ( needEventHandler )
	out << "    bool event( QEvent* );" << endl;

    // for multiple constructor initialization
    if ( dbAware )
	out << indent << "void init" << nameOfClass << "();" << endl;

    out << "};" << endl;
    out << endl;
    out << "#endif // " << protector << endl;
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
  Creates an implementation ( cpp-file ) for the form given in \a e

  \sa createFormDecl(), createObjectImpl()
 */
void Uic::createFormImpl( const QDomElement &e )
{
    QDomElement n;
    QDomNodeList nl;
    int i;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;
    QString objName = getObjectName( e );

    // generate local and local includes required
    QStringList globalIncludes, localIncludes;
    QStringList::Iterator it;

    // additional includes (local or global ) and forward declaractions
    nl = e.parentNode().toElement().elementsByTagName( "include" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in implementation" &&
	     n2.attribute( "location" ) != "local" )
	    globalIncludes += s;
    }

    bool dbAware = FALSE;
    if ( objClass == "QSqlDialog" || objClass == "QSqlWidget" ) {
	dbAware = TRUE;
    }
    if ( dbAware ) {
	registerDatabases( e );
	if ( dbConnections.count() )
	    globalIncludes += "qsqldatabase.h";
	if ( dbCursors.count() )
	    globalIncludes += "qsqlcursor.h";
	if ( dbForms[ "(default)" ].count() ) {
	    globalIncludes += "qsqlform.h";
	    globalIncludes += "qsqlrecord.h";
	}
    }

    // do the local includes afterwards, since global includes have priority on clashes
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in implementation" &&
	     n2.attribute( "location" ) == "local" &&!globalIncludes.contains( s ) )
	    localIncludes += s;
    }

    // additional custom widget headers
    nl = e.parentNode().toElement().elementsByTagName( "header" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "location" ) != "local" )
	    globalIncludes += s;
	else
	    localIncludes += s;
    }

    // includes for child widgets
    for ( it = tags.begin(); it != tags.end(); ++it ) {
	nl = e.elementsByTagName( *it );
	for ( i = 0; i < (int) nl.length(); i++ ) {
	    QString name = getClassName( nl.item(i).toElement() );
	    if ( name != objClass )
		globalIncludes += getInclude( name );
	    if ( name.mid( 1 ) == "ListView" )
		globalIncludes += "qheader.h";
	}
    }

    out << "#include <qvariant.h>   // first for gcc 2.7.2" << endl;

    globalIncludes = unique( globalIncludes );
    for ( it = globalIncludes.begin(); it != globalIncludes.end(); ++it ) {
	if ( !(*it).isEmpty() )
	    out << "#include <" << *it << ">" << endl;
    }
    localIncludes = unique( localIncludes );
    for ( it = localIncludes.begin(); it != localIncludes.end(); ++it ) {
	if ( !(*it).isEmpty() )
	    out << "#include \"" << *it << "\"" << endl;
    }

    out << "#include <qlayout.h>" << endl;
    out << "#include <qtooltip.h>" << endl;
    out << "#include <qwhatsthis.h>" << endl;
    if ( objClass == "QMainWindow" ) {
	out << "#include <qaction.h>" << endl;
	out << "#include <qmenubar.h>" << endl;
	out << "#include <qpopupmenu.h>" << endl;
	out << "#include <qtoolbar.h>" << endl;
    }

    // find out what images are required
    QStringList requiredImages;
    nl = e.elementsByTagName( "pixmap" );
    int j;
    for ( j = 0; j < (int) nl.length(); j++ ) {
	requiredImages += nl.item(j).firstChild().toText().data();
    }
    nl = e.parentNode().toElement().elementsByTagName( "iconset" );
    for ( j = 0; j < (int) nl.length(); j++ ) {
	requiredImages += nl.item(j).firstChild().toText().data();
    }

    if (!requiredImages.isEmpty() ) {
	out << "#include <qimage.h>" << endl;
	out << "#include <qpixmap.h>" << endl << endl;
    }

    QStringList images;
    QStringList xpmImages;
    if ( pixmapLoaderFunction.isEmpty() ) {
	// create images
	for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( n.tagName()  == "images" ) {
		nl = n.elementsByTagName( "image" );
		for ( i = 0; i < (int) nl.length(); i++ ) {
		    QString img = registerObject(  nl.item(i).toElement().attribute( "name" ) );
		    if ( !requiredImages.contains( img ) )
			continue;
		    QDomElement tmp = nl.item(i).firstChild().toElement();
		    if ( tmp.tagName() != "data" )
			continue;
		    QString format = tmp.attribute("format", "PNG" );
		    QString data = tmp.firstChild().toText().data();
		    if ( format == "XPM.GZ" ) {
			xpmImages += img;
			ulong length = tmp.attribute("length").toULong();
			QByteArray baunzip = unzipXPM( data, length );
			int a = 0;
			out << "static const char* const " << img << "_data[] = { " << endl;
			while ( baunzip[a] != '\"' )
			    a++;
			for ( ; a < (int) length; a++ )
			    out << baunzip[a];
			out << endl;
		    } else {
			images += img;
			out << "static const unsigned char const " << img << "_data[] = { " << endl;
			out << "    ";
			int a ;
			for ( a = 0; a < (int) (data.length()/2)-1; a++ ) {
			    out << "0x" << QString(data[2*a]) << QString(data[2*a+1]) << ",";
			    if ( a % 12 == 11 )
				out << endl << "    ";
			    else
				out << " ";
			}
			out << "0x" << QString(data[2*a]) << QString(data[2*a+1]) << endl;
			out << "};" << endl << endl;
		    }
		}
	    }
	}
	out << endl;
    }

    // register the object and unify its name
    objName = registerObject( objName );


    // constructor(s)
    if ( objClass == "QDialog" || objClass == "QWizard" || objClass == "QSqlDialog" ) {
	out << "/* " << endl;
	out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f'." << endl;
	out << " *" << endl;
	out << " *  The " << objClass.mid(1).lower() << " will by default be modeless, unless you set 'modal' to" << endl;
	out << " *  TRUE to construct a modal " << objClass.mid(1).lower() << "." << endl;
	out << " */" << endl;
	out << nameOfClass << "::" << nameOfClass << "( QWidget* parent,  const char* name, bool modal, WFlags fl )" << endl;
	out << "    : " << objClass << "( parent, name, modal, fl )" << endl;
    } else if ( objClass == "QWidget" || objClass == "QSqlWidget" )  {
	out << "/* " << endl;
	out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f'." << endl;
	out << " */" << endl;
	out << nameOfClass << "::" << nameOfClass << "( QWidget* parent,  const char* name, WFlags fl )" << endl;
	out << "    : " << objClass << "( parent, name, fl )" << endl;
    } else if ( objClass == "QMainWindow" ) {
	out << "/* " << endl;
	out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f'." << endl;
	out << " *" << endl;
	out << " */" << endl;
	out << nameOfClass << "::" << nameOfClass << "( QWidget* parent,  const char* name, WFlags fl )" << endl;
	out << "    : " << objClass << "( parent, name, fl )" << endl;
	isMainWindow = TRUE;
    } else {
	out << "/* " << endl;
	out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name'.' " << endl;
	out << " */" << endl;
	out << nameOfClass << "::" << nameOfClass << "( QWidget* parent,  const char* name )" << endl;
	out << "    : " << objClass << "( parent, name )" << endl;
    }

    out << "{" << endl;

    if ( objClass == "QMainWindow" )
	out << indent << "setCentralWidget( new QWidget( this, \"qt_central_widget\" ) );" << endl;

    if ( dbAware ) {
	out << indent << "init" << nameOfClass << "();" << endl;
	out << "}" << endl << endl;

	if ( objClass == "QSqlDialog" ) {
	    out << "/* " << endl;
	    out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	    out << " *  name 'name' and widget flags set to 'f' " << endl;
	    out << " *" << endl;
	    out << " *  The " << objClass.mid(1).lower() << " will be initialized with the default cursor 'cursor'." << endl;
	    out << " *" << endl;
	    out << " *  The " << objClass.mid(1).lower() << " will by default be modeless, unless you set 'modal' to" << endl;
	    out << " *  TRUE to construct a modal " << objClass.mid(1).lower() << "." << endl;
	    out << " */" << endl;
	    out << nameOfClass << "::" << nameOfClass << "( QSqlCursor* cursor, QWidget* parent,  const char* name, bool modal, WFlags fl )" << endl;
	    out << "    : " << objClass << "( parent, name, modal, fl )" << endl;
	    out << "{" << endl;
	    out << indent << "init" << nameOfClass << "();" << endl;
	    out << indent << "setCursor( cursor );" << endl;
	    out << "}" << endl << endl;
	} else if ( objClass == "QSqlWidget" ) {
	    out << "/* " << endl;
	    out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	    out << " *  name 'name' and widget flags set to 'f'." << endl;
	    out << " *" << endl;
	    out << " *  The " << objClass.mid(1).lower() << " will be initialized with the default cursor 'cursor'." << endl;
	    out << " */" << endl;
	    out << nameOfClass << "::" << nameOfClass << "( QSqlCursor* cursor, QWidget* parent,  const char* name, WFlags fl )" << endl;
	    out << "    : " << objClass << "( parent, name, fl )" << endl;
	    out << "{" << endl;
	    out << indent << "init" << nameOfClass << "();" << endl;
	    out << indent << "setCursor( cursor );" << endl;
	    out << "}" << endl << endl;
	}

	out << "/* " << endl;
	out << " *  Initializes " << nameOfClass << "." << endl;
	out << " */" << endl;
	out << "void " << nameOfClass << "::init" << nameOfClass <<"()" << endl;
	out << "{" << endl;
    }

    // create pixmaps for all images
    if ( !images.isEmpty() ) {
	out << indent << "QImage img;" << endl;
	out << indent << "QPixmap ";
	QStringList::Iterator it;
	for ( it = images.begin(); it != images.fromLast(); ++it )
	    out << (*it) << ", ";
	out << (*it)  << ";" << endl;
	for ( it = images.begin(); it != images.end(); ++it ) {
	    out << indent << "img.loadFromData( " << (*it) << "_data, sizeof( " << (*it) << "_data ), \"PNG\" );" << endl;
	    out << indent << (*it) << " = img;" << endl;
	}
    }
    // create pixmaps for all images
    if ( !xpmImages.isEmpty() ) {
	for ( it = xpmImages.begin(); it != xpmImages.end(); ++it ) {
	    out << indent << "QPixmap " << (*it) << "( ( const char** ) " << (*it) << "_data );" << endl;
	}
    }


    // set the properties
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    bool stdset = stdsetdef;
	    if ( n.hasAttribute( "stdset" ) )
		stdset = toBool( n.attribute( "stdset" ) );
	    QString prop = n.attribute("name");
	    QDomElement n2 = n.firstChild().toElement();
	    QString value = setObjectProperty( objClass, QString::null, prop, n2, stdset );
	    if ( value.isEmpty() )
		continue;
	    if ( prop == "name" ) {
		if ( dbAware )
		    out << "    if ( !name() )" << endl;
		else
		    out << "    if ( !name )" << endl;
		out << "\t";
	    } else {
		out << indent;
	    }
	    if ( dbAware && prop == "sort" ) {
		QString defaultTable = getDatabaseInfo( e, "table" );
		out << "QStringList " << defaultTable << "Sort;" << endl;
		out << indent << defaultTable << "Sort << \"" << value << "\";" << endl;
		out << indent;
		value = defaultTable + "Sort";
	    }

	    if ( prop == "geometry" && n2.tagName() == "rect") {
		QDomElement n3 = n2.firstChild().toElement();
		int w = 0, h = 0;
		while ( !n3.isNull() ) {
		    if ( n3.tagName() == "width" )
			w = n3.firstChild().toText().data().toInt();
		    else if ( n3.tagName() == "height" )
			h = n3.firstChild().toText().data().toInt();
		    n3 = n3.nextSibling().toElement();
		}
		out << "resize( " << w << ", " << h << " ); " << endl;
	    } else {
		if ( stdset )
		    out << mkStdSet(prop ) << "( " << value << " );" << endl;
		else
		    out << "setProperty( \"" << prop << "\", " << value << " );" << endl;
	    }
	}
    }

    // create all children, some forms have special requirements

    if ( objClass == "QWizard" ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName()  ) ) {
		QString page = createObjectImpl( n, objClass, "this" );
		QString label = DomTool::readAttribute( n, "title", "" ).toString();
		out << indent << "addPage( " << page << ", "<< trmacro << "( " << fixString( label ) << " ) );" << endl;
		QVariant def( FALSE, 0 );
		if ( DomTool::hasAttribute( n, "backEnabled" ) )
		    out << indent << "setBackEnabled( " << page << ", " << mkBool( DomTool::readAttribute( n, "backEnabled", def).toBool() ) << endl;
		if ( DomTool::hasAttribute( n, "nextEnabled" ) )
		    out << indent << "setNextEnabled( " << page << ", " << mkBool( DomTool::readAttribute( n, "nextEnabled", def).toBool() ) << endl;
		if ( DomTool::hasAttribute( n, "finishEnabled" ) )
		    out << indent << "setFinishEnabled( " << page << ", " << mkBool( DomTool::readAttribute( n, "finishEnabled", def).toBool() ) << " );" << endl;
		if ( DomTool::hasAttribute( n, "helpEnabled" ) )
		    out << indent << "setHelpEnabled( " << page << ", " << mkBool( DomTool::readAttribute( n, "helpEnabled", def).toBool() ) << endl;
		if ( DomTool::hasAttribute( n, "finish" ) )
		    out << indent << "setFinish( " << page << ", " << mkBool( DomTool::readAttribute( n, "finish", def).toBool() ) << endl;
	    }
	}
    } else { // standard widgets
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName()  ) )
		createObjectImpl( n, objName, "this" );
	}
    }

    // database support
    if ( dbAware ) {
	QString defaultTable = getDatabaseInfo( e, "table" );
	out << endl << indent << "// database support" << endl;
	int dbFormCount = 0;
	dbConnections = unique( dbConnections );
	for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	    if ( !(*it).isEmpty() ) {
		if ( (*it) == "(default)" )
		    out << indent << "defaultConnection = QSqlDatabase::database();" << endl;
		else
		    out << indent << (*it) << "Connection = QSqlDatabase::database( \"" <<(*it) << "\" );" << endl;
		QStringList::Iterator it2;
		dbCursors[ (*it) ] = unique( dbCursors[ (*it) ] );
		for( it2 = dbCursors[ (*it) ].begin();
		      it2 != dbCursors[ (*it) ].end();
		      ++it2 ) {
		    if ( !(*it2).isEmpty() )
			out << indent << (*it2) << "Cursor = 0;" << endl;
		}
		dbForms[ (*it) ] = unique( dbForms[ (*it) ] );
		for ( it2 = dbForms[ (*it) ].begin();
		      it2 != dbForms[ (*it) ].end();
		      ++it2 ) {
		    dbFormCount++;
		    if ( !(*it2).isEmpty() ) {
			out << indent << (*it2) << "Form = new QSqlForm( this, \"" << (*it2) << "\" );" << endl;
			for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
			    createFormImpl( n, (*it2), (*it), (*it2) );
			}
		    }
		}
	    }
	}
	out << indent << "autoDeleteCursors.resize( " << dbFormCount << " );" << endl;
	out << indent << "autoDeleteCursors.setAutoDelete( TRUE );" << endl;
    }

    // actions, toolbars, menubar
    out << endl;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "actions" )
	    createActionImpl( n.firstChild().toElement(), "this" );
    }
    out << endl;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "toolbars" )
	    createToolbarImpl( n );
    }
    out << endl;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "menubar" )
	    createMenuBarImpl( n );
    }
    out << endl;

    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "connections" ) {
	    // setup signals and slots connections
	    out << endl << indent << "// signals and slots connections" << endl;
	    nl = n.elementsByTagName( "connection" );
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString sender, receiver, signal, slot;
		for ( QDomElement n2 = nl.item(i).firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		    if ( n2.tagName() == "sender" )
			sender = n2.firstChild().toText().data();
		    else if ( n2.tagName() == "receiver" )
			receiver = n2.firstChild().toText().data();
		    else if ( n2.tagName() == "signal" )
			signal = n2.firstChild().toText().data();
		    else if ( n2.tagName() == "slot" )
			slot = n2.firstChild().toText().data();
		}
		if ( sender.isEmpty() || receiver.isEmpty() || signal.isEmpty() || slot.isEmpty() )
		    continue;

		sender = registeredName( sender );
		receiver = registeredName( receiver );

		 // translate formwindow name to "this"
		if ( sender == objName )
		    sender = "this";
		if ( receiver == objName )
		    receiver = "this";

		out << indent << "connect( " << sender << ", SIGNAL( " << signal << " ), "
		    << receiver << ", SLOT( " << slot << " ) );" << endl;
	    }
	} else if ( n.tagName()  == "tabstops" ) {
	    // setup tab order
	    out << endl << indent << "// tab order" << endl;
	    QString lastName;
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "tabstop" ) {
		    QString name = n2.firstChild().toText().data();
		    name = registeredName( name );
		    if ( !lastName.isEmpty() )
			out << indent << "setTabOrder( " << lastName << ", " << name << " );" << endl;
		    lastName = name;
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
    }


    // buddies
    bool firstBuddy = TRUE;
    for ( QValueList<Buddy>::Iterator buddy = buddies.begin(); buddy != buddies.end(); ++buddy ) {
	if ( isObjectRegistered( (*buddy).buddy ) ) {
	    if ( firstBuddy ) {
		out << endl << indent << "// buddies" << endl;
	    }
	    out << indent << (*buddy).key << "->setBuddy( " << registeredName( (*buddy).buddy ) << " );" << endl;
	    firstBuddy = FALSE;
	}

    }

    // end of constructor
    out << "}" << endl;
    out << endl;

	// destructor
    out << "/*  " << endl;
    out << " *  Destroys the object and frees any allocated resources" << endl;
    out << " */" << endl;
    out << nameOfClass << "::~" << nameOfClass << "()" << endl;
    out << "{" << endl;
    out << indent << "// no need to delete child widgets, Qt does it all for us" << endl;
    out << "}" << endl;
    out << endl;

    // handle application events if required
    bool needFontEventHandler = FALSE;
    bool needSqlTableEventHandler = FALSE;
    bool needSqlFormEventHandler = dbAware;
    nl = e.elementsByTagName( "widget" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	if ( !DomTool::propertiesOfType( nl.item(i).toElement() , "font" ).isEmpty() )
	    needFontEventHandler = TRUE;
	QString s = getClassName( nl.item(i).toElement() );
	if ( s == "QSqlTable" )
	    needSqlTableEventHandler = TRUE;
	if ( needFontEventHandler && needSqlTableEventHandler )
	    break;
    }
    if ( needFontEventHandler ) {
	//	indent = "\t"; // increase indentation for if-clause below
	out << "/*  " << endl;
	out << " *  Main event handler. Reimplemented to handle" << endl;
	out << " *  application font changes";
	out << " */" << endl;
	out << "bool " << nameOfClass  << "::event( QEvent* ev )" << endl;
	out << "{" << endl;
	out << "    bool ret = " << objClass << "::event( ev ); " << endl;
	if ( needFontEventHandler ) {
	    indent += "\t";
	    out << "    if ( ev->type() == QEvent::ApplicationFontChange ) {" << endl;
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		n = nl.item(i).toElement();
		QStringList list = DomTool::propertiesOfType( n, "font" );
		for ( it = list.begin(); it != list.end(); ++it )
		    createExclusiveProperty( n, *it );
	    }
	    out << "    }" << endl;
	    indent = "    ";
	}
    }

    if ( needSqlTableEventHandler || needSqlFormEventHandler ) {
	out << "/*  " << endl;
	out << " *  Widget polish.  Reimplemented to handle" << endl;
	if ( needSqlTableEventHandler ) {
	    out << " *  default SQL table initialization";
	    if ( needSqlFormEventHandler )
		out << " and" << endl;
	    else
		out << endl;
	}
	if ( needSqlFormEventHandler )
	    out << " *  default SQL form initialization" << endl;
	out << " */" << endl;
	out << "void " << nameOfClass  << "::polish()" << endl;
	out << "{" << endl;
	if ( needSqlTableEventHandler ) {
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString s = getClassName( nl.item(i).toElement() );
		if ( s == "QSqlTable" ) {
		    n = nl.item(i).toElement();
		    QString c = getObjectName( n );
		    QString conn = getDatabaseInfo( n, "connection" );
		    QString tab = getDatabaseInfo( n, "table" );
		    if ( !( conn.isEmpty() || tab.isEmpty() ) ) {
			out << indent << "if ( " << c << " ) {" << endl;
			out << indent << indent << "QSqlCursor* c = " << c << "->sqlCursor();" << endl;
			out << indent << indent << "if ( !c ) {" << endl;
			if ( conn == "(default)" )
			    out << indent << indent << indent << "c = new QSqlCursor( \"" << tab << "\" );" << endl;
			else
			    out << indent << indent << indent << "QSqlCursor* c = new QSqlCursor( \"" << tab << "\", " << conn << "Connection );" << endl;
			out << indent << indent << indent << c << "->setCursor( c, FALSE, TRUE );" << endl;
			out << indent << indent << "}" << endl;
			out << indent << indent << c << "->refresh();" << endl;
			out << indent << "}" << endl;
		    }
		}
	    }
	}
	if ( needSqlFormEventHandler ) {
	    out << indent << "if ( !sqlCursor() )" << endl;
	    QString defaultTable = getDatabaseInfo( e, "table" );
	    out << indent << indent << "setCursor( createCursor( \"" << defaultTable << "\" ) );" << endl;
	    out << indent << "if ( !form() )" << endl;
	    out << indent << indent << "setForm( " << defaultTable << "Form );" << endl;
	    out << indent << "QSqlCursor* cur = sqlCursor();" << endl;
	    out << indent << "if ( cur && !cur->isActive() ) {" << endl;
	    out << indent << indent << "refresh();" << endl;
	    out << indent << indent << "firstRecord();" << endl;
	    out << indent << "}" << endl;
	}
	out << indent << objClass << "::polish();" << endl;
	out << "}" << endl;
	out << endl;
    }


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
	for ( it = publicSlots.begin(); it != publicSlots.end(); ++it ) {
	    out << "void " << nameOfClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << nameOfClass << "::" << (*it) << ": Not implemented yet!\" );" << endl;
	    out << "}" << endl;
	    out << endl;
	}
    }

    // create protected additional slots as pure-virtual functions
    if ( !protectedSlots.isEmpty() ) {
	for ( it = protectedSlots.begin(); it != protectedSlots.end(); ++it ) {
	    out << "void " << nameOfClass << "::" << (*it) << endl;
	    out << "{" << endl;
	    out << "    qWarning( \"" << nameOfClass << "::" << (*it) << ": Not implemented yet!\" );" << endl;
	    out << "}" << endl;
	    out << endl;
	}
    }

    // database support
    if ( dbAware ) {
	createDatabaseImpl( e );
    }
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

/*! Creates form support implementation code for the widgets given
  in \a e.

  Traverses recursively over all children.
 */

void Uic::createFormImpl( const QDomElement& e, const QString& form, const QString& connection, const QString& table )
{
    if ( e.tagName() == "widget" &&
	 e.attribute( "class" ) != "QSqlTable" ) {
	if ( isWidgetInTable( e, connection, table ) )
	    out << indent << form << "Form->insert( " << getObjectName( e ) << ", " << fixString( getDatabaseInfo( e, "field" ) ) << " );" << endl;
    }
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	createFormImpl( n, form, connection, table );
    }
}

/*! Creates database support implementation code for the object given
  in \a e.
 */

void Uic::createDatabaseImpl( const QDomElement& e )
{
    QStringList::Iterator it;
    QDomElement n;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;
    QString objName = getObjectName( e );
    bool dbForm = ( dbForms[ "(default)" ].count() != 0 );
    int i;

    QString defaultTable = getDatabaseInfo( e, "table" );

    out << "/*  " << endl;
    out << " *  Sets the cursor identified by 'cursor' to be used" << endl;
    out << " *  by the " << objName << ".  Returns TRUE if 'cursor' was recognized," << endl;
    out << " *  otherwise FALSE is returned." << endl;
    if ( dbForm ) {
	out << " *  If the cursor is recognized, the appropriate form fields are" << endl;
	out << " *  mapped to the cursor." << endl;
    }
    out << " */" << endl;
    out << "bool " << nameOfClass << "::setCursor( QSqlCursor* cursor )" << endl;
    out << "{" << endl;
    out << indent << "if ( !cursor )" << endl;
    out << indent << indent << "return FALSE;" << endl;
    out << indent << "QString n = cursor->name();" << endl;
    out << indent << "bool recognized = FALSE;" << endl;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() ) {
	    QStringList::Iterator it2;
	    dbCursors[ (*it) ] = unique( dbCursors[ (*it) ] );
	    for( it2 = dbCursors[ (*it) ].begin();
		 it2 != dbCursors[ (*it) ].end();
		 ++it2 ) {
		out << indent << "if ( n == \"" << (*it2) << "\" ) {" << endl;
		out << indent << indent << (*it2) << "Cursor = cursor;" << endl;
		if ( (*it2) == defaultTable )
		    out << indent << indent << "setSqlCursor( cursor );" << endl;
		out << indent << indent << "recognized = TRUE;" << endl;
		QStringList::Iterator it3;
		dbForms[ (*it) ] = unique( dbForms[ (*it) ] );
		for ( it3 = dbForms[ (*it) ].begin();
		      it3 != dbForms[ (*it) ].end();
		      ++it3 ) {
		    if ( (*it3) == (*it2) ) { // form for table
			out << indent << indent << "QSqlRecord* buf = " << (*it2) << "Cursor->editBuffer();" << endl;
			out << indent << indent << (*it3) << "Form->setRecord( buf );" << endl;
		    }
		    out << indent << indent << (*it3) << "Form->readFields();" << endl;
		}
		out << indent << "}" << endl;
	    }
	}
    }
    out << indent << "if ( !recognized )" << endl;
    out << indent << indent << "qWarning(\"" << nameOfClass << "::setCursor: unknown cursor:'\" + n + \"'\");" << endl;
    out << indent << "return recognized;" << endl;
    out << "}" << endl;
    out << endl;

//     out << "/*  " << endl;
//     out << " *  Returns a pointer to the default cursor." << endl;
//     if ( dbForm )
//	out << " *  Reimplemented from QSqlNavigator which relies on a default cursor" << endl;
//     out << " *  If the default cursor does not exist, it is first created with " << endl;
//     out << " *  " << nameOfClass << "::createCursor()." << endl;
//     out << " */" << endl;
//     out << "QSqlCursor* " << nameOfClass << "::defaultCursor()" << endl;
//     out << "{" << endl;
//     if ( !defaultTable.isEmpty() ) {
//	out << indent << "if ( !" << defaultTable << "Cursor ) {" << endl;
//	out << indent << indent << defaultTable << "Cursor = createCursor( \"" << defaultTable << "\" );" << endl;
//	out << indent << indent << "setCursor( " << defaultTable << "Cursor );" << endl;
//	out << indent << "}" << endl;
//	out << indent << "return " << defaultTable << "Cursor;" << endl;
//     } else {
//	out << indent << "return 0;" << endl;
//     }
//     out << "}" << endl;
//     out << endl;

    out << "/*  " << endl;
    out << " *  Returns a pointer to a new cursor based on 'name'." << endl;
    out << " *  If 'name' is not recognized, 0 is returned." << endl;
    out << " */" << endl;
    out << "QSqlCursor* " << nameOfClass << "::createCursor( const QString& name  )" << endl;
    out << "{" << endl;
    out << indent << "bool recognized = FALSE;" << endl;
    out << indent << "QSqlCursor*  cursor = 0;" << endl;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() ) {
	    QStringList::Iterator it2;
	    dbCursors[ (*it) ] = unique( dbCursors[ (*it) ] );
	    i = 0;
	    for( it2 = dbCursors[ (*it) ].begin();
		 it2 != dbCursors[ (*it) ].end();
		 ++it2, ++i ) {
		out << indent << "if ( name == \"" << (*it2) << "\" ) {" << endl;
		if ( (*it) == "(default)" )
		    out << indent << indent << "cursor = new QSqlCursor( name );" << endl;
		else
		    out << indent << indent <<  "cursor = new QSqlCursor( name, " << (*it) << "Connection );" << endl;
		out << indent << indent << "autoDeleteCursors.insert( " << i << ", cursor );" << endl;
		out << indent << indent << "recognized = TRUE;" << endl;
		out << indent << "}" << endl;
	    }
	}
    }
    out << indent << "if ( !recognized )" << endl;
    out << indent << indent << "qWarning(\"" << nameOfClass << "::createCursor: unknown cursor:'\" + name + \"'\");" << endl;
    out << indent << "return cursor;" << endl;
    out << "}" << endl;
    out << endl;
}

/*!
  Creates a declaration for the object given in \a e.

  Any children are ignored, this function does _not_ travesere
  recursively.

  \sa createObjectImpl()
 */
void Uic::createObjectDecl( const QDomElement& e )
{
    if ( e.tagName() == "vbox" ) {
	out << "    QVBoxLayout* " << registerObject(getLayoutName(e) ) << ";" << endl;
    } else if ( e.tagName() == "hbox" ) {
	out << "    QHBoxLayout* " << registerObject(getLayoutName(e) ) << ";" << endl;
    } else if ( e.tagName() == "grid" ) {
	out << "    QGridLayout* " << registerObject(getLayoutName(e) ) << ";" << endl;
    } else {
	QString objClass = getClassName( e );
	if ( objClass.isEmpty() )
	    return;
	QString objName = getObjectName( e );
	if ( objName.isEmpty() )
	    return;
	// ignore QLayoutWidgets
	if ( objClass == "QLayoutWidget" )
	    return;
	// register the object and unify its name
	objName = registerObject( objName );
	if ( objClass == "Line" )
	    objClass = "QFrame";
	out << "    " << objClass << "* " << objName << ";" << endl;
    }
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

/*!
  Creates an implementation for the object given in \a e.

  Traverses recursively over all children.

  Returns the name of the generated child object.

  \sa createObjectDecl()
 */
QString Uic::createObjectImpl( const QDomElement &e, const QString& parentClass, const QString& par, const QString& layout )
{
    QString parent( par );
    if ( parent == "this" && isMainWindow )
	parent = "centralWidget()";
    QDomElement n;
    QString objClass, objName;

    if ( layouts.contains( e.tagName() ) )
	return createLayoutImpl( e, parentClass, parent, layout );

    objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return objName;
    objName = getObjectName( e );

    QString definedName = objName;
    bool isTmpObject = objName.isEmpty() || objClass == "QLayoutWidget";
    if ( isTmpObject ) {
	if ( objClass[0] == 'Q' )
	    objName = objClass.mid(1);
	else
	    objName = objClass.lower();
	objName.prepend( "private" );
    }

    bool isLine = objClass == "Line";
    if ( isLine )
	objClass = "QFrame";

    out << endl;
    if ( objClass == "QLayoutWidget" ) {
	if ( layout.isEmpty() ) {
	    // register the object and unify its name
	    objName = registerObject( objName );
	    out << "    QWidget* " << objName << " = new QWidget( " << parent << ", \"" << definedName << "\" );" << endl;
	} else {
	    // the layout widget is not necessary, hide it by creating its child in the parent
	    QString result;
	    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
		if (tags.contains( n.tagName()  ) )
		    result = createObjectImpl( n, parentClass, parent, layout );
	    }
	    return result;
	}
    }   else {
	// register the object and unify its name
	objName = registerObject( objName );
	out << "    ";
	if ( isTmpObject )
	    out << objClass << "* ";
	out << objName << " = new " << createObjectInstance( objClass, parent, objName ) << ";" << endl;
    }

    lastItem = "0";
    // set the properties and insert items
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    bool stdset = stdsetdef;
	    if ( n.hasAttribute( "stdset" ) )
		stdset = toBool( n.attribute( "stdset" ) );
	    QString prop = n.attribute("name");
	    QString value = setObjectProperty( objClass, objName, prop, n.firstChild().toElement(), stdset );
	    if ( value.isEmpty() )
		continue;
	    if ( prop == "name" )
		continue;
	    if ( prop == "buddy" && value[0] == '\"' && value[(int)value.length()-1] == '\"' ) {
		buddies << Buddy( objName, value.mid(1, value.length() - 2 ) );
		continue;
	    }
	    if ( isLine && prop == "orientation" ) {
		prop = "frameStyle";
		if ( value.right(10)  == "Horizontal" )
		    value = "QFrame::HLine | QFrame::Sunken";
		else
		    value = "QFrame::VLine | QFrame::Sunken";
	    }
	    if ( prop == "buttonGroupId" ) {
		if ( parentClass == "QButtonGroup" )
		    out << indent << parent << "->insert( " << objName << ", " << value << " );" << endl;
		continue;
	    }
	    if ( prop == "sort" ) {
		out << indent << "QStringList " << objName << "Sort;" << endl;
		out << indent << objName << "Sort << \"" << value << "\";" << endl;
		value = objName + "Sort";
	    }
	    if ( prop == "geometry") {
		out << indent << objName << "->setGeometry( " << value << " ); " << endl;
	    } else {
		if ( stdset )
		    out << indent << objName << "->" << mkStdSet(prop ) << "( " << value << " );" << endl;
		else
		    out << indent << objName << "->setProperty( \"" << prop << "\", " << value << " );" << endl;
	    }
	} else if ( n.tagName() == "item" ) {
	    if ( objClass.mid( 1 ) == "ListBox" ) {
		QString s = createListBoxItemImpl( n, objName );
		if ( !s.isEmpty() )
		    out << indent << s << endl;
	    } else if ( objClass.mid( 1 ) == "ComboBox" ) {
		QString s = createListBoxItemImpl( n, objName );
		if ( !s.isEmpty() )
		    out << indent << s << endl;
	    } else if ( objClass.mid( 1 ) == "IconView" ) {
		QString s = createIconViewItemImpl( n, objName );
		if ( !s.isEmpty() )
		    out << indent << s << endl;
	    } else if ( objClass.mid( 1 ) == "ListView" ) {
		QString s = createListViewItemImpl( n, objName, QString::null );
		if ( !s.isEmpty() )
		    out << s << endl;
	    }
	} else if ( n.tagName() == "column" || n.tagName() == "row" ) {
	    if ( objClass.mid( 1 ) == "ListView" ) {
		QString s = createListViewColumnImpl( n, objName );
		if ( !s.isEmpty() )
		    out << s;
	    } else if ( objClass ==  "QTable" || objClass == "QSqlTable" ) {
		QString s = createTableRowColumnImpl( n, objName );
		if ( !s.isEmpty() )
		    out << s;
	    }
	}
    }

    // create all children, some widgets have special requirements

    if ( objClass == "QTabWidget" ) {
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName()  ) ) {
		QString page = createObjectImpl( n, objClass, objName );
		QString label = DomTool::readAttribute( n, "title", "" ).toString();
		out << indent << objName << "->insertTab( " << page << ", " << trmacro << "( " << fixString( label ) << " ) );" << endl;
	    }
	}
     } else { // standard widgets
	for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	    if ( tags.contains( n.tagName()  ) )
		createObjectImpl( n, objClass, objName );
	}
    }

    return objName;
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
		    pix.prepend( pixmapLoaderFunction + "( " );
		    pix.append( " )" );
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
		    pix.prepend( pixmapLoaderFunction + "( " );
		    pix.append( " )" );
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
		    pix.prepend( pixmapLoaderFunction + "( " );
		    pix.append( " )" );
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
		    pix.prepend( pixmapLoaderFunction + "( " );
		    pix.append( " )" );
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
		    pix.prepend( pixmapLoaderFunction + "( " );
		    pix.append( " )" );
		}
	    } else if ( attrib == "field" )
		field = v.toString();
	}
	n = n.nextSibling().toElement();
    }

    // ### This generated code sucks! We have to set the number of
    // rows/cols before and then only do setLabel/()
    // ### careful, though, since QSqlTable has an API which makes this code pretty good

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
	} else if ( objClass == "QSqlTable" ) {
	    // ### also need pixmap support in QSqlTable
	    if ( !txt.isEmpty() && !field.isEmpty() )
		out << indent << parent << "->addColumn( " << fixString( field ) << ", " << fixString( txt ) << " );" << endl;
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

/*!
  Creates a set-call for property \a exclusiveProp of the object
  given in \a e.

  If the object does not have this property, the function does nothing.

  Exclusive properties are used to generate the implementation of
  application font or palette change handlers in createFormImpl().

 */
void Uic::createExclusiveProperty( const QDomElement & e, const QString& exclusiveProp )
{
    QDomElement n;
    QString objClass = getClassName( e );
    if ( objClass.isEmpty() )
	return;
    QString objName = getObjectName( e );
    if ( objClass.isEmpty() )
	return;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "property" ) {
	    bool stdset = stdsetdef;
	    if ( n.hasAttribute( "stdset" ) )
		stdset = toBool( n.attribute( "stdset" ) );
	    QString prop = n.attribute("name");
	    if ( prop != exclusiveProp )
		continue;
	    QString value = setObjectProperty( objClass, objName, prop, n.firstChild().toElement(), stdset );
	    if ( value.isEmpty() )
		continue;
	    out << indent << indent << objName << "->setProperty( \"" << prop << "\", " << value << " );" << endl;
	}
    }
}


const char* const ColorRole[] = {
    "Foreground", "Button", "Light", "Midlight", "Dark", "Mid",
    "Text", "BrightText", "ButtonText", "Base", "Background", "Shadow",
    "Highlight", "HighlightedText", 0
};


/*!  Attention: this function has to be in sync with
  Resource::saveProperty() and DomTool::elementToVariant. If you
  change one, change all.
 */
QString Uic::setObjectProperty( const QString& objClass, const QString& obj, const QString &prop, const QDomElement &e, bool stdset )
{
    QString v;
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
	v = "QRect( %1, %2, %3, %4 )";
	v = v.arg(x).arg(y).arg(w).arg(h);

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
	v = "QPoint( %1, %2 )";
	v = v.arg(x).arg(y);
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
	v = "QSize( %1, %2 )";
	v = v.arg(w).arg(h);
    } else if ( e.tagName() == "color" ) {
	QDomElement n3 = e.firstChild().toElement();
	int r= 0, g = 0, b = 0;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "red" )
		r = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "green" )
		g = n3.firstChild().toText().data().toInt();
	    else if ( n3.tagName() == "blue" )
		b = n3.firstChild().toText().data().toInt();
	    n3 = n3.nextSibling().toElement();
	}
	v = "QColor( %1, %2, %3 )";
	v = v.arg(r).arg(g).arg(b);
    } else if ( e.tagName() == "font" ) {
	QDomElement n3 = e.firstChild().toElement();
	QString fontname = "f";
	if ( !obj.isEmpty() ) {
	    fontname = obj + "_font";
	    out << indent << "QFont "  << fontname << "(  " << obj << "->font() );" << endl;
	} else {
	    out << indent << "QFont "  << fontname << "( font() );" << endl;
	}
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "family" )
		out << indent << fontname << ".setFamily( \"" << n3.firstChild().toText().data() << "\" );" << endl;
	    else if ( n3.tagName() == "pointsize" )
		out << indent << fontname << ".setPointSize( " << n3.firstChild().toText().data() << " );" << endl;
	    else if ( n3.tagName() == "bold" )
		out << indent << fontname << ".setBold( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
	    else if ( n3.tagName() == "italic" )
		out << indent << fontname << ".setItalic( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
	    else if ( n3.tagName() == "underline" )
		out << indent << fontname << ".setUnderline( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
	    else if ( n3.tagName() == "strikeout" )
		out << indent << fontname << ".setStrikeOut( " << mkBool( n3.firstChild().toText().data() ) << " );" << endl;
	    n3 = n3.nextSibling().toElement();
	}

	if ( prop == "font" ) {
	    if ( !obj.isEmpty() )
		out << indent << obj << "->setFont( " << fontname << " ); " << endl;
	    else
		out << indent << "setFont( " << fontname << " ); " << endl;
	} else {
	    v = fontname;
	}
    } else if ( e.tagName() == "string" ) {
	if ( prop == "toolTip" ) {
	    if ( !obj.isEmpty() )
		out << indent << "QToolTip::add(  " << obj << ", " + trmacro + "( " << fixString( e.firstChild().toText().data() ) << " ) );" << endl;
	    else
		out << indent << "QToolTip::add(  this, " + trmacro + "( " << fixString( e.firstChild().toText().data() ) << " ) );" << endl;
	} else if ( prop == "whatsThis" ) {
	    if ( !obj.isEmpty() )
		out << indent << "QWhatsThis::add(  " << obj << ", " << trmacro << "( " << fixString( e.firstChild().toText().data() ) << " ) );" << endl;
	    else
		out << indent << "QWhatsThis::add(  this, " << trmacro << "( " << fixString( e.firstChild().toText().data() ) << " ) );" << endl;
	} else {
	    v = trmacro + "( %1 )";
	    v = v.arg( fixString( e.firstChild().toText().data() ) );
	}
    } else if ( e.tagName() == "cstring" ) {
	    v = "\"%1\"";
	    v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "number" ) {
	v = "%1";
	v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "bool" ) {
	if ( stdset )
	    v = "%1";
	else
	    v = "QVariant( %1, 0 )";
	v = v.arg( mkBool( e.firstChild().toText().data() ) );
    } else if ( e.tagName() == "pixmap" ) {
	v = e.firstChild().toText().data();
	if ( !pixmapLoaderFunction.isEmpty() ) {
	    v.prepend( pixmapLoaderFunction + "( " );
	    v.append( " )" );
	}
    } else if ( e.tagName() == "iconset" ) {
	v = "QIconSet( %1 )";
	v = v.arg( e.firstChild().toText().data() );
    } else if ( e.tagName() == "image" ) {
	v = e.firstChild().toText().data() + ".convertToImage()";
    } else if ( e.tagName() == "enum" ) {
	if ( stdset )
	    v = "%1::%2";
	else
	    v = "(int)%1::%2";
	QString oc = objClass;
	QString ev = e.firstChild().toText().data();
	if ( oc == "QListView" && ev == "Manual" ) // #### workaround, rename QListView::Manual of WithMode enum in 3.0
	    oc = "QScrollView";
	v = v.arg( oc ).arg( ev );
    } else if ( e.tagName() == "set" ) {
	QString keys( e.firstChild().toText().data() );
	QStringList lst = QStringList::split( '|', keys );
	v = "int( ";
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    v += objClass + "::" + *it;
	    if ( it != lst.fromLast() )
		v += " | ";
	}
	v += " )";
    } else if ( e.tagName() == "sizepolicy" ) {
	QDomElement n3 = e.firstChild().toElement();
	QSizePolicy sp;
	while ( !n3.isNull() ) {
	    if ( n3.tagName() == "hsizetype" )
		sp.setHorData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
	    else if ( n3.tagName() == "vsizetype" )
		sp.setVerData( (QSizePolicy::SizeType)n3.firstChild().toText().data().toInt() );
	    n3 = n3.nextSibling().toElement();
	}
	QString tmp;
	if ( !obj.isEmpty() )
	    tmp = obj + "->";
	v = "QSizePolicy( (QSizePolicy::SizeType)%1, (QSizePolicy::SizeType)%2, " +tmp + "sizePolicy().hasHeightForWidth() )";
	v = v.arg( (int)sp.horData() ).arg( (int)sp.verData() );
    } else if ( e.tagName() == "palette" ) {
	QPalette pal;
	bool no_pixmaps = e.elementsByTagName( "pixmap" ).count() == 0;
	QDomElement n;
	if ( no_pixmaps ) {
	    n = e.firstChild().toElement();
	    while ( !n.isNull() ) {
		QColorGroup cg;
		if ( n.tagName() == "active" ) {
		    cg = loadColorGroup( n );
		    pal.setActive( cg );
		} else if ( n.tagName() == "inactive" ) {
		    cg = loadColorGroup( n );
		    pal.setInactive( cg );
		} else if ( n.tagName() == "disabled" ) {
		    cg = loadColorGroup( n );
		    pal.setDisabled( cg );
		}
		n = n.nextSibling().toElement();
	    }
	}
	if ( no_pixmaps && pal == QPalette( pal.active().button(), pal.active().background() ) ) {
	    v = "QPalette( QColor( %1, %2, %3 ), QColor( %1, %2, %3 ) )";
	    v = v.arg( pal.active().button().red() ).arg( pal.active().button().green() ).arg( pal.active().button().blue() );
	    v = v.arg( pal.active().background().red() ).arg( pal.active().background().green() ).arg( pal.active().background().blue() );
	} else {
	    QString palette = "pal";
	    if ( !pal_used ) {
		out << indent << "QPalette " << palette << ";" << endl;
		pal_used = TRUE;
	    }
	    QString cg = "cg";
	    if ( !cg_used ) {
		out << indent << "QColorGroup " << cg << ";" << endl;
		cg_used = TRUE;
	    }
	    n = e.firstChild().toElement();
	    while ( !n.isNull() && n.tagName() != "active")
		n = n.nextSibling().toElement();
	    createColorGroupImpl( cg, n );
	    out << indent << palette << ".setActive( " << cg << " );" << endl;

	    n = e.firstChild().toElement();
	    while ( !n.isNull() && n.tagName() != "inactive")
		n = n.nextSibling().toElement();
	    createColorGroupImpl( cg, n );
	    out << indent << palette << ".setInactive( " << cg << " );" << endl;

	    n = e.firstChild().toElement();
	    while ( !n.isNull() && n.tagName() != "disabled")
		n = n.nextSibling().toElement();
	    createColorGroupImpl( cg, n );
	    out << indent << palette << ".setDisabled( " << cg << " );" << endl;
	    v = palette;
	}
    } else if ( e.tagName() == "cursor" ) {
	v = "QCursor( %1 )";
	v = v.arg( e.firstChild().toText().data() );
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
	v = "QDate( %1, %2, %3 )";
	v = v.arg(y).arg(m).arg(d);
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
	v = "QTime( %1, %2, %3 )";
	v = v.arg(h).arg(m).arg(s);
    } else if ( e.tagName() == "datetime" ) {
	QDomElement n3 = e.firstChild().toElement();
	int h, mi, s, y, mo, d;
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
	v = "QDateTime( QDate( %1, %2, %3 ), QTime( %4, %5, %6 ) )";
	v = v.arg(y).arg(mo).arg(d).arg(h).arg(mi).arg(s);
    } else if ( e.tagName() == "stringlist" ) {
	if ( prop == "sort" ) {
	    QStringList l;
	    QDomElement n3 = e.firstChild().toElement();
	    while ( !n3.isNull() ) {
		if ( n3.tagName() == "string" )
		    l << n3.firstChild().toText().data().simplifyWhiteSpace();
		n3 = n3.nextSibling().toElement();
	    }
	    if ( l.count() )
		v = l.join( "\" << \"" );
	}
    }
    return v;
}



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
		pixmap.prepend( pixmapLoaderFunction + "( " );
		pixmap.append( " )" );
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
