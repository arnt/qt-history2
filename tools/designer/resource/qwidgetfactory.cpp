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

#include "../interfaces/eventinterface.h"
#include "../interfaces/interpreterinterface.h"
#include "../interfaces/languageinterface.h"
#include "../interfaces/widgetinterface.h"

#include "qwidgetfactory.h"
#include <qfeatures.h>
#include "../integration/kdevelop/kdewidgets.h"
#include "../designer/config.h"
#include "../designer/database.h"
#include <qdom.h>
#include <qfile.h>
#include <qlayout.h>
#include <qmetaobject.h>
#include <domtool.h>
#include <qapplication.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <zlib.h>
#include <qobjectlist.h>
#include <stdlib.h>
#include <qinterfacemanager.h>
#include <qmime.h>
#include <qdragobject.h>

#ifndef QT_NO_SQL
#include <qsqlrecord.h>
#include <qsqldatabase.h>
#include <qdatatable.h>
#include <qdatetimeedit.h>
#endif

// include all Qt widgets we support
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qiconview.h>
#include <qheader.h>
#ifndef QT_NO_TABLE
#include <qtable.h>
#endif
#include <qlistbox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qmultilineedit.h>
#include <qlabel.h>
#include <qwidget.h>
#include <qtabwidget.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qwizard.h>
#include <qlcdnumber.h>
#include <qprogressbar.h>
#include <qtextview.h>
#include <qtextbrowser.h>
#include <qdial.h>
#include <qslider.h>
#include <qframe.h>
#include <qwidgetstack.h>
#include <qtextedit.h>
#include <qscrollbar.h>
#include <qmainwindow.h>
#include <qsplitter.h>
#include <qaction.h>
#include <qpopupmenu.h>
#include <qmenubar.h>

static QList<QWidgetFactory> widgetFactories;
static QInterfaceManager<EventInterface> *eventInterfaceManager = 0;
static QInterfaceManager<InterpreterInterface> *interpreterInterfaceManager = 0;
static QInterfaceManager<LanguageInterface> *languageInterfaceManager = 0;
static QInterfaceManager<WidgetInterface> *widgetInterfaceManager = 0;

QMap<QWidget*, QString> *qwf_functions = 0;
QMap<QWidget*, QString> *qwf_forms = 0;
QString *qwf_language = 0;
bool qwf_execute_code = TRUE;
bool qwf_stays_on_top = FALSE;

/*!
  \class QWidgetFactory

  \brief A class to dynamically create widgets from Qt Designer user
  interface description files

  This class basically offers two things:

  <ul>

  <li>Dynamically creating widgets from Qt Designer user interface
  description files. You can do that using the static function
  QWidgetFactory::create(). This function also performs signal and
  slot connections, tab ordering, etc. as defined in the ui file and
  returns the toplevel widget of the ui file. After that you can use
  QObject::child() and QObject::queryList() to access child widgets of
  this returned widget.

  <li>Adding additional widget factories to be able to create custom
  widgets. See createWidget() for details.

  </ul>
*/

/*! Constructs a QWidgetFactory. */

QWidgetFactory::QWidgetFactory()
    : dbControls( 0 ), usePixmapCollection( FALSE )
{
    widgetFactories.setAutoDelete( TRUE );
}

/*! Loads the Qt Designer user interface description file \a uiFile
  and returns the toplevel widget of that description. \a parent and
  \a name are passed to the constructor of the toplevel widget.

  This function also performs signal and slot connections, tab
  ordering, etc. as described in the ui file. In the Qt Designer it is
  possible to add custom slots to a form and connect to them. If you
  want that these connections are performed as well, you have to
  create a class derived from QObject, which implementes all these
  slots. Then pass an instance of it as \a connector to this
  function. This way these connections to custom slots will be done
  using the \a connector as slot.

  If something fails, 0 is returned.

  The ownership of the returned widget is passed to the caller.
*/

QWidget *QWidgetFactory::create( const QString &uiFile, QObject *connector, QWidget *parent, const char *name )
{
    QFile f( uiFile );
    if ( !f.open( IO_ReadOnly ) )
	return 0;

    QWidget *w = QWidgetFactory::create( &f, connector, parent, name );
    if ( !qwf_forms )
	qwf_forms = new QMap<QWidget*, QString>;
    qwf_forms->insert( w, uiFile );
    return w;
}

/*!  \overload
 */

