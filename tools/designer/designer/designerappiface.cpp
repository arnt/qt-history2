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
    : QComponentInterface()
{
    MainWindow* mw = (MainWindow*)qApp->mainWidget();
    FormList* fl = mw ? mw->formlist() : 0;
    if ( !fl )
	return;

    new DesignerMainWindowInterfaceImpl( mw, this );
    new DesignerFormListInterfaceImpl( fl, this );
    new DesignerConfigurationInterface( this );
}

/*
 * DesignerMainWindowInterface
*/
DesignerMainWindowInterfaceImpl::DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent )
    : DesignerMainWindowInterface( parent ), mainWindow( mw )
{
    PropertyEditor* pe = mw ? mw->propertyeditor() : 0;
    HierarchyView* hv = mw ? mw->objectHierarchy() : 0;

    new DesignerStatusBarInterfaceImpl( mw->statusBar(), this );
    new DesignerPropertyEditorInterface( this );
    new DesignerHierarchyViewInterface( this );
}

/*
 * DesignerStatusBarInterface
*/
DesignerStatusBarInterfaceImpl::DesignerStatusBarInterfaceImpl( QStatusBar *sb, QUnknownInterface* parent )
    : DesignerStatusBarInterface( parent ), statusBar( sb )
{
}

void DesignerStatusBarInterfaceImpl::setMessage( const QString &m, int ms )
{
    if ( statusBar )
	statusBar->message( m, ms );
}

/*
 * DesignerFormListInterface
*/
DesignerFormListInterfaceImpl::DesignerFormListInterfaceImpl( FormList *fl, QUnknownInterface* parent )
    : DesignerFormListInterface( parent ), formList( fl )
{
    new DesignerActiveFormWindowInterfaceImpl( fl, this );
    listIterator = 0;
}

unsigned long DesignerFormListInterfaceImpl::addRef()
{
    unsigned long rc = DesignerFormListInterface::addRef();
    if ( !rc ) {
	delete listIterator;
	listIterator = new QListViewItemIterator( formList );
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

void DesignerFormListInterfaceImpl::setText( DesignerFormWindowInterface *form, int col, const QString& s )
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

const QPixmap* DesignerFormListInterfaceImpl::pixmap( DesignerFormWindowInterface *form, int col ) const
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

void DesignerFormListInterfaceImpl::setPixmap( DesignerFormWindowInterface *form, int col, const QPixmap& pix )
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

QString DesignerFormWindowInterfaceImpl::fileName() const
{
    if ( !item )
	return QString::null;
    return item->text( 1 );
}

void DesignerFormWindowInterfaceImpl::setFileName( const QString & fn )
{
    qDebug( "TODO: DesignerFormWindowInterfaceImpl::setFileName" );
}

QPixmap DesignerFormWindowInterfaceImpl::icon() const
{
    if ( !item )
	return QPixmap();
    return *(item->pixmap( 0 ));
}

void DesignerFormWindowInterfaceImpl::setIcon( const QPixmap & fn )
{
    qDebug( "TODO: DesignerFormWindowInterfaceImpl::setIcon" );
}

bool DesignerFormWindowInterfaceImpl::connect( const char *signal, QObject *receiver, const char *slot )
{
    FormWindow *fw = 0;
    if ( item )
	fw = item->formWindow();

    return fw ? fw->connect( fw, signal, receiver, slot ) : FALSE;
}

/*
 * DesignerActiveFormWindowInterface
*/
DesignerActiveFormWindowInterfaceImpl::DesignerActiveFormWindowInterfaceImpl( FormList *fl, QUnknownInterface *parent )
: DesignerFormWindowInterfaceImpl( 0, parent ), formList( fl )
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
DesignerWidgetListInterfaceImpl::DesignerWidgetListInterfaceImpl( FormWindow *fw, QUnknownInterface *parent )
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
DesignerWidgetInterfaceImpl::DesignerWidgetInterfaceImpl( QWidget *w, QUnknownInterface *parent )
: DesignerWidgetInterface( parent ), widget( w )
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
DesignerActiveWidgetInterfaceImpl::DesignerActiveWidgetInterfaceImpl( PropertyEditor *pe, QUnknownInterface *parent )
: DesignerWidgetInterfaceImpl( 0, parent ), propertyEditor( pe )
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
