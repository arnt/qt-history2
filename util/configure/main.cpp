/*
** Configure tool
**
*/

#include "configureapp.h"

int main( int argc, char** argv )
{
    Configure app( argc, argv );

    app.parseCmdLine();
    app.validateArgs();
    app.generateOutputVars();
    if( app.displayHelp() )
	return 1;
    else {
	if( !app.isDone() )
	    app.generateCachefile();
	if( !app.isDone() )
	    app.generateConfigfiles();
	if( !app.isDone() )
	    app.displayConfig();
	if( !app.isDone() )
	    app.buildQmake();
	if( !app.isDone() )
	    app.generateMakefiles();
	if( !app.isDone() )
	    app.showSummary();
    }

    return 0;
}