QWidget *QWidgetFactory::create( QIODevice *dev, QObject *connector, QWidget *parent, const char *name )
{
    QDomDocument doc;
    if ( !doc.setContent( dev ) )
	return 0;

    DomTool::fixDocument( doc );

    QWidgetFactory *widgetFactory = new QWidgetFactory;
    widgetFactory->toplevel = 0;

    QDomElement firstWidget = doc.firstChild().toElement().firstChild().toElement();

    while ( firstWidget.tagName() != "widget" ) {
	if ( firstWidget.tagName() == "variable" )
	    widgetFactory->variables << firstWidget.firstChild().toText().data();
	else if ( firstWidget.tagName() == "pixmapinproject" )
	    widgetFactory->usePixmapCollection = TRUE;
	firstWidget = firstWidget.nextSibling().toElement();
    }

    QDomElement connections = firstWidget;
    while ( connections.tagName() != "connections" && !connections.isNull() )
	connections = connections.nextSibling().toElement();

    QDomElement imageCollection = firstWidget;
    while ( imageCollection.tagName() != "images" && !imageCollection.isNull() )
	imageCollection = imageCollection.nextSibling().toElement();

    QDomElement tabOrder = firstWidget;
    while ( tabOrder.tagName() != "tabstops" && !tabOrder.isNull() )
	tabOrder = tabOrder.nextSibling().toElement();

    QDomElement actions = firstWidget;
    while ( actions.tagName() != "actions" && !actions.isNull() )
	actions = actions.nextSibling().toElement();

    QDomElement toolbars = firstWidget;
    while ( toolbars.tagName() != "toolbars" && !toolbars.isNull() )
	toolbars = toolbars.nextSibling().toElement();

    QDomElement menubar = firstWidget;
    while ( menubar.tagName() != "menubar" && !menubar.isNull() )
	menubar = menubar.nextSibling().toElement();

    QDomElement functions = firstWidget;
    while ( functions.tagName() != "functions" && !functions.isNull() )
	functions = functions.nextSibling().toElement();

    if ( !imageCollection.isNull() )
	widgetFactory->loadImageCollection( imageCollection );

    widgetFactory->createWidgetInternal( firstWidget, parent, 0, firstWidget.attribute("class", "QWidget") );
    QWidget *w = widgetFactory->toplevel;
    if ( !w ) {
	delete widgetFactory;
	return 0;
    }

    if ( !actions.isNull() )
	widgetFactory->loadActions( actions );
    if ( !toolbars.isNull() )
	widgetFactory->loadToolBars( toolbars );
    if ( !menubar.isNull() )
	widgetFactory->loadMenuBar( menubar );

    if ( w && name && qstrlen( name ) > 0 )
	w->setName( name );

    if ( !connections.isNull() )
	widgetFactory->loadConnections( connections, connector );
    if ( !tabOrder.isNull() )
	widgetFactory->loadTabOrder( tabOrder );
    if ( !functions.isNull() )
	widgetFactory->loadFunctions( functions );

    if ( widgetFactory->toplevel ) {
#ifndef QT_NO_SQL
	QMap<QWidget*, SqlWidgetConnection>::Iterator cit = widgetFactory->sqlWidgetConnections.begin();
	for( ; cit != widgetFactory->sqlWidgetConnections.end(); ++cit ) {
	    if ( widgetFactory->noDatabaseWidgets.find( cit.key()->name() ) != widgetFactory->noDatabaseWidgets.end() )
		continue;
	    if ( cit.key()->inherits( "QDesignerDataBrowser" ) )
		( (QDesignerDataBrowser*)cit.key() )->initPreview( (*cit).conn, (*cit).table, cit.key(), *(*cit).dbControls );
	    else if ( cit.key()->inherits( "QDesignerDataView" ) )
		( (QDesignerDataView*)cit.key() )->initPreview( (*cit).conn, (*cit).table, cit.key(), *(*cit).dbControls );
	}

	for ( QMap<QString, QStringList>::Iterator it = widgetFactory->dbTables.begin(); it != widgetFactory->dbTables.end(); ++it ) {
	    QDataTable *table = (QDataTable*)widgetFactory->toplevel->child( it.key(), "QDataTable" );
	    if ( !table )
		continue;
	    if ( widgetFactory->noDatabaseWidgets.find( table->name() ) != widgetFactory->noDatabaseWidgets.end() )
		continue;
	    QValueList<Field> fieldMap = *widgetFactory->fieldMaps.find( table );
	    QString conn = (*it)[ 0 ];
	    QSqlCursor* c = 0;
	    QSqlDatabase *db = 0;
	    if ( conn.isEmpty() || conn == "(default)" ) {
		db = QSqlDatabase::database();
		c = new QSqlCursor( (*it)[ 1 ] );
	    } else {
		db = QSqlDatabase::database( conn );
		c = new QSqlCursor( (*it)[ 1 ], TRUE, db );
	    }
	    if ( db ) {
		table->setCursor( c, fieldMap.isEmpty(), TRUE );
		table->refresh( QDataTable::RefreshAll );
	    }
	}
#endif

	QString dir = getenv( "QTDIR" );
	dir += "/plugins/designer";
	if ( !eventInterfaceManager )
	    eventInterfaceManager = new QInterfaceManager<EventInterface>( IID_EventInterface, dir );
	if ( !interpreterInterfaceManager )
	    interpreterInterfaceManager = new QInterfaceManager<InterpreterInterface>( IID_InterpreterInterface, dir );
	if ( !languageInterfaceManager )
	    languageInterfaceManager = new QInterfaceManager<LanguageInterface>( IID_LanguageInterface, dir );

	if ( eventInterfaceManager && interpreterInterfaceManager && languageInterfaceManager ) {
	    QStringList langs = languageInterfaceManager->featureList();
	    for ( QStringList::Iterator lit = langs.begin(); lit != langs.end(); ++lit ) {
		EventInterface *eventInterface = (EventInterface*)eventInterfaceManager->queryInterface( *lit );
		InterpreterInterface *interpreterInterface = (InterpreterInterface*)interpreterInterfaceManager->queryInterface( *lit );
		if ( eventInterface && interpreterInterface ) {
		    QMap<QString, Functions*>::Iterator fit = widgetFactory->languageFunctions.find( *lit );
		    if ( fit != widgetFactory->languageFunctions.end() ) {
			QString funcs = (*fit)->functions;
			funcs += "\n";
			for ( QStringList::Iterator vit = widgetFactory->variables.begin(); vit != widgetFactory->variables.end(); ++vit )
			    funcs += interpreterInterface->createVariableDeclaration( *vit ) + "\n";
			funcs += "init();\n";
			if ( qwf_execute_code )
			    interpreterInterface->exec( widgetFactory->toplevel, funcs );
		    }
		    if ( widgetFactory->languageFunctions.isEmpty() && qwf_execute_code )
			interpreterInterface->exec( widgetFactory->toplevel, "dummy=0;" );
		    for ( QMap<QObject *, EventFunction>::Iterator it = widgetFactory->eventMap.begin();
			  it != widgetFactory->eventMap.end(); ++it ) {
			QStringList::Iterator eit;
			QValueList<QStringList>::Iterator fit;
			for ( eit = (*it).events.begin(), fit = (*it).functions.begin(); eit != (*it).events.end(); ++eit, ++fit ) {
			    QStringList funcs = *fit;
			    for ( QStringList::Iterator fit2 = funcs.begin(); fit2 != funcs.end(); ++fit2 ) {
				if ( widgetFactory->languageSlots.find( *fit2 ) != widgetFactory->languageSlots.end() && qwf_execute_code )
				    eventInterface->addEventHandler( it.key(), widgetFactory->toplevel, *eit, *fit2 );
			    }
			}
		    }
		    eventInterface->release();
		    interpreterInterface->release();
		}
	    }
	}

    }

    for ( QMap<QString, QString>::Iterator it = widgetFactory->buddies.begin(); it != widgetFactory->buddies.end(); ++it ) {
	QLabel *label = (QLabel*)widgetFactory->toplevel->child( it.key(), "QLabel" );
	QWidget *buddy = (QWidget*)widgetFactory->toplevel->child( *it, "QWidget" );
	if ( label && buddy )
	    label->setBuddy( buddy );
    }

    delete widgetFactory;

    return w;
}

