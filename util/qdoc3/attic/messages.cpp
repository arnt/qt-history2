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
static int numAll = 0;
static int numOmitted = 0;

static int maxSame = INT_MAX;
static int maxAll = INT_MAX;

static bool tooMany( const char *message )
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
	fprintf( stderr, "qdoc: "
		 "Reached limit (%d) for this warning type:\n", maxSame );
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

static QString currentDirectory;

/*
  Level 0 is used for errors and for messages in supervisor mode.
  These are not really warnings.

  They don't go through tooMany() and it takes a warningLevel of -1
  to disable them (impossible in qdoc).
*/

void warning( int level, const Location& loc, const char *message, ... )
{
    static bool previousWasEmitted = TRUE;

    if ( message[0] == '(' ) {
	// parenthesized warnings have the same fate as the previous warning
	if ( !previousWasEmitted )
	    return;
    } else if ( warningLevel < level || (level > 0 && tooMany(message)) ) {
	previousWasEmitted = FALSE;
	return;
    }

    QString fileName = loc.shortFilePath();
    QString fileNameBase = loc.filePath();
    if ( fileName == fileNameBase ) {
	fileNameBase = "";
    } else {
	fileNameBase = fileNameBase.left( fileNameBase.length() -
					  fileName.length() - 1 );
    }

    if ( !fileNameBase.isEmpty() && currentDirectory != fileNameBase ) {
	// we changed directory; find the longest shared prefix
	int i = 0;
	int j;

	do {
	    j = currentDirectory.find( QChar('/'), i + 1 );
	    if ( j < 0 )
		j = currentDirectory.length();
	    if ( j > i &&
		 (j == (int) fileNameBase.length()
		  || fileNameBase[j] == QChar('/')) &&
		 fileNameBase.left(j) == currentDirectory.left(j) )
		i = j;
	    else
		j = -1;
	} while ( j >= 0 );

	// now, should we change to fileNameBase or to the prefix? we
	// change to the prefix if it's any good at all
	if ( i > 4 ) {
	    QString extra = fileNameBase.mid( i );
	    fileNameBase = fileNameBase.left( i );
	    if ( !extra.isEmpty() )
		fileName.prepend( extra.mid(1) + extra[0] );
	}
    }
    if ( !fileNameBase.isEmpty() &&
	 currentDirectory != fileNameBase ) {
	if ( currentDirectory.length() > 0 )
	    fprintf( stderr, "qdoc: Leaving directory `%s'\n",
		     currentDirectory.latin1() );
	currentDirectory = fileNameBase;
	fprintf( stderr, "qdoc: Entering directory `%s'\n",
		 currentDirectory.latin1() );
    }

    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "%s:%d:%d: ", fileName.latin1(), loc.lineNum(),
	     loc.columnNum() );
    vfprintf( stderr, message, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
    previousWasEmitted = TRUE;
}

void message( int level, const char *message, ... )
{
    if ( warningLevel < level )
	return;
    if ( level > 0 && tooMany(message) )
	return;

    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "qdoc %s: ", level == 0 ? "error" : "warning" );
    vfprintf( stderr, message, ap );
    fprintf( stderr, "\n" );
    va_end( ap );
}

void syswarning( const char *message, ... )
{
    va_list ap;

    va_start( ap, message );
    fprintf( stderr, "qdoc error: " );
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
	fprintf( stderr, "qdoc: Gave %d warning%s of %d type%s\n",
		 numAll - numOmitted, (numAll - numOmitted) == 1 ? "" : "s",
		 types, types == 1 ? "" : "s" );
    }
    if ( numOmitted > 0 )
	fprintf( stderr,
		 "qdoc: Omitted %d more warning%s of %d type%s\n",
		 numOmitted, numOmitted == 1 ? "" : "s",
		 omitted, omitted == 1 ? "" : "s" );
}
