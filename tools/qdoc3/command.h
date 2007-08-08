/*
  command.h
*/

#ifndef COMMAND_H
#define COMMAND_H

#include <qstringlist.h>

#include "location.h"

void executeCommand( const Location& location, const QString& commandFormat,
		     const QStringList& args );

#endif
