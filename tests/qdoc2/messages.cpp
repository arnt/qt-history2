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

static QMap<QString, int> *msgMap = 0;
static int warningLevel = INT_MAX;
#if 0
static bool paranoia = FALSE;
#endif
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

#if 0 // ### isn't used
void setParanoiaEnabled( bool enabled )
{
    paranoia = enabled;
}
#endif

static QString currentDirectory;

void warning( int level, const Location& loc, const char *message, ... )
{
    if ( warningLevel < level )
	return;
    if ( omit(message) )
	return;

    QString filename = loc.shortFilePath();
    QString filenameBase = loc.filePath();
    filenameBase
	= filenameBase.left( filenameBase.length() - filename.length() - 1 );
    if ( !filenameBase.isEmpty() &&
	 currentDirectory != filenameBase ) {
	if ( currentDirectory.length() )
	    fprintf( stderr, "qdoc: Leaving directory '%s'\n",
		     currentDirectory.latin1() );
	currentDirectory = filenameBase;
	fprintf( stderr, "qdoc: Entering directory '%s'\n",
		 currentDirectory.latin1() );
    }

    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "%s:%d: ", loc.shortFilePath().latin1(), loc.lineNum() );
    vfprintf( stderr, message, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
}

void warning( int level, const char *message, ... )
{
    if ( warningLevel <= level )
	return;
    if ( omit(message) )
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
	int total = 0;
	QMap<QString, int>::ConstIterator it = msgMap->begin();
	while( it != msgMap->end() ) {
	    total += it.data();
	    types++;
	    if ( it.data() > maxSame )
		omitted++;
	    ++it;
	}
	fprintf( stderr, "qdoc warning: gave %d warning%s of %d type%s\n",
		 total, total == 1 ? "" : "s",
		 types, types == 1 ? "" : "s" );
    }
    if ( numOmitted > 0 )
	fprintf( stderr,
		 "qdoc warning: omitted %d more warning%s of %d type%s\n",
		 numOmitted, numOmitted == 1 ? "" : "s",
		 omitted, omitted == 1 ? "" : "s" );
}
