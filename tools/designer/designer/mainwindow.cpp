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

#include "designerappiface.h"
#include "designerapp.h"

#include "mainwindow.h"
#include "defs.h"
#include "globaldefs.h"
#include "formwindow.h"
#include "widgetdatabase.h"
#include "widgetfactory.h"
#include "propertyeditor.h"
#include "metadatabase.h"
#include "resource.h"
#include "pixmapchooser.h"
#include "config.h"
#include "hierarchyview.h"
#include "newformimpl.h"
#include "formlist.h"
#include "about.h"
#include "multilineeditorimpl.h"
#include "wizardeditorimpl.h"
#include "outputwindow.h"
#include <qinputdialog.h>
#if defined(HAVE_KDE)
#include <ktoolbar.h>
#else
#include <qtoolbar.h>
#endif
#include <qfeatures.h>
#include <qmetaobject.h>
#include <qaction.h>
#include <qpixmap.h>
#include <qworkspace.h>
#include <qfiledialog.h>
#include <qclipboard.h>
#include <qmessagebox.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qstyle.h>
#include <qlabel.h>
#include <qstatusbar.h>
#include <qfile.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qwizard.h>
#include <qdir.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <stdlib.h>
#include <qdockwindow.h>
#include <qregexp.h>
#include <qstylefactory.h>
#include "actioneditorimpl.h"
#include "actiondnd.h"
#include "project.h"
#include "projectsettingsimpl.h"
#include "../uilib/qwidgetfactory.h"
#include <qvbox.h>
#include <qprocess.h>
#include <qsettings.h>
#include "pixmapcollection.h"
#include "sourcefile.h"

static int forms = 0;
static bool mblockNewForms = FALSE;
extern QMap<QWidget*, QString> *qwf_functions;
extern QMap<QWidget*, QString> *qwf_forms;
extern QString *qwf_language;
extern bool qwf_execute_code;
extern bool qwf_stays_on_top;
extern void set_splash_status( const QString &txt );
static bool tbSettingsRead = FALSE;

MainWindow *MainWindow::self = 0;

static QString textNoAccel( const QString& text)
{
    QString t = text;
    int i;
    while ( (i = t.find('&') )>= 0 ) {
	t.remove(i,1);
    }
    return t;
}


MainWindow::MainWindow( bool asClient )
#if defined(HAVE_KDE)
    : KMainWindow( 0, "mainwindow", WType_TopLevel | WDestructiveClose ),
#else
    : QMainWindow( 0, "mainwindow", WType_TopLevel | WDestructiveClose ),
#endif
      grd( 10, 10 ), sGrid( TRUE ), snGrid( TRUE ), restoreConfig( TRUE ), splashScreen( TRUE ),
      docPath( "$QTDIR/doc/html" ), fileFilter( tr( "Qt User-Interface Files (*.ui)" ) ), client( asClient ),
      previewing( FALSE ), databaseAutoEdit( FALSE )
{
    init_colors();

    desInterface = new DesignerInterfaceImpl( this );
    desInterface->addRef();
    inDebugMode = FALSE;

    pluginDir = getenv( "QTDIR" );
    pluginDir += "/plugins/designer";
    libDir = getenv( "QTDIR" );
    libDir += "/lib";

    updateSlotsTimer = new QTimer( this );
    connect( updateSlotsTimer, SIGNAL( timeout() ),
	     this, SLOT( doSlotsChanged() ) );

    set_splash_status( "Loading Plugins..." );
    setupPluginManagers();

    qApp->setMainWidget( this );
    QWidgetFactory::addWidgetFactory( new CustomWidgetFactory );
    self = this;
    setIcon( PixmapChooser::loadPixmap( "logo" ) );

    actionGroupTools = 0;
    prefDia = 0;
    windowMenu = 0;
    hierarchyView = 0;
    actionEditor = 0;
    currentProject = 0;
    formList = 0;
    oWindow = 0;
    actionEditPixmapCollection = 0;

    statusBar()->clear();
    statusBar()->addWidget( new QLabel("Ready", statusBar()), 1 );

    set_splash_status( "Setting up GUI..." );
    setupMDI();
    setupMenuBar();

    setupFileActions();
    setupEditActions();
    setupSearchActions();
#if defined(HAVE_KDE)
    layoutToolBar = new KToolBar( this, "Layout" );
    ( (KToolBar*)layoutToolBar )->setFullSize( FALSE );
#else
    layoutToolBar = new QToolBar( this, "Layout" );
    layoutToolBar->setCloseMode( QDockWindow::Undocked );
#endif
    addToolBar( layoutToolBar, tr( "Layout" ) );
    setupToolActions();
    setupLayoutActions();
    setupPreviewActions();
    setupWindowActions();

    setupFormList();
    setupHierarchyView();
    setupPropertyEditor();
    setupActionEditor();
    setupOutputWindow();

    setupActionManager();
    setupHelpActions();

    setupRMBMenus();

    emit hasActiveForm( FALSE );
    emit hasActiveWindow( FALSE );

    lastPressWidget = 0;
    qApp->installEventFilter( this );

    QSize as( qApp->desktop()->size() );
    as -= QSize( 30, 30 );
    resize( QSize( 1200, 1000 ).boundedTo( as ) );

    connect( qApp->clipboard(), SIGNAL( dataChanged() ),
	     this, SLOT( clipboardChanged() ) );
    clipboardChanged();
    layoutChilds = FALSE;
    layoutSelected = FALSE;
    breakLayout = FALSE;
    backPix = TRUE;

    set_splash_status( "Loading User Settings..." );
    readConfig();

    // hack to make WidgetFactory happy (so it knows QWidget and QDialog for resetting properties)
    QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWidget" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QDialog" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QLabel" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QTabWidget" ), this, 0, FALSE );
    delete w;
    w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QFrame" ), this, 0, FALSE );
    delete w;

    setAppropriate( (QDockWindow*)actionEditor->parentWidget(), FALSE );
    actionEditor->parentWidget()->hide();

    statusBar()->setSizeGripEnabled( TRUE );
    set_splash_status( "Initialization Done." );
}

MainWindow::~MainWindow()
{
    QMap< QAction*, Project* >::Iterator it = projects.begin();
    while ( it != projects.end() ) {
	Project *p = *it;
	++it;
	delete p;
    }
    projects.clear();

    delete oWindow;
    oWindow = 0;

    desInterface->release();
    desInterface = 0;

    delete actionPluginManager;
    delete preferencePluginManager;
    delete projectSettingsPluginManager;
    delete interpreterPluginManager;
    delete programPluginManager;
    delete templateWizardPluginManager;
    delete editorPluginManager;

    MetaDataBase::clearDataBase();
}

void MainWindow::setupMDI()
{
    QVBox *vbox = new QVBox( this );
    setCentralWidget( vbox );
    vbox->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    vbox->setMargin( 1 );
    vbox->setLineWidth( 1 );
    workspace = new QWorkspace( vbox );
    workspace->setBackgroundMode( PaletteDark );
    workspace->setBackgroundPixmap( PixmapChooser::loadPixmap( "background.png", PixmapChooser::NoSize ) );
    workspace->setScrollBarsEnabled( TRUE );
    connect( workspace, SIGNAL( windowActivated( QWidget * ) ),
	     this, SLOT( activeWindowChanged( QWidget * ) ) );
    lastActiveFormWindow = 0;
    workspace->setAcceptDrops( TRUE );
}

void MainWindow::setupMenuBar()
{
    menubar = menuBar();
}

void MainWindow::setupPropertyEditor()
{
    QDockWindow *dw = new QDockWindow;
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    propertyEditor = new PropertyEditor( dw );
    addToolBar( dw, Qt::Left );
    dw->setWidget( propertyEditor );
    dw->setFixedExtentWidth( 300 );
    dw->setCaption( tr( "Property Editor/Signal Handlers" ) );
    QWhatsThis::add( propertyEditor, tr("<b>The Property Editor</b>"
					"<p>You can change the appearance and behaviour of the selected widget in the "
					"property editor.</p>"
					"<p>You can set properties for components and forms at design time and see the "
					"changes immediately. Each property has its own editor which you can use to enter "
					"new values, open a special dialog or select values from a predefined list. "
					"Use <b>F1</b> to get detailed help for the selected property.</p>"
					"<p>You can resize the columns of the editor by dragging the separators of the list "
					"header.</p>"
					"<p><b>Signal Handlers</b></p>"
					"<p>In the Signal Handlers tab you can define connections between "
					"signals of widgets and slots of the form. That is just a convenience "
					"way, you can use the connection tool to do that as well." ) );
    dw->show();
}

void MainWindow::setupOutputWindow()
{
    QDockWindow *dw = new QDockWindow;
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    addToolBar( dw, Qt::Bottom );
    oWindow = new OutputWindow( dw );
    dw->setWidget( oWindow );
    dw->setFixedExtentHeight( 200 );
    dw->setCaption( tr( "Output Window" ) );
    dw->hide();
}

void MainWindow::setupHierarchyView()
{
    if ( hierarchyView )
	return;
    QDockWindow *dw = new QDockWindow;
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    hierarchyView = new HierarchyView( dw );
    addToolBar( dw, Qt::Left );
    dw->setWidget( hierarchyView );

    dw->setCaption( tr( "Object Explorer" ) );
    dw->setFixedExtentWidth( 300 );
    QWhatsThis::add( hierarchyView, tr("<b>The Object Explorer</b>"
				      "<p>The object explorer gives a quick overview about the relations "
				      "between the widgets in your form. You can use the clipboard functions using "
				      "a context menu for each item in the view.</p>"
				      "<p>The columns can be resized by dragging the separator in the list header.</p>"
				       "<p>On the second tab you can see all the declared slots, variables, includes, etc. of the form.</p>") );
    dw->show();
}

void MainWindow::setupFormList()
{
    QDockWindow *dw = new QDockWindow;
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    formList = new FormList( dw, this, currentProject );
    addToolBar( dw, Qt::Left );
    dw->setWidget( formList );

    dw->setCaption( tr( "Files" ) );
    QWhatsThis::add( formList, tr("<b>The File List</b>"
				  "<p>The File List displays all files of the project, including forms and pixmaps</p>") );
    dw->setFixedExtentHeight( 100 );
    dw->show();
}

void MainWindow::setupActionEditor()
{
    QDockWindow *dw = new QDockWindow( QDockWindow::OutsideDock, this, 0 );
    addDockWindow( dw, Qt::TornOff );
    dw->setResizeEnabled( TRUE );
    dw->setCloseMode( QDockWindow::Always );
    actionEditor = new ActionEditor( dw );
    dw->setWidget( actionEditor );
    actionEditor->show();
    dw->setCaption( tr( "Action Editor" ) );
    QWhatsThis::add( actionEditor, tr("<b>The Action Editor</b><p>Todo Whatsthis</p>" ) );
    dw->hide();
    setAppropriate( dw, FALSE );
}

void MainWindow::setupRMBMenus()
{
    rmbWidgets = new QPopupMenu( this );
    actionEditCut->addTo( rmbWidgets );
    actionEditCopy->addTo( rmbWidgets );
    actionEditPaste->addTo( rmbWidgets );
    actionEditDelete->addTo( rmbWidgets );
#if 0
    rmbWidgets->insertSeparator();
    actionEditLower->addTo( rmbWidgets );
    actionEditRaise->addTo( rmbWidgets );
#endif
    rmbWidgets->insertSeparator();
    actionEditAdjustSize->addTo( rmbWidgets );
    actionEditHLayout->addTo( rmbWidgets );
    actionEditVLayout->addTo( rmbWidgets );
    actionEditGridLayout->addTo( rmbWidgets );
    actionEditSplitHorizontal->addTo( rmbWidgets );
    actionEditSplitVertical->addTo( rmbWidgets );
    actionEditBreakLayout->addTo( rmbWidgets );
    rmbWidgets->insertSeparator();
    actionEditConnections->addTo( rmbWidgets );
    actionEditSource->addTo( rmbWidgets );

    rmbFormWindow = new QPopupMenu( this );
    actionEditPaste->addTo( rmbFormWindow );
    actionEditSelectAll->addTo( rmbFormWindow );
    actionEditAccels->addTo( rmbFormWindow );
    rmbFormWindow->insertSeparator();
    actionEditAdjustSize->addTo( rmbFormWindow );
    actionEditHLayout->addTo( rmbFormWindow );
    actionEditVLayout->addTo( rmbFormWindow );
    actionEditGridLayout->addTo( rmbFormWindow );
    actionEditBreakLayout->addTo( rmbFormWindow );
    rmbFormWindow->insertSeparator();
    actionEditSlots->addTo( rmbFormWindow );
    actionEditConnections->addTo( rmbFormWindow );
    actionEditSource->addTo( rmbFormWindow );
    rmbFormWindow->insertSeparator();
    actionEditFormSettings->addTo( rmbFormWindow );
}

void MainWindow::toolSelected( QAction* action )
{
    actionCurrentTool = action;
    emit currentToolChanged();
    if ( formWindow() )
	formWindow()->commandHistory()->emitUndoRedo();
}

int MainWindow::currentTool() const
{
    if ( !actionCurrentTool )
	return POINTER_TOOL;
    return QString::fromLatin1(actionCurrentTool->name()).toInt();
}

