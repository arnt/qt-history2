/****************************************************************************
** $Id: //depot/qt/main/src/tools/qnpipe.cpp#1 $
**
** Implementation of QNPipe class
**
** Author  : Haavard Nord
** Created : 940921
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qnpipe.h"

#undef MKNOD_NPIPE
#undef WIN32_NPIPE
#undef OS2_NPIPE

#define HAS_NPIPE_API

#if defined(UNIX)
#define MKNOD_NPIPE
#elif defined(_OS_WIN32_)
#define WIN32_NPIPE
#elif defined(_OS_OS2_)
#define OS2_NPIPE
#else
#undef HAS_NPIPE_API
#endif

#if defined(HAS_NPIPE_API)
#include <errno.h>

#if defined(MKNOD_NPIPE)
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#elif defined(WIN32_NPIPE)

#elif defined(OS2_NPIPE)
#endif

RCSTAG("$Id: //depot/qt/main/src/tools/qnpipe.cpp#1 $")


#if defined(MKNOD_NPIPE)

// -----------------------------------------------------------------------
// QNPipe implementation based on UNIX FIFOs, using mknod()
//

QNPipe::QNPipe()
{
    readfd = writefd = -1;
    isserver = FALSE;
}

QNPipe::~QNPipe()
{
    disconnect();
}


static void get_fifo_names( char *n, QString &fifo_r, QString &fifo_w )
{
    fifo_r.sprintf( "/tmp/%s.r", n );
    fifo_w.sprintf( "/tmp/%s.w", n );
}

bool QNPipe::listen()				// create FIFOs
{
    QString fifo_w;				// FIFO for writing
    QString fifo_r;				// FIFO for reading
    int rperms = 0666;				// read permissions
    int wperms = 0666;				// write permissions
    int err;
    if ( state() != 0 ) {
	return FALSE;
    }
    get_fifo_names( sap(), fifo_r, fifo_w );
    err = (mknod(fifo_w.data(), S_IFIFO | wperms, 0) < 0) && (errno != EEXIST);
    if ( err ) {
#if defined(CHECK_NULL)
	warning( "QNPipe::listen: Cannot create FIFO for writing" );
#endif
	return FALSE;
    }
    err = (mknod(fifo_r.data(), S_IFIFO | rperms, 0) < 0) && (errno != EEXIST);
    if ( err ) {
#if defined(CHECK_NULL)
	warning( "QNPipe::listen: Cannot create FIFO for reading" );
#endif
	::unlink( fifo_w.data() );
	return FALSE;
    }
    writefd = ::open( fifo_w.data(), 1 );
    if ( writefd < 0 ) {
#if defined(CHECK_NULL)
	warning( "QNPipe::listen: Cannot open FIFO for writing" );
#endif
	return FALSE;
    }
    readfd = ::open( fifo_r.data(), 0 );
    if ( readfd < 0 ) {
#if defined(CHECK_NULL)
	warning( "QNPipe::listen: Cannot open FIFO for reading" );
#endif
	return FALSE;
    }
    setState( IO_Connected );
    setMode( IO_ReadWrite | IO_Raw );
    isserver = TRUE;
    if ( isBuffered() )
	buffer.open( IO_ReadWrite );
    return TRUE;
}


void QNPipe::hangup()				// remove FIFOs
{
    QString fifo_r;				// FIFO for reading
    QString fifo_w;				// FIFO for writing
    if ( !isserver )				// did not create them
	return;
    disconnect();
    get_fifo_names( sap(), fifo_r, fifo_w );
    ::close( readfd );				// first close'em
    ::close( writefd );
    unlink( (const char *)fifo_r );		// then remove'em
    unlink( (const char *)fifo_w );
    isserver = FALSE;
    setState( 0 );
}


bool QNPipe::connect( int mode )		// connect to r/w FIFO
{
    if ( isserver || isConnected() )
	return FALSE;
    QString fifo_r;				// FIFO for reading
    QString fifo_w;				// FIFO for writing
    get_fifo_names( sap(), fifo_w, fifo_r );
    readfd = ::open( fifo_r.data(), 0 );
    if ( readfd < 0 ) {
#if defined(CHECK_NULL)
	warning( "QNPipe::connect: Cannot open FIFO for reading" );
#endif
	return FALSE;
    }
    writefd = ::open( fifo_w.data(), 1 );
    if ( writefd < 0 ) {
#if defined(CHECK_NULL)
	warning( "QNPipe::connect: Cannot open FIFO for writing" );
#endif
	return FALSE;
    }
    setState( IO_Connected );
    setMode( mode );
    if ( isBuffered() )
	buffer.open( IO_ReadWrite );
    return TRUE;
}


bool QNPipe::disconnect()			// close FIFOs
{
    if ( !isConnected() )
	return FALSE;
    ::close( readfd );
    ::close( writefd );
    setState( 0 );
    setMode( 0 );
    if ( isBuffered() )
	buffer.close();				// close buffer
    return TRUE;
}

void QNPipe::abort()				// same as close
{
    disconnect();
}


bool QNPipe::transmit()				// transmit buffered data
{
    if ( isRaw() )				// no flushing for raw device
	return TRUE;
    setMode( mode() | IO_Raw );			// fake mode
    bool res;
    res = writeBlock( buffer.buffer().data(), buffer.size() ) == buffer.size();
    setMode( mode() & ~IO_Raw );		// restore from fake mode
    return res;
}


int QNPipe::readBlock( char *data, uint len )
{
    if ( !isConnected() )			// not connected
	return -1;
    int nleft = (int)len;			// number of bytes to read
    int nread = 0;
    int ntotal = 0;
    while ( nleft > 0 ) {			// there's more to read
	nread = ::read( readfd, data, nleft );
	if ( nread == -1 ) {			// read error
	    setStatus( IO_ReadError );
	    return -1;
	}
	else if ( nread == 0 ) {		// connection broken (EOF)
	    // !!! Generate callback message
	    setStatus( IO_AbortError );
	    return -1;
	}
	nleft -= nread;
	data += nread;
	ntotal += nread;
	if ( !readComplete() )
	    return nread;
    }
    return ntotal;
}


int QNPipe::writeBlock( const char *data, uint len )
{
    if ( !isConnected() )			// not connected
	return -1;
    if ( isBuffered() )
	return buffer.writeBlock( data, len );
    int nleft = (int)len;			// number of bytes to write
    int nwritten;
    while ( nleft > 0 ) {			// there's more to write
	nwritten = ::write( writefd, data, nleft );
	if ( nwritten == -1 ) {
	    setStatus( IO_WriteError );
	    return -1;
	}
	nleft -= nwritten;
	data += nwritten;
    }
    return (int)len;
}


#endif // MKNOD_NPIPE


#endif // HAS_NPIPE_API
