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
    : QUnknownInterface()
{
    MainWindow* mw = (MainWindow*)qApp->mainWidget();
    FormList* fl = mw ? mw->formlist() : 0;
    if ( !fl )
	return;

    new DesignerMainWindowInterfaceImpl( mw, this );
    new DesignerFormListInterfaceImpl( fl, this );
    new DesignerConfigurationInterface( 0, this );
}

/*
 * DesignerMainWindowInterface
*/
DesignerMainWindowInterfaceImpl::DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent )
    : DesignerMainWindowInterface( mw, parent )
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
    : DesignerStatusBarInterface( sb, parent )
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
    : DesignerFormListInterface( fl, parent )
{
    new DesignerActiveFormWindowInterfaceImpl( fl, this );
    listIterator = 0;
}

unsigned long DesignerFormListInterfaceImpl::addRef()
{
    unsigned long rc = DesignerFormListInterface::addRef();
    if ( !rc ) {
	delete listIterator;
	listIterator = new QListViewItemIterator( (FormList*)component() );
    }

    return rc;
}

unsigned long DesignerFormListInterfaceImpl::release()
{
    unsigned long rc = DesignerFormListInterface::release();
    if ( !rc ) {
	delete listIterator;
        listIterator = 0;
    }

    return rc;
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
    QString formname = form->requestProperty( "fileName" ).toString();
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 1 ) == formname ) {
	    item->setPixmap( col, pix );
	    return;
	}
    }
}

uint DesignerFormListInterfaceImpl::count() const
{
    FormList *fl = (FormList*)component();

    return fl ? fl->childCount() : 0;
}

DesignerFormWindowInterface *DesignerFormListInterfaceImpl::current()
{
    FormListItem *item = (FormListItem*)listIterator->current();

    return item ? new DesignerFormWindowInterfaceImpl( item, this ) : 0;
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

bool DesignerFormListInterfaceImpl::newForm()
{
    MainWindow *mw = (MainWindow*)qApp->mainWidget();
    if ( !mw )
	return FALSE;

    qDebug( "Not yet implemented" );

    return TRUE;
}

bool DesignerFormListInterfaceImpl::loadForm()
{
    MainWindow *mw = (MainWindow*)qApp->mainWidget();
    if ( !mw )
	return FALSE;

    qDebug( "Not yet implemented" );

    return TRUE;
}

bool DesignerFormListInterfaceImpl::saveAll() const
{
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormWindow* fw = ((FormListItem*)it.current())->formWindow();
	++it;
	fw->save( "Dudeldi" );
    }

    return TRUE;
}

void DesignerFormListInterfaceImpl::closeAll() const
{
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormWindow* fw = ((FormListItem*)it.current())->formWindow();
	++it;
	fw->close( TRUE );
    }
}

/*
 * DesignerFormWindowInterface
*/

DesignerFormWindowInterfaceImpl::DesignerFormWindowInterfaceImpl( FormListItem *fw, QUnknownInterface *parent )
    : DesignerFormWindowInterface( parent ), item( fw )
{
    MainWindow *mw = (MainWindow*)qApp->mainWidget();
    PropertyEditor *pe = mw->propertyeditor();

    new DesignerWidgetListInterfaceImpl( item ? item->formWindow() : 0, this );
    new DesignerActiveWidgetInterfaceImpl( pe, this );
}

void DesignerFormWindowInterfaceImpl::save() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->save( "Dudeldi" );
}

void DesignerFormWindowInterfaceImpl::close() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->close( TRUE );
}

void DesignerFormWindowInterfaceImpl::undo() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->undo();
}

void DesignerFormWindowInterfaceImpl::redo() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->redo();
}

QObject *DesignerFormWindowInterfaceImpl::component() const
{ 
    return item ? item->formWindow() : 0; 
}

void DesignerFormWindowInterfaceImpl::setFormListItem( FormListItem *fl )
{
    item = fl;
}

FormListItem *DesignerFormWindowInterfaceImpl::formListItem() const
{
    return item;
}

QVariant DesignerFormWindowInterfaceImpl::requestProperty( const QCString &p )
{
    if ( !item )
	return QVariant();
    if ( p == "fileName" )
	return item->text( 1 );
    else if ( p == "icon" )
	return *(item->pixmap( 0 ));
    return DesignerFormWindowInterface::requestProperty( p );
}

/*
 * DesignerActiveFormWindowInterface
*/
DesignerActiveFormWindowInterfaceImpl::DesignerActiveFormWindowInterfaceImpl( FormList *fl, QUnknownInterface *parent )
: DesignerFormWindowInterfaceImpl( (FormListItem*)fl->currentItem(), parent ), formList( fl ), lastForm( 0 ), inReconnect( FALSE )
{
}

unsigned long DesignerActiveFormWindowInterfaceImpl::addRef()
{
    unsigned long rc = DesignerFormWindowInterfaceImpl::addRef();
    // update the internal component object each time this interface is used
    if ( !inReconnect ) {
	reconnect();
	inReconnect = FALSE;
    }

    return rc;
}

