/*
  messages.h
*/

#ifndef MESSAGES_H
#define MESSAGES_H

#include <qobject.h>
#include <qstring.h>

#include "location.h"
#include "qdoc.h"

class Config;

class Messages
{
public:
    static void initialize( const Config& config );
    static void terminate();
    static void information( const QString& message );
    static void warning( const Location& location, const QString& message,
			 const QString& details = "" );
    static void error( const Location& location, const QString& message,
		       const QString& details = "" );
    static void fatal( const Location& location, const QString& message,
		       const QString& details = "" );
    static void internalError( const QString& hint );

private:
    static void emitMessage( bool isError, const Location& location,
			     const QString& message, const QString& details );
    static QString toString( const Location& location );
    static QString top( const Location& location );

    static QString programName;
    static QRegExp *spuriousRegExp;
};

#endif
