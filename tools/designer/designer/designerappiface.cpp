#include "designerappiface.h"

#include "mainwindow.h"
#include "formwindow.h"
#include "formlist.h"
#include "propertyeditor.h"
#include "hierarchyview.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include <qstatusbar.h>

/*
 * Application Interface
*/
DesignerApplicationInterfaceImpl::DesignerApplicationInterfaceImpl()
: ref( 0 )
{
}

QUnknownInterface *DesignerApplicationInterfaceImpl::queryInterface( const QGuid &guid )
{
    QUnknownInterface *iface = 0;
    if ( guid == IID_QUnknownInterface )
	iface = (QUnknownInterface*)(QComponentInterface*)this;
    else if ( guid == IID_QComponentInterface )
	iface = (QComponentInterface*)this;
    else if ( guid == IID_DesignerStatusBarInterface )
	iface = new DesignerStatusBarInterfaceImpl( this );
    else if ( guid == IID_DesignerFormListInterface )
	iface = new DesignerFormListInterfaceImpl( this );
    else if ( guid == IID_DesignerFormInterface )
	iface = new DesignerFormInterfaceImpl( 0, this );

    if ( iface )
	iface->addRef();
    return iface;
}

unsigned long DesignerApplicationInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerApplicationInterfaceImpl::release()
{
    if ( !--ref ) {
	delete this;
	return 0;
    }

    return ref;
}

QString DesignerApplicationInterfaceImpl::name() const 
{ 
    return "Qt Designer"; 
}

QString DesignerApplicationInterfaceImpl::description() const 
{ 
    return "GUI Editor for the Qt Toolkit"; 
}

QString DesignerApplicationInterfaceImpl::version() const 
{ 
    return "1.1"; 
}

QString DesignerApplicationInterfaceImpl::author() const 
{ 
    return "Trolltech"; 
}

/*
 * StatusBar Interface
*/
DesignerStatusBarInterfaceImpl::DesignerStatusBarInterfaceImpl( QUnknownInterface *ai )
: DesignerStatusBarInterface(), appIface( ai ), statusBar( 0 ), ref( 0 )
{
    QWidget *mw = qApp ? qApp->mainWidget() : 0;
    if ( mw && mw->inherits( "QMainWindow" ) )
	statusBar = ((QMainWindow*)mw)->statusBar();
    
    appIface->addRef();
}

QUnknownInterface *DesignerStatusBarInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerStatusBarInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerStatusBarInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

void DesignerStatusBarInterfaceImpl::setMessage( const QString &text, int ms )
{
    if ( statusBar )
	statusBar->message( text, ms );
}

void DesignerStatusBarInterfaceImpl::clear()
{
    if ( statusBar )
	statusBar->clear();
}

/*
 * FormList Interface
*/
DesignerFormListInterfaceImpl::DesignerFormListInterfaceImpl( QUnknownInterface *ai )
: DesignerFormListInterface(), appIface( ai ), formList( 0 ), listIterator( 0 ), ref( 0 )
{
    QWidget *mw = qApp ? qApp->mainWidget() : 0;
    if ( mw && mw->inherits( "MainWindow" ) )
	formList = ((MainWindow*)mw)->formlist();
    if ( formList )
	listIterator = new QListViewItemIterator( formList );

    appIface->addRef();
}

QUnknownInterface *DesignerFormListInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerFormListInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerFormListInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }
    return ref;;
}

QString DesignerFormListInterfaceImpl::text( DesignerFormInterface * /*form*/, int /*col*/ ) const
{
/*    QString formname = form->requestProperty( "name" ).toString();
    DesignerFormListInterfaceImpl* that = (DesignerFormListInterfaceImpl*)this;
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname )
	    return item->text( col );
    }*/
    return QString::null;
}

void DesignerFormListInterfaceImpl::setText( DesignerFormInterface * /*form*/, int /*col*/, const QString& /*s*/ )
{
/*    QString formname = form->requestProperty( "name" ).toString();
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname ) {
	    item->setText( col, s );
	    return;
	}
    }*/
}

const QPixmap* DesignerFormListInterfaceImpl::pixmap( DesignerFormInterface * /*form*/, int /*col*/ ) const
{
/*    QString formname = form->requestProperty( "name" ).toString();
    DesignerFormListInterfaceImpl* that = (DesignerFormListInterfaceImpl*)this;
    QListViewItemIterator it( (FormList*)that->component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 0 ) == formname )
	    return item->pixmap( col );
    }*/
    return 0;
}

void DesignerFormListInterfaceImpl::setPixmap( DesignerFormInterface * /*form*/, int /*col*/, const QPixmap& /*pix*/ )
{
 /*   QString formname = form->requestProperty( "fileName" ).toString();
    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	if ( item->text( 1 ) == formname ) {
	    item->setPixmap( col, pix );
	    return;
	}
    }*/
}

uint DesignerFormListInterfaceImpl::count() const
{
    return formList ? formList->childCount() : 0;
}

DesignerFormInterface *DesignerFormListInterfaceImpl::current()
{
    FormListItem *item = (FormListItem*)listIterator->current();
    DesignerFormInterface *iface;

    iface = item ? new DesignerFormInterfaceImpl( item, appIface ) : 0;
    if ( iface )
	iface->addRef();
    return iface;
}

DesignerFormInterface *DesignerFormListInterfaceImpl::next()
{
    ++(*listIterator);

    return current();
}