/*! Installs a widget factory \a factory, which normally contains
  additional widgets that can be created using a QWidgetFactory
  then. See createWidget() for further details.
*/

void QWidgetFactory::addWidgetFactory( QWidgetFactory *factory )
{
    widgetFactories.append( factory );
}

/*!  Creates the widget of the type \c className passing \a parent and
  \a name to its constructor. If \a className is a widget of the Qt
  library, it is directly created in this function. if this fails, all
  installed widget plugins are asked to create that widget. If this
  fails all installed widget factories are asked to create it (see
  addWidgetFactory()). If this fails as well, 0 is returned.

  If you have a custom widget, and want it to be created using the
  widget factory, you have two possibilities to add it:

  <ul>

  <li>First you can write a widget plugin. This allows you to use that
  widget in the Qt Designer and in this QWidgetFactory. See the widget
  plugin documentation for further details .

  <li>The other possibility is to subclass QWidgetFactory. Then
  reimplement this function. There create and return an instance of
  your custom widget, if \a className equals the name of your widget,
  otherwise return 0. Then at the beginning of your program where you
  want to use the widget factory to create widgets do a

  \code
  QWidgetFactory::addWidgetFactory( new MyWidgetFactory );
  \endcode

  (where MyWidgetFactory is your QWidgetFactory subclass)

  </ul>

*/

QWidget *QWidgetFactory::createWidget( const QString &className, QWidget *parent, const char *name ) const
{
    // create widgets we know
    if ( className == "QPushButton" ) {
	return new QPushButton( parent, name );
    } else if ( className == "QToolButton" ) {
	return new QToolButton( parent, name );
    } else if ( className == "QCheckBox" ) {
	return new QCheckBox( parent, name );
    } else if ( className == "QRadioButton" ) {
	return new QRadioButton( parent, name );
    } else if ( className == "QGroupBox" ) {
	return new QGroupBox( parent, name );
    } else if ( className == "QButtonGroup" ) {
	return new QButtonGroup( parent, name );
    } else if ( className == "QIconView" ) {
#if !defined(QT_NO_ICONVIEW)
	return new QIconView( parent, name );
#endif
    } else if ( className == "QTable" ) {
#if !defined(QT_NO_TABLE)
	return new QTable( parent, name );
#endif
    } else if ( className == "QListBox" ) {
	return new QListBox( parent, name );
    } else if ( className == "QListView" ) {
	return new QListView( parent, name );
    } else if ( className == "QLineEdit" ) {
	return new QLineEdit( parent, name );
    } else if ( className == "QSpinBox" ) {
	return new QSpinBox( parent, name );
    } else if ( className == "QMultiLineEdit" ) {
	return new QMultiLineEdit( parent, name );
    } else if ( className == "QLabel"  || className == "TextLabel" || className == "PixmapLabel" ) {
	return new QLabel( parent, name );
    } else if ( className == "QLayoutWidget" ) {
	return new QWidget( parent, name );
    } else if ( className == "QTabWidget" ) {
	return new QTabWidget( parent, name );
    } else if ( className == "QComboBox" ) {
	return new QComboBox( FALSE, parent, name );
    } else if ( className == "QWidget" ) {
	if ( !qwf_stays_on_top )
	    return new QWidget( parent, name );
	return new QWidget( parent, name, Qt::WStyle_StaysOnTop );
    } else if ( className == "QDialog" ) {
	if ( !qwf_stays_on_top )
	    return new QDialog( parent, name );
	return new QDialog( parent, name, FALSE, Qt::WStyle_StaysOnTop );
    } else if ( className == "QWizard" ) {
	return  new QWizard( parent, name );
    } else if ( className == "QLCDNumber" ) {
	return new QLCDNumber( parent, name );
    } else if ( className == "QProgressBar" ) {
	return new QProgressBar( parent, name );
    } else if ( className == "QTextView" ) {
	return new QTextView( parent, name );
    } else if ( className == "QTextBrowser" ) {
	return new QTextBrowser( parent, name );
    } else if ( className == "QDial" ) {
	return new QDial( parent, name );
    } else if ( className == "QSlider" ) {
	return new QSlider( parent, name );
    } else if ( className == "QFrame" ) {
	return new QFrame( parent, name );
    } else if ( className == "QSplitter" ) {
	return new QSplitter( parent, name );
    } else if ( className == "Line" ) {
	QFrame *f = new QFrame( parent, name );
	f->setFrameStyle( QFrame::HLine | QFrame::Sunken );
	return f;
    } else if ( className == "QTextEdit" ) {
	return new QTextEdit( parent, name );
    } else if ( className == "QScrollBar" ) {
	return new QScrollBar( parent, name );
    } else if ( className == "QMainWindow" ) {
	QMainWindow *mw = 0;
	if ( !qwf_stays_on_top )
	    mw = new QMainWindow( parent, name );
	else
	    mw = new QMainWindow( parent, name, Qt::WType_TopLevel | Qt::WStyle_StaysOnTop );
	mw->setCentralWidget( new QWidget( mw, "qt_central_widget" ) );
	mw->centralWidget()->show();
	(void)mw->statusBar();
	return mw;

    }
#if !defined(QT_NO_SQL)
    else if ( className == "QDataTable" ) {
	return new QDataTable( parent, name );
    } else if ( className == "QDateEdit" ) {
	return new QDateEdit( parent, name );
    } else if ( className == "QTimeEdit" ) {
	return new QTimeEdit( parent, name );
    } else if ( className == "QDateTimeEdit" ) {
	return new QDateTimeEdit( parent, name );
    } else if ( className == "QDataBrowser" ) {
	return new QDesignerDataBrowser( parent, name );
    } else if ( className == "QDataView" ) {
	return new QDesignerDataView( parent, name );
    }
#endif

    // maybe it is a KDE widget we support
    QWidget *w = qt_create_kde_widget( className, parent, name, FALSE );
    if ( w )
	return w;

    // try to create it using the loaded widget plugins
    if ( !widgetInterfaceManager ) {
	QString dir = getenv( "QTDIR" );
	dir += "/plugins/designer";
	widgetInterfaceManager = new QInterfaceManager<WidgetInterface>( IID_WidgetInterface, dir );
    }
    WidgetInterface *iface = widgetInterfaceManager->queryInterface( className );
    if ( iface ) {
	w = iface->create( className, parent, name );
	iface->release();
	if ( w )
	    return w;
    }

    // hope we have a factory which can do it
    for ( QWidgetFactory* f = widgetFactories.first(); f; f = widgetFactories.next() ) {
	QWidget *w = f->createWidget( className, parent, name );
	if ( w )
	    return w;
    }

    // no success
    return 0;
}

