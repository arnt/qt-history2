#include "plugmainwindow.h"
#include "qwidgetplugin.h"
#include "qactionplugin.h"

#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qfiledialog.h>
#include <qvbox.h>
#include <qtooltip.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qstatusbar.h>
#include <qaction.h>
#include <qlistview.h>

PlugMainWindow::PlugMainWindow( QWidget* parent, const char* name, WFlags f )
: QMainWindow( parent, name, f )
{
    QWidgetFactory::installWidgetFactory( new QWidgetFactory );
    QWidgetFactory::installWidgetFactory( widgetManager = new QWidgetPlugInManager );
    QActionFactory::installActionFactory( actionManager = new QActionPlugInManager );

    QPopupMenu* file = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    QToolBar* ft = (QToolBar*)QWidgetFactory::create( "QToolBar", this );
    ft->setName( "File" );

    QAction* a;
    a = new QAction( "Add plugin", QIconSet(), "&Add", CTRL+Key_A, this );
    connect( a, SIGNAL(activated()), this, SLOT(fileOpen()));
    a->addTo( file );
    a->addTo( ft );
    a = new QAction( "Remove plugin", QIconSet(), "&Remove", CTRL+Key_R, this );
    connect( a, SIGNAL(activated()), this, SLOT(fileClose()));
    a->addTo( file );
    a->addTo( ft );
    file->insertSeparator();
    a = new QAction( "Exit", QIconSet(), "E&xit", CTRL+Key_X, this );
    a->addTo( file );

    widgetMenu = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    connect( widgetMenu, SIGNAL( activated(int) ), this, SLOT( runWidget(int) ) );

    pluginMenu = 0;
    pluginTool = 0;

    menuBar()->insertItem( "&File", file );
    menuBar()->insertItem( "&Widgets", widgetMenu );
    addToolBar( ft );

    statusBar();

    QStringList wl = QWidgetFactory::widgetList();
    for ( uint w = 0; w < wl.count(); w++ )
	menuIDs.insert( widgetMenu->insertItem( wl[w] ), wl[w] );
}

PlugMainWindow::~PlugMainWindow()
{
    QWidgetFactory::removeWidgetFactory( widgetManager );
    QActionFactory::removeActionFactory( actionManager );
    delete widgetManager;
    delete actionManager;
}

void PlugMainWindow::fileOpen()
{
    QString file = QFileDialog::getOpenFileName( QString::null, "PlugIn (*.dll;*.so)", this, 0, "Select plugin" );

    if ( file.isEmpty() )
	return;

    QPlugIn* plugin = 0;
    if ( ( plugin = widgetManager->addLibrary( file ) ) ) {
	statusBar()->message( tr("Widget-Plugin \"%1\" loaded").arg( plugin->name() ), 3000 );
	QStringList wl = plugin->featureList();
	for ( uint i = 0; i < wl.count(); i++ )
	    menuIDs.insert( widgetMenu->insertItem( wl[i] ), wl[i] );
    } else if ( ( plugin = actionManager->addLibrary( file ) ) ) {
	statusBar()->message( tr("Action-Plugin \"%1\" loaded").arg( plugin->name() ), 3000 );
	QStringList al = plugin->featureList();
	for ( uint a = 0; a < al.count(); a++ ) {
	    QAction* action = QActionFactory::create( al[a], this );
	    if ( !addAction( action ) ) {
		QMessageBox::information( this, "Error", tr("Couldn't create action\n%1").arg( al[a] ) );
	    }
	}
    } else {
	QMessageBox::information( this, "Error", tr("Couldn't load plugin\n%1").arg( file ) );
	return;
    }
}

