/*
  messages.cpp
*/

#include <qmap.h>
#include <qstring.h>

#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "location.h"

static bool paranoia = FALSE;

static QMap<QString, int> *msgMap = 0;
static int warningLevel = INT_MAX;
static int numAll = 0;
static int numOmitted = 0;

static int maxSame = INT_MAX;
static int maxAll = INT_MAX;

static bool omit( const char *message )
{
    numAll++;

    if ( msgMap == 0 )
	msgMap = new QMap<QString, int>;
    int count = 0;
    if ( msgMap->contains(QString(message)) )
	count = (*msgMap)[QString(message)];
    count++;
    msgMap->insert( QString(message), count );

    if ( numAll > maxAll || count > maxSame ) {
	numOmitted++;
	return TRUE;
    } else if ( count == maxSame ) {
	fprintf( stderr, "qdoc warning: "
		 "Omitting further similar warnings after this:\n" );
    }
    return FALSE;
}

void setParanoiaEnabled( bool on )
{
    paranoia = on;
}

void setMaxSimilarMessages( int n )
{
    maxSame = n;
}

void setMaxMessages( int n )
{
    maxAll = n;
}

void setWarningLevel( int level )
{
    warningLevel = level;
}

static QString currentDirectory;

/*
  Level 0 is used for errors and for messages in supervisor mode.
  They don't go through omit() and it takes a warningLevel of -1
  to disable them (impossible in qdoc)..
*/

void warning( int level, const Location& loc, const char *message, ... )
{
    if ( warningLevel < level )
	return;
    if ( !paranoia && level > 0 && omit(message) )
	return;

    QString filename = loc.shortFilePath();
    QString filenameBase = loc.filePath();
    filenameBase
	= filenameBase.left( filenameBase.length() - filename.length() - 1 );
    if ( currentDirectory.length() > 5 &&
	 currentDirectory.length() < filenameBase.length() &&
	 filenameBase.left( currentDirectory.length() ) == currentDirectory &&
	 filenameBase.length() < currentDirectory.length() + 10 ) {
	// If we can avoid two lines of directory changes by adding at
	// most 10 characters to each line, we'll do it.
	QString extra = filenameBase.mid( currentDirectory.length() );
	filenameBase = currentDirectory;
	filename.prepend( extra.mid( 1 ) + extra[0] );
    }
    if ( !filenameBase.isEmpty() &&
	 currentDirectory != filenameBase ) {
	if ( currentDirectory.length() )
	    fprintf( stderr, "qdoc: Leaving directory `%s'\n",
		     currentDirectory.latin1() );
	currentDirectory = filenameBase;
	fprintf( stderr, "qdoc: Entering directory `%s'\n",
		 currentDirectory.latin1() );
    }

    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "%s:%d: ", filename.latin1(), loc.lineNum() );
    vfprintf( stderr, message, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
}

void warning( int level, const char *message, ... )
{
    if ( warningLevel < level )
	return;
    if ( level > 0 && omit(message) )
	return;

    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "qdoc warning: " );
    vfprintf( stderr, message, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
}

void syswarning( const char *message, ... )
{
    if ( omit(message) )
	return;

    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "qdoc warning: " );
    vfprintf( stderr, message, ap );
    fprintf( stderr, ": %s\n", strerror(errno) );
    va_end( ap );
}

void warnAboutOmitted()
{
    int omitted = 0;
    if ( msgMap ) {
	int types = 0;
	QMap<QString, int>::ConstIterator it = msgMap->begin();
	while( it != msgMap->end() ) {
	    types++;
	    if ( it.data() > maxSame )
		omitted++;
	    ++it;
	}
	fprintf( stderr, "qdoc warning: gave %d warning%s of %d type%s\n",
		 numAll - numOmitted, (numAll - numOmitted) == 1 ? "" : "s",
		 types, types == 1 ? "" : "s" );
    }
    if ( numOmitted > 0 )
	fprintf( stderr,
		 "qdoc warning: omitted %d more warning%s of %d type%s\n",
		 numOmitted, numOmitted == 1 ? "" : "s",
		 omitted, omitted == 1 ? "" : "s" );
}
