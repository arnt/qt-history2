#include "inspectorimpl.h"

#include <qfiledialog.h>
#include <qheader.h>
#include <qcomponentinterface.h>
#include <qlineedit.h>
#include <qprogressdialog.h>
#include <qapplication.h>
#include <qtextview.h>
#include <qregexp.h>

/* 
 *  Constructs a LibraryInspector which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
LibraryInspector::LibraryInspector( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : LibraryInspectorBase( parent, name, modal, fl )
{
    libDict.setAutoDelete( TRUE );
    path = "";
}

/*  
 *  Destroys the object and frees any allocated resources
 */
LibraryInspector::~LibraryInspector()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
  Shows the contents of the library
*/
void LibraryInspector::showLibrary( QListViewItem *item )
{
    if ( !item )
	return;

    QString intID = item->text( 0 );
    QListViewItem *pitem = item;
    while ( (pitem = item->parent() ) )
	item = pitem;

    QPlugIn* plugin = libDict[item->text( 0 )];
    if ( !plugin ) {
	details->setText( "<Can't find plugin!" );
	return;
    }

    QString text;

    if ( item->text( 0 ) == intID ) {
	QFile lib( plugin->library() );
	text = "<table>";
	text += QString("<tr><td><b>Size:</b></td><td>%1</td></tr>").arg( lib.size() );
	text += "</table>";

	details->setText( text );
	return;
    }
    QUnknownInterface *iface = (QUnknownInterface*)plugin->queryInterface( intID );
    if ( !iface ) {
	details->setText( "<No interface found>" );
	return;
    }

    text = "<table>";
    if ( intID == "QPlugInInterface" ) {
	QPlugInInterface *piface = (QPlugInInterface*)iface;
	text += QString("<tr><td><b>Name:</b></td><td>%1</td></tr>").arg( piface->name() );
	text += QString("<tr><td><b>Description:</b></td><td>%1</td></tr>").arg( piface->description() );
	text += QString("<tr><td><b>Author:</b></td><td>%1</td></tr>").arg( piface->author() );
	text += QString("<tr><td><b>Version:</b></td><td>%1</td></tr>").arg( piface->version() );
    }
    text += QString("<tr><td><b>Interface:</b></td><td>%1</td></tr>").arg(iface->name());
    text += "</table>";
    details->setText( text );

    iface->release();
}

/*
  Adds the interfaces of \a iface to the list.
*/
void LibraryInspector::addInterface( QListViewItem *parent, QUnknownInterface *iface )
{
    if ( !iface )
	return;

    QString intID = iface->interfaceID();
    int dollar  =intID.findRev( '$' );
    QString hier = ( dollar == -1 ) ? intID : intID.left( dollar );
    QString unique = ( dollar == -1 ) ? "" : intID.right( intID.length() - dollar - 1 );
    int lastSlash = hier.findRev( '/' );
    intID = ( lastSlash == -1 ) ? hier : hier.right( hier.length() - lastSlash - 1 );

    QListViewItem *item = new QListViewItem( parent, intID );
    QStringList ifaces = iface->interfaceList( FALSE );
    for ( QStringList::Iterator it = ifaces.begin(); it != ifaces.end(); ++it ) {
	if ( *it == iface->interfaceID() )
	    continue;
	QUnknownInterface *sub = iface->queryInterface( *it, FALSE );
	if ( sub )
	    addInterface( item, sub );
	else
	    new QListViewItem( item, *it );
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
	    QPlugIn *plugin = new QPlugIn( lib );
	    libDict.insert( lib, plugin );

	    QListViewItem *libItem = new QListViewItem( view, plugin->library() );
	    QPlugInInterface *iface = (QPlugInInterface*)plugin->queryInterface( "QPlugInInterface", FALSE );
	    addInterface( libItem, iface );
	}
    }
}