QWidget *QWidgetFactory::createWidgetInternal( const QDomElement &e, QWidget *parent, QLayout* layout, const QString &classNameArg )
{
    lastItem = 0;
    QDomElement n = e.firstChild().toElement();
    QWidget *w = 0; // the widget that got created
    QObject *obj = 0; // gets the properties

    QString className = classNameArg;

    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();
    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;
    if ( !className.isEmpty() ) {
	if ( !layout && className  == "QLayoutWidget" )
	    className = "QWidget";
	if ( layout && className == "QLayoutWidget" ) {
	    // hide layout widgets
	    w = parent;
	} else {
	    obj = QWidgetFactory::createWidget( className, parent, 0 );
	    if ( !obj )
		return 0;
	    w = (QWidget*)obj;
	    if ( !toplevel )
		toplevel = w;
	    if ( w->inherits( "QMainWindow" ) )
		w = ( (QMainWindow*)w )->centralWidget();
	    if ( layout ) {
		switch( layoutType( layout ) ) {
		case HBox:
		    ( (QHBoxLayout*)layout )->addWidget( w );
		    break;
		case VBox:
		    ( (QVBoxLayout*)layout )->addWidget( w );
		    break;
		case Grid:
		    ( (QGridLayout*)layout )->addMultiCellWidget( w, row, row + rowspan - 1,
								  col, col + colspan - 1 );
		    break;
		default:
		    break;
		}
	    }

	    layout = 0;
	}
    }

    EventFunction ef;

    while ( !n.isNull() ) {
	if ( n.tagName() == "spacer" ) {
	    createSpacer( n, layout );
	} else if ( n.tagName() == "widget" ) {
	    QMap< QString, QString> *oldDbControls = dbControls;
	    createWidgetInternal( n, w, layout, n.attribute( "class", "QWidget" ) );
	    dbControls = oldDbControls;
	} else if ( n.tagName() == "hbox" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, QWidgetFactory::HBox );
	    else
		layout = createLayout( w, layout, QWidgetFactory::HBox );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row, row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "grid" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, QWidgetFactory::Grid );
	    else
		layout = createLayout( w, layout, QWidgetFactory::Grid );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row, row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "vbox" ) {
	    QLayout *parentLayout = layout;
	    if ( layout && layout->inherits( "QGridLayout" ) )
		layout = createLayout( 0, 0, QWidgetFactory::VBox );
	    else
		layout = createLayout( w, layout, QWidgetFactory::VBox );
	    obj = layout;
	    n = n.firstChild().toElement();
	    if ( parentLayout && parentLayout->inherits( "QGridLayout" ) )
		( (QGridLayout*)parentLayout )->addMultiCellLayout( layout, row, row + rowspan - 1, col, col + colspan - 1 );
	    continue;
	} else if ( n.tagName() == "property" && obj ) {
	    setProperty( obj, n.attribute( "name" ), n.firstChild().toElement() );
	} else if ( n.tagName() == "attribute" && w ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( parent->inherits( "QTabWidget" ) ) {
		if ( attrib == "title" )
		    ( (QTabWidget*)parent )->insertTab( w, v.toString() );
	    } else if ( parent->inherits( "QWizard" ) ) {
		if ( attrib == "title" )
		    ( (QWizard*)parent )->addPage( w, v.toString() );
	    }
	} else if ( n.tagName() == "item" ) {
	    createItem( n, w );
	} else if ( n.tagName() == "column" || n.tagName() == "row" ) {
	    createColumn( n, w );
	} else if ( n.tagName() == "event" ) {
	    ef.events.append( n.attribute( "name" ) );
	    ef.functions.append( QStringList::split( ',', n.attribute( "functions" ) ) );
	}

	n = n.nextSibling().toElement();
    }

    eventMap.insert( obj, ef );

    return w;
}

#define BOXLAYOUT_DEFAULT_MARGIN 11
#define BOXLAYOUT_DEFAULT_SPACING 6

