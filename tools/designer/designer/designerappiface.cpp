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
}

QComponentInterface * DesignerApplicationInterface::queryInterface( const QString &request )
{
    MainWindow* mw = (MainWindow*)qApp->mainWidget();
    FormList* fl = mw->formlist();

    if ( request == "DesignerMainWindowInterface" && mw )
	return mwIface ? mwIface : ( mwIface = new DesignerMainWindowInterface( mw ) );
    else if ( request == "DesignerFormListInterface" && fl )
	return flIface ? flIface : ( flIface = new DesignerFormListInterface( fl ) );
    return 0;
}

/*
 * DesignerMainWindowInterface
*/
DesignerMainWindowInterface::DesignerMainWindowInterface( MainWindow *mw )
    : QComponentInterface( mw ), mainWindow( mw )
{
}

QComponentInterface *DesignerMainWindowInterface::queryInterface( const QString &request )
{
    QStatusBar* sb = mainWindow->statusBar();
    if ( request == "DesignerStatusBarInterface" && sb )
	return sbIface ? sbIface : ( sbIface = new DesignerStatusBarInterface( sb ) );
    return 0;
}

/*
 * DesignerStatusBarInterface
*/
DesignerStatusBarInterface::DesignerStatusBarInterface( QStatusBar *sb )
    : QComponentInterface( sb )
{
}

bool DesignerStatusBarInterface::requestSetProperty( const QCString &p, const QVariant &v )
{
    if ( p == "message" ) {
	QStatusBar* sb = (QStatusBar*)object();
	if ( sb ) {
	    sb->message( v.toString(), 3000 );
	    return TRUE;
	} else {
	    return FALSE;
	}
    }
    return QComponentInterface::requestSetProperty( p, v );
}

/*
 * DesignerFormListInterface
*/
DesignerFormListInterface::DesignerFormListInterface( FormList *fl )
    : QComponentInterface( fl )
{
}

QComponentInterface *DesignerFormListInterface::queryInterface( const QString &request )
{
    if ( request == "DesignerFormWindowInterface" )
	return fwIface ? fwIface : ( fwIface = new DesignerFormWindowInterface( (FormList*)object() ) );
    return QComponentInterface::queryInterface( request );
}

QVariant DesignerFormListInterface::requestProperty( const QCString& p )
{
    if ( p == "fileList" ) {
	QStringList list;
	QListViewItemIterator it( (FormList*)object() );
	while ( it.current() ) {
	    QListViewItem* item = it.current();
	    ++it;
	    list << item->text( 2 );
	}
	return QVariant( list );
    } else if ( p == "formList" ) {
	QStringList list;
	QListViewItemIterator it( (FormList*)object() );
	while ( it.current() ) {
	    QListViewItem* item = it.current();
	    ++it;
	    list << item->text( 0 );
	}
	return QVariant( list );
    }
    return QComponentInterface::requestProperty( p );
}

/*
 * DesignerFormWindowInterface
*/
DesignerFormWindowInterface::DesignerFormWindowInterface( FormList* fl )
    : QComponentInterface( fl ), formList( fl ), formWindow( 0 )
{
    connect( formList, SIGNAL( selectionChanged() ),
	     this, SLOT( reconnect() ) );
}

QVariant DesignerFormWindowInterface::requestProperty( const QCString& p )
{
    FormListItem* fli = (FormListItem*)(formList->currentItem());
    if ( !fli )
	return QVariant();

    return fli->formWindow()->property( p );
}

bool DesignerFormWindowInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    FormListItem* fli = (FormListItem*)(formList->currentItem());
    if ( !fli )
	return FALSE;

    return fli->formWindow()->setProperty( p, v );
}

bool DesignerFormWindowInterface::requestConnect( const char* signal, QObject* target, const char* slot )
{
    Connect1 c;
    c.signal = signal;
    c.target = target;
    c.slot = slot;
    connects1.append( c );
    return TRUE;
}

bool DesignerFormWindowInterface::requestConnect( QObject *sender, const char* signal, const char* slot )
{
    Connect2 c;
    c.signal = signal;
    c.sender = sender;
    c.slot = slot;
    connects2.append( c );
    return TRUE;
}

bool DesignerFormWindowInterface::requestEvents( QObject* f )
{
    filterObjects.append( f );
    FormListItem* item = (FormListItem*)formList->currentItem();
    if ( !item )
	return TRUE;

    formWindow = item->formWindow();
    if ( formWindow )
	formWindow->installEventFilter( f );

    return TRUE;
}

void DesignerFormWindowInterface::reconnect()
{
    FormWindow *oldFormWindow = formWindow;
    FormListItem* item = (FormListItem*)formList->currentItem();
    formWindow = item ? item->formWindow() : 0;

    for ( QValueList<Connect1>::Iterator cit1 = connects1.begin(); cit1 != connects1.end(); ++cit1 ) {
	if ( oldFormWindow )
	    disconnect( oldFormWindow, (*cit1).signal, (*cit1).target, (*cit1).slot );
	if ( formWindow )
	    connect( formWindow, (*cit1).signal, (*cit1).target, (*cit1).slot );
    }
    for ( QValueList<Connect2>::Iterator cit2 = connects2.begin(); cit2 != connects2.end(); ++cit2 ) {
	if ( oldFormWindow )
	    disconnect( (*cit2).sender, (*cit2).signal, oldFormWindow, (*cit2).slot );
	if ( formWindow )
	    connect( (*cit2).sender, (*cit2).signal, formWindow, (*cit2).slot );
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
