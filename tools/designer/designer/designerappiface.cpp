#include "designerappiface.h"
#include "mainwindow.h"
#include "formwindow.h"
#include "formlist.h"
#include "propertyeditor.h"
#include "hierarchyview.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include <qstatusbar.h>

DesignerApplicationInterface::DesignerApplicationInterface()
    : QApplicationInterface( "DesignerApplicationInterface" )
{
    MainWindow* mw = (MainWindow*)qApp->mainWidget();
    FormList* fl = mw ? mw->formlist() : 0;
    if ( !fl )
	return;

    new DesignerMainWindowInterfaceImpl( mw, this );
    new DesignerFormListInterfaceImpl( fl, this );
    new DesignerConfigurationInterface( 0, this );
}

QString DesignerApplicationInterface::interfaceID() const
{ 
    return createID( QApplicationInterface::interfaceID(), "DesignerApplicationInterface" );
}

/*
 * DesignerMainWindowInterface
*/
DesignerMainWindowInterfaceImpl::DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent )
    : DesignerMainWindowInterface( mw, parent, "DesignerMainWindowInterfaceImpl" )
{
    PropertyEditor* pe = mw ? mw->propertyeditor() : 0;
    HierarchyView* hv = mw ? mw->objectHierarchy() : 0;

    new DesignerStatusBarInterfaceImpl( mw->statusBar(), this );
    new DesignerPropertyEditorInterface( pe, this );
    new DesignerHierarchyViewInterface( hv, this );
}

/*
 * DesignerStatusBarInterface
*/
DesignerStatusBarInterfaceImpl::DesignerStatusBarInterfaceImpl( QStatusBar *sb, QUnknownInterface* parent )
    : DesignerStatusBarInterface( sb, parent, "DesignerStatusBarInterfaceImpl" )
{
}

bool DesignerStatusBarInterfaceImpl::requestSetProperty( const QCString &p, const QVariant &v )
{
    if ( p == "message" ) {
	QStatusBar* sb = (QStatusBar*)component();
	if ( sb ) {
	    sb->message( v.toString(), 3000 );
	    return TRUE;
	} else {
	    return FALSE;
	}
    }
    return DesignerStatusBarInterface::requestSetProperty( p, v );
}

/*
 * DesignerFormListInterface
*/
DesignerFormListInterfaceImpl::DesignerFormListInterfaceImpl( FormList *fl, QUnknownInterface* parent )
    : DesignerFormListInterface( fl, parent, "DesignerFormListInterfaceImpl" )
{
    new DesignerActiveFormWindowInterfaceImpl( fl, this );
    listIterator = 0;
}

bool DesignerFormListInterfaceImpl::initialize( QApplicationInterface *app )
{
    delete listIterator;
    listIterator = new QListViewItemIterator( (FormList*)component() );

    return DesignerFormListInterface::initialize( app );
}

bool DesignerFormListInterfaceImpl::cleanUp( QApplicationInterface *app )
{
    delete listIterator;
    listIterator = 0;

    return DesignerFormListInterface::cleanUp( app );
}

QString DesignerFormListInterfaceImpl::text( DesignerFormWindowInterface *form, int col ) const
{
    QString formname = form->requestProperty( "name" ).toString();
    DesignerFormListInterfaceImpl* that = (DesignerFormListInterfaceImpl*)this;
    QListViewItemIterator it( (FormList*)that->component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname )
	    return item->text( col );
    }
    return QString::null;
}

void DesignerFormListInterfaceImpl::setText( DesignerFormWindowInterface *form, int col, const QString& s )
{
    QString formname = form->requestProperty( "name" ).toString();
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname ) {
	    item->setText( col, s );
	    return;
	}
    }
}

const QPixmap* DesignerFormListInterfaceImpl::pixmap( DesignerFormWindowInterface *form, int col ) const
{
    QString formname = form->requestProperty( "name" ).toString();
    DesignerFormListInterfaceImpl* that = (DesignerFormListInterfaceImpl*)this;
    QListViewItemIterator it( (FormList*)that->component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname )
	    return item->pixmap( col );
    }
    return 0;
}

void DesignerFormListInterfaceImpl::setPixmap( DesignerFormWindowInterface *form, int col, const QPixmap& pix )
{
    QString formname = form->requestProperty( "name" ).toString();
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname ) {
	    item->setPixmap( col, pix );
	    return;
	}
    }
}

DesignerFormWindowInterface *DesignerFormListInterfaceImpl::current()
{
    FormListItem *item = (FormListItem*)listIterator->current();
   
    return item ? new DesignerFormWindowInterfaceImpl( item->formWindow(), this ) : 0;
}

DesignerFormWindowInterface *DesignerFormListInterfaceImpl::next()
{
    ++(*listIterator);

    return current();
}

DesignerFormWindowInterface *DesignerFormListInterfaceImpl::prev()
{
    --(*listIterator);

    return current();
}

/*
 * DesignerFormWindowInterface
*/

DesignerFormWindowInterfaceImpl::DesignerFormWindowInterfaceImpl( FormWindow *fw, QUnknownInterface *parent, const char *name )
    : DesignerFormWindowInterface( fw, parent, name )
{
    MainWindow *mw = (MainWindow*)qApp->mainWidget();
    PropertyEditor *pe = mw->propertyeditor();

    new DesignerWidgetListInterfaceImpl( fw, this );
    new DesignerActiveWidgetInterfaceImpl( pe, this );
}

bool DesignerFormWindowInterfaceImpl::initialize(  QApplicationInterface * app )
{
    if ( component() )
	return DesignerFormWindowInterface::initialize( app );

    return FALSE;
}