QObjectList *MainWindow::runProject()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	e->save();
	e->saveBreakPoints();
    }

    if ( currentTool() == ORDER_TOOL )
	resetTool();
    if ( !currentProject )
	return 0;
    oWindow->parentWidget()->show();
    oWindow->clearErrorMessages();
    oWindow->clearDebug();
    QApplication::setOverrideCursor( WaitCursor );

    delete qwf_functions;
    qwf_functions = 0;
    delete qwf_forms;
    qwf_forms = 0;
    delete qwf_language;
    qwf_language = new QString( currentProject->language() );
    qwf_execute_code = FALSE;

    previewing = TRUE;

    QStringList forms = currentProject->uiFiles();
    for ( QStringList::Iterator it = forms.begin(); it != forms.end(); ++it ) {
	QWidget *w = QWidgetFactory::create( currentProject->makeAbsolute( *it ) );

	if ( w ) {
	    w->hide();
	    if ( programPluginManager ) {
		QString lang = currentProject->language();
		ProgramInterface *piface = 0;
		programPluginManager->queryInterface( lang, &piface);
		if ( piface ) {
		    QStringList error;
		    QValueList<int> line;
		    if ( qwf_functions ) {
			QMap<QWidget*, QString>::Iterator it = qwf_functions->find( w );
			if ( it == qwf_functions->end() )
			    continue;
			if ( !piface->check( *it, error, line ) && !error.isEmpty() && !error[ 0 ].isEmpty() ) {
			    showSourceLine( it.key(), line[ 0 ] - 1, TRUE );
			    oWindow->setErrorMessages( error, line );
			    piface->release();
			    QApplication::restoreOverrideCursor();
			    return 0;
			}
		    }
		    QPtrList<SourceFile> sources = currentProject->sourceFiles();
		    for ( SourceFile *f = sources.first(); f; f = sources.next() ) {
			QStringList error;
			QValueList<int> line;
			if ( !piface->check( f->text(), error, line ) && !error.isEmpty() && !error[ 0 ].isEmpty() ) {
			    showSourceLine( f, line[ 0 ] - 1, TRUE );
			    oWindow->setErrorMessages( error, line );
			    piface->release();
			    QApplication::restoreOverrideCursor();
			    return 0;
			}
		    }
		    piface->release();
		}
	    }
	}
    }

    delete qwf_functions;
    qwf_functions = 0;
    delete qwf_forms;
    qwf_forms = 0;
    delete qwf_language;
    qwf_language = new QString( currentProject->language() );
    qwf_execute_code = TRUE;
    qwf_stays_on_top = TRUE;

    InterpreterInterface *iiface = 0;
    if ( interpreterPluginManager ) {
	QString lang = currentProject->language();
	iiface = 0;
	interpreterPluginManager->queryInterface( lang, &iiface );
	if ( iiface ) {
	    iiface->onShowDebugStep( this, SLOT( showDebugStep( QObject *, int ) ) );
	    iiface->onShowError( this, SLOT( showErrorMessage( QObject *, int, const QString & ) ) );
	    iiface->onFinish( this, SLOT( finishedRun() ) );
	}

	LanguageInterface *liface = MetaDataBase::languageInterface( lang );
	if ( liface && liface->supports( LanguageInterface::AdditionalFiles ) ) {
	    QPtrList<SourceFile> sources = currentProject->sourceFiles();
	    for ( SourceFile *f = sources.first(); f; f = sources.next() ) {
		iiface->exec( f, f->text() );
	    }
	}
    }

    QObjectList *l = new QObjectList;
    if ( iiface ) {
	QPtrList<FormWindow> frms = currentProject->forms();
	for ( FormWindow *fw = frms.first(); fw; fw = frms.next() ) {
	    QValueList<int> bps = MetaDataBase::breakPoints( fw );
	    if ( !bps.isEmpty() )
		iiface->setBreakPoints( fw, bps );
	}

	QPtrList<SourceFile> files = currentProject->sourceFiles();
	for ( SourceFile *f = files.first(); f; f = files.next() ) {
	    QValueList<int> bps = MetaDataBase::breakPoints( f );
	    if ( !bps.isEmpty() )
		iiface->setBreakPoints( f, bps );
	}

	iiface->exec( 0, "main" );

	for ( QStringList::Iterator it2 = forms.begin(); it2 != forms.end(); ++it2 ) {
	    QWidget *w = QWidgetFactory::create( currentProject->makeAbsolute( *it2 ) );
	    if ( w ) {
		l->append( w );
		w->hide();
	    }
	}

	for ( QObject *o = l->first(); o; o = l->next() ) {
	    QWidget *fw = findRealForm( (QWidget*)o );
	    if ( !fw )
		continue;
	    QValueList<int> bps = MetaDataBase::breakPoints( fw );
	    if ( !bps.isEmpty() )
		iiface->setBreakPoints( o, bps );
	}

	iiface->release();
    }

    QApplication::restoreOverrideCursor();
    inDebugMode = TRUE;

    debuggingForms = *l;

    enableAll( FALSE );

    qwf_stays_on_top = FALSE;

    for ( SourceEditor *e2 = sourceEditors.first(); e2; e2 = sourceEditors.next() ) {
	if ( e2->project() == currentProject )
	    e2->editorInterface()->setMode( EditorInterface::Debugging );
    }
    return l;
}

QWidget* MainWindow::previewFormInternal( QStyle* style, QPalette* palet )
{
    qwf_execute_code = FALSE;
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->save();
    if ( currentTool() == ORDER_TOOL )
	resetTool();

    FormWindow *fw = formWindow();
    if ( !fw )
	return 0;

    QStringList databases;
    QPtrDictIterator<QWidget> wit( *fw->widgets() );
    while ( wit.current() ) {
	QStringList lst = MetaDataBase::fakeProperty( wit.current(), "database" ).toStringList();
	if ( !lst.isEmpty() )
	    databases << lst [ 0 ];
	++wit;
    }

    if ( fw->project() ) {
	QStringList::Iterator it;
	for ( it = databases.begin(); it != databases.end(); ++it )
	    fw->project()->openDatabase( *it, FALSE );
    }
    QApplication::setOverrideCursor( WaitCursor );

    QCString s;
    QBuffer buffer( s );
    buffer.open( IO_WriteOnly );
    Resource resource( this );
    resource.setWidget( fw );
    QValueList<Resource::Image> images;
    resource.save( &buffer );

    buffer.close();
    buffer.open( IO_ReadOnly );

    QWidget *w = QWidgetFactory::create( &buffer );
    if ( w ) {
	if ( palet ) {
	    if ( style )
		style->polish( *palet );
	    w->setPalette( *palet );
	}

	if ( style )
	    w->setStyle( style );
	
	QObjectList *l = w->queryList( "QWidget" );
	for ( QObject *o = l->first(); o; o = l->next() ) {
	    if ( style )
		( (QWidget*)o )->setStyle( style );
	    if ( !databaseAutoEdit )
		o->setProperty( "autoEdit", QVariant( FALSE, 0 ) );
	}
	delete l;

	w->move( fw->mapToGlobal( QPoint(0,0) ) );
	((MainWindow*)w )->setWFlags( WDestructiveClose );
	previewing = TRUE;
	w->show();
	previewing = FALSE;
	QApplication::restoreOverrideCursor();
	if ( fw->project() ) {
	    QStringList lst = MetaDataBase::fakeProperty( fw, "database" ).toStringList();
	    fw->project()->closeDatabase( lst[ 0 ] );
	}
	return w;
    }
    QApplication::restoreOverrideCursor();
    return 0;
}

void MainWindow::previewForm()
{
    QWidget* w = previewFormInternal();
    if ( w )
	w->show();
}

