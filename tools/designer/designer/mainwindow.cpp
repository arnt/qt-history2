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

#define Q_INIT_INTERFACES
#include "designerappiface.h"
#include "designerapp.h"

#include "mainwindow.h"
#include "defs.h"
#include "formwindow.h"
#include "widgetdatabase.h"
#include "widgetfactory.h"
#include "propertyeditor.h"
#include "qmetaobject.h"
#include "qaction.h"
#include "metadatabase.h"
#include "resource.h"
#include "pixmapchooser.h"
#include "config.h"
#include "hierarchyview.h"
#include "editslotsimpl.h"
#include "newformimpl.h"
#include "formlist.h"
#include "connectionviewerimpl.h"
#include "customwidgeteditorimpl.h"
#include "preferences.h"
#include "styledbutton.h"
#include "formsettingsimpl.h"
#include "about.h"
#include "multilineeditorimpl.h"
#include "wizardeditorimpl.h"
#include "createtemplate.h"
#include "outputwindow.h"
#include <qinputdialog.h>
#if defined(HAVE_KDE)
#include <ktoolbar.h>
#include <kmenubar.h>
#else
#include <qtoolbar.h>
#include <qmenubar.h>
#endif
#include <qfeatures.h>
#include <qpixmap.h>
#include <qbuttongroup.h>
#include <qapplication.h>
#include <qworkspace.h>
#include <qfiledialog.h>
#include <qapplication.h>
#include <qworkspace.h>
#include <qclipboard.h>
#include <qmessagebox.h>
#include <qbuffer.h>
#include <qdir.h>
#include <qstyle.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qstatusbar.h>
#include <qfile.h>
#include <qmessagebox.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qobjectlist.h>
#include <qurl.h>
#include <qwhatsthis.h>
#include <qwizard.h>
#include <qpushbutton.h>
#include <qdir.h>
#include <qtimer.h>
#include <qlistbox.h>
#include <stdlib.h>
#include <qdockwindow.h>
#include <qregexp.h>
#include <qstylefactory.h>
#include <qsignalmapper.h>
#include "actioneditorimpl.h"
#include "actiondnd.h"
#include "project.h"
#include "projectsettingsimpl.h"
#ifndef QT_NO_SQL
#include "dbconnectionsimpl.h"
#include "dbconnectionimpl.h"
#endif
#include "../resource/qwidgetfactory.h"
#include <qvbox.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qspinbox.h>
#include "finddialog.h"
#include "replacedialog.h"
#include "gotolinedialog.h"
#include <qprocess.h>
#include <qsettings.h>
#include "pixmapcollectioneditor.h"
#include "pixmapcollection.h"
#include "sourcefile.h"

static int forms = 0;
static bool mblockNewForms = FALSE;
extern QMap<QWidget*, QString> *qwf_functions;
extern QMap<QWidget*, QString> *qwf_forms;
extern QString *qwf_language;
extern bool qwf_execute_code;
extern bool qwf_stays_on_top;
static bool tbSettingsRead = FALSE;

static const char * whatsthis_image[] = {
    "16 16 3 1",
    "	c None",
    "o	c #000000",
    "a	c #000080",
    "o        aaaaa  ",
    "oo      aaa aaa ",
    "ooo    aaa   aaa",
    "oooo   aa     aa",
    "ooooo  aa     aa",
    "oooooo  a    aaa",
    "ooooooo     aaa ",
    "oooooooo   aaa  ",
    "ooooooooo aaa   ",
    "ooooo     aaa   ",
    "oo ooo          ",
    "o  ooo    aaa   ",
    "    ooo   aaa   ",
    "    ooo         ",
    "     ooo        ",
    "     ooo        "};

const QString toolbarHelp = "<p>Toolbars contain a number of buttons to "
"provide quick access to often used functions.%1"
"<br>Click on the toolbar handle to hide the toolbar, "
"or drag and place the toolbar to a different location.</p>";

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

    setupMDI();
    setupMenuBar();

    setupFileActions();
    setupEditActions();
    setupSerachActions();
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
    connect( workspace, SIGNAL( windowActivated( QWidget * ) ),
	     this, SLOT( activeWindowChanged( QWidget * ) ) );
    lastActiveFormWindow = 0;
    workspace->setAcceptDrops( TRUE );
}

void MainWindow::setupMenuBar()
{
    menubar = menuBar();
}

static QIconSet createIconSet( const QString &name )
{
    QIconSet ic( PixmapChooser::loadPixmap( name, PixmapChooser::Small ) );
    ic.setPixmap( PixmapChooser::loadPixmap( name, PixmapChooser::Disabled ), QIconSet::Small, QIconSet::Disabled );
    return ic;
}

