#include "plugmainwindow.h"
#include "qwidgetplugin.h"
#include "qactionplugin.h"

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qhbox.h>
#include <qvbox.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qaction.h>

PlugMainWindow::PlugMainWindow( QWidget* parent, const char* name, WFlags f )
: QMainWindow( parent, name, f ), menuIDs( 53 )
{
    menuIDs.setAutoDelete( TRUE );
    QWidgetFactory::installWidgetFactory( new QWidgetFactory );
    QWidgetFactory::installWidgetFactory( widgetManager = new QWidgetPlugInManager );
    QActionFactory::installActionFactory( actionManager = new QActionPlugInManager );

    QPopupMenu* file = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    if ( file ) {
	file->insertItem( "&Add", this, SLOT( fileOpen() ) );
	file->insertItem( "&Remove", this, SLOT( fileClose() ) );
	menuBar()->insertItem( "&PlugIn", file );
    }
    actionMenu = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    if ( actionMenu ) {
	menuBar()->insertItem( "&Actions", actionMenu );
	connect( actionMenu, SIGNAL( activated(int) ), this, SLOT( runAction(int) ) );
    }
    widgetMenu = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    if ( widgetMenu ) {
	menuBar()->insertItem( "&Widgets", widgetMenu );
	connect( widgetMenu, SIGNAL( activated(int) ), this, SLOT( runWidget(int) ) );
    }

    pluginMenu = 0;
    pluginTool = 0;
    
    statusBar();

    sv = new QScrollView( this );
    box = new QHBox( sv->viewport() );
    box->setFixedHeight( 200 );
    sv->addChild( box );
    setCentralWidget( sv );
    
    QStringList wl = QWidgetFactory::widgetList();
    for ( uint w = 0; w < wl.count(); w++ )
	menuIDs.insert( wl[w], new int(widgetMenu->insertItem( wl[w] )) );
    QStringList al = QActionFactory::actionList();
    for ( uint a = 0; a < al.count(); a++ )
	menuIDs.insert( al[a], new int(widgetMenu->insertItem( al[a] )) );
}

void PlugMainWindow::fileOpen()
{
    QString file = QFileDialog::getOpenFileName( QString::null, "PlugIn (*.dll;*.so)", this, 0, "Select plugin" );

    if ( file.isEmpty() )
	return;

    // creating all available widgets and adding them to the little test scenario
    QPlugIn* plugin = 0;
    if ( plugin = widgetManager->addLibrary( file ) ) {
	statusBar()->message( tr("Widget-Plugin \"%1\" loaded").arg( plugin->name() ), 3000 );
	QStringList wl = ((QWidgetPlugIn*)plugin)->widgets();
	for ( uint i = 0; i < wl.count(); i++ )
	    menuIDs.insert( wl[i], new int(widgetMenu->insertItem( wl[i] )) );
    } else if ( plugin = actionManager->addLibrary( file ) ) {
	statusBar()->message( tr("Action-Plugin \"%1\" loaded").arg( plugin->name() ), 3000 );
	QStringList wl = ((QActionPlugIn*)plugin)->actions();
	for ( uint i = 0; i < wl.count(); i++ )
	    menuIDs.insert( wl[i], new int(actionMenu->insertItem( wl[i] )) );
    } else {
	QMessageBox::information( this, "Error", tr("Couldn't load plugin\n%1").arg( file ) );
	return;
    }
    // creating a nice popupmenu and add all available actions
/*    QPopupMenu* pop = (QPopupMenu*) QWidgetFactory::create( "QPopupMenu", &mw );
    QStringList actions = QActionFactory::actionList();
    for ( uint j = 0; j < actions.count(); j++ ) {
	bool self = TRUE;
	QAction* a = QActionFactory::create( actions[j], self, &mw );
	if ( a )
	    a->addTo( pop );
    }
*/
}

