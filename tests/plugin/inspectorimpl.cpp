#include "inspectorimpl.h"

#include <qfiledialog.h>
#include <qheader.h>
#include <qcomponentinterface.h>
#include <qlineedit.h>
#include <qprogressdialog.h>
#include <qapplication.h>
#include <qtextview.h>
#include <qregexp.h>
#include "../../tools/designer/plugins/designerinterface.h"

LibraryInspector::LibraryInspector( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : LibraryInspectorBase( parent, name, modal, fl | WGroupLeader )
{
    libDict.setAutoDelete( TRUE );
    path = "";
}

LibraryInspector::~LibraryInspector()
{
    // no need to delete child widgets, Qt does it all for us
}

void LibraryInspector::showLibrary( QListViewItem *item )
{
    if ( !item )
	return;

    QString intID = item->text( 0 );
    QListViewItem *pitem = item;
    while ( (pitem = item->parent() ) )
	item = pitem;

    QLibrary* plugin = libDict[item->text( 0 )];
    if ( !plugin ) {
	details->setText( "<Can't find plugin!>" );
	return;
    }

    QString text( "<table>" );

    if ( item->text( 0 ) == intID ) {
	QFile lib( plugin->library() );
	text += QString("<tr><td><b>Size:</b></td><td>%1</td></tr>").arg( lib.size() );
	text += "</table>";

	details->setText( text );
	return;
    }

    QRegExp intMatch( "*"+intID+"*" );

    QUnknownInterface *iface = (QUnknownInterface*)plugin->queryInterface( "*"+intID+"*" );
    if ( iface ) {
	QRegExp regexp( "*QComponentInterface*", TRUE, TRUE );
	if ( regexp.match( iface->interfaceId() ) ) {
	    QComponentInterface *piface = (QComponentInterface*)iface;
	    text += QString("<tr><td><b>Name:</b></td><td>%1</td></tr>").arg( piface->brief() );
	    text += QString("<tr><td><b>Description:</b></td><td>%1</td></tr>").arg( piface->description() );
	    text += QString("<tr><td><b>Author:</b></td><td>%1</td></tr>").arg( piface->author() );
	    text += QString("<tr><td><b>Version:</b></td><td>%1</td></tr>").arg( piface->version() );
	} else if ( intMatch.match( "WidgetInterface" ) ) {
	    text += QString("<tr><td><b>Widgets</b></td><td>%1</td></tr>").arg( ((WidgetInterface*)iface)->featureList().join("<br>") );
	} else if ( intMatch.match( "ActionInterface" ) ) {
	    text += QString("<tr><td><b>Actions</b></td><td>%1</td></tr>").arg( ((ActionInterface*)iface)->featureList().join("<br>") );
	} else if ( intMatch.match( "FilterInterface" ) ) {
	    text += QString("<tr><td><b>Filters</b></td><td>%1</td></tr>").arg( ((FilterInterface*)iface)->featureList().join("<br>") );
	}
    }
    text += QString("<tr><td><b>Debug info:</b></td><td>");
    if ( iface ) {
	text += QString("%1</td></tr>").arg( iface->name() );
	iface->release();
    } else {
	text += QString("%1</td></tr>").arg( "<i>Can't create interface</i>" );
    }
    text += "</table>";
    details->setText( text );
}

static QString demangle( const QString& id )
{
    int last = id.findRev( '/' );
    return ( last == -1 ) ? id : id.right( id.length() - last - 1 );
}

/*
  Adds the interfaces of \a iface to the list.
*/
void LibraryInspector::addInterface( QListViewItem *parent, QUnknownInterface *iface )
{
    if ( !iface )
	return;

    QString ID = iface->Id();

    QListViewItem *item = new QListViewItem( parent, iface->Id(), iface->interfaceId() );

    QStringList ifaces = iface->interfaceList( FALSE );
    for ( QStringList::Iterator it = ifaces.begin(); it != ifaces.end(); ++it ) {
	if ( *it == iface->interfaceId() )
	    continue;
	QUnknownInterface *sub = iface->queryInterface( *it, FALSE );
	if ( sub ) {
	    addInterface( item, sub );
	} else {
	    new QListViewItem( item, demangle( *it ), *it );
	}
    }
    iface->release();
    return;
}

/*
  Selects a library path
*/
void LibraryInspector::selectPath()
{
    QString p = pathEdit->text();
    if ( p == path )
	p = QFileDialog::getExistingDirectory( pathEdit->text() );
    if ( p != path ) {
	libDict.clear();
	view->clear();
	details->setText( QString::null );
	itemDict.clear();

	pathEdit->setText( p );
	path = pathEdit->text();
	if ( !QDir( p ).exists( ".", TRUE ) )
	    return;

	QStringList plugins = QDir(p).entryList( "*.dll;;*.so" );
	QProgressDialog progress( "Loading libraries...", "&Abort", plugins.count(), this );
	int file = 0;

	for ( QStringList::Iterator it = plugins.begin(); it != plugins.end(); ++it ) {
	    file ++;
	    progress.setProgress( file );
	    qApp->processEvents();
	    if ( progress.wasCancelled() )
		break;

	    QString lib = p + "/" + *it;
	    QLibrary *plugin = new QLibrary( lib );
	    libDict.insert( lib, plugin );

	    QListViewItem *libItem = new QListViewItem( view, plugin->library() );
	    QComponentInterface *iface = (QComponentInterface*)plugin->queryInterface( "*QComponentInterface*", FALSE );
	    addInterface( libItem, iface );
	}
    }
}
