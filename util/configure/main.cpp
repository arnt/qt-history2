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
    if( app.displayHelp() )
	return 1;

    // Read license now, and exit if it doesn't pass.
    // This lets the user see the command-line options of configure
    // without having to load and parse the license file.
    app.readLicense();
    if (!app.isOk())
        return 3;

    app.autoDetection();
    app.generateOutputVars();

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

    return 0;
}
