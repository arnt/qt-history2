/****************************************************************************
** $Id: //depot/qt/main/src/tools/qmemchk.cpp#1 $
**
** Implementation of memory checking routines
**
** Author  : Haavard Nord
** Created : 920703
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qmemchk.h"

#if defined(CHECK_MEMORY)

RCSTAG("$Id: //depot/qt/main/src/tools/qmemchk.cpp#1 $")

#include "qglobal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_OS_MAC_)
#include <malloc.h>
#endif


static const char *memchkLogFile= "MEMCHK.LOG"; // name of log file
static int	memchkBufSize	= 8192;		// size of history buffer
static bool	memchkReporting = TRUE;		// error reporting enabled
static bool	memchkActive	= FALSE;	// memory checking enabled

static void   **ptrbuf   = 0;			// alloc history buffer
static int	bufidx	 = 0;			// index in ptrbuf
static int	badentry = -1;			// index of bad pointer entry
static FILE    *file	 = 0;			// log file


// Semaphores are used to prevent reentrant calls to new and delete
// operators (not real UNIX semaphores, of course).

static int	new_sem = 0;			// semaphore for new
static int	delete_sem = 0;			// semaphore for delete


void memchk_error( const char *msg )		// breakpoint can be set here!
{
    warning( msg );
}

void memchkSetLogFile( const char *f )		// set log file name
{
    if ( memchkActive )
	warning( "MEMCHK: Cannot set log file during memory checking" );
    else
	memchkLogFile = f;
}

void memchkSetBufSize( int s )			// set internal buffer size
{
    if ( memchkActive )
	warning( "MEMCHK: Cannot modify buffer size during memory checking" );
    else
	memchkBufSize = s;
}

bool memchkSetReporting( bool enable )		// report memory errors
{
    bool v = memchkReporting;
    memchkReporting = enable;
    return v;
}

int memchkAllocCount()				// return number of allocs
{
    return bufidx;
}


static uint find_ptr( void *p )			// find pointer
{
    register int index;
    index = bufidx - 1;
    while ( index > 0 && ptrbuf[index] != p )	// search backwards in ptrbuf
	index--;
    return index;
}


void *operator new( uint size )			// allocate pointer
{
    void *p = malloc( size );
    if ( !memchkActive || new_sem )		// checking disabled
	return p;
    new_sem++;					// avoid reentrant calls
    ptrbuf[bufidx] = p;
    if ( file ) {				// give message if bad pointer
	while ( (bufidx > badentry) && !feof(file) )
	    fscanf( file, "%d ", &badentry );
	if ( bufidx == badentry && memchkReporting ) {
	    char buf[80];
	    sprintf( buf, "MEMCHK: Pointer alloc'd but never deleted: %p",
			  ptrbuf[bufidx] );
	    memchk_error( buf );
	}
    }
    bufidx++;
    if ( bufidx >= memchkBufSize ) {
	fatal( "MEMCHK: Internal buffer overflow" );
	memchkActive = 0;
    }
    new_sem--;					// now we are safe
    return p;
}


void operator delete( void *p )			// deallocate pointer
{
    free( (char *)p );
    if ( !memchkActive || delete_sem || p==0 )	// checking disabled
	return;
    delete_sem++;				// avoid reentrant calls
    int index = find_ptr( p );
    if ( ptrbuf[index] == p && p )		// pointer was allocated
	ptrbuf[index] = 0;
    else if ( memchkReporting ) {
	char buf[80];
	sprintf( buf, "MEMCHK: Attempt to delete invalid pointer: %p", p );
	memchk_error( buf );
    }
    delete_sem--;
}


void memchkStart()				// start memory checking
{
    if ( memchkActive )				// already active
	return;
    new_sem = 1;				// disable new detection
    delete_sem = 1;				// disable delete detection
    memchkActive = TRUE;			// checking enabled
    ptrbuf = (void **)malloc( memchkBufSize*sizeof(void*) );
    file = fopen( memchkLogFile, "r" );		// try to open log file
    if ( file ) {				// it exists...
	int nallocs;				//   then run in playback mode
	fscanf( file, "%d", &nallocs );
	if ( memchkReporting )
	    warning( "MEMCHK: %d pointers were allocated", nallocs );
    }
    new_sem = 0;				// enable tracing
    delete_sem = 0;
}


void memchkStop()				// stop memory checking
{
    if ( !memchkActive )
	return;
    new_sem = 1;				// disable tracing
    delete_sem = 1;
    register int index = 0;
    if ( !file ) {				// write recorded info to file
	file = fopen( memchkLogFile, "w" );
	fprintf( file, "%d\n", bufidx );
	while ( index < bufidx ) {
	    if ( ptrbuf[index] )
		fprintf( file, "%d\t", index );
	    index++;
	}
	fclose( file );
    }
    free( (char*)ptrbuf );
    memchkActive = FALSE;
    new_sem = 0;				// enable tracing
    delete_sem = 0;
}


#else // !CHECK_MEMORY


// Dummy functions when memory checking is not implemented

void memchk_error( const char * )
{
}

void memchkSetLogFile( const char * )
{
}

void memchkSetBufSize( int )
{
}

bool memchkSetReporting( bool )
{
    return FALSE;
}

int memchkAllocCount()
{
    return -1;
}

void memchkStart()
{
#if defined(DEBUG)
    warning( "MEMCHK: Memory checking not provided by library" );
#endif
}

void memchkStop()
{
}


#endif // CHECK_MEMORY
