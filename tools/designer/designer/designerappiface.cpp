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
    MainWindow* mw = (MainWindow*)qApp->mainWidget();
    if ( request == "DesignerFormWindowInterface" && mw )
	return fwIface ? fwIface : ( fwIface = new DesignerFormWindowInterface( mw ) );
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
	    list << item->text( 2);
	}
	return QVariant( list );
    }
    return QComponentInterface::requestProperty( p );
}

/*
 * DesignerFormWindowInterface
*/
DesignerFormWindowInterface::DesignerFormWindowInterface( MainWindow *mw )
    : QComponentInterface( mw ), mainWindow( mw )
{
    connect( mw, SIGNAL( formWindowsChanged() ),
	     this, SLOT( reconnect() ) );
}

QVariant DesignerFormWindowInterface::requestProperty( const QCString& p )
{
    if ( !mainWindow->formWindow() )
	return QVariant();
    return mainWindow->formWindow()->property( p );
}

bool DesignerFormWindowInterface::requestSetProperty( const QCString& p, const QVariant& v )
{
    if ( !mainWindow->formWindow() )
	return FALSE;
    return mainWindow->formWindow()->setProperty( p, v );
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
    QObjectList *l = mainWindow->queryList( "FormWindow" );
    if ( !l || l->isEmpty() ) {
	delete l;
	return FALSE;
    }

    for ( QObject *o = l->first(); o; o = l->next() )
	f->installEventFilter( o );
    delete l;
    return TRUE;
}

void DesignerFormWindowInterface::reconnect()
{
    QObjectList *l = mainWindow->queryList( "FormWindow" );
    if ( !l || l->isEmpty() ) {
	delete l;
	return;
    }

    for ( QObject *o = l->first(); o; o = l->next() ) {
	for ( QValueList<Connect1>::Iterator it = connects1.begin(); it != connects1.end(); ++it )
	    connect( (FormWindow*)o, (*it).signal, (*it).target, (*it).slot );
	for ( QValueList<Connect2>::Iterator it2 = connects2.begin(); it2 != connects2.end(); ++it2 )
	    connect( (*it2).sender, (*it2).signal, (FormWindow*)o, (*it2).slot );
    }
}

