#include "designerappiface.h"
#include "mainwindow.h"
#include "formwindow.h"
#include "formlist.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include <qstatusbar.h>

DesignerApplicationInterface::DesignerApplicationInterface()
    : QApplicationInterface()
{
    MainWindow* mw = (MainWindow*)qApp->mainWidget();
    FormList* fl = mw ? mw->formlist() : 0;
    if ( !fl )
	return;

    new DesignerMainWindowInterfaceImpl( mw, this );
    new DesignerFormListInterfaceImpl( fl, this );
}

/*
 * DesignerMainWindowInterface
*/
DesignerMainWindowInterfaceImpl::DesignerMainWindowInterfaceImpl( MainWindow *mw, QUnknownInterface* parent )
    : DesignerMainWindowInterface( mw, parent )
{
    new DesignerStatusBarInterfaceImpl( ((QMainWindow*)component())->statusBar(), this );
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
}

QList<DesignerFormWindowInterface>* DesignerFormListInterfaceImpl::queryFormInterfaceList()
{
    QList<DesignerFormWindowInterface>* list = new QList<DesignerFormWindowInterface>();
    list->setAutoDelete( TRUE );

    QListViewItemIterator it( (FormList*)component() );
    while ( it.current() ) {
	FormListItem* item = (FormListItem*)it.current();
	++it;
	list->append( new DesignerFormWindowInterface( item->formWindow() ) );
    }

    return list;
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



/*
 * DesignerActiveFormWindowInterface
*/
DesignerActiveFormWindowInterfaceImpl::DesignerActiveFormWindowInterfaceImpl ( FormList* fl, QUnknownInterface *parent )
    : DesignerActiveFormWindowInterface( fl, parent ), formWindow( 0 )
{
    connect( component(), SIGNAL( selectionChanged() ),
	     this, SLOT( reconnect() ) );
}

QVariant DesignerActiveFormWindowInterfaceImpl ::requestProperty( const QCString& p )
{
    FormListItem* fli = (FormListItem*)((FormList*)component())->currentItem();
    if ( !fli )
	return QVariant();

    return fli->formWindow()->property( p );
}

bool DesignerActiveFormWindowInterfaceImpl ::requestSetProperty( const QCString& p, const QVariant& v )
{
    FormListItem* fli = (FormListItem*)((FormList*)component())->currentItem();
    if ( !fli )
	return FALSE;

    return fli->formWindow()->setProperty( p, v );
}

bool DesignerActiveFormWindowInterfaceImpl ::requestConnect( const char* signal, QObject* target, const char* slot )
{
    ConnectSignal c;
    c.signal = qstrdup(signal);
    c.target = target;
    c.slot = qstrdup(slot);
    signalList.append( c );
    return TRUE;
}

bool DesignerActiveFormWindowInterfaceImpl ::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    ConnectSlot c;
    c.signal = qstrdup(signal);
    c.sender = sender;
    c.slot = qstrdup(slot);
    slotList.append( c );
    return TRUE;
}

bool DesignerActiveFormWindowInterfaceImpl ::requestEvents( QObject* f )
{
    filterObjects.append( f );
    FormListItem* item = (FormListItem*)((FormList*)component())->currentItem();
    if ( !item )
	return TRUE;

    formWindow = item->formWindow();
    if ( formWindow )
	formWindow->installEventFilter( f );

    return TRUE;
}

void DesignerActiveFormWindowInterfaceImpl ::reconnect()
{
    FormWindow *oldFormWindow = formWindow;
    FormListItem* item = (FormListItem*)((FormList*)component())->currentItem();
    formWindow = item ? item->formWindow() : 0;

    {
	for ( QValueList<ConnectSignal>::Iterator cit1 = signalList.begin(); cit1 != signalList.end(); ++cit1 ) {
	    if ( oldFormWindow && (*cit1).target )
		disconnect( oldFormWindow, (*cit1).signal, (*cit1).target, (*cit1).slot );
	    if ( formWindow && (*cit1).target )
		connect( formWindow, (*cit1).signal, (*cit1).target, (*cit1).slot );
	}
    }
    {
	for ( QValueList<ConnectSlot>::Iterator cit2 = slotList.begin(); cit2 != slotList.end(); ++cit2 ) {
	    if ( oldFormWindow && (*cit2).sender )
		disconnect( (*cit2).sender, (*cit2).signal, oldFormWindow, (*cit2).slot );
	    if ( formWindow && (*cit2).sender )
		connect( (*cit2).sender, (*cit2).signal, formWindow, (*cit2).slot );
	}
    }
    QObjectListIt eit( filterObjects );
    while ( eit ) {
	if ( oldFormWindow )
	    oldFormWindow->removeEventFilter( *eit );
	if ( formWindow )
	    formWindow->installEventFilter( *eit );
	++eit;
    }
}