void PlugMainWindow::fileClose()
{
    QDialog dialog( 0, 0, TRUE );
    QHBoxLayout hl( &dialog );
    hl.setSpacing( 6 );
    hl.setMargin( 10 );
    QListView _box( &dialog );
    _box.setRootIsDecorated( TRUE );
    QListViewItem* wplugins = new QListViewItem( &_box, "Widgets" );
    QListViewItem* aplugins = new QListViewItem( &_box, "Actions" );
    _box.addColumn( "Name" );
    _box.addColumn( "Description" );
    _box.addColumn( "Author" );
    _box.addColumn( "File" );

    QVBox v( &dialog );
    ((QVBoxLayout*)v.layout())->addStretch();
    hl.addWidget( &_box );
    hl.addWidget( &v );
    QPushButton ok( "&Ok", &v );
    connect( &ok, SIGNAL( clicked() ), &dialog, SLOT( accept() ) );

    QList<QPlugIn> pl;
    if ( widgetManager ) {
	pl = widgetManager->plugInList();
	QListIterator<QPlugIn> it( pl );
	while ( it.current() ) {
	    QPlugIn* p = it.current();
	    ++it;
	    QListViewItem* item = new QListViewItem( wplugins, p->name(), p->description(), p->author(), p->library() );
	    QStringList wl = p->featureList();
	    for ( uint i = 0; i < wl.count(); i++ )
		new QListViewItem( item, wl[i] );	    
	}
    }
    if ( actionManager ) {
	QList<QPlugIn> pl = actionManager->plugInList();
	QListIterator<QPlugIn> it( pl );
	while ( it.current() ) {
	    QPlugIn* p = it.current();	    
	    ++it;
	    QListViewItem* item = new QListViewItem( aplugins, p->name(), p->description(), p->author(), p->library() );
	    QStringList al = p->featureList();
	    for ( uint i = 0; i < al.count(); i++ )
		new QListViewItem( item, al[i] );
	}
    }

    if ( dialog.exec() && _box.currentItem() ) {
	QListViewItem* item = _box.currentItem();

	if ( !item )
	    return;
	if ( item->parent() != wplugins && item->parent() != aplugins )
	    if ( !(item = item->parent() ) )
		return;

	QString file = item->text( 3 );
	QString info;
	if ( item->parent() == wplugins ) {
	    QPlugIn* plugin = widgetManager->plugInFromFile( file );
	    if ( plugin ) {
		// Make sure to have a deep copy of the string, else it
		// would be corrupted with the unloading of the library
		// Also make sure the stringlist is deleted before the
		// plugin is unloaded for the same reason
		// (the data is part of the library)
		info = QString( "\"%1\"").arg( ((QPlugIn*)plugin)->name() );
		{
		    QStringList wl = plugin->featureList();
		    for ( uint i = 0; i < wl.count(); i++ ) {
			QString w = wl[i];
			QMapIterator<int,QString> it;
			for ( it = menuIDs.begin(); it != menuIDs.end(); ++it ) {
			    if ( it.data() == w )
				break;
			}
			if ( it != menuIDs.end() ) {
			    widgetMenu->removeItem( it.key() );
			    menuIDs.remove( it );
			}

		    }
		}
	    }
	    if ( !widgetManager->removeLibrary( file ) ) {
		QMessageBox::information( this, "Error", tr("Couldn't unload library\n%1").arg( file ) );
		return;
	    }
	} else if ( item->parent() == aplugins ) {
	    QPlugIn* plugin = actionManager->plugInFromFile( file );
	    if ( plugin )
		info = QString( "\"%1\"").arg( ((QPlugIn*)plugin)->name() );
	    if ( !actionManager->removeLibrary( file ) ) {
		QMessageBox::information( this, "Error", tr("Couldn't unload library\n%1").arg( file ) );
		return;
	    }
	} else {
	    qDebug( "Bogus plugin!" );
	    return;
	}
	statusBar()->message( tr("Plugin %1 unloaded").arg( info ), 3000 );
    }
}

void PlugMainWindow::runWidget( int id )
{
    if ( !menuIDs.contains(id) )
	return;
    QString wname = menuIDs[id];
    if ( centralWidget() )
	delete centralWidget();

    QWidget* w = QWidgetFactory::create( wname, this );
    if ( !w ) {
	QMessageBox::information( this, "Error", tr("Couldn't create widget\n%1").arg( wname ) );
	return;
    }
    setCentralWidget( w );
    QToolTip::add( w, QString("%1 ( %2 )").arg( wname ).arg( QWidgetFactory::widgetFactory( wname )->factoryName() ) );
}

bool PlugMainWindow::addAction( QAction* action )
{
    if ( !action )
	return FALSE;
    if ( !pluginMenu ) {
	pluginMenu = new QPopupMenu( this );
	menuBar()->insertItem( "&PlugIn", pluginMenu );	
    }
    if ( !pluginTool ) {
	pluginTool = new QToolBar( this );
	pluginTool->show();
	addToolBar( pluginTool, "PlugIns" );
    }
    action->addTo( pluginTool );
    action->addTo( pluginMenu );

    return TRUE;
}