/*
 * DesignerActiveFormWindowInterface
*/
DesignerActiveFormWindowInterfaceImpl::DesignerActiveFormWindowInterfaceImpl( FormList *fl, QUnknownInterface *parent, const char *name )
: DesignerFormWindowInterfaceImpl( 0, parent, name ), formList( fl ), lastForm( 0 )
{
}

bool DesignerActiveFormWindowInterfaceImpl::initialize( QApplicationInterface * app)
{
    // update the internal component object each time this interface is used
    reconnect();

    return DesignerFormWindowInterfaceImpl::initialize( app );
}

void DesignerActiveFormWindowInterfaceImpl::reconnect()
{
    FormListItem* fli = (FormListItem*)formList->currentItem();
    FormWindow *newForm = fli ? fli->formWindow() : 0;
    if ( newForm == lastForm )
	return;

    for ( QValueList<ConnectSignal>::Iterator cit1 = signalList.begin(); cit1 != signalList.end(); ++cit1 ) {
	if ( lastForm && (*cit1).target )
	    QObject::disconnect( lastForm, (*cit1).signal, (*cit1).target, (*cit1).slot );
	if ( newForm && (*cit1).target )
	    QObject::connect( lastForm, (*cit1).signal, (*cit1).target, (*cit1).slot );
    }
    for ( QValueList<ConnectSlot>::Iterator cit2 = slotList.begin(); cit2 != slotList.end(); ++cit2 ) {
	if ( lastForm && (*cit2).sender )
	    QObject::disconnect( (*cit2).sender, (*cit2).signal, lastForm, (*cit2).slot );
	if ( newForm && (*cit2).sender )
	    QObject::connect( (*cit2).sender, (*cit2).signal, lastForm, (*cit2).slot );
    }
    QObjectListIt eit( filterObjects );
    while ( eit ) {
	if ( lastForm )
	    lastForm->removeEventFilter( *eit );
	if ( newForm )
	    newForm->installEventFilter( *eit );
	++eit;
    }

    lastForm = newForm;
    setComponent( newForm );

    DesignerWidgetListInterfaceImpl *iface = (DesignerWidgetListInterfaceImpl*)QUnknownInterface::child( "*DesignerWidgetListInterface" );
    if ( !iface )
	return;

    iface->setComponent( newForm );
}

QVariant DesignerActiveFormWindowInterfaceImpl::requestProperty( const QCString& p )
{
    return DesignerFormWindowInterfaceImpl::requestProperty( p );
}

bool DesignerActiveFormWindowInterfaceImpl::requestSetProperty( const QCString& p, const QVariant& v )
{
    return DesignerFormWindowInterfaceImpl::requestSetProperty( p, v );
}

bool DesignerActiveFormWindowInterfaceImpl::requestConnect( const char* signal, QObject* target, const char* slot )
{
    ConnectSignal c;
    c.signal = qstrdup(signal);
    c.target = target;
    c.slot = qstrdup(slot);
    signalList.append( c );
    return TRUE;
}

bool DesignerActiveFormWindowInterfaceImpl::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    ConnectSlot c;
    c.signal = qstrdup(signal);
    c.sender = sender;
    c.slot = qstrdup(slot);
    slotList.append( c );
    return TRUE;
}

bool DesignerActiveFormWindowInterfaceImpl::requestEvents( QObject* o )
{
    filterObjects.append( o );

    if ( lastForm )
	lastForm->installEventFilter( o );

    return TRUE;
}

/*
 * DesignerWidgetListInterface implementation
 */
DesignerWidgetListInterfaceImpl::DesignerWidgetListInterfaceImpl( FormWindow *fw, QUnknownInterface *parent )
: DesignerWidgetListInterface( fw, parent ), dictIterator( 0 )
{
}

bool DesignerWidgetListInterfaceImpl::initialize( QApplicationInterface *app )
{
    FormWindow *fw = (FormWindow*)component();
    if ( !fw )
	return FALSE;

    delete dictIterator;
    if ( fw )
	dictIterator = new QPtrDictIterator<QWidget>( *fw->widgets() );

    return DesignerWidgetListInterface::initialize( app );
}

bool DesignerWidgetListInterfaceImpl::cleanUp( QApplicationInterface *app )
{
    delete dictIterator;
    dictIterator = 0;

    return DesignerWidgetListInterface::cleanUp( app );
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::toFirst()
{
    dictIterator->toFirst();

    return current();
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::current()
{
    QWidget *w = dictIterator->current();
    return w ? new DesignerWidgetInterface( w, this ) : 0;
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::next()
{
    ++(*dictIterator);

    return current();
}

/*
 * DesignerWidgetInterface implementation
 */
DesignerWidgetInterfaceImpl::DesignerWidgetInterfaceImpl( QWidget *w, QUnknownInterface *parent )
: DesignerWidgetInterface( w, parent )
{
}

/*
 * DesignerActiveWidgetInterface implementation
 */
DesignerActiveWidgetInterfaceImpl::DesignerActiveWidgetInterfaceImpl( PropertyEditor *pe, QUnknownInterface *parent )
: DesignerWidgetInterfaceImpl( 0, parent ), propertyEditor( pe )
{
}

bool DesignerActiveWidgetInterfaceImpl::initialize( QApplicationInterface *appIface )
{
    if ( !propertyEditor )
	return FALSE;
    QWidget *w = propertyEditor->widget();
    setComponent( w );

    if ( !w )
	return FALSE;

    return DesignerWidgetInterfaceImpl::initialize( appIface );
}