void PlugMainWindow::fileClose()
{
    QDialog dialog( 0, 0, TRUE );
    QHBoxLayout hl( &dialog );
    hl.setSpacing( 6 );
    hl.setMargin( 10 );
    QListView box( &dialog );
    box.setRootIsDecorated( TRUE );
    QListViewItem* wplugins = new QListViewItem( &box, "Widgets" );
    QListViewItem* aplugins = new QListViewItem( &box, "Actions" );
    box.addColumn( "Name" );
    box.addColumn( "Description" );
    box.addColumn( "Author" );
    box.addColumn( "File" );

    QVBox v( &dialog );
    ((QVBoxLayout*)v.layout())->addStretch();
    hl.addWidget( &box );
    hl.addWidget( &v );
    QPushButton ok( "&Ok", &v );
    connect( &ok, SIGNAL( clicked() ), &dialog, SLOT( accept() ) );

    QList<QPlugIn> pl;
    if ( widgetManager )
	pl = widgetManager->plugInList();
    if ( actionManager ) {
	QList<QPlugIn> al = actionManager->plugInList();
	QListIterator<QPlugIn> it( al );
	while ( it.current() ) {
	    pl.append( it.current() );
	    ++it;
	}
    }
//	pl.join( actionManager->plugInList() );

    QListIterator<QPlugIn> it( pl );
    while ( it.current() ) {
	QPlugIn* p = it.current();
	QString ifc = it.current()->queryInterface();
	if ( ifc == "QWidgetInterface" ) {
	    QListViewItem* item = new QListViewItem( wplugins, p->name(), p->description(), p->author(), p->library() );
	    QWidgetPlugIn* wp = (QWidgetPlugIn*)it.current();
	    QStringList wl = wp->widgets();
	    for ( uint i = 0; i < wl.count(); i++ )
		new QListViewItem( item, wl[i] );
	} else if ( ifc == "QActionInterface" ) {
	    QListViewItem* item = new QListViewItem( aplugins, p->name(), p->description(), p->author(), p->library() );
	    QActionPlugIn* ap = (QActionPlugIn*)it.current();
	    QStringList al = ap->actions();
	    for ( uint i = 0; i < al.count(); i++ )
		new QListViewItem( item, al[i] );
	} else {
	    qDebug("Bogus plugin!");
	}
	++it;
    }

    if ( dialog.exec() && box.currentItem() ) {
	QListViewItem* item = box.currentItem();

	if ( !item )
	    return;
	if ( item->parent() != wplugins && item->parent() != aplugins )
	    if ( !(item = item->parent() ) )
		return;

	QString file = item->text( 3 );
	QString info;
	if ( item->parent() == wplugins ) {
	    QWidgetPlugIn* plugin = (QWidgetPlugIn*)widgetManager->plugInFromFile( file );
	    if ( plugin ) {
		// Make sure to have a deep copy of the string, else it
		// would be corrupted with the unloading of the library
		// Also make sure the stringlist is deleted before the
		// plugin is unloaded for the same reason 
		// (the data is part of the library)
		info = QString( "\"%1\"").arg( ((QPlugIn*)plugin)->name() );
		{
		    QStringList wl = plugin->widgets();
		    for ( uint i = 0; i < wl.count(); i++ ) {
			QString w = wl[i];
			int* id = menuIDs[w];
			if ( id )
			    widgetMenu->removeItem( *id );
			menuIDs.remove( w );
		    }
		}
	    }
	    if ( !widgetManager->removeLibrary( file ) ) {
		QMessageBox::information( this, "Error", tr("Couldn't unload library\n%1").arg( file ) );
		return;
	    }
	} else if ( item->parent() == aplugins ) {
	    QActionPlugIn* plugin = (QActionPlugIn*)actionManager->plugInFromFile( file );
	    if ( plugin ) {
		info = QString( "\"%1\"").arg( ((QPlugIn*)plugin)->name() );
		{
		    QStringList wl = plugin->actions();
		    for ( uint i = 0; i < wl.count(); i++ ) {
			QString w = wl[i];
			int* id = menuIDs[w];
			if ( id )
			    actionMenu->removeItem( *id );
			menuIDs.remove( w );
		    }
		}
	    }
	    if ( !actionManager->removeLibrary( file ) ) {
		QMessageBox::information( this, "Error", tr("Couldn't unload library\n%1").arg( file ) );
		return;
	    }
	}
	statusBar()->message( tr("Plugin %1 unloaded").arg( info ), 3000 );
    }
}

void PlugMainWindow::runWidget( int id )
{
    QDictIterator<int> it( menuIDs );
    while ( it.current() ) {
	if ( *(it.current()) == id )
	    break;
	++it;
    }
    if ( centralWidget() )
	delete centralWidget();

    QWidget* w = QWidgetFactory::create( it.currentKey(), this );
    if ( !w ) {
	QMessageBox::information( this, "Error", tr("Couldn't create widget\n%1").arg( it.currentKey() ) );
	return;
    }
    setCentralWidget( w );
    QToolTip::add( w, QString("%1 ( %2 )").arg( w->className() ).arg( QWidgetFactory::widgetFactory( w->className() )->factoryName() ) );
}

void PlugMainWindow::runAction( int id )
{
    QDictIterator<int> it( menuIDs );
    while ( it.current() ) {
	if ( *(it.current()) == id )
	    break;
	++it;
    }

    bool self = TRUE;
    QAction* a = QActionFactory::create( it.currentKey(), self, this );
    if ( !a ) {
	QMessageBox::information( this, "Error", tr("Couldn't create action\n%1").arg( it.currentKey() ) );
	return;
    }
    if ( !self ) {
	if ( !pluginMenu ) {
	    pluginMenu = new QPopupMenu( this );
	    menuBar()->insertItem( "&PlugIn", pluginMenu );	    
	}
	if ( !pluginTool ) {
	    pluginTool = new QToolBar( this );
	    pluginTool->show();
	    addToolBar( pluginTool, "PlugIns" );
	}
	a->addTo( pluginTool );
	a->addTo( pluginMenu );
    }
}