void MainWindow::setupEditActions()
{
    actionEditUndo = new QAction( tr("Undo"), createIconSet( "undo.xpm" ),tr("&Undo: Not Available"), CTRL + Key_Z, this, 0 );
    actionEditUndo->setStatusTip( tr( "Reverses the last action" ) );
    actionEditUndo->setWhatsThis( tr( "Reverses the last action" ) );
    connect( actionEditUndo, SIGNAL( activated() ), this, SLOT( editUndo() ) );
    actionEditUndo->setEnabled( FALSE );

    actionEditRedo = new QAction( tr( "Redo" ), createIconSet("redo.xpm"), tr( "&Redo: Not Available" ), CTRL + Key_Y, this, 0 );
    actionEditRedo->setStatusTip( tr( "Redoes the last undone operation") );
    actionEditRedo->setWhatsThis( tr("Redoes the last undone operation") );
    connect( actionEditRedo, SIGNAL( activated() ), this, SLOT( editRedo() ) );
    actionEditRedo->setEnabled( FALSE );

    actionEditCut = new QAction( tr( "Cut" ), createIconSet("editcut.xpm"), tr( "Cu&t" ), CTRL + Key_X, this, 0 );
    actionEditCut->setStatusTip( tr( "Cuts the selected widgets and puts them on the clipboard" ) );
    actionEditCut->setWhatsThis( tr( "Cuts the selected widgets and puts them on the clipboard" ) );
    connect( actionEditCut, SIGNAL( activated() ), this, SLOT( editCut() ) );
    actionEditCut->setEnabled( FALSE );

    actionEditCopy = new QAction( tr( "Copy" ), createIconSet("editcopy.xpm"), tr( "&Copy" ), CTRL + Key_C, this, 0 );
    actionEditCopy->setStatusTip( tr( "Copies the selected widgets to the clipboard" ) );
    actionEditCopy->setWhatsThis( tr( "Copies the selected widgets to the clipboard" ) );
    connect( actionEditCopy, SIGNAL( activated() ), this, SLOT( editCopy() ) );
    actionEditCopy->setEnabled( FALSE );

    actionEditPaste = new QAction( tr( "Paste" ), createIconSet("editpaste.xpm"), tr( "&Paste" ), CTRL + Key_V, this, 0 );
    actionEditPaste->setStatusTip( tr( "Pastes clipboard contents" ) );
    actionEditPaste->setWhatsThis( tr( "Pastes the widgets on the clipboard into the formwindow" ) );
    connect( actionEditPaste, SIGNAL( activated() ), this, SLOT( editPaste() ) );
    actionEditPaste->setEnabled( FALSE );

    actionEditDelete = new QAction( tr( "Delete" ), QPixmap(), tr( "&Delete" ), Key_Delete, this, 0 );
    actionEditDelete->setStatusTip( tr( "Deletes the selected widgets" ) );
    actionEditDelete->setWhatsThis( tr( "Deletes the selected widgets" ) );
    connect( actionEditDelete, SIGNAL( activated() ), this, SLOT( editDelete() ) );
    actionEditDelete->setEnabled( FALSE );

    actionEditSelectAll = new QAction( tr( "Select All" ), QPixmap(), tr( "Select &All" ), CTRL + Key_A, this, 0 );
    actionEditSelectAll->setStatusTip( tr( "Selects all widgets" ) );
    actionEditSelectAll->setWhatsThis( tr( "Selects all widgets in the current form" ) );
    connect( actionEditSelectAll, SIGNAL( activated() ), this, SLOT( editSelectAll() ) );
    actionEditSelectAll->setEnabled( TRUE );

    actionEditRaise = new QAction( tr( "Bring to Front" ), createIconSet("editraise.xpm"), tr( "Bring to &Front" ), 0, this, 0 );
    actionEditRaise->setStatusTip( tr( "Raises the selected widgets" ) );
    actionEditRaise->setWhatsThis( tr( "Raises the selected widgets" ) );
    connect( actionEditRaise, SIGNAL( activated() ), this, SLOT( editRaise() ) );
    actionEditRaise->setEnabled( FALSE );

    actionEditLower = new QAction( tr( "Send to Back" ), createIconSet("editlower.xpm"), tr( "Send to &Back" ), 0, this, 0 );
    actionEditLower->setStatusTip( tr( "Lowers the selected widgets" ) );
    actionEditLower->setWhatsThis( tr( "Lowers the selected widgets" ) );
    connect( actionEditLower, SIGNAL( activated() ), this, SLOT( editLower() ) );
    actionEditLower->setEnabled( FALSE );

    actionEditAccels = new QAction( tr( "Check Accelerators" ), QPixmap(),
				    tr( "Check Accele&rators" ), ALT + Key_R, this, 0 );
    actionEditAccels->setStatusTip( tr("Checks if the accelerators used in the form are unique") );
    actionEditAccels->setWhatsThis( tr("<b>Check Accelerators</b>"
				       "<p>Checks if the accelerators used in the form are unique. If this "
				       "is not the case, the desiner helps you to fix that problem.</p>") );
    connect( actionEditAccels, SIGNAL( activated() ), this, SLOT( editAccels() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditAccels, SLOT( setEnabled(bool) ) );


    actionEditSlots = new QAction( tr( "Slots" ), createIconSet("editslots.xpm"),
				   tr( "S&lots..." ), 0, this, 0 );
    actionEditSlots->setStatusTip( tr("Opens a dialog to edit slots") );
    actionEditSlots->setWhatsThis( tr("<b>Edit slots</b>"
				      "<p>Opens a dialog where slots of the current form can be added and changed. "
				      "The slots will be virtual in the generated C++ source, and you may wish to "
				      "reimplement them in subclasses.</p>") );
    connect( actionEditSlots, SIGNAL( activated() ), this, SLOT( editSlots() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditSlots, SLOT( setEnabled(bool) ) );

    actionEditConnections = new QAction( tr( "Connections" ), createIconSet("connecttool.xpm"),
					 tr( "Co&nnections..." ), 0, this, 0 );
    actionEditConnections->setStatusTip( tr("Opens a dialog to edit connections") );
    actionEditConnections->setWhatsThis( tr("<b>Edit connections</b>"
					    "<p>Opens a dialog where the connections of the current form can be "
					    "changed.</p>") );
    connect( actionEditConnections, SIGNAL( activated() ), this, SLOT( editConnections() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditConnections, SLOT( setEnabled(bool) ) );

    actionEditSource = new QAction( tr( "Source" ), QIconSet(),
					 tr( "&Source..." ), CTRL + Key_E, this, 0 );
    actionEditConnections->setStatusTip( tr("Opens an editor to edit the source of the form") );
    actionEditConnections->setWhatsThis( tr("<b>Edit source</b>"
					    "<p>Opens an editor where the source of the current form can be "
					    "edited.</p>") );
    connect( actionEditSource, SIGNAL( activated() ), this, SLOT( editSource() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditSource, SLOT( setEnabled(bool) ) );

    actionEditFormSettings = new QAction( tr( "Form Settings" ), QPixmap(),
					  tr( "&Form Settings..." ), 0, this, 0 );
    actionEditFormSettings->setStatusTip( tr("Opens a dialog to change the settings of the form") );
    actionEditFormSettings->setWhatsThis( tr("<b>Edit settings of the form</b>"
					     "<p>Opens a dialog to change the classname and add comments to the current formwindow.</p>") );
    connect( actionEditFormSettings, SIGNAL( activated() ), this, SLOT( editFormSettings() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), actionEditFormSettings, SLOT( setEnabled(bool) ) );

    actionEditProjectSettings = new QAction( tr( "Project Settings..." ), QPixmap(),
					  tr( "&Project Settings..." ), 0, this, 0 );
    actionEditProjectSettings->setStatusTip( tr("Opens a dialog to change the settings of the project") );
    actionEditProjectSettings->setWhatsThis( tr("<b>Edit settings of the project</b>"
					     "<p>####TODO</p>") );
    connect( actionEditProjectSettings, SIGNAL( activated() ), this, SLOT( editProjectSettings() ) );

    actionEditPixmapCollection = new QAction( tr( "Pixmap Collection..." ), QPixmap(),
					  tr( "P&ixmap Collection..." ), 0, this, 0 );
    actionEditPixmapCollection->setStatusTip( tr("Opens a dialog to edit the pixmap collection of the current project") );
    actionEditPixmapCollection->setWhatsThis( tr("<b>Edit pixmap collection of the current project</b>"
						 "<p>####TODO</p>") );
    connect( actionEditPixmapCollection, SIGNAL( activated() ), this, SLOT( editPixmapCollection() ) );
    actionEditPixmapCollection->setEnabled( FALSE );

#ifndef QT_NO_SQL
    actionEditDatabaseConnections = new QAction( tr( "Database Connections..." ), QPixmap(),
						 tr( "&Database Connections..." ), 0, this, 0 );
    actionEditDatabaseConnections->setStatusTip( tr("Opens a dialog to edit the database connections of the current project") );
    actionEditDatabaseConnections->setWhatsThis( tr("<b>Edit the database connections of the current project</b>"
					     "<p>####TODO</p>") );
    connect( actionEditDatabaseConnections, SIGNAL( activated() ), this, SLOT( editDatabaseConnections() ) );
#endif

    actionEditPreferences = new QAction( tr( "Preferences" ), QPixmap(),
					 tr( "P&references..." ), 0, this, 0 );
    actionEditPreferences->setStatusTip( tr("Opens a dialog to change preferences") );
    actionEditPreferences->setWhatsThis( tr("<b>Change preferences</b>"
					    "<p>The settings will be saved on exit. They will be restored "
					    "the next time the Designer starts if \"Restore last Workspace\" "
					    "has been selected.</p>") );
    connect( actionEditPreferences, SIGNAL( activated() ), this, SLOT( editPreferences() ) );

#if defined(HAVE_KDE)
    KToolBar *tb = new KToolBar( this, "Edit" );
    tb->setFullSize( FALSE );
#else
    QToolBar *tb = new QToolBar( this, "Edit" );
    tb->setCloseMode( QDockWindow::Undocked );
#endif
    QWhatsThis::add( tb, tr( "<b>The Edit toolbar</b>%1").arg(tr(toolbarHelp).arg("")) );
    addToolBar( tb, tr( "Edit" ) );
    actionEditUndo->addTo( tb );
    actionEditRedo->addTo( tb );
    tb->addSeparator();
    actionEditCut->addTo( tb );
    actionEditCopy->addTo( tb );
    actionEditPaste->addTo( tb );
#if 0
    tb->addSeparator();
    actionEditLower->addTo( tb );
    actionEditRaise->addTo( tb );
#endif

    QPopupMenu *menu = new QPopupMenu( this, "Edit" );
    menubar->insertItem( tr( "&Edit" ), menu );
    actionEditUndo->addTo( menu );
    actionEditRedo->addTo( menu );
    menu->insertSeparator();
    actionEditCut->addTo( menu );
    actionEditCopy->addTo( menu );
    actionEditPaste->addTo( menu );
    actionEditDelete->addTo( menu );
    actionEditSelectAll->addTo( menu );
    actionEditAccels->addTo( menu );
#if 0
    menu->insertSeparator();
    actionEditLower->addTo( menu );
    actionEditRaise->addTo( menu );
#endif
    menu->insertSeparator();
    actionEditSlots->addTo( menu );
    actionEditConnections->addTo( menu );
    actionEditFormSettings->addTo( menu );
    menu->insertSeparator();
    actionEditProjectSettings->addTo( menu );
    actionEditPixmapCollection->addTo( menu );
#ifndef QT_NO_SQL
    actionEditDatabaseConnections->addTo( menu );
#endif
    menu->insertSeparator();
    actionEditPreferences->addTo( menu );
}

void MainWindow::setupSerachActions()
{
    actionSearchFind = new QAction( tr( "Find" ), createIconSet( "searchfind.xpm" ),
				    tr( "&Find..." ), CTRL + Key_F, this, 0 );
    connect( actionSearchFind, SIGNAL( activated() ), this, SLOT( searchFind() ) );
    actionSearchFind->setEnabled( FALSE );

    actionSearchIncremetal = new QAction( tr( "Find Incremental" ), QIconSet(),
					  tr( "Find &Incremetal" ), ALT + Key_I, this, 0 );
    connect( actionSearchIncremetal, SIGNAL( activated() ), this, SLOT( searchIncremetalFindMenu() ) );
    actionSearchIncremetal->setEnabled( FALSE );

    actionSearchReplace = new QAction( tr( "Replace" ), QIconSet(),
				    tr( "&Replace..." ), CTRL + Key_R, this, 0 );
    connect( actionSearchReplace, SIGNAL( activated() ), this, SLOT( searchReplace() ) );
    actionSearchReplace->setEnabled( FALSE );

    actionSearchGotoLine = new QAction( tr( "Goto Line" ), QIconSet(),
				    tr( "&Goto Line..." ), ALT + Key_G, this, 0 );
    connect( actionSearchGotoLine, SIGNAL( activated() ), this, SLOT( searchGotoLine() ) );
    actionSearchGotoLine->setEnabled( FALSE );

#if defined(HAVE_KDE)
    KToolBar *tb = new KToolBar( this, "Search" );
    tb->setFullSize( FALSE );
#else
    QToolBar *tb = new QToolBar( this, "Search" );
    tb->setCloseMode( QDockWindow::Undocked );
#endif
    addToolBar( tb, tr( "Search" ) );

    actionSearchFind->addTo( tb );
    incrementalSearch = new QLineEdit( tb );
    QToolTip::add( incrementalSearch, tr( "Incremetal Search (ALT+I)" ) );
    connect( incrementalSearch, SIGNAL( textChanged( const QString & ) ),
	     this, SLOT( searchIncremetalFind() ) );
    connect( incrementalSearch, SIGNAL( returnPressed() ),
	     this, SLOT( searchIncremetalFindNext() ) );
    incrementalSearch->setEnabled( FALSE );

    QPopupMenu *menu = new QPopupMenu( this, "Search" );
    menubar->insertItem( tr( "&Search" ), menu );
    actionSearchFind->addTo( menu );
    actionSearchIncremetal->addTo( menu );
    actionSearchReplace->addTo( menu );
    menu->insertSeparator();
    actionSearchGotoLine->addTo( menu );
}

void MainWindow::setupLayoutActions()
{
    if ( !actionGroupTools ) {
	actionGroupTools = new QActionGroup( this );
	actionGroupTools->setExclusive( TRUE );
	connect( actionGroupTools, SIGNAL( selected(QAction*) ), this, SLOT( toolSelected(QAction*) ) );
    }

    actionEditAdjustSize = new QAction( tr( "Adjust Size" ), createIconSet("adjustsize.xpm"),
					tr( "Adjust &Size" ), CTRL + Key_J, this, 0 );
    actionEditAdjustSize->setStatusTip(tr("Adjusts the size of the selected widget") );
    actionEditAdjustSize->setWhatsThis(tr("<b>Adjust the size</b>"
					  "<p>Calculates an appropriate size for the selected widget. This function "
					  "is disabled if the widget is part of a layout, and the layout will "
					  "control the widget\'s geometry.</p>") );
    connect( actionEditAdjustSize, SIGNAL( activated() ), this, SLOT( editAdjustSize() ) );
    actionEditAdjustSize->setEnabled( FALSE );

    actionEditHLayout = new QAction( tr( "Lay Out Horizontally" ), createIconSet("edithlayout.xpm"),
				     tr( "Lay Out &Horizontally" ), CTRL + Key_H, this, 0 );
    actionEditHLayout->setStatusTip(tr("Lays out the selected widgets horizontally") );
    actionEditHLayout->setWhatsThis(tr("<b>Layout widgets horizontally</b>"
				       "<p>The selected widgets will be laid out horizontally. "
				       "If only one widget is selected, its child-widgets will be laid out.</p>") );
    connect( actionEditHLayout, SIGNAL( activated() ), this, SLOT( editLayoutHorizontal() ) );
    actionEditHLayout->setEnabled( FALSE );

    actionEditVLayout = new QAction( tr( "Lay Out Vertically" ), createIconSet("editvlayout.xpm"),
				     tr( "Lay Out &Vertically" ), CTRL + Key_L, this, 0 );
    actionEditVLayout->setStatusTip(tr("Lays out the selected widgets vertically") );
    actionEditVLayout->setWhatsThis(tr("<b>Layout widgets vertically</b>"
				       "<p>The selected widgets will be laid out vertically. "
				       "If only one widget is selected, its child-widgets will be laid out.</p>") );
    connect( actionEditVLayout, SIGNAL( activated() ), this, SLOT( editLayoutVertical() ) );
    actionEditVLayout->setEnabled( FALSE );

    actionEditGridLayout = new QAction( tr( "Lay Out in a Grid" ), createIconSet("editgrid.xpm"),
					tr( "Lay Out in a &Grid" ), CTRL + Key_G, this, 0 );
    actionEditGridLayout->setStatusTip(tr("Lays out the selected widgets in a grid") );
    actionEditGridLayout->setWhatsThis(tr("<b>Layout widgets in a grid</b>"
					  "<p>The selected widgets will be laid out in a grid."
					  "If only one widget is selected, its child-widgets will be laid out.</p>") );
    connect( actionEditGridLayout, SIGNAL( activated() ), this, SLOT( editLayoutGrid() ) );
    actionEditGridLayout->setEnabled( FALSE );

    actionEditSplitHorizontal = new QAction( tr( "Lay Out Horizontally (in Splitter)" ), createIconSet("editvlayoutsplit.xpm"),
					     tr( "Lay Out Horizontally (in &Splitter)" ), 0, this, 0 );
    actionEditSplitHorizontal->setStatusTip(tr("Lays out the selected widgets horizontally in a splitter") );
    actionEditSplitHorizontal->setWhatsThis(tr("<b>Layout widgets horizontally in a splitter</b>"
				       "<p>The selected widgets will be laid out vertically in a splitter.</p>") );
    connect( actionEditSplitHorizontal, SIGNAL( activated() ), this, SLOT( editLayoutHorizontalSplit() ) );
    actionEditSplitHorizontal->setEnabled( FALSE );

    actionEditSplitVertical = new QAction( tr( "Lay Out Vertically (in Splitter)" ), createIconSet("edithlayoutsplit.xpm"),
					     tr( "Lay Out Vertically (in &Splitter)" ), 0, this, 0 );
    actionEditSplitVertical->setStatusTip(tr("Lays out the selected widgets vertically in a splitter") );
    actionEditSplitVertical->setWhatsThis(tr("<b>Layout widgets vertically in a splitter</b>"
				       "<p>The selected widgets will be laid out vertically in a splitter.</p>") );
    connect( actionEditSplitVertical, SIGNAL( activated() ), this, SLOT( editLayoutVerticalSplit() ) );
    actionEditSplitVertical->setEnabled( FALSE );

    actionEditBreakLayout = new QAction( tr( "Break Layout" ), createIconSet("editbreaklayout.xpm"),
					 tr( "&Break Layout" ), CTRL + Key_B, this, 0 );
    actionEditBreakLayout->setStatusTip(tr("Breaks the selected layout") );
    actionEditBreakLayout->setWhatsThis(tr("<b>Break the layout</b>"
					   "<p>The selected layout or the layout of the selected widget "
					   "will be removed.</p>") );
    connect( actionEditBreakLayout, SIGNAL( activated() ), this, SLOT( editBreakLayout() ) );

    int id = WidgetDatabase::idFromClassName( "Spacer" );
    QAction* a = new QAction( actionGroupTools, QString::number( id ).latin1() );
    a->setToggleAction( TRUE );
    a->setText( WidgetDatabase::className( id ) );
    a->setMenuText( tr( "Add ") + WidgetDatabase::className( id ) );
    a->setIconSet( WidgetDatabase::iconSet( id ) );
    a->setToolTip( WidgetDatabase::toolTip( id ) );
    a->setStatusTip( tr( "Insert a %1").arg(WidgetDatabase::toolTip( id )) );
    a->setWhatsThis( QString("<b>A %1</b><p>%2</p>"
			     "<p>Click to insert a single %3,"
			     "or double click to keep the tool selected.")
	.arg(WidgetDatabase::toolTip( id ))
	.arg(WidgetDatabase::whatsThis( id ))
	.arg(WidgetDatabase::toolTip( id ) ));

    QWhatsThis::add( layoutToolBar, tr( "<b>The Layout toolbar</b>%1" ).arg(tr(toolbarHelp).arg("")) );
    actionEditAdjustSize->addTo( layoutToolBar );
    layoutToolBar->addSeparator();
    actionEditHLayout->addTo( layoutToolBar );
    actionEditVLayout->addTo( layoutToolBar );
    actionEditGridLayout->addTo( layoutToolBar );
    actionEditSplitHorizontal->addTo( layoutToolBar );
    actionEditSplitVertical->addTo( layoutToolBar );
    actionEditBreakLayout->addTo( layoutToolBar );
    layoutToolBar->addSeparator();
    a->addTo( layoutToolBar );

    QPopupMenu *menu = new QPopupMenu( this, "Layout" );
    menubar->insertItem( tr( "&Layout" ), menu );
    actionEditAdjustSize->addTo( menu );
    menu->insertSeparator();
    actionEditHLayout->addTo( menu );
    actionEditVLayout->addTo( menu );
    actionEditGridLayout->addTo( menu );
    actionEditSplitHorizontal->addTo( menu );
    actionEditSplitVertical->addTo( menu );
    actionEditBreakLayout->addTo( menu );
    menu->insertSeparator();
    a->addTo( menu );
}

void MainWindow::setupToolActions()
{
    if ( !actionGroupTools ) {
	actionGroupTools = new QActionGroup( this );
	actionGroupTools->setExclusive( TRUE );
	connect( actionGroupTools, SIGNAL( selected(QAction*) ), this, SLOT( toolSelected(QAction*) ) );
    }

    actionPointerTool = new QAction( tr("Pointer"), createIconSet("pointer.xpm"), tr("&Pointer"),  Key_F2,
				     actionGroupTools, QString::number(POINTER_TOOL).latin1(), TRUE );
    actionPointerTool->setStatusTip( tr("Selects the pointer tool") );
    actionPointerTool->setWhatsThis( tr("<b>The pointer tool</b>"
					"<p>The default tool used to select and move widgets on your form. "
					"For some widgets, a double-click opens a dialog where you can enter "
					"the value for the basic property. A context menu with often used "
					"commands is available for all form elements.</p>") );

    actionConnectTool = new QAction( tr("Connect Signal/Slots"), createIconSet("connecttool.xpm"),
				     tr("&Connect Signal/Slots"),  Key_F3,
				     actionGroupTools, QString::number(CONNECT_TOOL).latin1(), TRUE );
    actionConnectTool->setStatusTip( tr("Selects the connection tool") );
    actionConnectTool->setWhatsThis( tr("<b>Connect signals and slots</b>"
					"<p>Create a connection by dragging with the LMB from the widget "
					"emitting a signal to the receiver, and connect the signal and slot "
					"in the opening dialog.</p>"
					"<p>Double click on this tool to keep it selected.</p>") );

    actionOrderTool = new QAction( tr("Tab Order"), createIconSet("ordertool.xpm"),
				   tr("Tab &Order"),  Key_F4,
				   actionGroupTools, QString::number(ORDER_TOOL).latin1(), TRUE );
    actionOrderTool->setStatusTip( tr("Selects the tab order tool") );
    actionOrderTool->setWhatsThis( tr("<b>Change the tab order</b>"
				      "<p>Click on one widget after the other to change the order in which "
				      "they receive the keyboard focus. A double-click on an item will make "
				      "it the first item in the chain and restart the ordering.</p>") );

#if defined(HAVE_KDE)
    KToolBar *tb = new KToolBar( this, "Tools" );
    tb->setFullSize( FALSE );
#else
    QToolBar *tb = new QToolBar( this, "Tools" );
    tb->setCloseMode( QDockWindow::Undocked );
#endif
    QWhatsThis::add( tb, tr( "<b>The Tools toolbar</b>%1" ).arg(tr(toolbarHelp).arg("")) );

    addToolBar( tb, tr( "Tools" ), QMainWindow::Top, TRUE );
    actionPointerTool->addTo( tb );
    actionConnectTool->addTo( tb );
    actionOrderTool->addTo( tb );

    QPopupMenu *mmenu = new QPopupMenu( this, "Tools" );
    menubar->insertItem( tr( "&Tools" ), mmenu );
    actionPointerTool->addTo( mmenu );
    actionConnectTool->addTo( mmenu );
    actionOrderTool->addTo( mmenu );
    mmenu->insertSeparator();

    customWidgetToolBar = 0;
    customWidgetMenu = 0;

    actionToolsCustomWidget = new QAction( tr("Custom Widgets"),
					   createIconSet( "customwidget.xpm" ), tr("Edit &Custom Widgets..."), 0, this, 0 );
    actionToolsCustomWidget->setStatusTip( tr("Opens a dialog to change the custom widgets") );
    actionToolsCustomWidget->setWhatsThis( tr("<b>Change custom widgets</b>"
					      "<p>You can add your own widgets into forms by providing classname "
					      "and name of the header file. You can add properties as well as "
					      "signals and slots to integrate them into the designer, "
					      "and provide a pixmap which will be used to represent the widget on the form.</p>") );

    connect( actionToolsCustomWidget, SIGNAL( activated() ), this, SLOT( toolsCustomWidget() ) );

    for ( int j = 0; j < WidgetDatabase::numWidgetGroups(); ++j ) {
	QString grp = WidgetDatabase::widgetGroup( j );
	if ( !WidgetDatabase::isGroupVisible( grp ) ||
	     WidgetDatabase::isGroupEmpty( grp ) )
	    continue;
#if defined(HAVE_KDE)
	KToolBar *tb = new KToolBar( this, grp.latin1() );
	tb->setFullSize( FALSE );
#else
	QToolBar *tb = new QToolBar( this, grp.latin1() );
	tb->setCloseMode( QDockWindow::Undocked );
#endif
	bool plural = grp[(int)grp.length()-1] == 's';
	if ( plural ) {
	    QWhatsThis::add( tb, tr( "<b>The %1</b>%2" ).arg(grp).arg(tr(toolbarHelp).
						arg( tr(" Click on a button to insert a single widget, "
						"or double click to insert multiple %1.") ).arg(grp)) );
	} else {
	    QWhatsThis::add( tb, tr( "<b>The %1 Widgets</b>%2" ).arg(grp).arg(tr(toolbarHelp).
						arg( tr(" Click on a button to insert a single %1 widget, "
						"or double click to insert multiple widgets.") ).arg(grp)) );
	}
	addToolBar( tb, grp );
	QPopupMenu *menu = new QPopupMenu( this, grp.latin1() );
	mmenu->insertItem( grp, menu );

	if ( grp == "Custom" ) {
	    if ( !customWidgetMenu )
		actionToolsCustomWidget->addTo( menu );
	    else
		menu->insertSeparator();
	    customWidgetMenu = menu;
	    customWidgetToolBar = tb;
	}

	for ( int i = 0; i < WidgetDatabase::count(); ++i ) {
	    if ( WidgetDatabase::group( i ) != grp )
		continue; // only widgets, i.e. not forms and temp stuff
	    QAction* a = new QAction( actionGroupTools, QString::number( i ).latin1() );
	    a->setToggleAction( TRUE );
	    if ( WidgetDatabase::className( i )[0] == 'Q' )
		a->setText( WidgetDatabase::className( i ).mid(1) );
	    else
		a->setText( WidgetDatabase::className( i ) );
	    QString ttip = WidgetDatabase::toolTip( i );
	    a->setIconSet( WidgetDatabase::iconSet( i ) );
	    a->setToolTip( ttip );
	    if ( !WidgetDatabase::isWhatsThisLoaded() )
		WidgetDatabase::loadWhatsThis( documentationPath() );
	    a->setStatusTip( tr( "Insert a %1").arg(WidgetDatabase::className( i )) );

	    QString whats = QString("<b>A %1</b>").arg( WidgetDatabase::className( i ) );
	    if ( !WidgetDatabase::whatsThis( i ).isEmpty() )
	    whats += QString("<p>%1</p>").arg(WidgetDatabase::whatsThis( i ));
	    a->setWhatsThis( whats + tr("<p>Double click on this tool to keep it selected.</p>") );

	    if ( grp != "KDE" )
		a->addTo( tb );
	    a->addTo( menu );
	}
    }

    if ( !customWidgetToolBar ) {
#if defined(HAVE_KDE)
	KToolBar *tb = new KToolBar( this, "Custom Widgets" );
	tb->setFullSize( FALSE );
#else
	QToolBar *tb = new QToolBar( this, "Custom Widgets" );
	tb->setCloseMode( QDockWindow::Undocked );
#endif
	QWhatsThis::add( tb, tr( "<b>The Custom Widgets toolbar</b>%1"
				 "<p>Select <b>Edit Custom Widgets...</b> in the <b>Tools->Custom</b> menu to "
				 "add and change custom widgets</p>" ).arg(tr(toolbarHelp).
				 arg( tr(" Click on the buttons to insert a single widget, "
				 "or double click to insert multiple widgets.") )) );
	addToolBar( tb, "Custom" );
	customWidgetToolBar = tb;
	QPopupMenu *menu = new QPopupMenu( this, "Custom Widgets" );
	mmenu->insertItem( "Custom", menu );
	customWidgetMenu = menu;
	customWidgetToolBar->hide();
	actionToolsCustomWidget->addTo( customWidgetMenu );
	customWidgetMenu->insertSeparator();
    }

    resetTool();
}

void MainWindow::setupFileActions()
{
#if defined(HAVE_KDE)
    KToolBar *tb = new KToolBar( this, "File" );
    tb->setFullSize( FALSE );
#else
    QToolBar* tb  = new QToolBar( this, "File" );
    tb->setCloseMode( QDockWindow::Undocked );
#endif
    QWhatsThis::add( tb, tr( "<b>The File toolbar</b>%1" ).arg(tr(toolbarHelp).arg("")) );
    addToolBar( tb, tr( "File" ) );
    fileMenu = new QPopupMenu( this, "File" );
    menubar->insertItem( tr( "&File" ), fileMenu );

    QAction *a = 0;

    a = new QAction( this, 0 );
    a->setText( tr( "New" ) );
    a->setMenuText( tr( "&New" ) );
    a->setIconSet( createIconSet("filenew.xpm") );
    a->setAccel( CTRL + Key_N );
    a->setStatusTip( tr( "Creates a new form" ) );
    a->setWhatsThis( tr("<b>Create a new form</b>"
			"<p>Select a template for the new form or start with an empty form. This form is added to the current project.</p>") );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( tb );
    a->addTo( fileMenu );

    a = new QAction( this, 0 );
    a->setText( tr( "Open" ) );
    a->setMenuText( tr( "&Open..." ) );
    a->setIconSet( createIconSet("fileopen.xpm") );
    a->setAccel( CTRL + Key_O );
    a->setStatusTip( tr( "Opens an existing form") );
    a->setWhatsThis( tr("<b>Open a User-Interface (ui) file</b>"
			"<p>Use the filedialog to select the file you want to "
			"open. You can also use Drag&Drop to open multiple files.</p>") );
    connect( a, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    a->addTo( tb );
    a->addTo( fileMenu );

    fileMenu->insertSeparator();

    a = new QAction( this, 0 );
    a->setText( tr( "New Project" ) );
    a->setMenuText( tr( "New &Project..." ) );
    a->setIconSet( createIconSet("filenew.xpm") );
    a->setStatusTip( tr( "Creates a new project" ) );
    a->setWhatsThis( tr("<b>Create a new project</b>"
			"<p>Creates a new Qt project</p>") );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNewProject() ) );
    a->addTo( fileMenu );

    a = new QAction( this, 0 );
    a->setText( tr( "Close Project" ) );
    a->setMenuText( tr( "Close P&roject" ) );
    a->setStatusTip( tr( "Closes the current project" ) );
    a->setWhatsThis( tr("<b>Closes the current project</b>"
			"<p>Closes the current project, if one exists.</p>") );
    connect( a, SIGNAL( activated() ), this, SLOT( fileCloseProject() ) );
    a->addTo( fileMenu );

    fileMenu->insertSeparator();

    a = new QAction( this, 0 );
    a->setText( tr( "Save" ) );
    a->setMenuText( tr( "&Save" ) );
    a->setIconSet( createIconSet("filesave.xpm") );
    a->setAccel( CTRL + Key_S );
    a->setStatusTip( tr( "Saves the current form" ) );
    a->setWhatsThis( tr("<b>Save the current form</b>"
			"<p>A filedialog will open if there is no filename already "
			"provided, otherwise the old name will be used.</p>") );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSave() ) );
    connect( this, SIGNAL( hasActiveWindow(bool) ), a, SLOT( setEnabled(bool) ) );
    a->addTo( tb );
    a->addTo( fileMenu );

    a = new QAction( this, 0 );
    a->setText( tr( "Save As" ) );
    a->setMenuText( tr( "Save &As..." ) );
    a->setStatusTip( tr( "Saves the current form with a new filename" ) );
    a->setWhatsThis( tr( "Save the current form with a new filename" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
    connect( this, SIGNAL( hasActiveWindow(bool) ), a, SLOT( setEnabled(bool) ) );
    a->addTo( fileMenu );

    a = new QAction( this, 0 );
    a->setText( tr( "Save All" ) );
    a->setMenuText( tr( "Sa&ve All" ) );
    a->setStatusTip( tr( "Saves all open forms" ) );
    a->setWhatsThis( tr( "Save all open forms" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSaveAll() ) );
    connect( this, SIGNAL( hasActiveWindow(bool) ), a, SLOT( setEnabled(bool) ) );
    a->addTo( fileMenu );

    fileMenu->insertSeparator();

    QActionGroup *ag = new QActionGroup( this, 0 );
    ag->setText( tr( "Project" ) );
    ag->setMenuText( tr( "Project" ) );
    ag->setExclusive( TRUE );
    ag->setUsesDropDown( TRUE );
    connect( ag, SIGNAL( selected( QAction * ) ), this, SLOT( projectSelected( QAction * ) ) );
    connect( ag, SIGNAL( selected( QAction * ) ), this, SIGNAL( projectChanged() ) );
    a = new QAction( tr( "<No Project>" ), tr( "<No Project>" ), 0, ag, 0, TRUE );
    eProject = new Project( "", tr( "<No Project>" ), projectSettingsPluginManager );
    projects.insert( a, eProject );
    a->setOn( TRUE );
    ag->addTo( fileMenu );
    ag->addTo( tb );
    projectToolBar = tb;
    actionGroupProjects = ag;

    fileMenu->insertSeparator();

    a = new QAction( this, 0 );
    a->setText( tr( "Create Template" ) );
    a->setMenuText( tr( "&Create Template..." ) );
    a->setStatusTip( tr( "Creates a new template" ) );
    a->setWhatsThis( tr( "Creates a new template" ) );
    connect( a, SIGNAL( activated() ), this, SLOT( fileCreateTemplate() ) );
    a->addTo( fileMenu );

    fileMenu->insertSeparator();

    recentlyFilesMenu = new QPopupMenu( this );
    recentlyProjectsMenu = new QPopupMenu( this );

    fileMenu->insertItem( tr( "Recenty opened files " ), recentlyFilesMenu );
    fileMenu->insertItem( tr( "Recenty opened projects" ), recentlyProjectsMenu );

    connect( recentlyFilesMenu, SIGNAL( aboutToShow() ),
	     this, SLOT( setupRecentlyFilesMenu() ) );
    connect( recentlyProjectsMenu, SIGNAL( aboutToShow() ),
	     this, SLOT( setupRecentlyProjectsMenu() ) );
    connect( recentlyFilesMenu, SIGNAL( activated( int ) ),
	     this, SLOT( recentlyFilesMenuActivated( int ) ) );
    connect( recentlyProjectsMenu, SIGNAL( activated( int ) ),
	     this, SLOT( recentlyProjectsMenuActivated( int ) ) );

    fileMenu->insertSeparator();

    a = new QAction( this, 0 );
    a->setText( tr( "Exit" ) );
    a->setMenuText( tr( "E&xit" ) );
    a->setStatusTip( tr( "Quits the application and prompts to save changed forms" ) );
    a->setWhatsThis( tr( "<b>Exit the designer</b>"
			 "<p>The Qt Designer will ask if you want to save changed forms before "
			 "the application closes.</p>") );
    connect( a, SIGNAL( activated() ), qApp, SLOT( closeAllWindows() ) );
    a->addTo( fileMenu );
}

void MainWindow::setupPreviewActions()
{
    QAction* a = 0;
    QPopupMenu *menu = new QPopupMenu( this, "Preview" );
    menubar->insertItem( tr( "&Preview" ), menu );

    a = new QAction( tr( "Preview Form" ), createIconSet("previewform.xpm"),
				     tr( "Preview &Form" ), 0, this, 0 );
    a->setAccel( CTRL + Key_T );
    a->setStatusTip( tr("Opens a preview") );
    a->setWhatsThis( tr("<b>Open a preview</b>"
			"<p>Use the preview to test the design and "
			"signal-slot connections of the current form.</p>") );
    connect( a, SIGNAL( activated() ), this, SLOT( previewForm() ) );
    connect( this, SIGNAL( hasActiveForm(bool) ), a, SLOT( setEnabled(bool) ) );
    a->addTo( menu );

    menu->insertSeparator();

    QSignalMapper *mapper = new QSignalMapper( this );
    connect( mapper, SIGNAL(mapped(const QString&)), this, SLOT(previewForm(const QString&)) );
    QStringList styles = QStyleFactory::styles();
    for ( QStringList::Iterator it = styles.begin(); it != styles.end(); ++it ) {
	QString info;
	if ( *it == "Motif" )
	    info = tr( "The preview will use the Motif Look&Feel used as the default style on most UNIX-Systems." );
	else if ( *it == "Windows" )
	    info = tr( "The preview will use the Windows Look&Feel used as the default style on Windows-Systems." );
	else if ( *it == "Platinum" )
	    info = tr( "The preview will use the Platinum Look&Feel resembling a Macinosh-like GUI style." );
	else if ( *it == "CDE" )
	    info = tr( "The preview will use the CDE Look&Feel which is similar to some versions of the Common Desktop Environment." );
	else if ( *it == "SGI" )
	    info = tr( "The preview will use the Motif Look&Feel used as the default style on SGI-systems." );
	else if ( *it == "MotifPlus" )
	    info = tr( "The preview will use an advanced Motif Look&Feel as used by the GIMP toolkit (GTK) on Linux." );

	a = new QAction( tr( "Preview Form in %1 Style" ).arg( *it ), createIconSet("previewform.xpm"),
					 tr( "... in %1 Style" ).arg( *it ), 0, this, 0 );
	a->setStatusTip( tr("Opens a preview in %1 style").arg( *it ) );
	a->setWhatsThis( tr("<b>Open a preview in %1 style.</b>"
			"<p>Use the preview to test the design and "
			"signal-slot connections of the current form. %2</p>").arg( *it ).arg( info ) );
	mapper->setMapping( a, *it );
	connect( a, SIGNAL(activated()), mapper, SLOT(map()) );
	connect( this, SIGNAL( hasActiveForm(bool) ), a, SLOT( setEnabled(bool) ) );
	a->addTo( menu );
    }
}

void MainWindow::setupWindowActions()
{
    static bool windowActionsSetup = FALSE;
    if ( !windowActionsSetup ) {
	windowActionsSetup = TRUE;

	actionWindowTile = new QAction( tr( "Tile" ), tr( "&Tile" ), 0, this );
	actionWindowTile->setStatusTip( tr("Arranges all windows tiled") );
	actionWindowTile->setWhatsThis( tr("Arrange all windows tiled") );
	connect( actionWindowTile, SIGNAL( activated() ), workspace, SLOT( tile() ) );
	actionWindowCascade = new QAction( tr( "Cascade" ), tr( "&Cascade" ), 0, this );
	actionWindowCascade->setStatusTip( tr("Arrange all windows cascaded") );
	actionWindowCascade->setWhatsThis( tr("Arrange all windows cascaded") );
	connect( actionWindowCascade, SIGNAL( activated() ), workspace, SLOT( cascade() ) );

	actionWindowClose = new QAction( tr( "Close" ), tr( "Cl&ose" ), CTRL + Key_F4, this );
	actionWindowClose->setStatusTip( tr( "Closes the active window") );
	actionWindowClose->setWhatsThis( tr( "Close the active window") );
	connect( actionWindowClose, SIGNAL( activated() ), workspace, SLOT( closeActiveWindow() ) );

	actionWindowCloseAll = new QAction( tr( "Close All" ), tr( "Close Al&l" ), 0, this );
	actionWindowCloseAll->setStatusTip( tr( "Closes all form windows") );
	actionWindowCloseAll->setWhatsThis( tr( "Close all form windows") );
	connect( actionWindowCloseAll, SIGNAL( activated() ), this, SLOT( closeAllForms() ) );

	actionWindowNext = new QAction( tr( "Next" ), tr( "Ne&xt" ), CTRL + Key_F6, this );
	actionWindowNext->setStatusTip( tr( "Activates the next window" ) );
	actionWindowNext->setWhatsThis( tr( "Activate the next window" ) );
	connect( actionWindowNext, SIGNAL( activated() ), workspace, SLOT( activateNextWindow() ) );

	actionWindowPrevious = new QAction( tr( "Previous" ), tr( "Pre&vious" ), CTRL + SHIFT + Key_F6, this );
	actionWindowPrevious->setStatusTip( tr( "Activates the previous window" ) );
	actionWindowPrevious->setWhatsThis( tr( "Activate the previous window" ) );
	connect( actionWindowPrevious, SIGNAL( activated() ), workspace, SLOT( activatePreviousWindow() ) );
    }

    if ( !windowMenu ) {
	windowMenu = new QPopupMenu( this, "Window" );
	menubar->insertItem( tr( "&Window" ), windowMenu );
	connect( windowMenu, SIGNAL( aboutToShow() ),
		 this, SLOT( setupWindowActions() ) );
    } else {
	windowMenu->clear();
    }

    actionWindowClose->addTo( windowMenu );
    actionWindowCloseAll->addTo( windowMenu );
    windowMenu->insertSeparator();
    actionWindowNext->addTo( windowMenu );
    actionWindowPrevious->addTo( windowMenu );
    windowMenu->insertSeparator();
    actionWindowTile->addTo( windowMenu );
    actionWindowCascade->addTo( windowMenu );
    windowMenu->insertSeparator();
    windowMenu->insertItem( tr( "&Views" ), createDockWindowMenu( NoToolBars ) );
    windowMenu->insertItem( tr( "&Toolbars" ), createDockWindowMenu( OnlyToolBars ) );
    QWidgetList windows = workspace->windowList();
    if ( windows.count() && formWindow() )
	windowMenu->insertSeparator();
    int j = 0;
    for ( int i = 0; i < int( windows.count() ); ++i ) {
	QWidget *w = windows.at( i );
	if ( !w->inherits( "FormWindow" ) && !w->inherits( "SourceEditor" ) )
	    continue;
	j++;
	QString itemText;
	if ( j < 10 )
	    itemText = QString("&%1 ").arg( j );
	if ( w->inherits( "FormWindow" ) )
	    itemText += w->name();
	else
	    itemText += w->caption();

	int id = windowMenu->insertItem( itemText, this, SLOT( windowsMenuActivated( int ) ) );
	windowMenu->setItemParameter( id, i );
	windowMenu->setItemChecked( id, workspace->activeWindow() == windows.at( i ) );
    }
}

void MainWindow::setupHelpActions()
{
    actionHelpContents = new QAction( tr( "Contents" ), tr( "&Contents" ), Key_F1, this, 0 );
    actionHelpContents->setStatusTip( tr("Opens the online help") );
    actionHelpContents->setWhatsThis( tr("<b>Open the online help</b>"
					 "<p>Use the online help to get detailed information "
					 "about selected components. Press the F1 key to open "
					 "context sensitive help on the selected item or property.</p>") );
    connect( actionHelpContents, SIGNAL( activated() ), this, SLOT( helpContents() ) );

    actionHelpManual = new QAction( tr( "Manual" ), tr( "&Manual" ), CTRL + Key_M, this, 0 );
    actionHelpManual->setStatusTip( tr("Opens the Qt Designer manual") );
    actionHelpManual->setWhatsThis( tr("<b>Open the Qt Designer manual</b>"
					 "<p>Use the Qt Designer Manual to get help about how to use the Qt Designer.</p>") );
    connect( actionHelpManual, SIGNAL( activated() ), this, SLOT( helpManual() ) );

    actionHelpAbout = new QAction( tr("About"), QPixmap(), tr("&About..."), 0, this, 0 );
    actionHelpAbout->setStatusTip( tr("Displays information about this product") );
    actionHelpAbout->setWhatsThis( tr("Get information about this product") );
    connect( actionHelpAbout, SIGNAL( activated() ), this, SLOT( helpAbout() ) );

    actionHelpAboutQt = new QAction( tr("About Qt"), QPixmap(), tr("About &Qt..."), 0, this, 0 );
    actionHelpAboutQt->setStatusTip( tr("Displays information about the Qt Toolkit") );
    actionHelpAboutQt->setWhatsThis( tr("Get information about the Qt Toolkit") );
    connect( actionHelpAboutQt, SIGNAL( activated() ), this, SLOT( helpAboutQt() ) );

    actionHelpWhatsThis = new QAction( tr("What's This?"), QIconSet( whatsthis_image, whatsthis_image ),
				       tr("What's This?"), SHIFT + Key_F1, this, 0 );
    actionHelpWhatsThis->setStatusTip( tr("\"What's This?\" context sensitive help") );
    actionHelpWhatsThis->setWhatsThis( tr("<b>That's me!</b>"
					  "<p>In What's This?-Mode, the mouse cursor shows an arrow with a questionmark, "
					  "and you can click on the interface elements to get a short "
					  "description of what they do and how to use them. In dialogs, "
					  "this feature can be accessed using the context help button in the titlebar.</p>") );
    connect( actionHelpWhatsThis, SIGNAL( activated() ), this, SLOT( whatsThis() ) );

#if defined(HAVE_KDE)
    KToolBar *tb = new KToolBar( this, "Help" );
    tb->setFullSize( FALSE );
#else
    QToolBar *tb = new QToolBar( this, "Help" );
    tb->setCloseMode( QDockWindow::Undocked );
#endif
    QWhatsThis::add( tb, tr( "<b>The Help toolbar</b>%1" ).arg(tr(toolbarHelp).arg("") ));
    addToolBar( tb, tr( "Help" ) );
    actionHelpWhatsThis->addTo( tb );

    QPopupMenu *menu = new QPopupMenu( this, "Help" );
    menubar->insertSeparator();
    menubar->insertItem( tr( "&Help" ), menu );
    actionHelpContents->addTo( menu );
    actionHelpManual->addTo( menu );
    menu->insertSeparator();
    actionHelpAbout->addTo( menu );
    actionHelpAboutQt->addTo( menu );
    menu->insertSeparator();
    actionHelpWhatsThis->addTo( menu );
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
    dw->setCaption( tr( "Property Editor/Events" ) );
    QWhatsThis::add( propertyEditor, tr("<b>The Property Editor</b>"
					"<p>You can change the appearance and behaviour of the selected widget in the "
					"property editor.</p>"
					"<p>You can set properties for components and forms at design time and see the "
					"changes immediately. Each property has its own editor which you can use to enter "
					"new values, open a special dialog or select values from a predefined list. "
					"Use <b>F1</b> to get detailed help for the selected property.</p>"
					"<p>You can resize the columns of the editor by dragging the separators of the list "
					"header.</p>" ) );
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

static void unifyFormName( FormWindow *fw, QWorkspace *workspace )
{
    QStringList lst;
    QWidgetList windows = workspace->windowList();
    for ( QWidget *w =windows.first(); w; w = windows.next() ) {
	if ( w == fw )
	    continue;
	lst << w->name();
    }

    if ( lst.findIndex( fw->name() ) == -1 )
	return;
    QString origName = fw->name();
    QString n = origName;
    int i = 1;
    while ( lst.findIndex( n ) != -1 ) {
	n = origName + QString::number( i++ );
    }
    fw->setName( n );
    fw->setCaption( n );
}

void MainWindow::fileNew()
{
    statusBar()->message( tr( "Select a template for the new form...") );
    NewForm dlg( this, templatePath() );
    if ( dlg.exec() == QDialog::Accepted ) {
	NewForm::Form f = dlg.formType();
	if ( f != NewForm::Custom ) {
	    insertFormWindow( f )->setFocus();
	} else {
	    QString filename = dlg.templateFile();
	    if ( !filename.isEmpty() && QFile::exists( filename ) ) {
		Resource resource( this );
		if ( !resource.load( filename, FALSE ) ) {
		    QMessageBox::information( this, tr("Load Template"),
			tr("Couldn't load form description from template "+ filename ) );
		    return;
		}
		if ( formWindow() )
		    formWindow()->setFileName( QString::null );
		unifyFormName( formWindow(), workspace );
	    }
	}
    }

    if ( formWindow() && formWindow()->project() != emptyProject() ) {
	formWindow()->setSavePixmapInProject( TRUE );
	formWindow()->setSavePixmapInline( FALSE );
    }

    statusBar()->clear();
}

void MainWindow::fileNewProject()
{
    Project *pro = new Project( "", "", projectSettingsPluginManager );
    ProjectSettings dia( pro, this, 0, TRUE );
    if ( dia.exec() != QDialog::Accepted ) {
	delete pro;
	return;
    }

    if ( !pro->isValid() ) {
	QMessageBox::information( this, tr("New Project"),
			tr("Cannot create invalid project." ) );
	delete pro;
	return;
    }

    QAction *a = new QAction( pro->projectName(), pro->projectName(), 0, actionGroupProjects, 0, TRUE );
    projects.insert( a, pro );
    addRecentlyOpened( pro->makeAbsolute( pro->fileName() ), recentlyProjects );
    a->setOn( TRUE );
    projectSelected( a );
}

void MainWindow::fileCloseProject()
{
    if ( currentProject->projectName() == "<No Project>" )
	return;
    Project *pro = currentProject;
    QAction* a = 0;
    QAction* lastValid = 0;
    for ( QMap<QAction*, Project* >::Iterator it = projects.begin(); it != projects.end(); ++it ) {
	if ( it.data() == pro ) {
	    a = it.key();
	    if ( lastValid )
		break;
	}
	lastValid = it.key();
    }
    if ( a ) {
	pro->save();
	QWidgetList windows = workSpace()->windowList();
	workSpace()->blockSignals( TRUE );
	for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	    if ( !w->inherits( "FormWindow" ) )
		continue;
	    if ( ( (FormWindow*)w )->project() == pro ) {
		if ( !closeForm( (FormWindow*)w ) )
		    return;
		w->close();
	    }
	}
	windows = workSpace()->windowList();
	workSpace()->blockSignals( FALSE );
	actionGroupProjects->removeChild( a );
	projects.remove( a );
	delete a;
	currentProject = 0;
	if ( lastValid ) {
	    projectSelected( lastValid );
	    lastValid->setOn( TRUE );
	    statusBar()->message( tr( currentProject->projectName() + " project selected...") );
	}
	if ( !windows.isEmpty() ) {
	    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
		if ( !w->inherits( "FormWindow" ) )
		    continue;
		w->setFocus();
		activeWindowChanged( w );
		break;
	    }
	}
    }
}


void MainWindow::fileOpen( const QString &filter, const QString &extension )
{
    statusBar()->message( tr( "Select a file...") );

    QInterfaceManager<ImportFilterInterface> manager( IID_ImportFilterInterface, pluginDir );
    QStringList paths(QApplication::libraryPaths());
    QStringList::Iterator it = paths.begin();
    while (it != paths.end()) {
	manager.addLibraryPath(*it + "/designer");
	it++;
    }

    {
	QString filename;
	QStringList filterlist;
	if ( filter.isEmpty() ) {
	    filterlist << tr( "Designer Files (*.ui *.pro)" );
	    filterlist << tr( "Qt User-Interface Files (*.ui)" );
	    filterlist << tr( "QMAKE Project Files (*.pro)" );
	    QStringList list = manager.featureList();
	    for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it )
		filterlist << *it;
	    filterlist << tr( "All Files (*)" );
	} else {
	    filterlist << filter;
	}

	QString filters = filterlist.join( ";;" );

	filename = QFileDialog::getOpenFileName( QString::null, filters, this );
	if ( !filename.isEmpty() ) {
	    QFileInfo fi( filename );

	    if ( fi.extension() == "pro" && ( extension.isEmpty() || extension == "pro" ) ) {
		addRecentlyOpened( filename, recentlyProjects );
		openProject( filename );
	    } else if ( fi.extension() == "ui" && ( extension.isEmpty() || extension == "ui" ) ) {
		openFile( filename );
		addRecentlyOpened( filename, recentlyFiles );
	    } else if ( !extension.isEmpty() && fi.extension() == extension ) {
		LanguageInterface *iface = MetaDataBase::languageInterface( currentProject->language() );
		if ( iface && iface->supports( LanguageInterface::AdditionalFiles ) ) {
		    QMap<QString, QString> extensionFilterMap;
		    iface->fileFilters( extensionFilterMap );
		    if ( extensionFilterMap.find( extension ) != extensionFilterMap.end() ) {
			SourceFile *sf = new SourceFile( currentProject->makeRelative( filename ) );
			MetaDataBase::addEntry( sf );
			currentProject->addSourceFile( sf );
			formList->setProject( currentProject );
			// ### show source file
		    }
		}
	    } else if ( extension.isEmpty() ) {
		QString filter;
		for ( QStringList::Iterator it2 = filterlist.begin(); it2 != filterlist.end(); ++it2 ) {
		    if ( (*it2).contains( fi.extension(), FALSE ) ) {
			filter = *it2;
			break;
		    }
		}

		ImportFilterInterface* iface = 0;
		manager.queryInterface( filter, (QUnknownInterface**)&iface );
		if ( !iface ) {
		    statusBar()->message( tr( "No import filter available for %1").arg( filename ), 3000 );
		    return;
		}
		statusBar()->message( tr( "Importing %1 using import filter ...").arg( filename ) );
		QStringList list = iface->import( filter, filename );
		iface->release();
		if ( list.isEmpty() ) {
		    statusBar()->message( tr( "Nothing to load in %1").arg( filename ), 3000 );
		    return;
		}
		for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
		    openFile( *it, FALSE );
		    QFile::remove( *it );
		}
		statusBar()->clear();
	    }
	}
    }
}

