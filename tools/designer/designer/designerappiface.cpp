#include "designerappiface.h"
#include "mainwindow.h"
#include "formwindow.h"
#include <qapplication.h>
#include <qobjectlist.h>

DesignerApplicationInterface::DesignerApplicationInterface()
    : QApplicationInterface()
{
}

QComponentInterface * DesignerApplicationInterface::requestInterface( const QCString &request )
{
    if ( request == "DesignerMainWindowInterface" )
	return mwIface ? mwIface : ( mwIface = new DesignerMainWindowInterface( (MainWindow*)qApp->mainWidget() ) );
    return 0;
}



DesignerMainWindowInterface::DesignerMainWindowInterface( MainWindow *mw )
    : QComponentInterface( mw ), mainWindow( mw )
{
}

QComponentInterface *DesignerMainWindowInterface::requestInterface( const QCString &request )
{
    if ( request == "DesignerFormWindowInterface" )
	return fwIface ? fwIface : ( fwIface = new DesignerFormWindowInterface( mainWindow ) );
    return 0;
}



DesignerFormWindowInterface::DesignerFormWindowInterface( MainWindow *mw )
    : QComponentInterface( mw ), mainWindow( mw )
{
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
    QObjectList *l = mainWindow->queryList( "FormWindow" );
    if ( !l || l->isEmpty() ) {
	delete l;
	return FALSE;
    }
    
    for ( QObject *o = l->first(); o; o = l->next() ) 
	connect( (FormWindow*)o, signal, target, slot );
    delete l;
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

