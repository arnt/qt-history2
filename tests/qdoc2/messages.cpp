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
static bool paranoia = FALSE;
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

    bool oh = ( numAll > maxAll || count > maxSame );
    if ( oh )
	numOmitted++;
    return oh;
}

void setMaxSameMessage( int n )
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

// ### isn't used
void setParanoiaEnabled( bool enabled )
{
    paranoia = enabled;
}

void warning( int level, const Location& loc, const char *message, ... )
{
    if ( warningLevel <= level )
	return;
    if ( omit(message) )
	return;

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
    if ( numOmitted > 0 )
	fprintf( stderr, "qdoc warning: %d warning%s omitted\n", numOmitted,
		 numOmitted == 1 ? "" : "s" );
}