void MainWindow::openFile( const QString &filename, bool validFileName )
{
    if ( filename.isEmpty() )
	return;

    bool makeNew = FALSE;

    if ( !QFile::exists( filename ) ) {
	makeNew = TRUE;
    } else {
	QFile f( filename );
	f.open( IO_ReadOnly );
	QTextStream ts( &f );
	makeNew = ts.read().length() < 2;
    }
    if ( !makeNew ) {
	statusBar()->message( tr( "Reading file %1...").arg( filename ) );
	if ( QFile::exists( filename ) ) {
	    QApplication::setOverrideCursor( WaitCursor );
	    Resource resource( this );
	    bool b = resource.load( filename ) && (FormWindow*)resource.widget();
	    if ( !validFileName && resource.widget() )
		( (FormWindow*)resource.widget() )->setFileName( QString::null );
	    QApplication::restoreOverrideCursor();
	    if ( b ) {
		rebuildCustomWidgetGUI();
		statusBar()->message( tr( "File %1 opened.").arg( filename ), 3000 );
	    } else {
		statusBar()->message( tr( "Failed to load file %1").arg( filename ), 5000 );
		QMessageBox::information( this, tr("Load File"), tr("Couldn't load file %1").arg( filename ) );
	    }
	} else {
	    statusBar()->clear();
	}
    } else {
	fileNew();
	if ( formWindow() )
	    formWindow()->setFileName( filename );
    }
}