QLayout *QWidgetFactory::createLayout( QWidget *widget, QLayout*  layout, LayoutType type )
{
    int spacing = BOXLAYOUT_DEFAULT_SPACING;
    int margin = BOXLAYOUT_DEFAULT_MARGIN;

    if ( !layout && widget && widget->inherits( "QTabWidget" ) )
	widget = ((QTabWidget*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWizard" ) )
	widget = ((QWizard*)widget)->currentPage();

    if ( !layout && widget && widget->inherits( "QWidgetStack" ) )
	widget = ((QWidgetStack*)widget)->visibleWidget();

    if ( !layout && widget && widget->inherits( "QGroupBox" ) ) {
	QGroupBox *gb = (QGroupBox*)widget;
	gb->setColumnLayout( 0, Qt::Vertical );
	gb->layout()->setMargin( 0 );
	gb->layout()->setSpacing( 0 );
	QLayout *l;
	switch ( type ) {
	case HBox:
	    l = new QHBoxLayout( gb->layout() );
	    l->setAlignment( Qt::AlignTop );
	    return l;
	case VBox:
	    l = new QVBoxLayout( gb->layout(), spacing );
	    l->setAlignment( Qt::AlignTop );
	    return l;
	case Grid:
	    l = new QGridLayout( gb->layout() );
	    l->setAlignment( Qt::AlignTop );
	    return l;
	default:
	    return 0;
	}
    } else {
	if ( layout ) {
	    QLayout *l;
	    switch ( type ) {
	    case HBox:
		l = new QHBoxLayout( layout );
		l->setSpacing( spacing );
		l->setMargin( margin );
		return l;
	    case VBox:
		l = new QVBoxLayout( layout );
		l->setSpacing( spacing );
		l->setMargin( margin );
		return l;
	    case Grid: {
		l = new QGridLayout( layout );
		l->setSpacing( spacing );
		l->setMargin( margin );
		return l;
	    }
	    default:
		return 0;
	    }
	} else {
	    QLayout *l;
	    switch ( type ) {
	    case HBox:
		l = new QHBoxLayout( widget );
		if ( !widget ) {
		    l->setMargin( margin );
		    l->setSpacing( margin );
		}
		return l;
	    case VBox:
		l = new QVBoxLayout( widget );
		if ( !widget ) {
		    l->setMargin( margin );
		    l->setSpacing( margin );
		}
		return l;
	    case Grid: {
		l = new QGridLayout( widget );
		if ( !widget ) {
		    l->setMargin( margin );
		    l->setSpacing( margin );
		}
		return l;
	    }
	    default:
		return 0;
	    }
	}
    }
}

QWidgetFactory::LayoutType QWidgetFactory::layoutType( QLayout *layout ) const
{
    if ( layout->inherits( "QHBoxLayout" ) )
	return HBox;
    else if ( layout->inherits( "QVBoxLayout" ) )
	return VBox;
    else if ( layout->inherits( "QGridLayout" ) )
	return Grid;
    return NoLayout;
}

void QWidgetFactory::setProperty( QObject* obj, const QString &prop, const QDomElement &e )
{
    const QMetaProperty *p = obj->metaObject()->property( prop, TRUE );

    QVariant defVarient;
    if ( e.tagName() == "font" ) {
	QFont f( qApp->font() );
	if ( obj->isWidgetType() && ( (QWidget*)obj )->parentWidget() )
	    f = ( (QWidget*)obj )->parentWidget()->font();
	defVarient = QVariant( f );
    }

    QString comment;
    QVariant v( DomTool::elementToVariant( e, defVarient, comment ) );

    if ( !p ) {
	if ( obj->isWidgetType() ) {
	    if ( prop == "toolTip" ) {
		if ( !v.toString().isEmpty() )
		    QToolTip::add( (QWidget*)obj, v.toString() );
	    } else if ( prop == "whatsThis" ) {
		if ( !v.toString().isEmpty() )
		    QWhatsThis::add( (QWidget*)obj, v.toString() );
	    }
#ifndef QT_NO_SQL
	    if ( prop == "database" && !obj->inherits( "QDataView" )
		 && !obj->inherits( "QDataBrowser" ) ) {
		QStringList lst = DomTool::elementToVariant( e, QVariant( QStringList() ) ).toStringList();
		if ( lst.count() > 2 ) {
		    if ( dbControls )
			dbControls->insert( obj->name(), lst[ 2 ] );
		} else if ( lst.count() == 2 ) {
		    dbTables.insert( obj->name(), lst );
		}
	    } else if ( prop == "database" ) {
		QStringList lst = DomTool::elementToVariant( e, QVariant( QStringList() ) ).toStringList();
		if ( lst.count() == 2 && obj->inherits( "QWidget" ) ) {
		    SqlWidgetConnection conn( lst[ 0 ], lst[ 1 ] );
		    sqlWidgetConnections.insert( (QWidget*)obj, conn );
		    dbControls = conn.dbControls;
		}
	    } else
#endif
		if ( prop == "buddy" ) {
		buddies.insert( obj->name(), v.toCString() );
	    } else if ( prop == "frameworkCode" ) {
		if ( !DomTool::elementToVariant( e, QVariant( TRUE, 0 ) ).toBool() ) {
		    noDatabaseWidgets << obj->name();
		}
	    }

	    return;
	}
    }

    if ( e.tagName() == "pixmap" ) {
	QPixmap pix = loadPixmap( e );
	v = QVariant( pix );
    } else if ( e.tagName() == "iconset" ) {
	QPixmap pix = loadPixmap( e );
	v = QVariant( QIconSet( pix ) );
    } else if ( e.tagName() == "image" ) {
	v = QVariant( loadFromCollection( v.toString() ) );
    } else if ( e.tagName() == "palette" ) {
	QDomElement n = e.firstChild().toElement();
	QPalette p;
	while ( !n.isNull() ) {
	    QColorGroup cg;
	    if ( n.tagName() == "active" ) {
		cg = loadColorGroup( n );
		p.setActive( cg );
	    } else if ( n.tagName() == "inactive" ) {
		cg = loadColorGroup( n );
		p.setInactive( cg );
	    } else if ( n.tagName() == "disabled" ) {
		cg = loadColorGroup( n );
		p.setDisabled( cg );
	    }
	    n = n.nextSibling().toElement();
	}
	v = QPalette( p );
    } else if ( e.tagName() == "enum" && p && p->isEnumType() ) {
	QString key( v.toString() );
	v = QVariant( p->keyToValue( key ) );
    } else if ( e.tagName() == "set" && p && p->isSetType() ) {
	QString keys( v.toString() );
	QStringList lst = QStringList::split( '|', keys );
	QStrList l;
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
	    l.append( *it );
	v = QVariant( p->keysToValue( l ) );
    }

    if ( prop == "geometry" ) {
	if ( obj == toplevel ) {
	    toplevel->resize( v.toRect().size() );
	    return;
	}
    }

    obj->setProperty( prop, v );
}

void QWidgetFactory::createSpacer( const QDomElement &e, QLayout *layout )
{
    QDomElement n = e.firstChild().toElement();
    int row = e.attribute( "row" ).toInt();
    int col = e.attribute( "column" ).toInt();
    int rowspan = e.attribute( "rowspan" ).toInt();
    int colspan = e.attribute( "colspan" ).toInt();

    Qt::Orientation orient = Qt::Horizontal;
    int w = 0, h = 0;
    QSizePolicy::SizeType sizeType = QSizePolicy::Preferred;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString prop = n.attribute( "name" );
	    if ( prop == "orientation" ) {
		if ( n.firstChild().firstChild().toText().data() == "Horizontal" )
		    orient = Qt::Horizontal;
		else
		    orient = Qt::Vertical;
	    } else if ( prop == "sizeType" ) {
		if ( n.firstChild().firstChild().toText().data() == "Fixed" )
		    sizeType = QSizePolicy::Fixed;
		else if ( n.firstChild().firstChild().toText().data() == "Minimum" )
		    sizeType = QSizePolicy::Minimum;
		else if ( n.firstChild().firstChild().toText().data() == "Maximum" )
		    sizeType = QSizePolicy::Maximum;
		else if ( n.firstChild().firstChild().toText().data() == "Preferred" )
		    sizeType = QSizePolicy::Preferred;
		else if ( n.firstChild().firstChild().toText().data() == "MinimumExpanding" )
		    sizeType = QSizePolicy::MinimumExpanding;
		else if ( n.firstChild().firstChild().toText().data() == "Expanding" )
		    sizeType = QSizePolicy::Expanding;
	    } else if ( prop == "sizeHint" ) {
		w = n.firstChild().firstChild().firstChild().toText().data().toInt();
		h = n.firstChild().firstChild().nextSibling().firstChild().toText().data().toInt();
	    }
	}
	n = n.nextSibling().toElement();
    }

    if ( rowspan < 1 )
	rowspan = 1;
    if ( colspan < 1 )
	colspan = 1;
    QSpacerItem *item = new QSpacerItem( w, h, orient == Qt::Horizontal ? sizeType : QSizePolicy::Minimum,
					 orient == Qt::Vertical ? sizeType : QSizePolicy::Minimum );
    if ( layout ) {
	if ( layout->inherits( "QBoxLayout" ) )
	    ( (QBoxLayout*)layout )->addItem( item );
	else
	    ( (QGridLayout*)layout )->addMultiCell( item, row, row + rowspan - 1, col, col + colspan - 1,
						    orient == Qt::Horizontal ? Qt::AlignVCenter : Qt::AlignHCenter );
    }
}

