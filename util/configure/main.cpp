/*
** Configure tool
**
*/

#include "configureapp.h"

int main( int argc, char** argv )
{
    Configure app( argc, argv );

    if (!app.isOk())
        return 3;

    app.parseCmdLine();
#if !defined(EVAL)
    app.validateArgs();
#endif
    app.autoDetection();
    app.generateOutputVars();
    if( app.displayHelp() )
	return 1;
    else {
#if !defined(EVAL)
	if( !app.isDone() )
	    app.generateCachefile();
	if( !app.isDone() )
	    app.generateConfigfiles();
	if( !app.isDone() )
	    app.displayConfig();
	if( !app.isDone() )
	    app.buildQmake();
	if( !app.isOk() )
	    return 2;
#endif
	if( !app.isDone() )
	    app.generateMakefiles();
	if( !app.isDone() )
	    app.showSummary();
    }

    return 0;
}