bool MainWindow::fileSave()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->object() == formWindow() || e == workSpace()->activeWindow() ) {
	    e->save();
	    e->setModified( FALSE );
	}
	if ( e->object() && e->object()->inherits( "SourceFile" ) &&
	     e == workSpace()->activeWindow() )
	    ( (SourceFile*)e->object() )->save();

    }
    if ( !formWindow() )
	return FALSE;
    if ( formWindow()->fileName().isEmpty() ) {
	return fileSaveAs();
    } else {
	QApplication::setOverrideCursor( WaitCursor );
	formWindow()->save( formWindow()->fileName() );
	QApplication::restoreOverrideCursor();
    }
    return TRUE;
}

bool MainWindow::fileSaveAs()
{
    statusBar()->message( tr( "Enter a filename..." ) );
    if ( !formWindow() )
	return FALSE;
    FormWindow *fw = formWindow();

    QString filename = QFileDialog::getSaveFileName( QString::null, tr( "Qt User-Interface Files (*.ui)" ) + ";;" +
								tr( "All Files (*)" ), this );

    if ( filename.isEmpty() )
	return FALSE;
    QFileInfo fi( filename );
    if ( fi.extension() != "ui" )
	filename += ".ui";
    fw->setFileName( filename );
    fileSave();
    return TRUE;
}