void MainWindow::previewForm( const QString & style )
{
    QStyle* st = QStyleFactory::create( style );
    QWidget* w = 0;
    if ( style == "Motif" ) {
	QPalette p( QColor( 192, 192, 192 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "Windows" ) {
	QPalette p( QColor( 212, 208, 200 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "Platinum" ) {
	QPalette p( QColor( 220, 220, 220 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "CDE" ) {
	QPalette p( QColor( 75, 123, 130 ) );
	p.setColor( QPalette::Active, QColorGroup::Base, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Inactive, QColorGroup::Base, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Disabled, QColorGroup::Base, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Active, QColorGroup::Highlight, Qt::white );
	p.setColor( QPalette::Active, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Inactive, QColorGroup::Highlight, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Disabled, QColorGroup::Highlight, Qt::white );
	p.setColor( QPalette::Disabled, QColorGroup::HighlightedText, QColor( 55, 77, 78 ) );
	p.setColor( QPalette::Active, QColorGroup::Foreground, Qt::white );
	p.setColor( QPalette::Active, QColorGroup::Text, Qt::white );
	p.setColor( QPalette::Active, QColorGroup::ButtonText, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::Foreground, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::Text, Qt::white );
	p.setColor( QPalette::Inactive, QColorGroup::ButtonText, Qt::white );
	p.setColor( QPalette::Disabled, QColorGroup::Foreground, Qt::lightGray );
	p.setColor( QPalette::Disabled, QColorGroup::Text, Qt::lightGray );
	p.setColor( QPalette::Disabled, QColorGroup::ButtonText, Qt::lightGray );

	w = previewFormInternal( st, &p );
    } else if ( style == "SGI" ) {
	QPalette p( QColor( 220, 220, 220 ) );
	w = previewFormInternal( st, &p );
    } else if ( style == "MotifPlus" ) {
	QColor gtkfg(0x00, 0x00, 0x00);
	QColor gtkdf(0x75, 0x75, 0x75);
	QColor gtksf(0xff, 0xff, 0xff);
	QColor gtkbs(0xff, 0xff, 0xff);
	QColor gtkbg(0xd6, 0xd6, 0xd6);
	QColor gtksl(0x00, 0x00, 0x9c);
	QColorGroup active(gtkfg,            // foreground
			   gtkbg,            // button
			   gtkbg.light(),    // light
			   gtkbg.dark(142),  // dark
			   gtkbg.dark(110),  // mid
			   gtkfg,            // text
			   gtkfg,            // bright text
			   gtkbs,            // base
			   gtkbg),           // background
	    disabled(gtkdf,            // foreground
		     gtkbg,            // button
		     gtkbg.light(), // light
		     gtkbg.dark(156),  // dark
		     gtkbg.dark(110),  // mid
		     gtkdf,            // text
		     gtkdf,            // bright text
		     gtkbs,            // base
		     gtkbg);           // background

	QPalette pal(active, disabled, active);

	pal.setColor(QPalette::Active, QColorGroup::Highlight,
		     gtksl);
	pal.setColor(QPalette::Active, QColorGroup::HighlightedText,
		     gtksf);
	pal.setColor(QPalette::Inactive, QColorGroup::Highlight,
		     gtksl);
	pal.setColor(QPalette::Inactive, QColorGroup::HighlightedText,
		     gtksf);
	pal.setColor(QPalette::Disabled, QColorGroup::Highlight,
		     gtksl);
	pal.setColor(QPalette::Disabled, QColorGroup::HighlightedText,
		     gtkdf);
	w = previewFormInternal( st, &pal );
    } else {
	w = previewFormInternal( st );
    }

    if ( !w )
	return;
    w->insertChild( st );
    w->show();
}

void MainWindow::helpContents()
{
    QWidget *focusWidget = qApp->focusWidget();
    bool showClassDocu = TRUE;
    while ( focusWidget ) {
	if ( focusWidget->isA( "PropertyList" ) ) {
	    showClassDocu = FALSE;
	    break;
	}
	focusWidget = focusWidget->parentWidget();
    }

    QString source = "book1.html";
    if ( propertyDocumentation.isEmpty() ) {
	QString indexFile = documentationPath() + "/propertyindex";
	QFile f( indexFile );
	if ( f.open( IO_ReadOnly ) ) {
	    QTextStream ts( &f );
	    while ( !ts.eof() ) {
		QString s = ts.readLine();
		int from = s.find( "\"" );
		if ( from == -1 )
		    continue;
		int to = s.findRev( "\"" );
		if ( to == -1 )
		    continue;
		propertyDocumentation[ s.mid( from + 1, to - from - 1 ) ] = s.mid( to + 2 ) + "-prop";
	    }
	    f.close();
	} else {
	    QMessageBox::critical( this, tr( "Error" ), tr( "Couldn't find the Qt documentation property index file!\n"
					    "Define the correct documentation path in the preferences dialog." ) );
	}
    }

    if ( propertyEditor->widget() && !showClassDocu ) {
	if ( !propertyEditor->currentProperty().isEmpty() ) {
	    QMetaObject* mo = propertyEditor->metaObjectOfCurrentProperty();
	    QString s;
	    QString cp = propertyEditor->currentProperty();
	    if ( cp == "layoutMargin" ) {
		source = propertyDocumentation[ "QLayout/margin" ];
	    } else if ( cp == "layoutSpacing" ) {
		source = propertyDocumentation[ "QLayout/spacing" ];
	    } else if ( cp == "toolTip" ) {
		source = "qtooltip.html#details";
	    } else if ( mo && qstrcmp( mo->className(), "Spacer" ) == 0 ) {
		if ( cp != "name" )
		    source = "qsizepolicy.html#SizeType";
		else
		    source = propertyDocumentation[ "QObject/name" ];
	    } else {
		while ( mo && !propertyDocumentation.contains( ( s = QString( mo->className() ) + "/" + cp ) ) )
		    mo = mo->superClass();
		if ( mo )
		    source = "p:" + propertyDocumentation[s];
	    }
	}

	QString classname =  WidgetFactory::classNameOf( propertyEditor->widget() );
	if ( source.isEmpty() || source == "book1.html" ) {
	    if ( classname.lower() == "spacer" )
		source = "qspaceritem.html#details";
	    else if ( classname == "QLayoutWidget" )
		source = "layout.html";
	    else
		source = QString( WidgetFactory::classNameOf( propertyEditor->widget() ) ).lower() + ".html#details";
	}
    } else if ( propertyEditor->widget() ) {
	source = QString( WidgetFactory::classNameOf( propertyEditor->widget() ) ).lower() + ".html#details";
    }

    if ( !source.isEmpty() ) {
	QStringList lst;
	lst << "assistant" << source;
	QProcess proc( lst );
	proc.start();
    }

}

void MainWindow::helpManual()
{
    QStringList lst;
    lst << "assistant" << "book1.html";
    QProcess proc( lst );
    proc.start();
}

void MainWindow::helpAbout()
{
    AboutDialog dlg( this, 0, TRUE );
    dlg.exec();
}

void MainWindow::helpAboutQt()
{
    QMessageBox::aboutQt( this, "Qt Designer" );
}

void MainWindow::showProperties( QObject *o )
{
    if ( !o->isWidgetType() ) { // ###### QObject stuff todo
	propertyEditor->setWidget( o, lastActiveFormWindow );
	return;
    }
    QWidget *w = (QWidget*)o;
    setupHierarchyView();
    FormWindow *fw = (FormWindow*)isAFormWindowChild( w );
    if ( fw ) {
	propertyEditor->setWidget( w, fw );
	hierarchyView->setFormWindow( fw, w );
    } else {
	propertyEditor->setWidget( 0, 0 );
	hierarchyView->setFormWindow( 0, 0 );
    }

    if ( currentTool() == POINTER_TOOL && fw )
	fw->setFocus();
}

void MainWindow::resetTool()
{
    actionPointerTool->setOn( TRUE );
}

void MainWindow::updateProperties( QObject * )
{
    if ( propertyEditor )
	propertyEditor->refetchData();
}

bool MainWindow::eventFilter( QObject *o, QEvent *e )
{
    if ( !o || !e || !o->isWidgetType() )
	return QMainWindow::eventFilter( o, e );

    QWidget *w = 0;
    bool passiveInteractor = WidgetFactory::isPassiveInteractor( o );
    switch ( e->type() ) {
    case QEvent::MouseButtonPress:
	if ( o->inherits( "QDesignerPopupMenu" ) )
	    break;
	if ( o && currentTool() == POINTER_TOOL && ( o->inherits( "QDesignerMenuBar" ) ||
		    o->inherits( "QDesignerToolBar" ) ||
		    ( o->inherits( "QComboBox") || o->inherits( "QToolButton" ) || o->inherits( "QDesignerToolBarSeparator" ) ) &&
		    o->parent() && o->parent()->inherits( "QDesignerToolBar" ) ) ) {
	    QWidget *w = (QWidget*)o;
	    if ( w->inherits( "QToolButton" ) || w->inherits( "QComboBox" ) || w->inherits( "QDesignerToolBarSeparator" ) )
		w = w->parentWidget();
	    QWidget *pw = w->parentWidget();
	    while ( pw ) {
		if ( pw->inherits( "FormWindow" ) ) {
		    ( (FormWindow*)pw )->emitShowProperties( w );
		    if ( !o->inherits( "QDesignerToolBar" ) )
			return !o->inherits( "QToolButton" ) && !o->inherits( "QMenuBar" ) &&
			    !o->inherits( "QComboBox" ) && !o->inherits( "QDesignerToolBarSeparator" );
		}
		pw = pw->parentWidget();
	    }
	}
	if ( o && ( o->inherits( "QDesignerToolBar" ) || o->inherits( "QDockWindowHandle" ) )
	     && ( (QMouseEvent*)e )->button() == RightButton )
	    break;
	if ( isAToolBarChild( o ) && currentTool() != CONNECT_TOOL )
	    break;
	if ( o && o->inherits( "QSizeGrip" ) )
	    break;
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	if ( !w->hasFocus() )
	    w->setFocus();
	if ( !passiveInteractor || currentTool() != ORDER_TOOL )
	    ( (FormWindow*)w )->handleMousePress( (QMouseEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	lastPressWidget = (QWidget*)o;
	if ( passiveInteractor )
	    QTimer::singleShot( 0, formWindow(), SLOT( visibilityChanged() ) );
	if ( currentTool() == CONNECT_TOOL )
	    return TRUE;
	return !passiveInteractor;
    case QEvent::ContextMenu:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	( (QContextMenuEvent*)e )->ignore(); // ### Reggie: have to implement that properly, for now just keep using mousePress
	return TRUE;
    case QEvent::MouseButtonRelease:
	lastPressWidget = 0;
	if ( isAToolBarChild( o )  && currentTool() != CONNECT_TOOL )
	    break;
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	if ( !passiveInteractor )
	    ( (FormWindow*)w )->handleMouseRelease( (QMouseEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	if ( passiveInteractor ) {
	    selectionChanged();
	    QTimer::singleShot( 0, formWindow(), SLOT( visibilityChanged() ) );
	}
	return !passiveInteractor;
    case QEvent::MouseMove:
	if ( isAToolBarChild( o ) && currentTool() != CONNECT_TOOL )
	    break;
	w = isAFormWindowChild( o );
	if ( lastPressWidget != (QWidget*)o && w &&
	     !o->inherits( "SizeHandle" ) && !o->inherits( "OrderIndicator" ) &&
	     !o->inherits( "QPopupMenu" ) && !o->inherits( "QMenuBar" ) )
	    return TRUE;
	if ( lastPressWidget != (QWidget*)o ||
	     ( !w || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) ) )
	    break;
	if ( !passiveInteractor )
	    ( (FormWindow*)w )->handleMouseMove( (QMouseEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	return !passiveInteractor;
    case QEvent::KeyPress:
	if ( ( (QKeyEvent*)e )->key() == Key_Escape && currentTool() != POINTER_TOOL ) {
	    resetTool();
	    return FALSE;
	}
	if ( ( (QKeyEvent*)e )->key() == Key_Escape && incrementalSearch->hasFocus() ) {
	    if ( workSpace()->activeWindow() && workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
		workSpace()->activeWindow()->setFocus();
		return TRUE;
	    }
	}
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	( (FormWindow*)w )->handleKeyPress( (QKeyEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	if ( ((QKeyEvent*)e)->isAccepted() )
	    return TRUE;
	break;
    case QEvent::MouseButtonDblClick:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) ) {
	    if ( o && o->inherits( "QToolButton" ) && ( ( QToolButton*)o )->isOn() &&
		 o->parent() && o->parent()->inherits( "QToolBar" ) && formWindow() )
		formWindow()->setToolFixed();
	    break;
	}
	if ( currentTool() == ORDER_TOOL ) {
	    ( (FormWindow*)w )->handleMouseDblClick( (QMouseEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	    return TRUE;
	}
	if ( !WidgetFactory::isPassiveInteractor( o ) )
	    return openEditor( ( (FormWindow*)w )->designerWidget( o ), (FormWindow*)w );
	return TRUE;
    case QEvent::KeyRelease:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	( (FormWindow*)w )->handleKeyRelease( (QKeyEvent*)e, ( (FormWindow*)w )->designerWidget( o ) );
	if ( ((QKeyEvent*)e)->isAccepted() )
	    return TRUE;
	break;
    case QEvent::Hide:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	if ( ( (FormWindow*)w )->isWidgetSelected( (QWidget*)o ) )
	    ( (FormWindow*)w )->selectWidget( (QWidget*)o, FALSE );
	break;
    case QEvent::Enter:
    case QEvent::Leave:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	return TRUE;
    case QEvent::Resize:
    case QEvent::Move:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	if ( WidgetFactory::layoutType( (QWidget*)o->parent() ) != WidgetFactory::NoLayout ) {
	    ( (FormWindow*)w )->updateSelection( (QWidget*)o );
	    if ( e->type() != QEvent::Resize )
		( (FormWindow*)w )->updateChildSelections( (QWidget*)o );
	}
	break;
    case QEvent::Close:
	if ( o->inherits( "FormWindow" ) ) {
	    if ( !closeForm( (FormWindow*)o ) ) {
		( (QCloseEvent*)e )->ignore();
	    } else {
		( (QCloseEvent*)e )->accept();
		unregisterClient( (FormWindow*)o );
	    }
	    return TRUE;
	}
	break;
    case QEvent::DragEnter:
	if ( o == workSpace() || o == formlist() || o == formlist()->viewport() ) {
	    formlist()->contentsDragEnterEvent( (QDragEnterEvent*)e );
	    return TRUE;
	}
	break;
    case QEvent::DragMove:
	if ( o == workSpace() || o == formlist() || o == formlist()->viewport() ) {
	    formlist()->contentsDragMoveEvent( (QDragMoveEvent*)e );
	    return TRUE;
	}
	break;
    case QEvent::Drop:
	if ( o == workSpace() || o == formlist() || o == formlist()->viewport() ) {
	    formlist()->contentsDropEvent( (QDropEvent*)e );
	    return TRUE;
	}
	break;
    case QEvent::Show:
	if ( o != this )
	    break;
	if ( ((QShowEvent*)e)->spontaneous() )
	    break;
	QApplication::sendPostedEvents( workspace, QEvent::ChildInserted );
	showEvent( (QShowEvent*)e );
	if ( !tbSettingsRead)
	    ( (QDockWindow*)formlist()->parentWidget() )->setFixedExtentHeight( 150 );
	checkTempFiles();
	return TRUE;
    case QEvent::Wheel:
	if ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) )
	    break;
	return TRUE;
    case QEvent::FocusIn:
    case QEvent::FocusOut:
	if ( isAFormWindowChild( o ) )
	    return TRUE;
	break;
    default:
	return QMainWindow::eventFilter( o, e );
    }

    return QMainWindow::eventFilter( o, e );
}

QWidget *MainWindow::isAFormWindowChild( QObject *o ) const
{
    if ( o->parent() && o->parent()->inherits( "QWizard" ) && !o->inherits( "QPushButton" ) )
	return 0;
    while ( o ) {
	if ( o->inherits( "FormWindow" ) )
	    return (QWidget*)o;
	o = o->parent();
    }
    return 0;
}

QWidget *MainWindow::isAToolBarChild( QObject *o ) const
{
    while ( o ) {
	if ( o->inherits( "QDesignerToolBar" ) )
	    return (QWidget*)o;
	if ( o->inherits( "FormWindow" ) )
	    return 0;
	o = o->parent();
    }
    return 0;
}

FormWindow *MainWindow::formWindow()
{
    if ( workspace->activeWindow() ) {
	FormWindow *fw = 0;
	if ( workspace->activeWindow()->inherits( "FormWindow" ) )
	    fw = (FormWindow*)workspace->activeWindow();
	else if ( lastActiveFormWindow &&
		    workspace->windowList().find( lastActiveFormWindow ) != -1)
	    fw = lastActiveFormWindow;
	return fw;
    }
    return 0;
}

FormWindow* MainWindow::insertFormWindow( int type )
{
    QString n = tr( "Form%1" ).arg( ++forms );
    FormWindow *fw = 0;
    fw = new FormWindow( this, workspace, n );
    MetaDataBase::addEntry( fw );
    if ( type == NewForm::Widget ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWidget" ), fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( type == NewForm::Dialog ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QDialog" ), fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( type == NewForm::Wizard ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QWizard" ), fw, n.latin1() );
	fw->setMainContainer( w );
    } else if ( type == NewForm::Mainwindow ) {
	QWidget *w = WidgetFactory::create( WidgetDatabase::idFromClassName( "QMainWindow" ), fw, n.latin1() );
	fw->setMainContainer( w );
    }

    fw->setCaption( n );
    fw->resize( 600, 480 );
    MetaDataBase::addEntry( fw );
    insertFormWindow( fw );

    TemplateWizardInterface *iface = templateWizardInterface( fw->mainContainer()->className() );
    if ( iface ) {
	iface->setup( fw->mainContainer()->className(), fw->mainContainer(), fw->iFace(), desInterface );
	iface->release();
    }

    // the wizard might have changed a lot, lets update everything
    actionEditor->setFormWindow( fw );
    hierarchyView->setFormWindow( fw, fw );
    hierarchyView->functionList()->refreshFunctions();
    fw->killAccels( fw );
    fw->project()->pixmapCollection()->createCppFile();

    return fw;
}

void MainWindow::insertFormWindow( FormWindow *fw )
{
    if ( fw )
	QWhatsThis::add( fw, tr( "<b>The Form Window</b>"
			       "<p>Use the different tools to add widgets or to change the layout "
			       "and behaviour of the components in your form. Select one or multiple "
			       "widgets and move them, or resize a single widget using the handles.</p>"
			       "<p>Changes in the <b>Property Editor</b> can be seen at design time, "
			       "and you can open a preview of your form in different styles.</p>"
			       "<p>You can change the grid resolution, or turn the grid off in the "
			       "<b>Preferences</b> dialog in the <b>Edit</b> menu."
			       "<p>You can have several forms open, and all open forms are listed "
			       "in the <b>Form List</b>.") );

    connect( fw, SIGNAL( showProperties( QObject * ) ),
	     this, SLOT( showProperties( QObject * ) ) );
    connect( fw, SIGNAL( updateProperties( QObject * ) ),
	     this, SLOT( updateProperties( QObject * ) ) );
    connect( this, SIGNAL( currentToolChanged() ),
	     fw, SLOT( currentToolChanged() ) );
    connect( fw, SIGNAL( selectionChanged() ),
	     this, SLOT( selectionChanged() ) );
    connect( fw, SIGNAL( undoRedoChanged( bool, bool, const QString &, const QString & ) ),
	     this, SLOT( updateUndoRedo( bool, bool, const QString &, const QString & ) ) );
    connect( fw, SIGNAL( fileNameChanged( const QString &, FormWindow * ) ),
	     formlist(), SLOT( fileNameChanged( const QString &, FormWindow * ) ) );
    connect( fw, SIGNAL( modificationChanged( bool, FormWindow * ) ),
	     formlist(), SLOT( modificationChanged( bool, FormWindow * ) ) );
    connect( fw, SIGNAL( modificationChanged( bool, FormWindow * ) ),
	     this, SIGNAL( formModified( bool ) ) );

    if ( !mblockNewForms ) {
	formlist()->addForm( fw );
    } else {
	fw->setProject( currentProject );
	currentProject->setFormWindow( fw->fileName(), fw );
    }
    fw->show();
    fw->currentToolChanged();
    if ( fw->caption().isEmpty() && qstrlen( fw->name() )  )
	fw->setCaption( fw->name() );
    fw->mainContainer()->setCaption( fw->caption() );
    activeWindowChanged( fw );
    emit formWindowsChanged();
}

bool MainWindow::unregisterClient( FormWindow *w )
{
    propertyEditor->closed( w );
    objectHierarchy()->closed( w );
    if ( w->fileName().isEmpty() )
	formList->removeForm( w );
    formList->closed( w );
    if ( w == lastActiveFormWindow )
	lastActiveFormWindow = 0;

    QPtrList<SourceEditor> waitingForDelete;
    waitingForDelete.setAutoDelete( TRUE );
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->object() == w )
	    waitingForDelete.append( e );
    }

    if ( actionEditor->form() == w ) {
	actionEditor->setFormWindow( 0 );
	actionEditor->parentWidget()->hide();
    }

    return TRUE;
}

void MainWindow::activeWindowChanged( QWidget *w )
{
    QWidget *old = formWindow();
    if ( w && w->inherits( "FormWindow" ) ) {
	lastActiveFormWindow = (FormWindow*)w;
	lastActiveFormWindow->updateUndoInfo();
	emit hasActiveForm( TRUE );
	if ( formWindow() ) {
	    formWindow()->emitShowProperties();
	    emit formModified( formWindow()->commandHistory()->isModified() );
	    if ( currentTool() != POINTER_TOOL )
		formWindow()->clearSelection();
	}
	formlist()->activeFormChanged( (FormWindow*)w );
	setAppropriate( (QDockWindow*)actionEditor->parentWidget(), lastActiveFormWindow->mainContainer()->inherits( "QMainWindow" ) );
	if ( appropriate( (QDockWindow*)actionEditor->parentWidget() ) )
	    actionEditor->parentWidget()->show();
	else
	    actionEditor->parentWidget()->hide();

	actionEditor->setFormWindow( lastActiveFormWindow );
	if ( formList && ( (FormWindow*)w )->project() && ( (FormWindow*)w )->project() != currentProject ) {
	    for ( QMap<QAction*, Project *>::Iterator it = projects.begin(); it != projects.end(); ++it ) {
		if ( *it == ( (FormWindow*)w )->project() ) {
		    projectSelected( it.key() );
		    it.key()->setOn( TRUE );
		    break;
		}
	    }
	}
	emit formWindowChanged();

    } else if ( w == propertyEditor ) {
	propertyEditor->resetFocus();
    } else if ( !lastActiveFormWindow ) {
	emit formWindowChanged();
	emit hasActiveForm( FALSE );
	actionEditUndo->setEnabled( FALSE );
	actionEditRedo->setEnabled( FALSE );
    }

    selectionChanged();

    if ( w && w->inherits( "SourceEditor" ) ) {
	actionSearchFind->setEnabled( TRUE );
	actionSearchIncremetal->setEnabled( TRUE );
	actionSearchReplace->setEnabled( TRUE );
	actionSearchGotoLine->setEnabled( TRUE );
	incrementalSearch->setEnabled( TRUE );

	actionEditUndo->setEnabled( TRUE );
	actionEditRedo->setEnabled( TRUE );
	actionEditCut->setEnabled( TRUE );
	actionEditCopy->setEnabled( TRUE );
	actionEditPaste->setEnabled( TRUE );
	actionEditSelectAll->setEnabled( TRUE );
	actionEditUndo->setMenuText( tr( "&Undo" ) );
	actionEditUndo->setToolTip( textNoAccel( actionEditUndo->menuText()) );
	actionEditRedo->setMenuText( tr( "&Redo" ) );
	actionEditRedo->setToolTip( textNoAccel( actionEditRedo->menuText()) );
    } else {
	actionSearchFind->setEnabled( FALSE );
	actionSearchIncremetal->setEnabled( FALSE );
	actionSearchReplace->setEnabled( FALSE );
	actionSearchGotoLine->setEnabled( FALSE );
	incrementalSearch->setEnabled( FALSE );
    }

    if ( currentTool() == ORDER_TOOL && w != old )
	emit currentToolChanged();

    emit hasActiveWindow( !!workspace->activeWindow() );
}

void MainWindow::updateUndoRedo( bool undoAvailable, bool redoAvailable,
				 const QString &undoCmd, const QString &redoCmd )
{
    actionEditUndo->setEnabled( undoAvailable );
    actionEditRedo->setEnabled( redoAvailable );
    if ( !undoCmd.isEmpty() )
	actionEditUndo->setMenuText( tr( "&Undo: %1" ).arg( undoCmd ) );
    else
	actionEditUndo->setMenuText( tr( "&Undo: Not Available" ) );
    if ( !redoCmd.isEmpty() )
	actionEditRedo->setMenuText( tr( "&Redo: %1" ).arg( redoCmd ) );
    else
	actionEditRedo->setMenuText( tr( "&Redo: Not Available" ) );

    actionEditUndo->setToolTip( textNoAccel( actionEditUndo->menuText()) );
    actionEditRedo->setToolTip( textNoAccel( actionEditRedo->menuText()) );

    if ( currentTool() == ORDER_TOOL ) {
	actionEditUndo->setEnabled( FALSE );
	actionEditRedo->setEnabled( FALSE );
    }
}

QWorkspace *MainWindow::workSpace() const
{
    return workspace;
}

void MainWindow::popupFormWindowMenu( const QPoint & gp, FormWindow *fw )
{
    QValueList<int> ids;
    QMap<QString, int> commands;

    setupRMBSpecialCommands( ids, commands, fw );
    setupRMBProperties( ids, commands, fw );

    qApp->processEvents();
    int r = rmbFormWindow->exec( gp );

    handleRMBProperties( r, commands, fw );
    handleRMBSpecialCommands( r, commands, fw );

    for ( QValueList<int>::Iterator i = ids.begin(); i != ids.end(); ++i )
	rmbFormWindow->removeItem( *i );
}

void MainWindow::popupWidgetMenu( const QPoint &gp, FormWindow * /*fw*/, QWidget * w)
{
    QValueList<int> ids;
    QMap<QString, int> commands;

    setupRMBSpecialCommands( ids, commands, w );
    setupRMBProperties( ids, commands, w );

    qApp->processEvents();
    int r = rmbWidgets->exec( gp );

    handleRMBProperties( r, commands, w );
    handleRMBSpecialCommands( r, commands, w );

    for ( QValueList<int>::Iterator i = ids.begin(); i != ids.end(); ++i )
	rmbWidgets->removeItem( *i );
}

void MainWindow::setupRMBProperties( QValueList<int> &ids, QMap<QString, int> &props, QWidget *w )
{
    const QMetaProperty* text = w->metaObject()->property( w->metaObject()->findProperty( "text", TRUE ), TRUE );
    if ( text && qstrcmp( text->type(), "QString") != 0 )
	text = 0;
    const QMetaProperty* title = w->metaObject()->property( w->metaObject()->findProperty( "title", TRUE ), TRUE );
    if ( title && qstrcmp( title->type(), "QString") != 0 )
	title = 0;
    const QMetaProperty* pagetitle =
	w->metaObject()->property( w->metaObject()->findProperty( "pageTitle", TRUE ), TRUE );
    if ( pagetitle && qstrcmp( pagetitle->type(), "QString") != 0 )
	pagetitle = 0;
    const QMetaProperty* pixmap =
	w->metaObject()->property( w->metaObject()->findProperty( "pixmap", TRUE ), TRUE );
    if ( pixmap && qstrcmp( pixmap->type(), "QPixmap") != 0 )
	pixmap = 0;

    if ( text && text->designable(w) ||
	 title && title->designable(w) ||
	 pagetitle && pagetitle->designable(w) ||
	 pixmap && pixmap->designable(w) ) {
	int id = 0;
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator(0);
	if ( pixmap && pixmap->designable(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Choose Pixmap..."), -1, 0) );
	    props.insert( "pixmap", id );
	}
	if ( text && text->designable(w) && !w->inherits( "QTextEdit" ) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Edit Text..."), -1, 0) );
	    props.insert( "text", id );
	}
	if ( title && title->designable(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Edit Title..."), -1, 0) );
	    props.insert( "title", id );
	}
	if ( pagetitle && pagetitle->designable(w) ) {
	    ids << ( id = rmbWidgets->insertItem( tr("Edit Page Title..."), -1, 0) );
	    props.insert( "pagetitle", id );
	}
    }
}

void MainWindow::setupRMBSpecialCommands( QValueList<int> &ids, QMap<QString, int> &commands, QWidget *w )
{
    int id;

    if ( w->inherits( "QTabWidget" ) ) {
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator( 0 );
	if ( ( (QDesignerTabWidget*)w )->count() > 1) {
	    ids << ( id = rmbWidgets->insertItem( tr("Remove Page"), -1, 0 ) );
	    commands.insert( "remove", id );
	}
	ids << ( id = rmbWidgets->insertItem( tr("Add Page"), -1, 0 ) );
	commands.insert( "add", id );
    }
    if ( WidgetFactory::hasSpecialEditor( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) ) {
	if ( ids.isEmpty() )
	    ids << rmbWidgets->insertSeparator( 0 );
	ids << ( id = rmbWidgets->insertItem( tr("Edit..."), -1, 0 ) );
	commands.insert( "edit", id );
    }
}

void MainWindow::setupRMBSpecialCommands( QValueList<int> &ids, QMap<QString, int> &commands, FormWindow *fw )
{
    int id;

    if ( fw->mainContainer()->inherits( "QWizard" ) ) {
	if ( ids.isEmpty() )
	    ids << rmbFormWindow->insertSeparator( 0 );

	if ( ( (QWizard*)fw->mainContainer() )->pageCount() > 1) {
	    ids << ( id = rmbFormWindow->insertItem( tr("Remove Page"), -1, 0 ) );
	    commands.insert( "remove", id );
	}

	ids << ( id = rmbFormWindow->insertItem( tr("Add Page"), -1, 0 ) );
	commands.insert( "add", id );

	ids << ( id = rmbFormWindow->insertItem( tr("Edit Page Title..."), -1, 0 ) );
	commands.insert( "rename", id );

	ids << ( id = rmbFormWindow->insertItem( tr("Edit Pages..."), -1, 0 ) );
	commands.insert( "edit", id );

    } else if ( fw->mainContainer()->inherits( "QMainWindow" ) ) {
	if ( ids.isEmpty() )
	    ids << rmbFormWindow->insertSeparator( 0 );
	ids << ( id = rmbFormWindow->insertItem( tr( "Add Menu Item" ), -1, 0 ) );
	commands.insert( "add_menu_item", id );
	ids << ( id = rmbFormWindow->insertItem( tr( "Add Toolbar" ), -1, 0 ) );
	commands.insert( "add_toolbar", id );
    }
}

void MainWindow::handleRMBProperties( int id, QMap<QString, int> &props, QWidget *w )
{
    if ( id == props[ "text" ] ) {
	bool ok = FALSE;
	QString text;
	if ( w->inherits( "QTextView" ) || w->inherits( "QLabel" ) ) {
	    text = TextEditor::getText( this, w->property("text").toString() );
	    ok = !text.isEmpty();
	} else {
	    text = QInputDialog::getText( tr("Text"), tr( "New text" ), QLineEdit::Normal, w->property("text").toString(), &ok, this );
	}
	if ( ok ) {
	    QString pn( tr( "Set 'text' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "text", w->property( "text" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "text", TRUE );
	}
    } else if ( id == props[ "title" ] ) {
	bool ok = FALSE;
	QString title = QInputDialog::getText( tr("Title"), tr( "New title" ), QLineEdit::Normal, w->property("title").toString(), &ok, this );
	if ( ok ) {
	    QString pn( tr( "Set 'title' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "title", w->property( "title" ),
							      title, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "title", TRUE );
	}
    } else if ( id == props[ "pagetitle" ] ) {
	bool ok = FALSE;
	QString text = QInputDialog::getText( tr("Page Title"), tr( "New page title" ), QLineEdit::Normal, w->property("pageTitle").toString(), &ok, this );
	if ( ok ) {
	    QString pn( tr( "Set 'pageTitle' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "pageTitle", w->property( "pageTitle" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "pageTitle", TRUE );
	}
    } else if ( id == props[ "pixmap" ] ) {
	QPixmap oldPix = w->property( "pixmap" ).toPixmap();
	QPixmap pix = qChoosePixmap( this, formWindow(), oldPix );
	if ( !pix.isNull() ) {
	    QString pn( tr( "Set 'pixmap' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "pixmap", w->property( "pixmap" ),
							      pix, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "pixmap", TRUE );
	}
    }
}

void MainWindow::handleRMBSpecialCommands( int id, QMap<QString, int> &commands, QWidget *w )
{
    if ( w->inherits( "QTabWidget" ) ) {
	QTabWidget *tw = (QTabWidget*)w;
	if ( id == commands[ "add" ] ) {
	    AddTabPageCommand *cmd = new AddTabPageCommand( tr( "Add Page to %1" ).arg( tw->name() ), formWindow(),
							    tw, "Tab" );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    if ( tw->currentPage() ) {
		QDesignerTabWidget *dtw = (QDesignerTabWidget*)tw;
		DeleteTabPageCommand *cmd = new DeleteTabPageCommand( tr( "Remove Page %1 of %2" ).
								      arg( dtw->pageTitle() ).arg( tw->name() ),
								      formWindow(), tw, tw->currentPage() );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	}
    }
    if ( WidgetFactory::hasSpecialEditor( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) ) {
	if ( id == commands[ "edit" ] )
	    WidgetFactory::editWidget( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ), this, w, formWindow() );
    }
}

void MainWindow::handleRMBSpecialCommands( int id, QMap<QString, int> &commands, FormWindow *fw )
{
    if ( fw->mainContainer()->inherits( "QWizard" ) ) {
	QWizard *wiz = (QWizard*)fw->mainContainer();
	if ( id == commands[ "add" ] ) {
	    AddWizardPageCommand *cmd = new AddWizardPageCommand( tr( "Add Page to %1" ).arg( wiz->name() ), formWindow(),
								  wiz, "Page" );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "remove" ] ) {
	    if ( wiz->currentPage() ) {
		QDesignerWizard *dw = (QDesignerWizard*)wiz;
		DeleteWizardPageCommand *cmd = new DeleteWizardPageCommand( tr( "Remove Page %1 of %2" ).
									    arg( dw->pageTitle() ).arg( wiz->name() ),
									    formWindow(), wiz, wiz->indexOf( wiz->currentPage() ) );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	} else if ( id == commands[ "edit" ] ) {
	    WizardEditor *e = new WizardEditor( this, wiz, fw );
	    e->exec();
	    delete e;
	} else if ( id == commands[ "rename" ] ) {

	    bool ok = FALSE;
	    QDesignerWizard *dw = (QDesignerWizard*)wiz;
	    QString text = QInputDialog::getText( tr("Page Title"), tr( "New page title" ), QLineEdit::Normal, dw->pageTitle(), &ok, this );
	    if ( ok ) {
		QString pn( tr( "Rename page %1 of %2" ).arg( dw->pageTitle() ).arg( wiz->name() ) );
		RenameWizardPageCommand *cmd = new RenameWizardPageCommand( pn, formWindow()
									    , wiz, wiz->indexOf( wiz->currentPage() ), text );
		formWindow()->commandHistory()->addCommand( cmd );
		cmd->execute();
	    }
	}
    } else if ( fw->mainContainer()->inherits( "QMainWindow" ) ) {
	QMainWindow *mw = (QMainWindow*)fw->mainContainer();
	if ( id == commands[ "add_toolbar" ] ) {
	    AddToolBarCommand *cmd = new AddToolBarCommand( tr( "Add Toolbar to '%1'" ).arg( formWindow()->name() ), formWindow(), mw );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	} else if ( id == commands[ "add_menu_item" ] ) {
	    AddMenuCommand *cmd = new AddMenuCommand( tr( "Add Menu to '%1'" ).arg( formWindow()->name() ), formWindow(), mw );
	    formWindow()->commandHistory()->addCommand( cmd );
	    cmd->execute();
	}
    }
}

void MainWindow::clipboardChanged()
{
    QString text( qApp->clipboard()->text() );
    QString start( "<!DOCTYPE UI-SELECTION>" );
    actionEditPaste->setEnabled( text.left( start.length() ) == start );
}

void MainWindow::selectionChanged()
{
    layoutChilds = FALSE;
    layoutSelected = FALSE;
    breakLayout = FALSE;
    if ( !formWindow() ) {
	actionEditCut->setEnabled( FALSE );
	actionEditCopy->setEnabled( FALSE );
	actionEditDelete->setEnabled( FALSE );
	actionEditAdjustSize->setEnabled( FALSE );
	actionEditHLayout->setEnabled( FALSE );
	actionEditVLayout->setEnabled( FALSE );
	actionEditSplitHorizontal->setEnabled( FALSE );
	actionEditSplitVertical->setEnabled( FALSE );
	actionEditGridLayout->setEnabled( FALSE );
	actionEditBreakLayout->setEnabled( FALSE );
	actionEditLower->setEnabled( FALSE );
	actionEditRaise->setEnabled( FALSE );
	actionEditAdjustSize->setEnabled( FALSE );
	return;
    }

    int selectedWidgets = formWindow()->numSelectedWidgets();
    bool enable = selectedWidgets > 0;
    actionEditCut->setEnabled( enable );
    actionEditCopy->setEnabled( enable );
    actionEditDelete->setEnabled( enable );
    actionEditLower->setEnabled( enable );
    actionEditRaise->setEnabled( enable );

    actionEditAdjustSize->setEnabled( FALSE );
    actionEditSplitHorizontal->setEnabled( FALSE );
    actionEditSplitVertical->setEnabled( FALSE );

    enable = FALSE;
    QWidgetList widgets = formWindow()->selectedWidgets();
    if ( selectedWidgets > 1 ) {
	int unlaidout = 0;
	int laidout = 0;
	for ( QWidget *w = widgets.first(); w; w = widgets.next() ) {
	    if ( !w->parentWidget() || WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout )
		unlaidout++;
	    else
		laidout++;
	}
	actionEditHLayout->setEnabled( unlaidout > 1 );
	actionEditVLayout->setEnabled( unlaidout > 1 );
	actionEditSplitHorizontal->setEnabled( unlaidout > 1 );
	actionEditSplitVertical->setEnabled( unlaidout > 1 );
	actionEditGridLayout->setEnabled( unlaidout > 1 );
	actionEditBreakLayout->setEnabled( laidout > 0 );
	actionEditAdjustSize->setEnabled( laidout > 0 );
	layoutSelected = unlaidout > 1;
	breakLayout = laidout > 0;
    } else if ( selectedWidgets == 1 ) {
	QWidget *w = widgets.first();
	bool isContainer = WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) ||
			   w == formWindow()->mainContainer();
	actionEditAdjustSize->setEnabled( !w->parentWidget() ||
					  WidgetFactory::layoutType( w->parentWidget() ) == WidgetFactory::NoLayout );

	if ( !isContainer ) {
	    actionEditHLayout->setEnabled( FALSE );
	    actionEditVLayout->setEnabled( FALSE );
	    actionEditGridLayout->setEnabled( FALSE );
	    if ( w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout ) {
		actionEditBreakLayout->setEnabled( !isAToolBarChild( w ) );
		breakLayout = TRUE;
	    } else {
		actionEditBreakLayout->setEnabled( FALSE );
	    }
	} else {
	    if ( WidgetFactory::layoutType( w ) == WidgetFactory::NoLayout ) {
		if ( !formWindow()->hasInsertedChildren( w ) ) {
		    actionEditHLayout->setEnabled( FALSE );
		    actionEditVLayout->setEnabled( FALSE );
		    actionEditGridLayout->setEnabled( FALSE );
		    actionEditBreakLayout->setEnabled( FALSE );
		} else {
		    actionEditHLayout->setEnabled( TRUE );
		    actionEditVLayout->setEnabled( TRUE );
		    actionEditGridLayout->setEnabled( TRUE );
		    actionEditBreakLayout->setEnabled( FALSE );
		    layoutChilds = TRUE;
		}
		if ( w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout ) {
		    actionEditBreakLayout->setEnabled( !isAToolBarChild( w ) );
		    breakLayout = TRUE;
		}
	    } else {
		actionEditHLayout->setEnabled( FALSE );
		actionEditVLayout->setEnabled( FALSE );
		actionEditGridLayout->setEnabled( FALSE );
		actionEditBreakLayout->setEnabled( !isAToolBarChild( w ) );
		breakLayout = TRUE;
	    }
	}
    } else if ( selectedWidgets == 0 && formWindow() ) {
	actionEditAdjustSize->setEnabled( TRUE );
	QWidget *w = formWindow()->mainContainer();
	if ( WidgetFactory::layoutType( w ) == WidgetFactory::NoLayout ) {
	    if ( !formWindow()->hasInsertedChildren( w ) ) {
		actionEditHLayout->setEnabled( FALSE );
		actionEditVLayout->setEnabled( FALSE );
		actionEditGridLayout->setEnabled( FALSE );
		actionEditBreakLayout->setEnabled( FALSE );
	    } else {
		actionEditHLayout->setEnabled( TRUE );
		actionEditVLayout->setEnabled( TRUE );
		actionEditGridLayout->setEnabled( TRUE );
		actionEditBreakLayout->setEnabled( FALSE );
		layoutChilds = TRUE;
	    }
	} else {
	    actionEditHLayout->setEnabled( FALSE );
	    actionEditVLayout->setEnabled( FALSE );
	    actionEditGridLayout->setEnabled( FALSE );
	    actionEditBreakLayout->setEnabled( TRUE );
	    breakLayout = TRUE;
	}
    } else {
	actionEditHLayout->setEnabled( FALSE );
	actionEditVLayout->setEnabled( FALSE );
	actionEditGridLayout->setEnabled( FALSE );
	actionEditBreakLayout->setEnabled( FALSE );
    }
}

static QString fixArgs( const QString &s2 )
{
    QString s = s2;
    return s.replace( QRegExp( "," ), ";" );
}

void MainWindow::writeConfig()
{
    QSettings config;

    // No search path for unix, only needs application name
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    QString keybase = DesignerApplication::settingsKey();
    config.writeEntry( keybase + "RestoreWorkspace", restoreConfig );
    config.writeEntry( keybase + "SplashScreen", splashScreen );
    config.writeEntry( keybase + "DocPath", docPath );
    config.writeEntry( keybase + "FileFilter", fileFilter );
    config.writeEntry( keybase + "TemplatePath", templPath );
    config.writeEntry( keybase + "RecentlyOpenedFiles", recentlyFiles, ',' );
    config.writeEntry( keybase + "RecentlyOpenedProjects", recentlyProjects, ',' );
    config.writeEntry( keybase + "DatabaseAutoEdit", databaseAutoEdit );

    config.writeEntry( keybase + "Grid/Snap", snGrid );
    config.writeEntry( keybase + "Grid/Show", sGrid );
    config.writeEntry( keybase + "Grid/x", grid().x() );
    config.writeEntry( keybase + "Grid/y", grid().y() );

    config.writeEntry( keybase + "Background/UsePixmap", backPix );
    config.writeEntry( keybase + "Background/Color", (int)workspace->backgroundColor().rgb() );
    if ( workspace->backgroundPixmap() )
	workspace->backgroundPixmap()->save( QDir::home().absPath() + "/.designer/" + "background.xpm", "XPM" );

    config.writeEntry( keybase + "Geometries/MainwindowX", x() );
    config.writeEntry( keybase + "Geometries/MainwindowY", y() );
    config.writeEntry( keybase + "Geometries/MainwindowWidth", width() );
    config.writeEntry( keybase + "Geometries/MainwindowHeight", height() );
    config.writeEntry( keybase + "Geometries/PropertyEditorX", propertyEditor->parentWidget()->x() );
    config.writeEntry( keybase + "Geometries/PropertyEditorY", propertyEditor->parentWidget()->y() );
    config.writeEntry( keybase + "Geometries/PropertyEditorWidth", propertyEditor->parentWidget()->width() );
    config.writeEntry( keybase + "Geometries/PropertyEditorHeight", propertyEditor->parentWidget()->height() );
    config.writeEntry( keybase + "Geometries/HierarchyViewX", hierarchyView->parentWidget()->x() );
    config.writeEntry( keybase + "Geometries/HierarchyViewY", hierarchyView->parentWidget()->y() );
    config.writeEntry( keybase + "Geometries/HierarchyViewWidth", hierarchyView->parentWidget()->width() );
    config.writeEntry( keybase + "Geometries/HierarchyViewHeight", hierarchyView->parentWidget()->height() );
    config.writeEntry( keybase + "Geometries/FormListX", formList->parentWidget()->x() );
    config.writeEntry( keybase + "Geometries/FormListY", formList->parentWidget()->y() );
    config.writeEntry( keybase + "Geometries/FormListWidth", formList->parentWidget()->width() );
    config.writeEntry( keybase + "Geometries/FormListHeight", formList->parentWidget()->height() );

    config.writeEntry( keybase + "View/TextLabels", usesTextLabel() );
    config.writeEntry( keybase + "View/BigIcons", usesBigPixmaps() );

    QString fn = QDir::homeDirPath() + "/.designerrctb2";
    QFile f( fn );
    f.open( IO_WriteOnly );
    QTextStream ts( &f );
    ts << *this;
    f.close();

    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    config.writeEntry( keybase + "CustomWidgets/num", (int)lst->count() );
    int j = 0;
    QDir::home().mkdir( ".designer" );
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	QStringList l;
	l << w->className;
	l << w->includeFile;
	l << QString::number( (int)w->includePolicy );
	l << QString::number( w->sizeHint.width() );
	l << QString::number( w->sizeHint.height() );
	l << QString::number( w->lstSignals.count() );
	for ( QValueList<QCString>::Iterator it = w->lstSignals.begin(); it != w->lstSignals.end(); ++it )
	    l << QString( fixArgs( *it ) );
	l << QString::number( w->lstSlots.count() );
	for ( QValueList<MetaDataBase::Slot>::Iterator it2 = w->lstSlots.begin(); it2 != w->lstSlots.end(); ++it2 ) {
	    l << fixArgs( (*it2).slot );
	    l << (*it2).access;
	}
	l << QString::number( w->lstProperties.count() );
	for ( QValueList<MetaDataBase::Property>::Iterator it3 = w->lstProperties.begin(); it3 != w->lstProperties.end(); ++it3 ) {
	    l << (*it3).property;
	    l << (*it3).type;
	}
	l << QString::number( size_type_to_int( w->sizePolicy.horData() ) );
	l << QString::number( size_type_to_int( w->sizePolicy.verData() ) );
	l << QString::number( (int)w->isContainer );
	config.writeEntry( keybase + "CustomWidgets/Widget" + QString::number( j++ ), l, ',' );
	w->pixmap->save( QDir::home().absPath() + "/.designer/" + w->className, "XPM" );
    }
}

static QString fixArgs2( const QString &s2 )
{
    QString s = s2;
    return s.replace( QRegExp( ";" ), "," );
}

void MainWindow::readConfig()
{
    QString keybase = DesignerApplication::settingsKey();
    QSettings config;
    config.insertSearchPath( QSettings::Windows, "/Trolltech" );

    bool ok;
    restoreConfig = config.readBoolEntry( keybase + "RestoreWorkspace", TRUE, &ok );
    if ( !ok ) {
	readOldConfig();
	return;
    }
    docPath = config.readEntry( keybase + "DocPath", docPath );
    fileFilter = config.readEntry( keybase + "FileFilter", fileFilter );
    templPath = config.readEntry( keybase + "TemplatePath", QString::null );
    databaseAutoEdit = config.readBoolEntry( keybase + "DatabaseAutoEdit", databaseAutoEdit );
    int num;

    if ( restoreConfig ) {
	splashScreen = config.readBoolEntry( keybase + "SplashScreen", TRUE );
	recentlyFiles = config.readListEntry( keybase + "RecentlyOpenedFiles", ',' );
	recentlyProjects = config.readListEntry( keybase + "RecentlyOpenedProjects", ',' );

	backPix = config.readBoolEntry( keybase + "Background/UsePixmap", TRUE );
	if ( backPix ) {
	    QPixmap pix;
	    pix.load( QDir::home().absPath() + "/.designer/" + "background.xpm" );
	    if ( !pix.isNull() )
		workspace->setBackgroundPixmap( pix );
	} else {
	    workspace->setBackgroundColor( QColor( (QRgb)config.readNumEntry( keybase + "Background/Color" ) ) );
	}

	sGrid = config.readBoolEntry( keybase + "Grid/Show", TRUE );
	snGrid = config.readBoolEntry( keybase + "Grid/Snap", TRUE );
	grd.setX( config.readNumEntry( keybase + "Grid/x", 10 ) );
	grd.setY( config.readNumEntry( keybase + "Grid/y", 10 ) );

	QRect r( pos(), size() );
	r.setX( config.readNumEntry( keybase + "Geometries/MainwindowX", r.x() ) );
	r.setY( config.readNumEntry( keybase + "Geometries/MainwindowY", r.y() ) );
	r.setWidth( config.readNumEntry( keybase + "Geometries/MainwindowWidth", r.width() ) );
	r.setHeight( config.readNumEntry( keybase + "Geometries/MainwindowHeight", r.height() ) );
	QRect desk = QApplication::desktop()->geometry();
	QRect inter = desk.intersect( r );
	resize( r.size() );
	if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	    move( r.topLeft() );
	}

	setUsesTextLabel( config.readBoolEntry( keybase + "View/TextLabels", FALSE ) );
	setUsesBigPixmaps( FALSE /*config.readBoolEntry( "BigIcons", FALSE )*/ ); // ### disabled for now
    }

    num = config.readNumEntry( keybase + "CustomWidgets/num" );
    for ( int j = 0; j < num; ++j ) {
	MetaDataBase::CustomWidget *w = new MetaDataBase::CustomWidget;
	QStringList l = config.readListEntry( keybase + "CustomWidgets/Widget" + QString::number( j ), ',' );
	w->className = l[ 0 ];
	w->includeFile = l[ 1 ];
	w->includePolicy = (MetaDataBase::CustomWidget::IncludePolicy)l[ 2 ].toInt();
	w->sizeHint.setWidth( l[ 3 ].toInt() );
	w->sizeHint.setHeight( l[ 4 ].toInt() );
	uint c = 5;
	if ( l.count() > c ) {
	    int numSignals = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numSignals; ++i, c++ )
		w->lstSignals.append( fixArgs2( l[ c ] ).latin1() );
	}
	if ( l.count() > c ) {
	    int numSlots = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numSlots; ++i ) {
		MetaDataBase::Slot slot;
		slot.slot = fixArgs2( l[ c ] );
		c++;
		slot.access = l[ c ];
		c++;
		w->lstSlots.append( slot );
	    }
	}
	if ( l.count() > c ) {
	    int numProperties = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numProperties; ++i ) {
		MetaDataBase::Property prop;
		prop.property = l[ c ];
		c++;
		prop.type = l[ c ];
		c++;
		w->lstProperties.append( prop );
	    }
	} if ( l.count() > c ) {
	    QSizePolicy::SizeType h, v;
	     h = int_to_size_type( l[ c++ ].toInt() );
	     v = int_to_size_type( l[ c++ ].toInt() );
	     w->sizePolicy = QSizePolicy( h, v );
	}
	if ( l.count() > c ) {
	    w->isContainer = (bool)l[ c++ ].toInt();
	}
	w->pixmap = new QPixmap( PixmapChooser::loadPixmap( QDir::home().absPath() + "/.designer/" + w->className ) );
	MetaDataBase::addCustomWidget( w );
    }
    if ( num > 0 )
	rebuildCustomWidgetGUI();

    if ( !restoreConfig )
	return;

    QApplication::sendPostedEvents();
    QString fn = QDir::homeDirPath() + "/.designerrc" + "tb2";
    QFile f( fn );
    if ( f.open( IO_ReadOnly ) ) {
	tbSettingsRead = TRUE;
	QTextStream ts( &f );
	ts >> *this;
	f.close();
    }

    rebuildCustomWidgetGUI();
}

void MainWindow::readOldConfig()
{
    QString fn = QDir::homeDirPath() + "/.designerrc";
    if ( !QFile::exists( fn ) ) {
	fn = "/etc/designerrc";
	if ( !QFile::exists( fn ) )
	    return;
    }
    Config config( fn );
    config.setGroup( "General" );
    restoreConfig = config.readBoolEntry( "RestoreWorkspace", TRUE );
    docPath = config.readEntry( "DocPath", docPath );
    fileFilter = config.readEntry( "FileFilter", fileFilter );
    templPath = config.readEntry( "TemplatePath", QString::null );
    databaseAutoEdit = config.readBoolEntry( "DatabaseAutoEdit", databaseAutoEdit );
    int num;
    config.setGroup( "General" );
    if ( restoreConfig ) {
	splashScreen = config.readBoolEntry( "SplashScreen", TRUE );
	recentlyFiles = config.readListEntry( "RecentlyOpenedFiles", ',' );
	recentlyProjects = config.readListEntry( "RecentlyOpenedProjects", ',' );
	config.setGroup( "Background" );
	backPix = config.readBoolEntry( "UsePixmap", TRUE );
	if ( backPix ) {
	    QPixmap pix;
	    pix.load( QDir::home().absPath() + "/.designer/" + "background.xpm" );
	    if ( !pix.isNull() )
		workspace->setBackgroundPixmap( pix );
	} else {
	    workspace->setBackgroundColor( QColor( (QRgb)config.readNumEntry( "Color" ) ) );
	}
	config.setGroup( "Grid" );
	sGrid = config.readBoolEntry( "Show", TRUE );
	snGrid = config.readBoolEntry( "Snap", TRUE );
	grd.setX( config.readNumEntry( "x", 10 ) );
	grd.setY( config.readNumEntry( "y", 10 ) );
	config.setGroup( "Geometries" );
	QRect r( pos(), size() );
	r.setX( config.readNumEntry( "MainwindowX", r.x() ) );
	r.setY( config.readNumEntry( "MainwindowY", r.y() ) );
	r.setWidth( config.readNumEntry( "MainwindowWidth", r.width() ) );
	r.setHeight( config.readNumEntry( "MainwindowHeight", r.height() ) );
	QRect desk = QApplication::desktop()->geometry();
	QRect inter = desk.intersect( r );
	resize( r.size() );
	if ( inter.width() * inter.height() > ( r.width() * r.height() / 20 ) ) {
	    move( r.topLeft() );
	}

	config.setGroup( "View" );
	setUsesTextLabel( config.readBoolEntry( "TextLabels", FALSE ) );
	setUsesBigPixmaps( FALSE /*config.readBoolEntry( "BigIcons", FALSE )*/ ); // ### disabled for now
    }

    config.setGroup( "CustomWidgets" );
    num = config.readNumEntry( "num" );
    for ( int j = 0; j < num; ++j ) {
	MetaDataBase::CustomWidget *w = new MetaDataBase::CustomWidget;
	QStringList l = config.readListEntry( "Widget" + QString::number( j ), ',' );
	w->className = l[ 0 ];
	w->includeFile = l[ 1 ];
	w->includePolicy = (MetaDataBase::CustomWidget::IncludePolicy)l[ 2 ].toInt();
	w->sizeHint.setWidth( l[ 3 ].toInt() );
	w->sizeHint.setHeight( l[ 4 ].toInt() );
	uint c = 5;
	if ( l.count() > c ) {
	    int numSignals = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numSignals; ++i, c++ )
		w->lstSignals.append( fixArgs2( l[ c ] ).latin1() );
	}
	if ( l.count() > c ) {
	    int numSlots = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numSlots; ++i ) {
		MetaDataBase::Slot slot;
		slot.slot = fixArgs2( l[ c ] );
		c++;
		slot.access = l[ c ];
		c++;
		w->lstSlots.append( slot );
	    }
	}
	if ( l.count() > c ) {
	    int numProperties = l[ c ].toInt();
	    c++;
	    for ( int i = 0; i < numProperties; ++i ) {
		MetaDataBase::Property prop;
		prop.property = l[ c ];
		c++;
		prop.type = l[ c ];
		c++;
		w->lstProperties.append( prop );
	    }
	} if ( l.count() > c ) {
	    QSizePolicy::SizeType h, v;
	     h = int_to_size_type( l[ c++ ].toInt() );
	     v = int_to_size_type( l[ c++ ].toInt() );
	     w->sizePolicy = QSizePolicy( h, v );
	}
	if ( l.count() > c ) {
	    w->isContainer = (bool)l[ c++ ].toInt();
	}
	w->pixmap = new QPixmap( PixmapChooser::loadPixmap( QDir::home().absPath() + "/.designer/" + w->className ) );
	MetaDataBase::addCustomWidget( w );
    }
    if ( num > 0 )
	rebuildCustomWidgetGUI();

    if ( !restoreConfig )
	return;

    QApplication::sendPostedEvents();
    fn = QDir::homeDirPath() + "/.designerrc" + "tb2";
    QFile f( fn );
    if ( f.open( IO_ReadOnly ) ) {
	QTextStream ts( &f );
	ts >> *this;
	f.close();
    }

    rebuildCustomWidgetGUI();
}

HierarchyView *MainWindow::objectHierarchy() const
{
    if ( !hierarchyView )
	( (MainWindow*)this )->setupHierarchyView();
    return hierarchyView;
}

QPopupMenu *MainWindow::setupNormalHierarchyMenu( QWidget *parent )
{
    QPopupMenu *menu = new QPopupMenu( parent );

    actionEditCut->addTo( menu );
    actionEditCopy->addTo( menu );
    actionEditPaste->addTo( menu );
    actionEditDelete->addTo( menu );

    return menu;
}

QPopupMenu *MainWindow::setupTabWidgetHierarchyMenu( QWidget *parent, const char *addSlot, const char *removeSlot )
{
    QPopupMenu *menu = new QPopupMenu( parent );

    menu->insertItem( tr( "Add Page" ), parent, addSlot );
    menu->insertItem( tr( "Remove Page" ), parent, removeSlot );
    menu->insertSeparator();
    actionEditCut->addTo( menu );
    actionEditCopy->addTo( menu );
    actionEditPaste->addTo( menu );
    actionEditDelete->addTo( menu );

    return menu;
}

void MainWindow::closeEvent( QCloseEvent *e )
{
    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;
	if ( !closeForm( (FormWindow*)w ) ) {
	    e->ignore();
	    return;
	}
    }
    writeConfig();
    hide();
    e->accept();

    if ( client ) {
	QDir home( QDir::homeDirPath() );
	home.remove( ".designerpid" );
    }
}

bool MainWindow::closeForm( FormWindow *fw )
{
    bool modified = FALSE;
    modified = fw->commandHistory()->isModified();
    if ( modified ) {
	switch ( QMessageBox::warning( this, tr( "Save Form" ),
				       tr( "Save changes to the form '%1'?" ).arg( fw->name() ),
				       tr( "&Yes" ), tr( "&No" ), tr( "&Cancel" ), 0, 2 ) ) {
	case 0: // save
	    fw->setFocus();
	    qApp->processEvents();
	    if ( !fileSave() )
		return FALSE;
	    break;
	case 1: // don't save
	    break;
	case 2: // cancel
	    return FALSE;
	default:
	    break;
	}
    }

    for ( QMap<QAction*, Project* >::Iterator it = projects.begin(); it != projects.end(); ++it )
	(*it)->formClosed( fw );

    return TRUE;
}

FormList *MainWindow::formlist() const
{
    if ( !formList )
	( (MainWindow*)this )->setupFormList();
    return formList;
}

PropertyEditor *MainWindow::propertyeditor() const
{
    if ( !propertyEditor )
	( (MainWindow*)this )->setupPropertyEditor();
    return propertyEditor;
}

ActionEditor *MainWindow::actioneditor() const
{
    if ( !actionEditor )
	( (MainWindow*)this )->setupActionEditor();
    return actionEditor;
}

bool MainWindow::openEditor( QWidget *w, FormWindow * )
{
    if ( WidgetFactory::hasSpecialEditor( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) ) {
	statusBar()->message( tr( "Edit %1..." ).arg( w->className() ) );
	WidgetFactory::editWidget( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ), this, w, formWindow() );
	statusBar()->clear();
	return TRUE;
    }

    const QMetaProperty* text = w->metaObject()->property( w->metaObject()->findProperty( "text", TRUE ), TRUE );
    const QMetaProperty* title = w->metaObject()->property( w->metaObject()->findProperty( "title", TRUE ), TRUE );
    if ( text && text->designable(w) ) {
	bool ok = FALSE;
	QString text;
	if ( w->inherits( "QTextView" ) || w->inherits( "QLabel" ) ) {
	    text = TextEditor::getText( this, w->property("text").toString() );
	    ok = !text.isEmpty();
	} else {
	    text = QInputDialog::getText( tr("Text"), tr( "New text" ), QLineEdit::Normal, w->property("text").toString(), &ok, this );
	}
	if ( ok ) {
	    QString pn( tr( "Set 'text' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "text", w->property( "text" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "text", TRUE );
	}
	return TRUE;
    }
    if ( title && title->designable(w) ) {
	bool ok = FALSE;
	QString text;
	text = QInputDialog::getText( tr("Title"), tr( "New title" ), QLineEdit::Normal, w->property("title").toString(), &ok, this );
	if ( ok ) {
	    QString pn( tr( "Set 'title' of '%2'" ).arg( w->name() ) );
	    SetPropertyCommand *cmd = new SetPropertyCommand( pn, formWindow(), w, propertyEditor,
							      "title", w->property( "title" ),
							      text, QString::null, QString::null );
	    cmd->execute();
	    formWindow()->commandHistory()->addCommand( cmd );
	    MetaDataBase::setPropertyChanged( w, "title", TRUE );
	}
	return TRUE;
    }

    if ( !WidgetFactory::isPassiveInteractor( w ) )
	editSource();

    return TRUE;
}

void MainWindow::rebuildCustomWidgetGUI()
{
    customWidgetToolBar->clear();
    customWidgetMenu->clear();
    int count = 0;
    QPtrList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();

    actionToolsCustomWidget->addTo( customWidgetMenu );
    customWidgetMenu->insertSeparator();

    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	QAction* a = new QAction( actionGroupTools, QString::number( w->id ).latin1() );
	a->setToggleAction( TRUE );
	a->setText( w->className );
	a->setIconSet( *w->pixmap );
	a->setStatusTip( tr( "Insert a " +w->className + " (custom widget)" ) );
	a->setWhatsThis( tr("<b>" + w->className + " (custom widget)</b>"
			    "<p>Select <b>Edit Custom Widgets...</b> in the <b>Tools->Custom</b> menu to "
			    "add and change the custom widgets. You can add properties as well as "
			    "signals and slots to integrate them into the designer, "
			    "and provide a pixmap which will be used to represent the widget on the form.</p>") );

	a->addTo( customWidgetToolBar );
	a->addTo( customWidgetMenu);
	count++;
    }

    if ( count == 0 )
	customWidgetToolBar->hide();
    else
	customWidgetToolBar->show();
}

bool MainWindow::isCustomWidgetUsed( MetaDataBase::CustomWidget *wid )
{
    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( w->inherits( "FormWindow" ) ) {
	    if ( ( (FormWindow*)w )->isCustomWidgetUsed( wid ) )
		return TRUE;
	}
    }
    return FALSE;
}

void MainWindow::setGrid( const QPoint &p )
{
    if ( p == grd )
	return;
    grd = p;
    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;
	( (FormWindow*)w )->mainContainer()->update();
    }
}

void MainWindow::setShowGrid( bool b )
{
    if ( b == sGrid )
	return;
    sGrid = b;
    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;
	( (FormWindow*)w )->mainContainer()->update();
    }
}

void MainWindow::setSnapGrid( bool b )
{
    if ( b == snGrid )
	return;
    snGrid = b;
}

QString MainWindow::documentationPath() const
{
    QString result = docPath;

    if ( docPath[0] == '$' ) {
	int fs = docPath.find('/');
	if ( fs == -1 )
	    fs = docPath.find('\\');

	if ( fs > -1 ) {
	    result = docPath.mid( 1, fs-1 );
	} else {
	    fs=docPath.length();
	    result = docPath.right(fs-1);
	}
	result = getenv(result.latin1()) + docPath.right( docPath.length()-fs );
    }

    return result;
}

void MainWindow::windowsMenuActivated( int id )
{
    QWidget* w = workspace->windowList().at( id );
    if ( w )
	w->setFocus();
}

void MainWindow::closeAllForms()
{
    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;
	w->close();
    }
}

void MainWindow::projectSelected( QAction *a )
{
    if ( currentProject )
	currentProject->setActive( FALSE );
    currentProject = *projects.find( a );
    if ( formList )
	formList->setProject( currentProject );
    if ( actionEditPixmapCollection )
	actionEditPixmapCollection->setEnabled( currentProject != emptyProject() );
    currentProject->setActive( TRUE );

#if 0
    workSpace()->blockSignals( TRUE );
    QWidgetList windows = workspace->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( w->inherits( "SourceEditor" ) ) {
	    if ( ( (SourceEditor*)w )->project() == currentProject )
		w->showNormal();
	    else
		w->showMinimized();
	} else if ( w->inherits( "FormWindow" ) ) {
	    if ( ( (FormWindow*)w )->project() == currentProject )
		w->showNormal();
	    else
		w->showMinimized();
	}
    }

    if ( workspace->activeWindow() ) {
	if ( workspace->activeWindow()->inherits( "FormWindow" ) ) {
	    lastActiveFormWindow = (FormWindow*)workspace->activeWindow();
	    setAppropriate( (QDockWindow*)actionEditor->parentWidget(),
			    lastActiveFormWindow->mainContainer()->inherits( "QMainWindow" ) );
	    if ( appropriate( (QDockWindow*)actionEditor->parentWidget() ) )
		actionEditor->parentWidget()->show();
	    else
		actionEditor->parentWidget()->hide();
	    actionEditor->setFormWindow( lastActiveFormWindow );
	} else {
	    emit formWindowChanged();
	    emit hasActiveForm( FALSE );
	    actionEditUndo->setEnabled( FALSE );
	    actionEditRedo->setEnabled( FALSE );
	    setAppropriate( (QDockWindow*)actionEditor->parentWidget(), FALSE );
	    actionEditor->parentWidget()->hide();
	    actionEditor->setFormWindow( 0 );
	}
    }
    workSpace()->blockSignals( FALSE );
#endif
}

void MainWindow::openProject( const QString &fn )
{
    for ( QMap<QAction*, Project*>::Iterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( (*it)->fileName() == fn ) {
	    projectSelected( it.key() );
	    return;
	}
    }
    Project *pro = new Project( fn, "", projectSettingsPluginManager );
    QAction *a = new QAction( pro->projectName(), pro->projectName(), 0, actionGroupProjects, 0, TRUE );
    projects.insert( a, pro );
    a->setOn( TRUE );
    projectSelected( a );
}

void MainWindow::checkTempFiles()
{
    QString s = QDir::homeDirPath() + "/.designer";
    QString baseName = s+ "/saved-form-";
    if ( !QFile::exists( baseName + "1.ui" ) )
	return;
    QDir d( s );
    d.setNameFilter( "*.ui" );
    QStringList lst = d.entryList();
    QApplication::restoreOverrideCursor();
    bool load = QMessageBox::information( this, tr( "Restoring last session" ),
					  tr( "The Qt Designer found some temporary saved files, which have been\n"
					      "written when the Qt Designer crashed last time. Do you want to\n"
					      "load these files?" ), tr( "&Yes" ), tr( "&No" ) ) == 0;
    QApplication::setOverrideCursor( waitCursor );
    for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	if ( load )
	    openFile( s + "/" + *it, FALSE );
	d.remove( *it );
    }
}

void MainWindow::openHelpForDialog( const QString &dia )
{
    QString manualdir = QString( getenv( "QTDIR" ) ) + "/tools/designer/manual/book1.html";
    if ( !QFile::exists( manualdir ) )
	manualdir = QString( getenv( "QTDIR" ) ) + "/doc/html/designer/book1.html";
    QFile file( manualdir );
    if ( !file.open( IO_ReadOnly ) )
	return;
    QTextStream ts( &file );
    QString text = ts.read();

    int i = text.find( dia );
    if ( i == -1 )
	return;
    i = text.findRev( "href=\"", i );
    if ( i == -1 )
	return;
    int end = text.find( "\"", i + 8 );
    i += QString( "href=\"" ).length();
    QString page = text.mid( i, end - i );

    if ( !page.isEmpty() ) {
	QStringList lst;
	lst << "assistant" << page;
	QProcess proc( lst );
	proc.start();
    }
}

void MainWindow::showDialogHelp()
{
    QWidget *w = (QWidget*)sender();
    w = w->topLevelWidget();

    // dialog classname to documentation title mapping
    if ( w->inherits( "AboutDialog" ) )
	openHelpForDialog( "The About Dialog" );
    else if ( w->inherits( "ConnectionEditorBase" ) )
	openHelpForDialog( "The Connection Editor Dialog (Edit Connections)" );
    else if ( w->inherits( "ConnectionViewerBase" ) )
	openHelpForDialog( "The Connection Viewer Dialog (Edit Connections)" );
    else if ( w->inherits( "CustomWidgetEditorBase" ) )
	openHelpForDialog( "The Edit Custom Widgets Dialog" );
    else if ( w->inherits( "IconViewEditorBase" ) )
	openHelpForDialog( "The Edit Icon View Dialog" );
    else if ( w->inherits( "ListBoxEditorBase" ) )
	openHelpForDialog( "The Edit List Box Dialog" );
    else if ( w->inherits( "ListViewEditorBase" ) )
	openHelpForDialog( "The Edit List View Dialog" );
    else if ( w->inherits( "MultiLineEditorBase" ) )
	openHelpForDialog( "The Edit Multiline Edit Dialog" );
    else if ( w->inherits( "EditSlotsBase" ) )
	openHelpForDialog( "The Edit Slots Dialog" );
    else if ( w->inherits( "FormSettingsBase" ) )
	openHelpForDialog( "The Form Settings Dialog" );
    else if ( w->inherits( "HelpDialogBase" ) )
	openHelpForDialog( "The Help Dialog" );
    else if ( w->inherits( "NewFormBase" ) )
	openHelpForDialog( "The New Form Dialog" );
    else if ( w->inherits( "PaletteEditorBase" ) )
	openHelpForDialog( "The Palette Editor Dialog" );
    else if ( w->inherits( "PixmapFunction" ) )
	openHelpForDialog( "The Pixmap Function Dialog" );
    else if ( w->inherits( "Preferences" ) )
	openHelpForDialog( "The Preferences Dialog" );
    else if ( w->inherits( "TopicChooserBase" ) )
	openHelpForDialog( "The Topic Chooser Dialog" );
}

void MainWindow::setupActionManager()
{
    actionPluginManager = new QPluginManager<ActionInterface>( IID_Action, pluginDir );
    QStringList paths(QApplication::libraryPaths());
    QStringList::Iterator it = paths.begin();
    while (it != paths.end()) {
	actionPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    QStringList lst = actionPluginManager->featureList();
    for ( QStringList::Iterator ait = lst.begin(); ait != lst.end(); ++ait ) {
	ActionInterface *iface = 0;
	actionPluginManager->queryInterface( *ait, &iface );
	if ( !iface )
	    continue;

	iface->connectTo( desInterface );
	QAction *a = iface->create( *ait, this );
	if ( !a )
	    continue;

	QString grp = iface->group( *ait );
	if ( grp.isEmpty() )
	    grp = "3rd party actions";
	QPopupMenu *menu = 0;
	QToolBar *tb = 0;

	if ( !( menu = (QPopupMenu*)child( grp.latin1(), "QPopupMenu" ) ) ) {
	    menu = new QPopupMenu( this, grp.latin1() );
	    menuBar()->insertItem( tr( grp ), menu );
	}
	if ( !( tb = (QToolBar*)child( grp.latin1(), "QToolBar" ) ) ) {
#if defined(HAVE_KDE)
	    KToolBar *tb = new KToolBar( this );
	    tb->setFullSize( FALSE, grp.latin1() );
#else
	    tb = new QToolBar( this, grp.latin1() );
	    tb->setCloseMode( QDockWindow::Undocked );
#endif
	    addToolBar( tb, grp );
	}

	a->addTo( menu );
	a->addTo( tb );

	iface->release();
    }
}

void MainWindow::editFunction( const QString &func, const QString &l, bool rereadSource )
{
    if ( !formWindow() )
	return;
    SourceEditor *editor = 0;
    QString lang = l;
    if ( lang.isEmpty() )
	lang = MetaDataBase::languageOfSlot( formWindow(), func.latin1() );
    if ( !MetaDataBase::hasEditor( lang ) )
	return;
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->language() == lang ) {
	    editor = e;
	    break;
	}
    }
    if ( !editor ) {
	EditorInterface *eIface = 0;
	editorPluginManager->queryInterface( lang, &eIface );
	if ( !eIface )
	    return;
	LanguageInterface *lIface = MetaDataBase::languageInterface( lang );
	if ( !lIface )
	    return;
	editor = new SourceEditor( workSpace(), eIface, lIface );
	eIface->release();
	lIface->release();

	editor->setLanguage( lang );
	sourceEditors.append( editor );
    }
    editor->show();
    editor->setFocus();
    if ( editor->object() != formWindow() )
	editor->setObject( formWindow(), formWindow()->project() );
    else if ( rereadSource )
	editor->refresh();
    editor->setFunction( func );
}

void MainWindow::setupRecentlyFilesMenu()
{
    recentlyFilesMenu->clear();
    int id = 0;
    for ( QStringList::Iterator it = recentlyFiles.begin(); it != recentlyFiles.end(); ++it ) {
	recentlyFilesMenu->insertItem( *it, id );
	id++;
    }
}

void MainWindow::setupRecentlyProjectsMenu()
{
    recentlyProjectsMenu->clear();
    int id = 0;
    for ( QStringList::Iterator it = recentlyProjects.begin(); it != recentlyProjects.end(); ++it ) {
	recentlyProjectsMenu->insertItem( *it, id );
	id++;
    }
}

QPtrList<DesignerProject> MainWindow::projectList() const
{
    QPtrList<DesignerProject> list;
    QMapConstIterator<QAction*, Project*> it = projects.begin();

    while( it != projects.end() ) {
	Project *p = it.data();
	++it;
	list.append( p->iFace() );
    }

    return list;
}

void MainWindow::recentlyFilesMenuActivated( int id )
{
    if ( id != -1 ) {
	fileOpen( "", "", *recentlyFiles.at( id ) );
	QString fn( *recentlyFiles.at( id ) );
	addRecentlyOpened( fn, recentlyFiles );
    }
}

void MainWindow::recentlyProjectsMenuActivated( int id )
{
    if ( id != -1 ) {
	openProject( *recentlyProjects.at( id ) );
	QString fn( *recentlyProjects.at( id ) );
	addRecentlyOpened( fn, recentlyProjects );
    }
}

void MainWindow::addRecentlyOpened( const QString &fn, QStringList &lst )
{
    if ( lst.find( fn ) != lst.end() )
	lst.remove( fn );
    if ( lst.count() >= 10 )
	lst.remove( lst.begin() );
    lst << fn;
}

TemplateWizardInterface * MainWindow::templateWizardInterface( const QString& className )
{
    TemplateWizardInterface* iface = 0;
    templateWizardPluginManager->queryInterface( className, & iface );
    return iface;
}

void MainWindow::setupPluginManagers()
{
    editorPluginManager = new QPluginManager<EditorInterface>( IID_Editor, pluginDir );
    QStringList paths(QApplication::libraryPaths());
    QStringList::Iterator it = paths.begin();
    while (it != paths.end()) {
	editorPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    MetaDataBase::setEditor( editorPluginManager->featureList() );
    templateWizardPluginManager = new QPluginManager<TemplateWizardInterface>( IID_TemplateWizard, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	templateWizardPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    MetaDataBase::setupInterfaceManagers();
    programPluginManager = new QPluginManager<ProgramInterface>( IID_Program, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	programPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    interpreterPluginManager = new QPluginManager<InterpreterInterface>( IID_Interpreter, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	interpreterPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    preferencePluginManager = new QPluginManager<PreferenceInterface>( IID_Preference, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	preferencePluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    projectSettingsPluginManager = new QPluginManager<ProjectSettingsInterface>( IID_ProjectSettings, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	projectSettingsPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    if ( preferencePluginManager ) {
	QStringList lst = preferencePluginManager->featureList();
	for ( it = lst.begin(); it != lst.end(); ++it ) {
	    PreferenceInterface *i = 0;
	    preferencePluginManager->queryInterface( *it, &i );
	    i->connectTo( designerInterface() );
	    if ( !i )
		continue;
	    PreferenceInterface::Preference *pf = i->preference();
	    if ( pf )
		addPreferencesTab( pf->tab, pf->title, pf->receiver, pf->init_slot, pf->accept_slot );
	    i->deletePreferenceObject( pf );

	    i->release();
	}
    }
    if ( projectSettingsPluginManager ) {
	QStringList lst = projectSettingsPluginManager->featureList();
	for ( QStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
	    ProjectSettingsInterface *i = 0;
	    projectSettingsPluginManager->queryInterface( *it, &i );
	    i->connectTo( designerInterface() );
	    if ( !i )
		continue;

	    ProjectSettingsInterface::ProjectSettings *pf = i->projectSetting();
	    if ( pf )
		addProjectTab( pf->tab, pf->title, pf->receiver, pf->init_slot, pf->accept_slot );
	    i->deleteProjectSettingsObject( pf );
	    i->release();
	}
    }
}

void MainWindow::addPreferencesTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot )
{
    Tab t;
    t.w = tab;
    t.title = title;
    t.receiver = receiver;
    t.init_slot = init_slot;
    t.accept_slot = accept_slot;
    preferenceTabs << t;
}

void MainWindow::addProjectTab( QWidget *tab, const QString &title, QObject *receiver, const char *init_slot, const char *accept_slot )
{
    Tab t;
    t.w = tab;
    t.title = title;
    t.receiver = receiver;
    t.init_slot = init_slot;
    t.accept_slot = accept_slot;
    projectTabs << t;
}

void MainWindow::setModified( bool b, QWidget *window )
{
    QWidget *w = window;
    while ( w ) {
	if ( w->inherits( "FormWindow" ) ) {
	    ( (FormWindow*)w )->modificationChanged( b );
	    return;
	} else if ( w->inherits( "SourceEditor" ) ) {
	    if ( ( (SourceEditor*)w )->object()->inherits( "FormWindow" ) ) {
		FormWindow *fw = (FormWindow*)( (SourceEditor*)w )->object();
		fw->commandHistory()->setModified( b );
		fw->modificationChanged( b );
	    } else {
		// ############# for outher source files
	    }
	}
	w = w->parentWidget();
    }
}

void MainWindow::editorClosed( SourceEditor *e )
{
    sourceEditors.take( sourceEditors.findRef( e ) );
}

void MainWindow::slotsChanged()
{
    updateSlotsTimer->start( 0, TRUE );
}

void MainWindow::doSlotsChanged()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->refresh();
    hierarchyView->functionList()->refreshFunctions();
}

void MainWindow::updateFunctionList()
{
    if ( !workSpace()->activeWindow() || !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	return;
    ( (SourceEditor*)workSpace()->activeWindow() )->save();
    hierarchyView->functionList()->refreshFunctions();
}

void MainWindow::updateFormList()
{
    formList->setProject( currentProject );
}

void MainWindow::showDebugStep( QObject *o, int line )
{
    if ( !o || line == -1 ) {
	for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	    e->clearStep();
	return;
    }
    showSourceLine( o, line, FALSE );
}

void MainWindow::showErrorMessage( QObject *o, int errorLine, const QString &errorMessage )
{
    errorLine--; // ######
    QValueList<int> l;
    l << errorLine;
    QStringList l2;
    l2 << errorMessage;
    oWindow->setErrorMessages( l2, l, TRUE );
    showSourceLine( o, errorLine, TRUE );
}

void MainWindow::finishedRun()
{
    inDebugMode = FALSE;
    previewing = FALSE;
    debuggingForms.clear();
    enableAll( TRUE );
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->project() == currentProject )
	    e->editorInterface()->setMode( EditorInterface::Editing );
    }
}

void MainWindow::enableAll( bool enable )
{
    menuBar()->setEnabled( enable );
    QObjectList *l = queryList( "QDockWindow" );
    for ( QObject *o = l->first(); o; o = l->next() ) {
	if ( o == formList->parentWidget() || o == oWindow->parentWidget() || o == hierarchyView->parentWidget() )
	    continue;
	( (QWidget*)o )->setEnabled( enable );
    }
    delete l;
}

void MainWindow::showSourceLine( QObject *o, int line, bool error )
{
    bool found = FALSE;
    QString lang = currentProject->language();
    EditorInterface *eiface = 0;
    editorPluginManager->queryInterface( lang, & eiface );
    QWidgetList windows = workspace->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;
	FormWindow *fw = (FormWindow*)w;
	if ( fw->project() != currentProject )
	    continue;
	if ( QString( fw->name() ) == QString( o->name() ) ) {

	    if ( workSpace()->activeWindow() && workSpace()->activeWindow()->inherits( "SourceEditor" ) &&
		 ( (SourceEditor*)workSpace()->activeWindow() )->object() == fw ) {
		if ( error )
		    eiface->setError( line );
		else
		    eiface->setStep( line );
		eiface->release();
		return;
	    }

	    fw->setFocus();
	    lastActiveFormWindow = fw;
	    qApp->processEvents();
	    editSource( (bool)FALSE );
	    if ( error )
		eiface->setError( line );
	    else
		eiface->setStep( line );
	    found = TRUE;
	    break;
	}
    }

    if ( !found ) {
	QPtrList<SourceFile> sources = currentProject->sourceFiles();
	for ( SourceFile *f = sources.first(); f; f = sources.next() ) {
	    if ( f == o ) {
		editSource( f );
		if ( error )
		    eiface->setError( line );
		else
		    eiface->setStep( line );
		eiface->release();
		return;
	    }
	}
    }

    if ( !found ) {
	mblockNewForms = TRUE;
	openFile( currentProject->makeAbsolute( *qwf_forms->find( (QWidget*)o ) ) );
	qApp->processEvents(); // give all views the chance to get the formwindow
	editSource();
	if ( error )
	    eiface->setError( line );
	else
	    eiface->setStep( line );
	mblockNewForms = FALSE;
    }
    eiface->release();
}

Project *MainWindow::emptyProject()
{
    return eProject;
}

QWidget *MainWindow::findRealForm( QWidget *wid )
{
    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( QString( w->name() ) == QString( wid->name() ) )
	    return w;
    }
    return 0;
}

void MainWindow::formNameChanged( FormWindow *fw )
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->object() == fw ) {
	    e->refresh();
	    break;
	}
    }
}

void MainWindow::breakPointsChanged()
{
    if ( !inDebugMode )
	return;
    if ( !workSpace()->activeWindow() || !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	return;
    SourceEditor *e = (SourceEditor*)workSpace()->activeWindow();
    if ( !e->object() || !e->project() )
	return;
    if ( e->project() != currentProject )
	return;

    InterpreterInterface *iiface = 0;
    if ( interpreterPluginManager ) {
	QString lang = currentProject->language();
	iiface = 0;
	interpreterPluginManager->queryInterface( lang, &iiface );
	if ( !iiface )
	    return;
    }

    e->saveBreakPoints();

    for ( QObject *o = debuggingForms.first(); o; o = debuggingForms.next() ) {
	if ( qstrcmp( o->name(), e->object()->name() ) == 0 ) {
	    iiface->setBreakPoints( o, MetaDataBase::breakPoints( e->object() ) );
	    break;
	}
    }

    for ( e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->project() == currentProject && e->object()->inherits( "SourceFile" ) ) {
	    QValueList<int> bps = MetaDataBase::breakPoints( e->object() );
	    iiface->setBreakPoints( e->object(), bps );
	}
    }

    iiface->release();
}