void DesignerActiveFormWindowInterfaceImpl::reconnect()
{
    inReconnect = TRUE;

    FormListItem* fli = (FormListItem*)formList->currentItem();
    setFormListItem( fli );
    FormWindow *newForm = fli ? fli->formWindow() : 0;    

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

    DesignerWidgetListInterfaceImpl *iface = (DesignerWidgetListInterfaceImpl*)queryInterface( "*DesignerWidgetListInterface" );
    if ( !iface )
	return;

    iface->setFormWindow( newForm );
    iface->release();
}

bool DesignerActiveFormWindowInterfaceImpl::requestConnect( const char* signal, QObject* target, const char* slot )
{
    ConnectSignal c;
    c.signal = qstrdup(signal);
    c.target = target;
    c.slot = qstrdup(slot);
    signalList.append( c );

    reconnect();

    return TRUE;
}

bool DesignerActiveFormWindowInterfaceImpl::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    ConnectSlot c;
    c.signal = qstrdup(signal);
    c.sender = sender;
    c.slot = qstrdup(slot);
    slotList.append( c );

    reconnect();

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

unsigned long DesignerWidgetListInterfaceImpl::release()
{
    unsigned long rc = DesignerWidgetListInterface::release();
    if ( !rc ) {
	delete dictIterator;
	dictIterator = 0;
    }

    return rc;
}

FormWindow *DesignerWidgetListInterfaceImpl::formWindow() const
{
    return (FormWindow*)component();
}

void DesignerWidgetListInterfaceImpl::setFormWindow( FormWindow *fw )
{
    if ( fw ) {
	delete dictIterator;
	dictIterator = new QPtrDictIterator<QWidget>( *fw->widgets() );
    }
    setComponent( fw );
}

uint DesignerWidgetListInterfaceImpl::count() const
{
    FormWindow *fw = formWindow();
    if ( !fw )
	return 0;

    QWidget *mc = fw->mainContainer();
    if ( !mc )
	return 0;

    QObjectList *list = mc->queryList( "QWidget" );
    int count = list->count();
    delete list;

    return count;
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::toFirst()
{
    dictIterator->toFirst();

    return current();
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::current()
{
    QWidget *w = dictIterator->current();
    return w ? new DesignerWidgetInterfaceImpl( w, this ) : 0;
}

DesignerWidgetInterface* DesignerWidgetListInterfaceImpl::next()
{
    ++(*dictIterator);

    return current();
}

void DesignerWidgetListInterfaceImpl::selectAll() const
{
    FormWindow *fw = (FormWindow*)component();
    if ( !fw )
	return;

    fw->selectAll();
}

void DesignerWidgetListInterfaceImpl::removeAll() const
{
    FormWindow *fw = (FormWindow*)component();
    if ( !fw )
	return;

    fw->selectAll();
    fw->deleteWidgets();
}

/*
 * DesignerWidgetInterface implementation
 */
DesignerWidgetInterfaceImpl::DesignerWidgetInterfaceImpl( QWidget *w, QUnknownInterface *parent )
: DesignerWidgetInterface( w, parent )
{
}

void DesignerWidgetInterfaceImpl::setSelected( bool sel )
{
    DesignerWidgetListInterfaceImpl* wlIface = (DesignerWidgetListInterfaceImpl*)parent();
    FormWindow* fw = wlIface->formWindow();
    if ( !fw->isA( "FormWindow" ) )
	return;

    fw->selectWidget( (QWidget*)component(), sel );
}

bool DesignerWidgetInterfaceImpl::selected() const
{
    DesignerWidgetListInterfaceImpl* wlIface = (DesignerWidgetListInterfaceImpl*)parent();
    FormWindow* fw = wlIface->formWindow();
    if ( !fw->isA( "FormWindow" ) )
	return FALSE;

    return fw->isWidgetSelected( (QWidget*)component() );
}

void DesignerWidgetInterfaceImpl::remove()
{
    DesignerWidgetListInterfaceImpl* wlIface = (DesignerWidgetListInterfaceImpl*)parent();
    FormWindow* fw = wlIface->formWindow();
    if ( !fw->isA( "FormWindow" ) )
	return;

    QWidget *w = (QWidget*)component();
    QWidgetList sw = fw->selectedWidgets();
    QPtrDictIterator<QWidget> it( *fw->widgets() );
    while ( it.current() ) {
	fw->selectWidget( it.current(), FALSE );
	++it;
    }
    fw->selectWidget( w );
    fw->deleteWidgets();
    sw.removeRef( w );
    QListIterator<QWidget> sit( sw );
    while ( sit.current() ) {
	fw->selectWidget( sit.current(), TRUE );
	++sit;
    }
}

/*
 * DesignerActiveWidgetInterface implementation
 */
DesignerActiveWidgetInterfaceImpl::DesignerActiveWidgetInterfaceImpl( PropertyEditor *pe, QUnknownInterface *parent )
: DesignerWidgetInterfaceImpl( 0, parent ), propertyEditor( pe )
{
}

unsigned long DesignerActiveWidgetInterfaceImpl::addRef()
{
    unsigned long rc = DesignerWidgetInterfaceImpl::addRef();
    if ( !rc ) {
	QObject *w;
	if ( propertyEditor && ( w = propertyEditor->widget() ) && w->isWidgetType() )
	    setComponent( (QWidget*)w );
    }

    return rc;
}
