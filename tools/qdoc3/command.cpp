/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  command.cpp
*/

#include <QProcess>

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

    QProcess process;
    process.start("sh", QStringList() << "-c" << actualCommand );
    process.waitForFinished();

    if (process.exitCode() == 127)
	location.fatal( tr("Couldn't launch the '%1' tool")
			.arg(toolName),
			tr("Make sure the tool is installed and in the"
			   " path.") );

    QString errors = process.readAllStandardError();
    while ( errors.endsWith("\n") )
	errors.truncate( errors.length() - 1 );
    if ( !errors.isEmpty() )
	location.fatal( tr("The '%1' tool encountered some problems")
			.arg(toolName),
			tr("The tool was invoked like this:\n%1\n"
			   "It emitted these errors:\n%2")
			.arg(actualCommand).arg(errors) );
}
