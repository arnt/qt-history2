/*
** Configure tool
**
*/

#include "configureapp.h"

int main( int argc, char** argv )
{
    ConfigureApp app( argc, argv );

    app.parseCmdLine();
    app.validateArgs();
    if( app.displayHelp() )
	return 0;
    app.generateOutputVars();
    app.generateCachefile();
    app.displayConfig();
    app.buildQmake();

    app.exec();

    app.showSummary();

    return 0;
}
