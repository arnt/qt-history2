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
#include <qvariant.h>

PlugMainWindow::PlugMainWindow( QWidget* parent, const char* name, WFlags f )
: QMainWindow( parent, name, f )
{
    QWidgetFactory::installWidgetFactory( new QWidgetFactory );
    QWidgetFactory::installWidgetFactory( widgetManager = new QWidgetPlugInManager );
    QActionFactory::installActionFactory( actionManager = new QActionPlugInManager );

    QPopupMenu* file = (QPopupMenu*)QWidgetFactory::create( "QPopupMenu", this );
    if ( file ) {
	file->insertItem( "&Add", this, SLOT( fileOpen() ) );
	file->insertItem( "&Remove", this, SLOT( fileClose() ) );
	file->insertSeparator();
	file->insertItem( "&Exit", qApp, SLOT( quit() ) );
	menuBar()->insertItem( "&PlugIn", file );
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
	menuIDs.insert( widgetMenu->insertItem( wl[w] ), wl[w] );
}

void PlugMainWindow::fileOpen()
{
    QString file = QFileDialog::getOpenFileName( QString::null, "PlugIn (*.dll;*.so)", this, 0, "Select plugin" );

    if ( file.isEmpty() )
	return;

    // creating all available widgets and adding them to the little test scenario
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
	QString ifc = it.current()->queryPlugInInterface();
	if ( ifc == "QWidgetInterface" ) {
	    QListViewItem* item = new QListViewItem( wplugins, p->name(), p->description(), p->author(), p->library() );
	    QStringList wl = it.current()->featureList();
	    for ( uint i = 0; i < wl.count(); i++ )
		new QListViewItem( item, wl[i] );
	} else if ( ifc == "QActionInterface" ) {
	    QListViewItem* item = new QListViewItem( aplugins, p->name(), p->description(), p->author(), p->library() );
	    QStringList al = it.current()->featureList();
	    for ( uint i = 0; i < al.count(); i++ )
		new QListViewItem( item, al[i] );
	} else {
	    qDebug("Bogus plugin!");
	}
	++it;
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

void PlugMainWindowInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    ((PlugMainWindow*)qApp->mainWidget())->setProperty( p, v );
}

void PlugMainWindowInterface::requestProperty( const QCString& p, QVariant& v )
{
    if ( p == "usesTextLabel" ) {
	v = ((PlugMainWindow*)qApp->mainWidget())->property( p );
    } else if ( p == "mainWindow" ) {
	v = QVariant( (uint)qApp->mainWidget() );
    }
}