void MainWindow::fileSaveAll()
{
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	e->save();
	e->setModified( FALSE );
	if ( e->object() && e->object()->inherits( "SourceFile" ) &&
	     e == workSpace()->activeWindow() )
	    ( (SourceFile*)e->object() )->save();
    }

    QWidgetList windows = workSpace()->windowList();
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;
	w->setFocus();
	qApp->processEvents();
	fileSave();
    }
}

static bool inSaveAllTemp = FALSE;
void MainWindow::saveAllTemp()
{
    if ( inSaveAllTemp )
	return;
    inSaveAllTemp = TRUE;
    statusBar()->message( tr( "Qt Designer is crashing - attempting to save work..." ) );
    QWidgetList windows = workSpace()->windowList();
    QString baseName = QDir::homeDirPath() + "/.designer/saved-form-";
    int i = 1;
    for ( QWidget *w = windows.first(); w; w = windows.next() ) {
	if ( !w->inherits( "FormWindow" ) )
	    continue;

	QString fn = baseName + QString::number( i++ ) + ".ui";
	( (FormWindow*)w )->setFileName( fn );
	( (FormWindow*)w )->save( fn );
    }
    inSaveAllTemp = FALSE;
}

void MainWindow::fileCreateTemplate()
{
    CreateTemplate dia( this, 0, TRUE );

    int i = 0;
    for ( i = 0; i < WidgetDatabase::count(); ++i ) {
	if ( WidgetDatabase::isForm( i ) && WidgetDatabase::widgetGroup( i ) != "Temp") {
	    dia.listClass->insertItem( WidgetDatabase::className( i ) );
	}
    }
    for ( i = 0; i < WidgetDatabase::count(); ++i ) {
	if ( WidgetDatabase::isContainer( i ) && !WidgetDatabase::isForm(i) &&
	     WidgetDatabase::className( i ) != "QTabWidget" && WidgetDatabase::widgetGroup( i ) != "Temp" ) {
	    dia.listClass->insertItem( WidgetDatabase::className( i ) );
	}
    }

    QList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
    for ( MetaDataBase::CustomWidget *w = lst->first(); w; w = lst->next() ) {
	if ( w->isContainer )
	    dia.listClass->insertItem( w->className );
    }

    dia.editName->setText( tr( "NewTemplate" ) );
    connect( dia.buttonCreate, SIGNAL( clicked() ),
	     this, SLOT( createNewTemplate() ) );
    dia.exec();
}

