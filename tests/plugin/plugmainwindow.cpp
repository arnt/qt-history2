#include "plugmainwindow.h"
#include "qwidgetplugin.h"

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

PlugMainWindow::PlugMainWindow( QWidget* parent, const char* name, WFlags f )
: QMainWindow( parent, name, f ), menuIDs( 53 )
{
    menuIDs.setAutoDelete( TRUE );
    QWidgetFactory::installWidgetFactory( new QWidgetFactory );

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
    
    statusBar();

    sv = new QScrollView( this );
    box = new QHBox( sv->viewport() );
    box->setFixedHeight( 200 );
    sv->addChild( box );
    setCentralWidget( sv );
    
    manager = 0;

    QStringList wl = QWidgetFactory::widgetList();
    for ( uint i = 0; i < wl.count(); i++ )
	menuIDs.insert( wl[i], new int(widgetMenu->insertItem( wl[i] )) );

//    QActionFactory::installActionFactory( manager );
}

void PlugMainWindow::fileOpen()
{
    QString file = QFileDialog::getOpenFileName( QString::null, "PlugIn (*.dll)", this, 0, "Select plugin" );

    if ( file.isEmpty() )
	return;

    // creating all available widgets and adding them to the little test scenario
    QWidgetPlugIn* plugin = 0;
    if ( !manager ) {
	manager = new QWidgetPlugInManager( QString::null, QPlugIn::OptimizeMemory );
	plugin = manager->addLibrary( file );
	if ( plugin ) {
	    QWidgetFactory::installWidgetFactory( manager );
	} else {
	    delete manager;
	    manager = 0;
	}
    } else {
	plugin = manager->addLibrary( file );
    }

    if ( !plugin ) {
	QMessageBox::information( this, "Error", tr("Couldn't load plugin\n%1").arg( file ) );
	return;
    } else {
	statusBar()->message( tr("Plugin \"%1\" loaded").arg( ((QPlugIn*)plugin)->name() ), 3000 );
    }

    QStringList wl = plugin->widgets();
    for ( uint i = 0; i < wl.count(); i++ )
	menuIDs.insert( wl[i], new int(widgetMenu->insertItem( wl[i] )) );
/*	
	QWidget* w = QWidgetFactory::create( wl[i], box, wl[i] );
	if ( w ) {
	    QToolTip::add( w, QString("%1 ( %2 )").arg( w->className() ).arg( QWidgetFactory::widgetFactory( w->className() )->factoryName() ) );
	    w->show();
	}
    }*/
/*
    // creating a nice popupmenu and add all available actions
    QPopupMenu* pop = (QPopupMenu*) QWidgetFactory::create( "QPopupMenu", &mw );
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
    if ( !manager )
	return;

    QDialog dialog( 0, 0, TRUE );
    QHBoxLayout hl( &dialog );
    hl.setSpacing( 6 );
    hl.setMargin( 10 );
    QListView box( &dialog );
    box.setRootIsDecorated( TRUE );
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

    QList<QPlugIn> pl = manager->plugInList();
    QListIterator<QPlugIn> it( pl );
    while ( it.current() ) {
	QWidgetPlugIn* p = (QWidgetPlugIn*)pl.current();
	QListViewItem* item = new QListViewItem( &box, ((QPlugIn*)p)->name(), ((QPlugIn*)p)->description(), 
	    ((QPlugIn*)p)->author(), ((QPlugIn*)p)->library() );
	QStringList wl = p->widgets();
	for ( uint i = 0; i < wl.count(); i++ ) {
	    new QListViewItem( item, wl[i] );
	}
	++it;
    }    

    if ( dialog.exec() && box.currentItem() ) {
	QListViewItem* item = box.currentItem();
	while ( item->parent() ) 
	    item = item->parent();

	QString file = item->text( 3 );
	QWidgetPlugIn* plugin = (QWidgetPlugIn*)manager->plugInFromFile( file );
        QString info;
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
	if ( !manager->removeLibrary( file ) ) {
	    QMessageBox::information( this, "Error", tr("Couldn't unload library\n%1").arg( file ) );
	} else {
	    statusBar()->message( tr("Plugin %1 unloaded").arg( info ), 3000 );
	}
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
    setCentralWidget( w );
    QToolTip::add( w, QString("%1 ( %2 )").arg( w->className() ).arg( QWidgetFactory::widgetFactory( w->className() )->factoryName() ) );
}

void PlugMainWindow::runAction( int id )
{
}