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
#include "parser.h"
#include "widgetdatabase.h"
#include "domtool.h"
#include <qstringlist.h>
#define NO_STATIC_COLORS
#include <globaldefs.h>
#include <zlib.h>

static QByteArray unzipXPM( QString data, ulong& length )
{
    uchar *ba = new uchar[ data.length() / 2 ];
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
    // I'm not sure this makes sense. Why couldn't the compressed data be
    // less than 20% of the original data? Maybe it's enough to trust the
    // `length' passed as an argument. Quoting the zlib header:
    // 		Upon entry, destLen is the total size of the destination
    // 		buffer, which must be large enough to hold the entire
    // 		uncompressed data. (The size of the uncompressed data must
    // 		have been saved previously by the compressor and transmitted
    // 		to the decompressor by some mechanism outside the scope of
    // 		this compression library.)
    // Which is the role of `length'. On the other hand this could prevent
    // crashes in some cases of slightly corrupt UIC files.
    if ( length <  data.length() * 5 )
	length = data.length() * 5;
    QByteArray baunzip( length );
    ::uncompress( (uchar*) baunzip.data(), &length, ba, data.length()/2 );
    delete[] ba;
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

    // at first the images, we need to ensure the names are unique
    QStringList forwardDecl;
    QStringList forwardDecl2;
    QString exportMacro;
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
		    WidgetDatabaseRecord *r = new WidgetDatabaseRecord;
		    while ( !n3.isNull() ) {
			if ( n3.tagName() == "class" ) {
			    forwardDecl << n3.firstChild().toText().data();
			    cl = n3.firstChild().toText().data();
			    r->name = cl;
			} else if ( n3.tagName() == "header" ) {
			    CustomInclude ci;
			    ci.header = n3.firstChild().toText().data();
			    ci.location = n3.attribute( "location", "global" );
			    r->includeFile = ci.header;
			    customWidgetIncludes.insert( cl, ci );
			}
			WidgetDatabase::append( r );
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
    out << "#include <qvariant.h>" << endl; // for broken HP-UX compilers

    QStringList globalIncludes, localIncludes;
    int wid = WidgetDatabase::idFromClassName( objClass );
    {
	QMap<QString, CustomInclude>::Iterator it = customWidgetIncludes.find( objClass );
	if ( it != customWidgetIncludes.end() ) {
	    if ( ( *it ).location == "global" )
		globalIncludes += (*it).header;
	    else
		localIncludes += (*it).header;
	} else {
	    globalIncludes += WidgetDatabase::includeFile( wid );
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
    registerDatabases( e );
    dbConnections = unique( dbConnections );
    if ( dbConnections.count() )
	forwardDecl += "QSqlDatabase";
    if ( dbCursors.count() )
	forwardDecl += "QSqlCursor";
    if ( dbForms[ "(default)" ].count() )
	dbForm = TRUE;
    bool subDbForms = FALSE;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() && (*it) != "(default)" ) {
	    if ( dbForms[ (*it) ].count() ) {
		subDbForms = TRUE;
		break;
	    }
	}
    }
    if ( dbForm || subDbForms )
	forwardDecl += "QSqlForm";

    for ( it = tags.begin(); it != tags.end(); ++it ) {
	nl = e.parentNode().toElement().elementsByTagName( *it );
	for ( i = 1; i < (int) nl.length(); i++ ) { // begin at 1, 0 is the toplevel widget
	    QString s = getClassName( nl.item(i).toElement() );
	    if ( s == "QLayoutWidget" )
		continue; // hide qlayoutwidgets
	    if ( s == "Line" )
		s = "QFrame";
	    forwardDecl += s;
	    if ( s.mid( 1 ) == "ListBox" || s.mid( 1 ) == "ListView" || s.mid( 1 ) == "IconView" )
		forwardDecl += "Q" + s.mid( 1 ) + "Item";
	    if ( s == "QDataTable" ) { // other convenience classes which are used in QDataTable signals, and thus should be forward-declared by uic for us
		forwardDecl += "QSqlRecord";
	    }
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
    nl = e.parentNode().toElement().elementsByTagName( "exportmacro" );
    if ( nl.length() == 1 )
	exportMacro = nl.item( 0 ).firstChild().toText().data();

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
    out << "class " << ( !exportMacro.isEmpty() ? QString( exportMacro + " " ) : QString( "" ) ) <<
	nameOfClass << " : public " << objClass << endl;
    out << "{ " << endl;

/* tmake ignore Q_OBJECT */
    out << "    Q_OBJECT" << endl;
    out << endl;
    out << "public:" << endl;

    // constructor(s)
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );" << endl;
    } else if ( objClass == "QWidget" ) {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );" << endl;
    } else if ( objClass == "QMainWindow" ) {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0, WFlags fl = WType_TopLevel );" << endl;
	isMainWindow = TRUE;
    } else {
	out << "    " << nameOfClass << "( QWidget* parent = 0, const char* name = 0 );" << endl;
    }

    // destructor
    out << "    ~" << nameOfClass << "();" << endl;
    out << endl;

    // children
    bool needEventHandler = FALSE;
    bool needPolish = FALSE;
    nl = e.parentNode().toElement().elementsByTagName( "widget" );
    for ( i = 1; i < (int) nl.length(); i++ ) { // start at 1, 0 is the toplevel widget
	n = nl.item(i).toElement();
	createObjectDecl( n );
	needEventHandler = needEventHandler ||
			   !DomTool::propertiesOfType( n, "font" ).isEmpty() ;
	QString s = getClassName( n );
	if ( s == "QDataTable" || s == "QDataBrowser" || s == "QDataView" ) {
	    if ( isFrameworkCodeGenerated( n ) )
		 needPolish = TRUE;
	}
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
	    out << "    " << "QMenuBar *" << getObjectName( n ) << ";" << endl;

	    for ( QDomElement a = n.firstChild().toElement(); !a.isNull(); a = a.nextSibling().toElement() )
		createMenuBarDecl( a );
	}
    }

    out << endl;

    // database connections
    dbConnections = unique( dbConnections );
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() ) {
	    // only need pointers to non-default connections
	    if ( (*it) != "(default)" && !(*it).isEmpty() )
		out << indent << "QSqlDatabase* " << *it << "Connection;" << endl;
	}
    }

    out << endl;

    // find additional slots
    QStringList publicSlots, protectedSlots;
    QStringList publicSlotTypes, protectedSlotTypes;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "connections" ) {
	    for ( QDomElement n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		if ( n2.tagName() == "slot" ) {
		    QString access = n2.attribute( "access" );
		    if ( n2.attribute( "language", "C++" ) == "C++" ) {
			if ( access == "protected" ) {
			    protectedSlots += n2.firstChild().toText().data();
			    protectedSlotTypes += n2.attribute( "returnType", "void" );
			} else {
			    publicSlots += n2.firstChild().toText().data();
			    publicSlotTypes += n2.attribute( "returnType", "void" );
			}
		    }
		}
	    }
	}
    }

    // create public additional slots as pure-virtual functions
    if ( !publicSlots.isEmpty() || needPolish ) {
	out << "public slots:" << endl;
	QStringList::Iterator it2;
	for ( it = publicSlots.begin(), it2 = publicSlotTypes.begin(); it != publicSlots.end(); ++it, ++it2 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    out << indent << "virtual " << type << " " << (*it) << ";" << endl;
	}
	if ( needPolish )
	    out << indent << "void polish();" << endl;
	out << endl;
    }

    // create protected additional slots as pure-virtual functions
    if ( !protectedSlots.isEmpty() ) {
	out << "protected slots:" << endl;
	QStringList::Iterator it2;
	for ( it = protectedSlots.begin(), it2 = protectedSlotTypes.begin(); it != protectedSlots.end(); ++it, ++it2 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    out << "    virtual " << type << " " << (*it) << ";" << endl;
	}
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

    //## multiple ctor initialization, needed anymore?
    //    out << indent << "void init" << nameOfClass << "();" << endl;

    nl = e.parentNode().toElement().elementsByTagName( "variable" );
    if ( nl.length() > 0 ) {
	if ( !needProtected )
	    out << "protected:" << endl;
	out << endl;
	for ( i = 0; i < (int) nl.length(); i++ ) {
	    QString v = nl.item( i ).firstChild().toText().data();
	    v = v.stripWhiteSpace();
	    if ( v[ (int)v.length() - 1 ] != ';' )
		v += ";";
	    out << indent << v << endl;
	}
    }

    out << "};" << endl;
    out << endl;
    out << "#endif // " << protector << endl;
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

    QMap<QString, CustomInclude> customWidgetIncludes;
    QMap<QString, QString> functionImpls;
    // find additional slots and functions
    QStringList publicSlots, protectedSlots;
    QStringList publicSlotTypes, protectedSlotTypes;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName()  == "connections" ) {
	    for ( QDomElement n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		if ( n2.tagName() == "slot" ) {
		    if ( n2.attribute( "language", "C++" ) == "C++" ) {
			QString access = n2.attribute( "access" );
			if ( access == "protected" ) {
			    protectedSlots += n2.firstChild().toText().data();
			    protectedSlotTypes += n2.attribute( "returnType", "void" );
			} else {
			    publicSlots += n2.firstChild().toText().data();
			    publicSlotTypes += n2.attribute( "returnType", "void" );
			}
		    }
		}
	    }
	} else if ( n.tagName() == "functions" ) {
	    for ( QDomElement n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() ) {
		if ( n2.tagName() == "function" ) {
		    QString fname = n2.attribute( "name" );
		    fname = Parser::cleanArgs( fname );
		    functionImpls.insert( fname, n2.firstChild().toText().data() );
		}
	    }
	} else if ( n.tagName() == "customwidgets" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "customwidget" ) {
		    QDomElement n3 = n2.firstChild().toElement();
		    QString cl;
		    WidgetDatabaseRecord *r = new WidgetDatabaseRecord;
		    while ( !n3.isNull() ) {
			if ( n3.tagName() == "class" ) {
			    cl = n3.firstChild().toText().data();
			    r->name = cl;
			} else if ( n3.tagName() == "header" ) {
			    CustomInclude ci;
			    ci.header = n3.firstChild().toText().data();
			    ci.location = n3.attribute( "location", "global" );
			    r->includeFile = ci.header;
			    customWidgetIncludes.insert( cl, ci );
			}
			WidgetDatabase::append( r );
			n3 = n3.nextSibling().toElement();
		    }
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
    }

    // additional includes (local or global ) and forward declaractions
    nl = e.parentNode().toElement().elementsByTagName( "include" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	QDomElement n2 = nl.item(i).toElement();
	QString s = n2.firstChild().toText().data();
	if ( n2.attribute( "impldecl", "in implementation" ) == "in implementation" &&
	     n2.attribute( "location" ) != "local" )
	    globalIncludes += s;
    }

    registerDatabases( e );
    dbConnections = unique( dbConnections );
    if ( dbConnections.count() )
	globalIncludes += "qsqldatabase.h";
    if ( dbCursors.count() )
	globalIncludes += "qsqlcursor.h";
    bool dbForm = FALSE;
    if ( dbForms[ "(default)" ].count() )
	dbForm = TRUE;
    bool subDbForms = FALSE;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty()  && (*it) != "(default)" ) {
	    if ( dbForms[ (*it) ].count() ) {
		subDbForms = TRUE;
		break;
	    }
	}
    }
    if ( dbForm || subDbForms ) {
	globalIncludes += "qsqlform.h";
	globalIncludes += "qsqlrecord.h";
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
	nl = e.parentNode().toElement().elementsByTagName( *it );
	for ( i = 1; i < (int) nl.length(); i++ ) { // start at 1, 0 is the toplevel widget
	    QString name = getClassName( nl.item(i).toElement() );
	    if ( name.mid( 1 ) == "ListView" )
		globalIncludes += "qheader.h";
	    if ( name != objClass ) {
		int wid = WidgetDatabase::idFromClassName( name );
		QMap<QString, CustomInclude>::Iterator it = customWidgetIncludes.find( name );
		if ( it == customWidgetIncludes.end() )
		    globalIncludes += WidgetDatabase::includeFile( wid );
	    }
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

    if ( externPixmaps ) {
	out << "#include <qmime.h>" << endl;
	out << "#include <qdragobject.h>" << endl;
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

    if ( !requiredImages.isEmpty() || externPixmaps ) {
	out << "#include <qimage.h>" << endl;
	out << "#include <qpixmap.h>" << endl << endl;
    }

    // register the object and unify its name
    objName = registerObject( objName );

    QStringList images;
    QStringList xpmImages;
    if ( pixmapLoaderFunction.isEmpty() && !externPixmaps ) {
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
			// shouldn't we test the initial `length' against the
			// resulting `length' to catch corrupt UIC files?
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
    } else if ( externPixmaps ) {
	out << "static QPixmap uic_load_pixmap_" << objName << "( const QString &name )" << endl;
	out << "{" << endl;
	out << "    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );" << endl;
	out << "    if ( !m )" << endl;
	out << "\treturn QPixmap();" << endl;
	out << "    QPixmap pix;" << endl;
	out << "    QImageDrag::decode( m, pix );" << endl;
	out << "    return pix;" << endl;
	out << "}" << endl;
	pixmapLoaderFunction = "uic_load_pixmap_" + objName;
    }


    // constructor(s)
    if ( objClass == "QDialog" || objClass == "QWizard" ) {
	out << "/* " << endl;
	out << " *  Constructs a " << nameOfClass << " which is a child of 'parent', with the " << endl;
	out << " *  name 'name' and widget flags set to 'f'." << endl;
	out << " *" << endl;
	out << " *  The " << objClass.mid(1).lower() << " will by default be modeless, unless you set 'modal' to" << endl;
	out << " *  TRUE to construct a modal " << objClass.mid(1).lower() << "." << endl;
	out << " */" << endl;
	out << nameOfClass << "::" << nameOfClass << "( QWidget* parent,  const char* name, bool modal, WFlags fl )" << endl;
	out << "    : " << objClass << "( parent, name, modal, fl )" << endl;
    } else if ( objClass == "QWidget" )  {
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
    if ( isMainWindow )
	out << indent << "(void)statusBar();" << endl;


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
		out << "    if ( !name )" << endl;
		out << "\t";
	    } else {
		out << indent;
	    }
	    if ( prop == "sort" ) {
		QString defaultTable = getDatabaseInfo( e, "table" );
		if ( !defaultTable.isEmpty() ) {
		    out << "QStringList " << defaultTable << "Sort;" << endl;
		    out << indent << defaultTable << "Sort << \"" << value << "\";" << endl;
		    out << indent;
		    value = defaultTable + "Sort";
		}
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
		    out << mkStdSet( prop ) << "( " << value << " );" << endl;
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
		out << indent << "addPage( " << page << ", "<< trcall( label ) << " );" << endl;
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
    dbConnections = unique( dbConnections );
    if ( dbConnections.count() )
	out << endl << indent << "// database support" << endl;
    for ( it = dbConnections.begin(); it != dbConnections.end(); ++it ) {
	if ( !(*it).isEmpty() && (*it) != "(default)") {
	    out << indent << (*it) << "Connection = QSqlDatabase::database( \"" <<(*it) << "\" );" << endl;
	}
    }

    nl = e.parentNode().toElement().elementsByTagName( "widget" );
    for ( i = 1; i < (int) nl.length(); i++ ) { // start at 1, 0 is the toplevel widget
	n = nl.item(i).toElement();
	QString s = getClassName( n );
	if ( s == "QDataBrowser" || s == "QDataView" ) {
	    QString objName = getObjectName( n );
	    QString tab = getDatabaseInfo( n, "table" );
	    QString con = getDatabaseInfo( n, "connection" );
	    out << indent << "QSqlForm* " << objName << "Form =  new QSqlForm( this, \"" << objName << "Form\" );" << endl;
	    QDomElement n2;
	    for ( n2 = n.firstChild().toElement(); !n2.isNull(); n2 = n2.nextSibling().toElement() )
		createFormImpl( n2, objName, con, tab );
	    out << indent << objName << "->setForm( " << objName << "Form );" << endl;
	}
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
	    createToolbarImpl( n, objClass, objName );
    }
    out << endl;
    for ( n = e; !n.isNull(); n = n.nextSibling().toElement() ) {
	if ( n.tagName() == "menubar" )
	    createMenuBarImpl( n, objClass, objName );
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

    if ( protectedSlots.find( "init()" ) != protectedSlots.end() )
	out << indent << "init();" << endl;

    // end of constructor
    out << "}" << endl;
    out << endl;

    // destructor
    out << "/*  " << endl;
    out << " *  Destroys the object and frees any allocated resources" << endl;
    out << " */" << endl;
    out << nameOfClass << "::~" << nameOfClass << "()" << endl;
    out << "{" << endl;
    if ( protectedSlots.find( "destroy()" ) != protectedSlots.end() )
	out << indent << "destroy();" << endl;
    out << indent << "// no need to delete child widgets, Qt does it all for us" << endl;
    out << "}" << endl;
    out << endl;

    // handle application events if required
    bool needFontEventHandler = FALSE;
    bool needSqlTableEventHandler = FALSE;
    bool needSqlDataBrowserEventHandler = FALSE;
    nl = e.elementsByTagName( "widget" );
    for ( i = 0; i < (int) nl.length(); i++ ) {
	if ( !DomTool::propertiesOfType( nl.item(i).toElement() , "font" ).isEmpty() )
	    needFontEventHandler = TRUE;
	QString s = getClassName( nl.item(i).toElement() );
	if ( s == "QDataTable" || s == "QDataBrowser" ) {
	    if ( !isFrameworkCodeGenerated( nl.item(i).toElement() ) )
		 continue;
	    if ( s == "QDataTable" )
		needSqlTableEventHandler = TRUE;
	    if ( s == "QDataBrowser" )
		needSqlDataBrowserEventHandler = TRUE;
	}
	if ( needFontEventHandler && needSqlTableEventHandler && needSqlDataBrowserEventHandler )
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

    if ( needSqlTableEventHandler || needSqlDataBrowserEventHandler ) {
	out << "/*  " << endl;
	out << " *  Widget polish.  Reimplemented to handle" << endl;
	if ( needSqlTableEventHandler )
	    out << " *  default data table initialization" << endl;
	if ( needSqlDataBrowserEventHandler )
	    out << " *  default data browser initialization" << endl;
	out << " */" << endl;
	out << "void " << nameOfClass  << "::polish()" << endl;
	out << "{" << endl;
	if ( needSqlTableEventHandler ) {
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString s = getClassName( nl.item(i).toElement() );
		if ( s == "QDataTable" ) {
		    n = nl.item(i).toElement();
		    QString c = getObjectName( n );
		    QString conn = getDatabaseInfo( n, "connection" );
		    QString tab = getDatabaseInfo( n, "table" );
		    if ( !( conn.isEmpty() || tab.isEmpty() ) ) {
			out << indent << "if ( " << c << " ) {" << endl;
			out << indent << indent << "QSqlCursor* cursor = " << c << "->sqlCursor();" << endl;
			out << indent << indent << "if ( !cursor ) {" << endl;
			if ( conn == "(default)" )
			    out << indent << indent << indent << "cursor = new QSqlCursor( \"" << tab << "\" );" << endl;
			else
			    out << indent << indent << indent << "cursor = new QSqlCursor( \"" << tab << "\", " << conn << "Connection );" << endl;
			out << indent << indent << indent << c << "->setCursor( cursor, FALSE, TRUE );" << endl;
			out << indent << indent << "}" << endl;
			out << indent << indent << "if ( !cursor->isActive() )" << endl;
			out << indent << indent << indent << c << "->refresh( QDataTable::RefreshAll );" << endl;
			out << indent << "}" << endl;
		    }
		}
	    }
	}
	if ( needSqlDataBrowserEventHandler ) {
	    nl = e.elementsByTagName( "widget" );
	    for ( i = 0; i < (int) nl.length(); i++ ) {
		QString s = getClassName( nl.item(i).toElement() );
		if ( s == "QDataBrowser" ) {
		    QString obj = getObjectName( nl.item(i).toElement() );
		    QString tab = getDatabaseInfo( nl.item(i).toElement(),
						   "table" );
		    QString conn = getDatabaseInfo( nl.item(i).toElement(),
						    "connection" );
		    if ( !(tab).isEmpty() ) {
			out << indent << "if ( " << obj << " ) {" << endl;
			out << indent << indent << "if ( !" << obj << "->sqlCursor() ) {" << endl;
			if ( conn == "(default)" )
			    out << indent << indent << indent << "QSqlCursor* cursor = new QSqlCursor( \"" << tab << "\" );" << endl;
			else
			    out << indent << indent << indent << "QSqlCursor* cursor = new QSqlCursor( \"" << tab << "\", " << conn << "Connection );" << endl;
			out << indent << indent << indent << obj << "->setCursor( cursor, TRUE );" << endl;
			out << indent << indent << indent << obj << "->refresh();" << endl;
			out << indent << indent << indent << obj << "->first();" << endl;
			out << indent << indent << "}" << endl;
			out << indent << "}" << endl;
		    }
		}
	    }
	}
	out << indent << objClass << "::polish();" << endl;
	out << "}" << endl;
	out << endl;
    }


    // create public additional slots as pure-virtual functions
    if ( !publicSlots.isEmpty() ) {
	QStringList::Iterator it2;
	for ( it = publicSlots.begin(), it2 = publicSlotTypes.begin(); it != publicSlots.end(); ++it, ++it2 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    out << type << " " << nameOfClass << "::" << (*it) << endl;
	    bool createWarning = TRUE;
	    QString fname = Parser::cleanArgs( *it );
	    QMap<QString, QString>::Iterator fit = functionImpls.find( fname );
	    if ( fit != functionImpls.end() ) {
		int begin = (*fit).find( "{" );
		QString body = (*fit).mid( begin + 1, (*fit).find( "}" ) - begin - 1 );
		createWarning = body.simplifyWhiteSpace().isEmpty();
		if ( !createWarning )
		    out << *fit << endl;
	    }
	    if ( createWarning ) {
		out << "{" << endl;
		out << "    qWarning( \"" << nameOfClass << "::" << (*it) << ": Not implemented yet!\" );" << endl;
		out << "}" << endl;
	    }
	    out << endl;
	}
    }

    // create protected additional slots as pure-virtual functions
    if ( !protectedSlots.isEmpty() ) {
	QStringList::Iterator it2;
	for ( it = protectedSlots.begin(), it2 = protectedSlotTypes.begin(); it != protectedSlots.end(); ++it, ++it2 ) {
	    QString type = *it2;
	    if ( type.isEmpty() )
		type = "void";
	    out << type << " " << nameOfClass << "::" << (*it) << endl;
	    bool createWarning = TRUE;
	    QString fname = Parser::cleanArgs( *it );
	    QMap<QString, QString>::Iterator fit = functionImpls.find( fname );
	    if ( fit != functionImpls.end() ) {
		int begin = (*fit).find( "{" );
		QString body = (*fit).mid( begin + 1, (*fit).find( "}" ) - begin - 1 );
		createWarning = body.simplifyWhiteSpace().isEmpty();
		if ( !createWarning )
		    out << *fit << endl;
	    }
	    if ( createWarning ) {
		out << "{" << endl;
		if ( *it != "init()" && *it != "destroy()" )
		    out << "    qWarning( \"" << nameOfClass << "::" << (*it) << ": Not implemented yet!\" );" << endl;
		out << "}" << endl;
	    }
	    out << endl;
	}
    }
}


/*! Creates form support implementation code for the widgets given
  in \a e.

  Traverses recursively over all children.
 */

void Uic::createFormImpl( const QDomElement& e, const QString& form, const QString& connection, const QString& table )
{
    if ( e.tagName() == "widget" &&
	 e.attribute( "class" ) != "QDataTable" ) {
	QString field = getDatabaseInfo( e, "field" );
	if ( !field.isEmpty() ) {
	    if ( isWidgetInTable( e, connection, table ) )
		out << indent << form << "Form->insert( " << getObjectName( e ) << ", " << fixString( field ) << " );" << endl;
	}
    }
    QDomElement n;
    for ( n = e.firstChild().toElement(); !n.isNull(); n = n.nextSibling().toElement() ) {
	createFormImpl( n, form, connection, table );
    }
}
