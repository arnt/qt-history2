/*
  command.cpp
*/

#include <qprocess.h>

#include "command.h"

void executeCommand( const Location& location, const QString& format,
		     const QStringList& args )
{
    QString actualCommand;
    for ( int i = 0; i < (int) format.length(); i++ ) {
	int ch = format[i].unicode();
	if ( ch > 0 && ch < 8 ) {
	    actualCommand += args[ch - 1];
	} else {
	    actualCommand += format[i];
	}
    }

    QString toolName = actualCommand;
    int space = toolName.indexOf( " " );
    if ( space != -1 )
	toolName.truncate( space );

    QProcess process( QStringList() << "sh" << "-c" << actualCommand );
    process.start();
    while ( process.isRunning() )
	;

    if ( process.exitStatus() == 127 )
	location.fatal( tr("Couldn't launch the '%1' tool")
			.arg(toolName),
			tr("Make sure the tool is installed and in the"
			   " path.") );

    QString errors = process.readStderr();
    while ( errors.endsWith("\n") )
	errors.truncate( errors.length() - 1 );
    if ( !errors.isEmpty() )
	location.fatal( tr("The '%1' tool encountered some problems")
			.arg(toolName),
			tr("The tool was invoked like this:\n%1\n"
			   "It emitted these errors:\n%2")
			.arg(actualCommand).arg(errors) );
}