static QImage loadImageData( QDomElement &n2 )
{
    QImage img;
    QString data = n2.firstChild().toText().data();
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
    QString format = n2.attribute( "format", "PNG" );
    if ( format == "XPM.GZ" ) {
	ulong len = n2.attribute( "length" ).toULong();
	if ( len < data.length() * 5 )
	    len = data.length() * 5;
	QByteArray baunzip( len );
	::uncompress( (uchar*) baunzip.data(), &len, (uchar*) ba, data.length()/2 );
	img.loadFromData( (const uchar*)baunzip.data(), len, "XPM" );
    }  else {
	img.loadFromData( (const uchar*)ba, data.length() / 2, format );
    }
    delete [] ba;

    return img;
}

void QWidgetFactory::loadImageCollection( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "image" ) {
	    Image img;
	    img.name =  n.attribute( "name" );
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "data" )
		    img.img = loadImageData( n2 );
		n2 = n2.nextSibling().toElement();
	    }
	    images.append( img );
	    n = n.nextSibling().toElement();
	}
    }
}

QImage QWidgetFactory::loadFromCollection( const QString &name )
{
    QValueList<Image>::Iterator it = images.begin();
    for ( ; it != images.end(); ++it ) {
	if ( ( *it ).name == name )
	    return ( *it ).img;
    }
    return QImage();
}

QPixmap QWidgetFactory::loadPixmap( const QDomElement &e )
{
    QString arg = e.firstChild().toText().data();
    if ( usePixmapCollection ) {
	const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( arg );
	if ( !m )
	    return QPixmap();
	QPixmap pix;
	QImageDrag::decode( m, pix );
	return pix;
    }

    QImage img = loadFromCollection( arg );
    QPixmap pix;
    pix.convertFromImage( img );
    return pix;
}

QColorGroup QWidgetFactory::loadColorGroup( const QDomElement &e )
{
    QColorGroup cg;
    int r = -1;
    QDomElement n = e.firstChild().toElement();
    QColor col;
    while ( !n.isNull() ) {
	if ( n.tagName() == "color" ) {
	    r++;
	    cg.setColor( (QColorGroup::ColorRole)r, (col = DomTool::readColor( n ) ) );
	} else if ( n.tagName() == "pixmap" ) {
	    QImage img = loadFromCollection( n.firstChild().toText().data() );
	    QPixmap pix = loadPixmap( n );
	    cg.setBrush( (QColorGroup::ColorRole)r, QBrush( col, pix ) );
	}
	n = n.nextSibling().toElement();
    }
    return cg;
}

struct Connection
{
    QObject *sender, *receiver;
    QCString signal, slot;
    bool operator==( const Connection &c ) const {
	return sender == c.sender && receiver == c.receiver &&
	       signal == c.signal && slot == c.slot ;
    }
};

class NormalizeObject : public QObject
{
public:
    NormalizeObject() : QObject() {}
    static QCString normalizeSignalSlot( const char *signalSlot ) { return QObject::normalizeSignalSlot( signalSlot ); }
};

