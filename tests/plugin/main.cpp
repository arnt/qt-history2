#include <qapplication.h>
#include <qmainwindow.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qaction.h>

#include "qplugin.h"

int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    int ret;

    QPlugInManager pm( "./plugin" );
    
    {
	QMainWindow mw;
	mw.setCentralWidget( pm.createWidget( pm.enumerateWidgets()[1], &mw ) );

	QPopupMenu* p = new QPopupMenu( &mw );
	QAction* a = pm.createAction( "CustomAction", &mw );
	if ( a )
	    a->addTo( p );

	mw.menuBar()->insertItem( "&AddIns", p );
	mw.statusBar();

	app.setMainWidget( &mw );
	mw.show();

	ret = app.exec();
    }

    return ret;
}