void MainWindow::editUndo()
{
    if ( workSpace()->activeWindow() &&
	 workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
	( (SourceEditor*)workSpace()->activeWindow() )->editUndo();
	return;
    }
    if ( formWindow() )
	formWindow()->undo();
}

void MainWindow::editRedo()
{
    if ( workSpace()->activeWindow() &&
	 workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
	( (SourceEditor*)workSpace()->activeWindow() )->editRedo();
	return;
    }
    if ( formWindow() )
	formWindow()->redo();
}

void MainWindow::editCut()
{
    if ( workSpace()->activeWindow() &&
	 workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
	( (SourceEditor*)workSpace()->activeWindow() )->editCut();
	return;
    }
    editCopy();
    editDelete();
}

void MainWindow::editCopy()
{
    if ( workSpace()->activeWindow() &&
	 workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
	( (SourceEditor*)workSpace()->activeWindow() )->editCopy();
	return;
    }
    if ( formWindow() )
	qApp->clipboard()->setText( formWindow()->copy() );
}

void MainWindow::editPaste()
{
    if ( workSpace()->activeWindow() &&
	 workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
	( (SourceEditor*)workSpace()->activeWindow() )->editPaste();
	return;
    }
    if ( !formWindow() )
	return;

    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 ) {
	w = l.first();
	if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
	     ( !WidgetDatabase::isContainer( WidgetDatabase::idFromClassName( WidgetFactory::classNameOf( w ) ) ) &&
	       w != formWindow()->mainContainer() ) )
	    w = formWindow()->mainContainer();
    }

    if ( w && WidgetFactory::layoutType( w ) == WidgetFactory::NoLayout ) {
	formWindow()->paste( qApp->clipboard()->text(), WidgetFactory::containerOfWidget( w ) );
	hierarchyView->widgetInserted( 0 );
	formWindow()->commandHistory()->setModified( TRUE );
    } else {
	// #### should we popup a messagebox here which says that
	// nothing has been pasted because you can't paste into a
	// laid out widget? (RS)
    }
}

void MainWindow::editDelete()
{
    if ( formWindow() )
	formWindow()->deleteWidgets();
}