void QWidgetFactory::loadConnections( const QDomElement &e, QObject *connector )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "connection" ) {
	    QDomElement n2 = n.firstChild().toElement();
	    Connection conn;
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "sender" ) {
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.sender = toplevel;
		    } else {
			if ( name == "this" )
			    name = toplevel->name();
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.sender = l->first();
			    delete l;
			}
		    }
		} else if ( n2.tagName() == "signal" ) {
		    conn.signal = n2.firstChild().toText().data();
		} else if ( n2.tagName() == "receiver" ) {
		    QString name = n2.firstChild().toText().data();
		    if ( name == "this" || qstrcmp( toplevel->name(), name ) == 0 ) {
			conn.receiver = toplevel;
		    } else {
			QObjectList *l = toplevel->queryList( 0, name, FALSE );
			if ( l ) {
			    if ( l->first() )
				conn.receiver = l->first();
			    delete l;
			}
		    }
		} else if ( n2.tagName() == "slot" ) {
		    conn.slot = n2.firstChild().toText().data();
		}
		n2 = n2.nextSibling().toElement();
	    }

	    conn.signal = NormalizeObject::normalizeSignalSlot( conn.signal );
	    conn.slot = NormalizeObject::normalizeSignalSlot( conn.slot );

	    QObject *sender = 0, *receiver = 0;
	    QObjectList *l = toplevel->queryList( 0, conn.sender->name(), FALSE );
	    if ( qstrcmp( conn.sender->name(), toplevel->name() ) == 0 ) {
		sender = toplevel;
	    } else {
		if ( !l || !l->first() ) {
		    delete l;
		    n = n.nextSibling().toElement();
		    continue;
		}
		sender = l->first();
		delete l;
	    }

	    if ( qstrcmp( conn.receiver->name(), toplevel->name() ) == 0 ) {
		receiver = toplevel;
	    } else {
		l = toplevel->queryList( 0, conn.receiver->name(), FALSE );
		if ( !l || !l->first() ) {
		    delete l;
		    n = n.nextSibling().toElement();
		    continue;
		}
		receiver = l->first();
		delete l;
	    }
	    QString s = "2""%1";
	    s = s.arg( conn.signal );
	    QString s2 = "1""%1";
	    s2 = s2.arg( conn.slot );

	    QStrList signalList = sender->metaObject()->signalNames( TRUE );
	    QStrList slotList = receiver->metaObject()->slotNames( TRUE );

	    // if this is a connection to a custom slot and we have a connector, try this as receiver
	    if ( slotList.find( conn.slot ) == -1 && receiver == toplevel && connector )
		slotList = connector->metaObject()->slotNames( TRUE );

	    // avoid warnings
	    if ( signalList.find( conn.signal ) == -1 ||
		 slotList.find( conn.slot ) == -1 ) {
		n = n.nextSibling().toElement();
		continue;
	    }
	    QObject::connect( sender, s, receiver, s2 );
	} else if ( n.tagName() == "slot" ) {
	    QString s = n.firstChild().toText().data();
	    languageSlots.insert( s.left( s.find( "(" ) ) , n.attribute( "language" ) );
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadTabOrder( const QDomElement &e )
{
    QWidget *last = 0;
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "tabstop" ) {
	    QString name = n.firstChild().toText().data();
	    QObjectList *l = toplevel->queryList( 0, name, FALSE );
	    if ( l ) {
		if ( l->first() ) {
		    QWidget *w = (QWidget*)l->first();
		    if ( last )
			toplevel->setTabOrder( last, w );
		    last = w;
		}
		delete l;
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::createColumn( const QDomElement &e, QWidget *widget )
{
    if ( widget->inherits( "QListView" ) && e.tagName() == "column" ) {
	QListView *lv = (QListView*)widget;
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	bool clickable = TRUE, resizeable = TRUE;
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = v.toString();
		else if ( attrib == "pixmap" ) {
		    pix = loadPixmap( n.firstChild().toElement().toElement() );
		    hasPixmap = TRUE;
		} else if ( attrib == "clickable" )
		    clickable = v.toBool();
		else if ( attrib == "resizeable" )
		    resizeable = v.toBool();
	    }
	    n = n.nextSibling().toElement();
	}
	lv->addColumn( txt );
	int i = lv->header()->count() - 1;
	if ( hasPixmap ) {
	    lv->header()->setLabel( i, pix, txt );
	}
	if ( !clickable )
	    lv->header()->setClickEnabled( clickable, i );
	if ( !resizeable )
	    lv->header()->setResizeEnabled( resizeable, i );
    }
#ifndef QT_NO_TABLE
    else if ( widget->inherits( "QTable" ) ) {
	QTable *table = (QTable*)widget;
#ifndef QT_NO_SQL
	bool isSql = (widget->inherits( "QDataTable" ));
#endif
	bool isRow;
	if ( ( isRow = e.tagName() == "row" ) )
	    table->setNumRows( table->numRows() + 1 );
	else {
#ifndef QT_NO_SQL
	    if ( !isSql )
#endif
		table->setNumCols( table->numCols() + 1 );
	}

	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	QString field;
	QValueList<Field> fieldMap;
	if ( fieldMaps.find( table ) != fieldMaps.end() ) {
	    fieldMap = *fieldMaps.find( table );
	    fieldMaps.remove( table );
	}
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    txt = v.toString();
		else if ( attrib == "pixmap" ) {
		    hasPixmap = !n.firstChild().firstChild().toText().data().isEmpty();
		    if ( hasPixmap )
			pix = loadPixmap( n.firstChild().toElement().toElement() );
		} else if ( attrib == "field" )
		    field = v.toString();
	    }
	    n = n.nextSibling().toElement();
	}

	int i = isRow ? table->numRows() - 1 : table->numCols() - 1;
	QHeader *h = !isRow ? table->horizontalHeader() : table->verticalHeader();
	if ( hasPixmap ) {
#ifndef QT_NO_SQL
	    if ( isSql )
		((QDataTable*)table)->addColumn( field, txt, -1, pix );
	    else
#endif
		h->setLabel( i, pix, txt );
	} else {
#ifndef QT_NO_SQL
	    if ( isSql ) {
		((QDataTable*)table)->addColumn( field, txt );
	    }
	    else
#endif
		h->setLabel( i, txt );
	}
	if ( !isRow && !field.isEmpty() ) {
	    fieldMap.append( Field( txt, (hasPixmap ? pix : QPixmap()), field ) );
	    fieldMaps.insert( table, fieldMap );
	}
    }
#endif
}

