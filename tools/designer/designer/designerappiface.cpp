#include "designerappiface.h"
#include "mainwindow.h"
#include <qapplication.h>

DesignerApplicationInterface::DesignerApplicationInterface()
    : QApplicationInterface()
{
}


QComponentInterface * DesignerApplicationInterface::requestInterface( const QCString &request )
{
    if ( request == "MainWindowInterface" ) {
	QComponentInterface *i = *interfaces.find( request );
	if ( !i ) {
	    i = new QComponentInterface( (MainWindow*)qApp->mainWidget() );
	    interfaces.insert( request, i );
	}
	return i;
    }
	
    return 0;
}
