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
    app.generateOutputVars();
    if( app.displayHelp() )
	return 0;
    else {
	app.generateCachefile();
	app.copyDefsFile();
	app.displayConfig();
	app.buildQmake();

	app.exec();

	app.showSummary();
    }

    return 0;
}