void QWidgetFactory::loadItem( const QDomElement &e, QPixmap &pix, QString &txt, bool &hasPixmap )
{
    QDomElement n = e;
    hasPixmap = FALSE;
    while ( !n.isNull() ) {
	if ( n.tagName() == "property" ) {
	    QString attrib = n.attribute( "name" );
	    QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
	    if ( attrib == "text" )
		txt = v.toString();
	    else if ( attrib == "pixmap" ) {
		pix = loadPixmap( n.firstChild().toElement() );
		hasPixmap = TRUE;
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::createItem( const QDomElement &e, QWidget *widget, QListViewItem *i )
{
    if ( widget->inherits( "QListBox" ) || widget->inherits( "QComboBox" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );
	QListBox *lb = 0;
	if ( widget->inherits( "QListBox" ) )
	    lb = (QListBox*)widget;
	else
	    lb = ( (QComboBox*)widget)->listBox();
	if ( hasPixmap ) {
	    new QListBoxPixmap( lb, pix, txt );
	} else {
	    new QListBoxText( lb, txt );
	}
    } else if ( widget->inherits( "QIconView" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	bool hasPixmap = FALSE;
	QString txt;
	loadItem( n, pix, txt, hasPixmap );

	QIconView *iv = (QIconView*)widget;
	new QIconViewItem( iv, txt, pix );
    } else if ( widget->inherits( "QListView" ) ) {
	QDomElement n = e.firstChild().toElement();
	QPixmap pix;
	QValueList<QPixmap> pixmaps;
	QStringList textes;
	QListViewItem *item = 0;
	QListView *lv = (QListView*)widget;
	if ( i )
	    item = new QListViewItem( i, lastItem );
	else
	    item = new QListViewItem( lv, lastItem );
	while ( !n.isNull() ) {
	    if ( n.tagName() == "property" ) {
		QString attrib = n.attribute( "name" );
		QVariant v = DomTool::elementToVariant( n.firstChild().toElement(), QVariant() );
		if ( attrib == "text" )
		    textes << v.toString();
		else if ( attrib == "pixmap" ) {
		    QString s = v.toString();
		    if ( s.isEmpty() ) {
			pixmaps << QPixmap();
		    } else {
			pix = loadPixmap( n.firstChild().toElement() );
			pixmaps << pix;
		    }
		}
	    } else if ( n.tagName() == "item" ) {
		item->setOpen( TRUE );
		createItem( n, widget, item );
	    }

	    n = n.nextSibling().toElement();
	}

	for ( int i = 0; i < lv->columns(); ++i ) {
	    item->setText( i, textes[ i ] );
	    item->setPixmap( i, pixmaps[ i ] );
	}
	lastItem = item;
    }
}



void QWidgetFactory::loadChildAction( QObject *parent, const QDomElement &e )
{
    QDomElement n = e;
    QAction *a = 0;
    if ( n.tagName() == "action" ) {
	a = new QAction( parent );
	QDomElement n2 = n.firstChild().toElement();
	while ( !n2.isNull() ) {
	    if ( n2.tagName() == "property" )
		setProperty( a, n2.attribute( "name" ), n2.firstChild().toElement() );
	    n2 = n2.nextSibling().toElement();
	}
	if ( !parent->inherits( "QAction" ) )
	    actionList.append( a );
    } else if ( n.tagName() == "actiongroup" ) {
	a = new QActionGroup( parent );
	QDomElement n2 = n.firstChild().toElement();
	while ( !n2.isNull() ) {
	    if ( n2.tagName() == "property" )
		setProperty( a, n2.attribute( "name" ), n2.firstChild().toElement() );
	    else if ( n2.tagName() == "action" ||
		      n2.tagName() == "actiongroup" )
		loadChildAction( a, n2 );
	    n2 = n2.nextSibling().toElement();
	}
	if ( !parent->inherits( "QAction" ) )
	    actionList.append( a );
    }
}

void QWidgetFactory::loadActions( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    while ( !n.isNull() ) {
	if ( n.tagName() == "action" ) {
	    loadChildAction( toplevel, n );
	} else if ( n.tagName() == "actiongroup" ) {
	    loadChildAction( toplevel, n );
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadToolBars( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    QMainWindow *mw = ( (QMainWindow*)toplevel );
    QToolBar *tb = 0;
    while ( !n.isNull() ) {
	if ( n.tagName() == "toolbar" ) {
	    Qt::Dock dock = (Qt::Dock)n.attribute( "dock" ).toInt();
	    tb = new QToolBar( QString::null, mw, dock );
	    tb->setLabel( n.attribute( "label" ) );
	    tb->setName( n.attribute( "name" ) );
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "action" ) {
		    QAction *a = findAction( n2.attribute( "name" ) );
		    if ( a )
			a->addTo( tb );
		} else if ( n2.tagName() == "separator" ) {
		    tb->addSeparator();
		} else if ( n2.tagName() == "widget" ) {
		    (void)createWidgetInternal( n2, tb, 0, n2.attribute( "class", "QWidget" ) );
		}
		n2 = n2.nextSibling().toElement();
	    }
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadMenuBar( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    QMainWindow *mw = ( (QMainWindow*)toplevel );
    QMenuBar *mb = mw->menuBar();
    while ( !n.isNull() ) {
	if ( n.tagName() == "item" ) {
	    QPopupMenu *popup = new QPopupMenu( mw );
	    popup->setName( n.attribute( "name" ) );
	    QDomElement n2 = n.firstChild().toElement();
	    while ( !n2.isNull() ) {
		if ( n2.tagName() == "action" ) {
		    QAction *a = findAction( n2.attribute( "name" ) );
		    if ( a )
			a->addTo( popup );
		} else if ( n2.tagName() == "separator" ) {
		    popup->insertSeparator();
		}
		n2 = n2.nextSibling().toElement();
	    }
	    mb->insertItem( n.attribute( "text" ), popup );
	}
	n = n.nextSibling().toElement();
    }
}

void QWidgetFactory::loadFunctions( const QDomElement &e )
{
    QDomElement n = e.firstChild().toElement();
    QMap<QString, QString> bodies;
    QString s;
    while ( !n.isNull() ) {
	if ( n.tagName() == "function" ) {
	    QString name = n.attribute( "name" );
	    QMap<QString, QString>::Iterator it = languageSlots.find( name.left( name.find( "(" ) ) );
	    if ( it != languageSlots.end() ) {
		Functions *funcs = 0;
		QMap<QString, Functions*>::Iterator fit = languageFunctions.find( *it );
		if ( fit == languageFunctions.end() ) {
		    funcs = new Functions;
		    languageFunctions.insert( *it, funcs );
		} else {
		    funcs = *fit;
		}
		QString s;
		QString body = n.firstChild().toText().data();
		s += "function " + name + "\n" + body + "\n\n";
		funcs->functions += s;
		if ( qwf_language && *qwf_language == *it ) {
		    if ( !qwf_functions )
			qwf_functions = new QMap<QWidget*, QString>;
		    if ( !qwf_forms )
			qwf_forms = new QMap<QWidget*, QString>;
		    (*(qwf_functions))[ toplevel ].append( s );
		}

	    }
	}
	n = n.nextSibling().toElement();
    }
}

QAction *QWidgetFactory::findAction( const QString &name )
{
    for ( QAction *a = actionList.first(); a; a = actionList.next() ) {
	if ( QString( a->name() ) == name )
	    return a;
	QAction *ac = (QAction*)a->child( name.latin1(), "QAction" );
	if ( ac )
	    return ac;
    }
    return 0;
}

/* If the pixmaps of the form are not saved in the XML file, but you
   used a project with a pixmap collection, you have to load this
   pixmap collection. QWidgetFactory looks in the default
   QMimeSourceFactory for the pixmaps. Either you add it there
   manually, or call this function and specify the directory, where
   the images can be found, as \a dir. This is normally the directory
   \c images in the project directory.
*/

void QWidgetFactory::loadImages( const QString &dir )
{
    QDir d( dir );
    QStringList l = d.entryList( QDir::Files );
    for ( QStringList::Iterator it = l.begin(); it != l.end(); ++it )
	QMimeSourceFactory::defaultFactory()->setPixmap( *it, QPixmap( d.path() + "/" + *it, "PNG" ) );

}