DesignerFormInterface *DesignerFormListInterfaceImpl::prev()
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
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormWindow* fw = ((FormListItem*)it.current())->formWindow();
	++it;
	fw->save( "Dudeldi" );
    }

    return TRUE;
}

void DesignerFormListInterfaceImpl::closeAll() const
{
    QListViewItemIterator it( formList );
    while ( it.current() ) {
	FormWindow* fw = ((FormListItem*)it.current())->formWindow();
	++it;
	fw->close( TRUE );
    }
}

bool DesignerFormListInterfaceImpl::connect( const char * signal, QObject *receiver, const char *slot )
{
    return formList ? formList->connect( formList, signal, receiver, slot ) : FALSE;
}

/*
 * DesignerFormWindowInterface
*/
DesignerFormInterfaceImpl::DesignerFormInterfaceImpl( FormListItem *fw, QUnknownInterface *ai )
    : DesignerFormInterface(), item( fw ), appIface( ai ), ref( 0 )
{
    appIface->addRef();

    if ( !item ) {
	FormList *fl = 0;
	QWidget *mw = qApp ? qApp->mainWidget() : 0;
	if ( mw && mw->inherits( "MainWindow" ) )
	    fl = ((MainWindow*)mw)->formlist();
	item = fl ? (FormListItem*)fl->currentItem() : 0;
    }
}

QUnknownInterface *DesignerFormInterfaceImpl::queryInterface( const QGuid& guid )
{
    return appIface->queryInterface( guid );
}

unsigned long DesignerFormInterfaceImpl::addRef()
{
    return ref++;
}

unsigned long DesignerFormInterfaceImpl::release()
{
    if ( !--ref ) {
	appIface->release();
	delete this;
	return 0;
    }

    return ref;
}

QVariant DesignerFormInterfaceImpl::property( const QCString& p )
{
    return ( item && item->formWindow() ) ? item->formWindow()->property( p ) : QVariant();
}

bool DesignerFormInterfaceImpl::setProperty( const QCString& p, const QVariant& v )
{
    return ( item && item->formWindow() ) ? item->formWindow()->setProperty( p, v ) : FALSE;
}

void DesignerFormInterfaceImpl::save() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->save( "Dudeldi" );
}

void DesignerFormInterfaceImpl::close() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->close( TRUE );
}

void DesignerFormInterfaceImpl::undo() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->undo();
}

void DesignerFormInterfaceImpl::redo() const
{
    FormWindow *fw = item->formWindow();
    if ( !fw )
	return;

    fw->redo();
}

bool DesignerFormInterfaceImpl::connect( const char *signal, QObject *receiver, const char *slot )
{
    FormWindow *fw = 0;
    if ( item )
	fw = item->formWindow();

    return fw ? fw->connect( fw, signal, receiver, slot ) : FALSE;
}

#if 0
/*
 * DesignerActiveFormWindowInterface
*/
DesignerActiveFormWindowInterfaceImpl::DesignerActiveFormWindowInterfaceImpl( FormList *fl )
: DesignerFormWindowInterfaceImpl( 0 ), formList( fl )
{
}

unsigned long DesignerActiveFormWindowInterfaceImpl::addRef()
{
    unsigned long rc = DesignerFormWindowInterfaceImpl::addRef();
    // update the internal formlist object each time this interface is used
    setFormListItem( formList ? (FormListItem*)formList->currentItem() : 0 );

    return rc;
}

/*
 * DesignerWidgetListInterface implementation
 */
DesignerWidgetListInterfaceImpl::DesignerWidgetListInterfaceImpl( FormWindow *fw )
: DesignerWidgetListInterface( parent ), dictIterator( 0 ), formWindow( fw )
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
/*
FormWindow *DesignerWidgetListInterfaceImpl::formWindow() const
{
    return formWindow;
}

void DesignerWidgetListInterfaceImpl::setFormWindow( FormWindow *fw )
{
    if ( fw ) {
	delete dictIterator;
	dictIterator = new QPtrDictIterator<QWidget>( *fw->widgets() );
    }
    setComponent( fw );
}
*/
uint DesignerWidgetListInterfaceImpl::count() const
{
    FormWindow *fw = formWindow;
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
    if ( formWindow )
        formWindow->selectAll();
}

void DesignerWidgetListInterfaceImpl::removeAll() const
{
    if ( formWindow ) {
	formWindow->selectAll();
	formWindow->deleteWidgets();
    }
}

/*
 * DesignerWidgetInterface implementation
 */
DesignerWidgetInterfaceImpl::DesignerWidgetInterfaceImpl( QWidget *w )
: DesignerWidgetInterface(), widget( w )
{
}

void DesignerWidgetInterfaceImpl::setSelected( bool sel )
{
}

bool DesignerWidgetInterfaceImpl::selected() const
{
    return TRUE;
}

void DesignerWidgetInterfaceImpl::remove()
{
    /*
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
    }*/
}

/*
 * DesignerActiveWidgetInterface implementation
 */
DesignerActiveWidgetInterfaceImpl::DesignerActiveWidgetInterfaceImpl( PropertyEditor *pe )
: DesignerWidgetInterfaceImpl(), propertyEditor( pe )
{
}

unsigned long DesignerActiveWidgetInterfaceImpl::addRef()
{
    unsigned long rc = DesignerWidgetInterfaceImpl::addRef();
/*    if ( !rc ) {
	QObject *w;
	if ( propertyEditor && ( w = propertyEditor->widget() ) && w->isWidgetType() )
	    setComponent( (QWidget*)w );
    }
*/
    return rc;
}
#endif