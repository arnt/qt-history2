#include <qapplication.h>
#include "setupwizardimpl.h"


int main( int argc, char** argv )
{
    QApplication app( argc, argv );
    SetupWizardImpl* w;
    bool reconfig( false );
    int res( -1 );

    for( int i = 0; i < app.argc(); i++ ) {
	if( QString( app.argv()[ i ] ) == "-reconfig" ) {
	    reconfig = true;
	    break;
	}
    }

    if( w = new SetupWizardImpl( NULL, NULL, false, 0, reconfig ) ) {
	w->show();

	app.setMainWidget( w );

	res = app.exec();

	w->stopProcesses();
    }

    return res;
}