void MainWindow::editSelectAll()
{
    if ( workSpace()->activeWindow() &&
	 workSpace()->activeWindow()->inherits( "SourceEditor" ) ) {
	( (SourceEditor*)workSpace()->activeWindow() )->editSelectAll();
	return;
    }
    if ( formWindow() )
	formWindow()->selectAll();
}


void MainWindow::editLower()
{
    if ( formWindow() )
	formWindow()->lowerWidgets();
}

void MainWindow::editRaise()
{
    if ( formWindow() )
	formWindow()->raiseWidgets();
}

void MainWindow::editAdjustSize()
{
    if ( formWindow() )
	formWindow()->editAdjustSize();
}

void MainWindow::editLayoutHorizontal()
{
    if ( layoutChilds )
	editLayoutContainerHorizontal();
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutHorizontal();
}

void MainWindow::editLayoutVertical()
{
    if ( layoutChilds )
	editLayoutContainerVertical();
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutVertical();
}

void MainWindow::editLayoutHorizontalSplit()
{
    if ( layoutChilds )
	; // no way to do that
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutHorizontalSplit();
}

void MainWindow::editLayoutVerticalSplit()
{
    if ( layoutChilds )
	; // no way to do that
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutVerticalSplit();
}

void MainWindow::editLayoutGrid()
{
    if ( layoutChilds )
	editLayoutContainerGrid();
    else if ( layoutSelected && formWindow() )
	formWindow()->layoutGrid();
}

void MainWindow::editLayoutContainerVertical()
{
    if ( !formWindow() )
	return;
    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 )
	w = l.first();
    if ( w )
	formWindow()->layoutVerticalContainer( w  );
}

void MainWindow::editLayoutContainerHorizontal()
{
    if ( !formWindow() )
	return;
    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 )
	w = l.first();
    if ( w )
	formWindow()->layoutHorizontalContainer( w );
}

void MainWindow::editLayoutContainerGrid()
{
    if ( !formWindow() )
	return;
    QWidget *w = formWindow()->mainContainer();
    QWidgetList l( formWindow()->selectedWidgets() );
    if ( l.count() == 1 )
	w = l.first();
    if ( w )
	formWindow()->layoutGridContainer( w  );
}

void MainWindow::editBreakLayout()
{
    if ( !formWindow() || !breakLayout )
	return;
    QWidget *w = formWindow()->mainContainer();
    if ( formWindow()->currentWidget() )
	w = formWindow()->currentWidget();
    if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
	 w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout ) {
	formWindow()->breakLayout( w );
	return;
    } else {
	QWidgetList widgets = formWindow()->selectedWidgets();
	for ( w = widgets.first(); w; w = widgets.next() ) {
	    if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
		 w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout )
		break;
	}
	if ( w ) {
	    formWindow()->breakLayout( w );
	    return;
	}
    }

    w = formWindow()->mainContainer();
    if ( WidgetFactory::layoutType( w ) != WidgetFactory::NoLayout ||
	 w->parentWidget() && WidgetFactory::layoutType( w->parentWidget() ) != WidgetFactory::NoLayout )
	formWindow()->breakLayout( w );
}

void MainWindow::editAccels()
{
    if ( !formWindow() )
	return;
    formWindow()->checkAccels();
}

void MainWindow::editSlots()
{
    if ( !formWindow() )
	return;

    statusBar()->message( tr( "Edit slots of current form..." ) );
    EditSlots dlg( this, formWindow() );
    dlg.exec();
    statusBar()->clear();
}

void MainWindow::editConnections()
{
    if ( !formWindow() )
	return;

    statusBar()->message( tr( "Edit connections in current form..." ) );
    ConnectionViewer dlg( this, formWindow() );
    dlg.exec();
    statusBar()->clear();
}

void MainWindow::editSource( bool /*resetSame*/ )
{
    if ( !formWindow() )
	return;
    SourceEditor *editor = 0;
    QString lang = currentProject->language();
    if ( !MetaDataBase::hasEditor( lang ) ) {
	QMessageBox::information( this, tr( "Edit Source" ),
				  tr( "There is no editor plugin to edit " + lang + " code installed" ) );
	return;
    }
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->language() == lang ) {
	    editor = e;
	    break;
	}
    }
    if ( !editor ) {
	EditorInterface *eIface = 0;
	editorPluginManager->queryInterface( lang, (QUnknownInterface**)&eIface );
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
}

void MainWindow::editSource( SourceFile *f )
{
    SourceEditor *editor = 0;
    QString lang = currentProject->language();
    if ( !MetaDataBase::hasEditor( lang ) ) {
	QMessageBox::information( this, tr( "Edit Source" ),
				  tr( "There is no editor plugin to edit " + lang + " code installed" ) );
	return;
    }
    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() ) {
	if ( e->language() == lang ) {
	    editor = e;
	    break;
	}
    }
    if ( !editor ) {
	EditorInterface *eIface = 0;
	editorPluginManager->queryInterface( lang, (QUnknownInterface**)&eIface );
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
    if ( editor->object() != f )
	editor->setObject( f, currentProject );
}

void MainWindow::editFormSettings()
{
    if ( !formWindow() )
	return;

    statusBar()->message( tr( "Edit settings of current form..." ) );
    FormSettings dlg( this, formWindow() );
    dlg.exec();
    statusBar()->clear();
}

class SenderObject : public QObject
{
    Q_OBJECT

public:
    SenderObject( QUnknownInterface *i ) : iface( i ) { iface->addRef(); }
    ~SenderObject() { iface->release(); }

public slots:
    void emitInitSignal() { emit initSignal( iface ); }
    void emitAcceptSignal() { emit acceptSignal( iface ); }

signals:
    void initSignal( QUnknownInterface * );
    void acceptSignal( QUnknownInterface * );

private:
    QUnknownInterface *iface;

};


void MainWindow::editProjectSettings()
{
    ProjectSettings dia( currentProject, this, 0, TRUE );

    SenderObject *senderObject = new SenderObject( designerInterface() );
    QValueList<Tab>::Iterator it;
    for ( it = projectTabs.begin(); it != projectTabs.end(); ++it ) {
	Tab t = *it;
	// #### take something else than t.title to get the language, support a default to add tab to each project setting dialog
	if ( t.title != currentProject->language() )
	    continue;
	dia.tabWidget->addTab( t.w, t.title );
	if ( t.receiver ) {
	    connect( dia.buttonOk, SIGNAL( clicked() ), senderObject, SLOT( emitAcceptSignal() ) );
	    connect( senderObject, SIGNAL( acceptSignal( QUnknownInterface * ) ), t.receiver, t.accept_slot );
	    connect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	    senderObject->emitInitSignal();
	    disconnect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	}
    }

    dia.exec();

    delete senderObject;
    for ( it = projectTabs.begin(); it != projectTabs.end(); ++it ) {
	Tab t = *it;
	dia.tabWidget->removePage( t.w );
	t.w->reparent( 0, QPoint(0,0), FALSE );
    }

    formList->setProject( currentProject );
}

void MainWindow::editPixmapCollection()
{
    PixmapCollectionEditor dia( this, 0, TRUE );
    dia.setProject( currentProject );
    dia.exec();
}

void MainWindow::editDatabaseConnections()
{
#ifndef QT_NO_SQL
    DatabaseConnectionsEditor dia( currentProject, this, 0, TRUE );
    dia.exec();
#endif
}

void MainWindow::editPreferences()
{
    statusBar()->message( tr( "Edit preferences..." ) );
    Preferences *dia = new Preferences( this, 0, TRUE );
    prefDia = dia;
    connect( dia->helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    dia->buttonColor->setEditor( StyledButton::ColorEditor );
    dia->buttonPixmap->setEditor( StyledButton::PixmapEditor );
    dia->checkBoxShowGrid->setChecked( sGrid );
    dia->checkBoxGrid->setChecked( snGrid );
    dia->spinGridX->setValue( grid().x() );
    dia->spinGridY->setValue( grid().y() );
    dia->checkBoxWorkspace->setChecked( restoreConfig );
    dia->checkBoxBigIcons->setChecked( usesBigPixmaps() );
    dia->checkBoxBigIcons->hide(); // ##### disabled for now
    dia->checkBoxTextLabels->setChecked( usesTextLabel() );
    dia->buttonColor->setColor( workspace->backgroundColor() );
    if ( workspace->backgroundPixmap() )
	dia->buttonPixmap->setPixmap( *workspace->backgroundPixmap() );
    if ( backPix )
	dia->radioPixmap->setChecked( TRUE );
    else
	dia->radioColor->setChecked( TRUE );
    dia->checkBoxSplash->setChecked( splashScreen );
    dia->editDocPath->setText( docPath );
    dia->checkAutoEdit->setChecked( !databaseAutoEdit );
    connect( dia->buttonDocPath, SIGNAL( clicked() ),
	     this, SLOT( chooseDocPath() ) );

    SenderObject *senderObject = new SenderObject( designerInterface() );
    QValueList<Tab>::Iterator it;
    for ( it = preferenceTabs.begin(); it != preferenceTabs.end(); ++it ) {
	Tab t = *it;
	dia->tabWidget->addTab( t.w, t.title );
	if ( t.receiver ) {
	    connect( dia->buttonOk, SIGNAL( clicked() ), senderObject, SLOT( emitAcceptSignal() ) );
	    connect( senderObject, SIGNAL( acceptSignal( QUnknownInterface * ) ), t.receiver, t.accept_slot );
	    connect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	    senderObject->emitInitSignal();
	    disconnect( senderObject, SIGNAL( initSignal( QUnknownInterface * ) ), t.receiver, t.init_slot );
	}
    }

    if ( dia->exec() == QDialog::Accepted ) {
	setSnapGrid( dia->checkBoxGrid->isChecked() );
	setShowGrid( dia->checkBoxShowGrid->isChecked() );
	setGrid( QPoint( dia->spinGridX->value(),
			 dia->spinGridY->value() ) );
	restoreConfig = dia->checkBoxWorkspace->isChecked();
	setUsesBigPixmaps( FALSE /*dia->checkBoxBigIcons->isChecked()*/ ); // ### disable for now
	setUsesTextLabel( dia->checkBoxTextLabels->isChecked() );
	if ( dia->radioPixmap->isChecked() && dia->buttonPixmap->pixmap() ) {
	    workspace->setBackgroundPixmap( *dia->buttonPixmap->pixmap() );
	    backPix = TRUE;
	} else {
	    workspace->setBackgroundColor( dia->buttonColor->color() );
	    backPix = FALSE;
	}
	splashScreen = dia->checkBoxSplash->isChecked();
	docPath = dia->editDocPath->text();
	databaseAutoEdit = !dia->checkAutoEdit->isChecked();
    }
    delete senderObject;
    for ( it = preferenceTabs.begin(); it != preferenceTabs.end(); ++it ) {
	Tab t = *it;
	dia->tabWidget->removePage( t.w );
	t.w->reparent( 0, QPoint(0,0), FALSE );
    }

    for ( SourceEditor *e = sourceEditors.first(); e; e = sourceEditors.next() )
	e->configChanged();

    delete dia;
    prefDia = 0;
    statusBar()->clear();
}

void MainWindow::searchFind()
{
    if ( !workSpace()->activeWindow() ||
	 !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	 return;

    if ( !findDialog )
	findDialog = new FindDialog( this, 0, FALSE );
    findDialog->show();
    findDialog->raise();
    findDialog->setEditor( ( (SourceEditor*)workSpace()->activeWindow() )->editorInterface(),
			   ( (SourceEditor*)workSpace()->activeWindow() )->object() );
    findDialog->comboFind->setFocus();
    findDialog->comboFind->lineEdit()->selectAll();
}

void MainWindow::searchIncremetalFindMenu()
{
    incrementalSearch->selectAll();
    incrementalSearch->setFocus();
}

void MainWindow::searchIncremetalFind()
{
    if ( !workSpace()->activeWindow() ||
	 !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	 return;

    ( (SourceEditor*)workSpace()->activeWindow() )->editorInterface()->find( incrementalSearch->text(),
									     FALSE, FALSE, TRUE, FALSE );
}

void MainWindow::searchIncremetalFindNext()
{
    if ( !workSpace()->activeWindow() ||
	 !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	 return;

    ( (SourceEditor*)workSpace()->activeWindow() )->editorInterface()->find( incrementalSearch->text(),
									     FALSE, FALSE, TRUE, TRUE );
}

void MainWindow::searchReplace()
{
    if ( !workSpace()->activeWindow() ||
	 !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	 return;

    if ( !replaceDialog )
	replaceDialog = new ReplaceDialog( this, 0, FALSE );
    replaceDialog->show();
    replaceDialog->raise();
    replaceDialog->setEditor( ( (SourceEditor*)workSpace()->activeWindow() )->editorInterface(),
			   ( (SourceEditor*)workSpace()->activeWindow() )->object() );
    replaceDialog->comboFind->setFocus();
    replaceDialog->comboFind->lineEdit()->selectAll();
}

void MainWindow::searchGotoLine()
{
    if ( !workSpace()->activeWindow() ||
	 !workSpace()->activeWindow()->inherits( "SourceEditor" ) )
	 return;

    if ( !gotoLineDialog )
	gotoLineDialog = new GotoLineDialog( this, 0, FALSE );
    gotoLineDialog->show();
    gotoLineDialog->raise();
    gotoLineDialog->setEditor( ( (SourceEditor*)workSpace()->activeWindow() )->editorInterface() );
    gotoLineDialog->spinLine->setFocus();
    gotoLineDialog->spinLine->setMaxValue( ( (SourceEditor*)workSpace()->activeWindow() )->numLines() - 1 );
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
		programPluginManager->queryInterface( lang, (QUnknownInterface**)&piface);
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
		    QList<SourceFile> sources = currentProject->sourceFiles();
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
	interpreterPluginManager->queryInterface( lang, (QUnknownInterface**)&iiface );
	if ( iiface ) {
	    iiface->onShowDebugStep( this, SLOT( showDebugStep( QObject *, int ) ) );
	    iiface->onShowError( this, SLOT( showErrorMessage( QObject *, int, const QString & ) ) );
	    iiface->onFinish( this, SLOT( finishedRun() ) );
	}

	LanguageInterface *liface = MetaDataBase::languageInterface( lang );
	if ( liface && liface->supports( LanguageInterface::AdditionalFiles ) ) {
	    QList<SourceFile> sources = currentProject->sourceFiles();
	    for ( SourceFile *f = sources.first(); f; f = sources.next() ) {
		iiface->exec( f, f->text() );
	    }
	}
    }

    QObjectList *l = new QObjectList;
    if ( iiface ) {
	QList<FormWindow> frms = currentProject->forms();
	for ( FormWindow *fw = frms.first(); fw; fw = frms.next() ) {
	    QValueList<int> bps = MetaDataBase::breakPoints( fw );
	    if ( !bps.isEmpty() )
		iiface->setBreakPoints( fw, bps );
	}

	QList<SourceFile> files = currentProject->sourceFiles();
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
	if ( style )
	    w->setStyle( style );
	if ( palet )
	    w->setPalette( *palet );

	QObjectList *l = w->queryList( "QWidget" );
	for ( QObject *o = l->first(); o; o = l->next() ) {
	    if ( style )
		( (QWidget*)o )->setStyle( style );
	    if ( palet )
		( (QWidget*)o )->setPalette( *palet );
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

void MainWindow::toolsCustomWidget()
{
    statusBar()->message( tr( "Edit custom widgets..." ) );
    CustomWidgetEditor edit( this, this );
    edit.exec();
    rebuildCustomWidgetGUI();
    statusBar()->clear();
}

void MainWindow::helpContents()
{
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
		propertyDocumentation[ s.mid( from + 1, to - from - 1 ) ] = s.mid( to + 2 );
	    }
	    f.close();
	} else {
	    QMessageBox::critical( this, tr( "Error" ), tr( "Couldn't find the Qt documentation property index file!\n"
					    "Define the correct documentation path in the preferences dialog." ) );
	}
    }

    if ( propertyEditor->widget() ) {
	if ( workspace->activeWindow() == propertyEditor && !propertyEditor->currentProperty().isEmpty() ) {
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
		    source = propertyDocumentation[s];
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
	if ( isAToolBarChild( o )  && currentTool() != CONNECT_TOOL )
	    break;
	if ( lastPressWidget != (QWidget*)o ||
	     ( !( w = isAFormWindowChild( o ) ) || o->inherits( "SizeHandle" ) || o->inherits( "OrderIndicator" ) ) )
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
	return openEditor( ( (FormWindow*)w )->designerWidget( o ), (FormWindow*)w );
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

    QList<SourceEditor> waitingForDelete;
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

    if ( currentTool() == ORDER_TOOL )
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

void MainWindow::popupFormWindoMenu( const QPoint & gp, FormWindow *fw )
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

        ids << ( id = rmbFormWindow->insertItem( tr("Edit Pages"), -1, 0 ) );
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

    QList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();
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
				       tr( "Save changes of form '%1'?" ).arg( fw->name() ),
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

    editSource();

    return TRUE;
}

void MainWindow::rebuildCustomWidgetGUI()
{
    customWidgetToolBar->clear();
    customWidgetMenu->clear();
    int count = 0;
    QList<MetaDataBase::CustomWidget> *lst = MetaDataBase::customWidgets();

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

void MainWindow::chooseDocPath()
{
    if ( !prefDia )
	return;
    QString fn = QFileDialog::getExistingDirectory( QString::null, this );
    if ( !fn.isEmpty() )
	prefDia->editDocPath->setText( fn );
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

void MainWindow::createNewTemplate()
{
    CreateTemplate *dia = (CreateTemplate*)sender()->parent();
    QString fn = dia->editName->text();
    QString cn = dia->listClass->currentText();
    if ( fn.isEmpty() || cn.isEmpty() ) {
	QMessageBox::information( this, tr( "Create Template" ), tr( "Couldn't create the template" ) );
	return;
    }
    fn.prepend( QString( getenv( "QTDIR" ) ) + "/tools/designer/templates/" );
    fn.append( ".ui" );
    QFile f( fn );
    if ( !f.open( IO_WriteOnly ) ) {
	QMessageBox::information( this, tr( "Create Template" ), tr( "Couldn't create the template" ) );
	return;
    }
    QTextStream ts( &f );

    ts << "<!DOCTYPE UI><UI>" << endl;
    ts << "<widget>" << endl;
    ts << "<class>" << cn << "</class>" << endl;
    ts << "<property stdset=\"1\">" << endl;
    ts << "    <name>name</name>" << endl;
    ts << "    <cstring>" << cn << "Form</cstring>" << endl;
    ts << "</property>" << endl;
    ts << "<property stdset=\"1\">" << endl;
    ts << "    <name>geometry</name>" << endl;
    ts << "    <rect>" << endl;
    ts << "        <width>300</width>" << endl;
    ts << "        <height>400</height>" << endl;
    ts << "    </rect>" << endl;
    ts << "</property>" << endl;
    ts << "</widget>" << endl;
    ts << "</UI>" << endl;

    dia->editName->setText( tr( "NewTemplate" ) );

    f.close();
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
    actionPluginManager = new QInterfaceManager<ActionInterface>( IID_ActionInterface, pluginDir );
    QStringList paths(QApplication::libraryPaths());
    QStringList::Iterator it = paths.begin();
    while (it != paths.end()) {
	actionPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    QStringList lst = actionPluginManager->featureList();
    for ( QStringList::Iterator ait = lst.begin(); ait != lst.end(); ++ait ) {
	ActionInterface *iface = 0;
	actionPluginManager->queryInterface( *ait, (QUnknownInterface**)&iface );
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
	editorPluginManager->queryInterface( lang, (QUnknownInterface**)&eIface );
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

QList<DesignerProject> MainWindow::projectList() const
{
    QList<DesignerProject> list;
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
    if ( id != -1 )
	openFile( *recentlyFiles.at( id ) );
}

void MainWindow::recentlyProjectsMenuActivated( int id )
{
    if ( id != -1 ) {
	openProject( *recentlyProjects.at( id ) );
    }
}

void MainWindow::addRecentlyOpened( const QString &fn, QStringList &lst )
{
    if ( lst.find( fn ) != lst.end() )
	return;
    if ( lst.count() >= 10 )
	lst.remove( lst.begin() );
    lst << fn;
}

TemplateWizardInterface * MainWindow::templateWizardInterface( const QString& className )
{
    TemplateWizardInterface* iface = 0;
    templateWizardPluginManager->queryInterface( className, (QUnknownInterface**)& iface );
    return iface;
}

void MainWindow::setupPluginManagers()
{
    editorPluginManager = new QInterfaceManager<EditorInterface>( IID_EditorInterface, pluginDir );
    QStringList paths(QApplication::libraryPaths());
    QStringList::Iterator it = paths.begin();
    while (it != paths.end()) {
	editorPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    MetaDataBase::setEditor( editorPluginManager->featureList() );
    templateWizardPluginManager = new QInterfaceManager<TemplateWizardInterface>( IID_TemplateWizardInterface, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	templateWizardPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    MetaDataBase::setupInterfaceManagers();
    programPluginManager = new QInterfaceManager<ProgramInterface>( IID_ProgramInterface, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	programPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    interpreterPluginManager = new QInterfaceManager<InterpreterInterface>( IID_InterpreterInterface, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	interpreterPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    preferencePluginManager = new QInterfaceManager<PreferenceInterface>( IID_PreferenceInterface, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	preferencePluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    projectSettingsPluginManager = new QInterfaceManager<ProjectSettingsInterface>( IID_ProjectSettingsInterface, pluginDir );
    it = paths.begin();
    while (it != paths.end()) {
	projectSettingsPluginManager->addLibraryPath(*it + "/designer");
	it++;
    }

    if ( preferencePluginManager ) {
	QStringList lst = preferencePluginManager->featureList();
	for ( it = lst.begin(); it != lst.end(); ++it ) {
	    PreferenceInterface *i = 0;
	    preferencePluginManager->queryInterface( *it, (QUnknownInterface**)&i );
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
	    projectSettingsPluginManager->queryInterface( *it, (QUnknownInterface**)&i );
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
    editorPluginManager->queryInterface( lang, (QUnknownInterface**)& eiface );
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
	QList<SourceFile> sources = currentProject->sourceFiles();
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
	interpreterPluginManager->queryInterface( lang, (QUnknownInterface**)&iiface );
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

#include "mainwindow.moc"
