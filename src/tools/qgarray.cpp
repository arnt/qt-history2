/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgarray.cpp#8 $
**
** Implementation of QGArray class
**
** Author  : Haavard Nord
** Created : 930906
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** The internal array is normally allocated with malloc(), deallocated with
** free() and resized with realloc(). Overloaded global new and delete oper-
** ators will not be called!
** There are two flags you can set to customize memory management.
**
** USE_MALLOC is defined by default. If you comment it, new and delete will
** be used insted of malloc() and free(). The advantage of using malloc() and
** free() is that realloc() will be used to resize arrays.
**
** DONT_USE_REALLOC can be defined to make manual array reallocations.  This
** flag is defined when using new and delete (i.e. not USE_MALLOC).
*****************************************************************************/

#define QGARRAY_CPP
#include "qgarray.h"
#include <string.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qgarray.cpp#8 $";
#endif


#if !defined(CHECK_MEMORY)
#define USE_MALLOC				// comment to use new/delete
#endif

#undef NEW
#undef DELETE

#if defined(USE_MALLOC)
#if !defined(_OS_MAC_) && !defined(VXWORKS)
#include <malloc.h>
#endif
#define NEW(type,size)	((type*)malloc(size*sizeof(type)))
#define DELETE(array)	(free((char*)array))
#else
#define NEW(type,size)	(new type[size])
#define DELETE(array)	(delete[] array)
#define DONT_USE_REALLOC			// comment to use realloc()
#endif


QGArray::QGArray()				// create empty array
{
    p = new array_data;
    CHECK_PTR( p );
}

QGArray::QGArray( int size )			// allocate room
{
    if ( size < 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGArray: Cannot allocate array with negative length" );
#endif
	size = 0;
    }
    p = new array_data;
    CHECK_PTR( p );
    if ( size == 0 )				// zero length
	return;
    p->data = NEW(char,size);
    CHECK_PTR( p->data );
    p->len = size;
}

QGArray::QGArray( const QGArray &a )		// shallow copy
{
    p = a.p;
    p->ref();
}

QGArray::~QGArray()
{
    if ( p->deref() ) {				// delete when last reference
	if ( p->data )				// is lost
	    DELETE(p->data);
	delete p;
    }
}


bool QGArray::isEqual( const QGArray &a ) const // is arrays equal to a
{
    if ( size() != a.size() )			// different size
	return FALSE;
    if ( data() == a.data() )			// has same data
	return TRUE;
    return (size() ? memcmp( data(), a.data(), size() ) : 0) == 0;
}


bool QGArray::resize( uint newsize )		// resize array
{
    if ( newsize == p->len )			// nothing to do
	return TRUE;
    if ( newsize == 0 ) {			// remove array
	duplicate( 0, 0 );
	return TRUE;
    }
    if ( p->data ) {				// existing data
#if defined(DONT_USE_REALLOC)
	char *newdata = NEW(char,newsize);	// manual realloc
	memcpy( newdata, p->data, p->len < newsize ? p->len : newsize );
	DELETE(p->data);
	p->data = newdata;
#else
	p->data = (char *)realloc( p->data, newsize );
#endif
    }
    else
	p->data = NEW(char,newsize);
    CHECK_PTR( p->data );
    if ( !p->data )				// no memory
	return FALSE;
    p->len = newsize;
    return TRUE;
}


bool QGArray::fill( const char *d, int len, uint sz )
{						// resize and fill array
    if ( len < 0 )
	len = p->len/sz;			// default: use array length
    else
    if ( !resize( len*sz ) )
	return FALSE;
    if ( sz == 1 )				// 8 bit elements
	memset( data(), *d, len );		// fast and simple
    else
    if ( sz == 4 ) {				// 32 bit elements
	register INT32 *x = (INT32*)data();
	INT32 v = *((INT32*)d);
	while ( len-- )
	    *x++ = v;
    }
    else
    if ( sz == 2 ) {				// 16 bit elements
	register INT16 *x = (INT16*)data();
	INT16 v = *((INT16*)d);
	while ( len-- )
	    *x++ = v;
    }
    else {					// any other size elements
	register char *x = data();
	while ( len-- ) {			// more complicated
	    memcpy( x, d, sz );
	    x += sz;
	}
    }
    return TRUE;
}


QGArray &QGArray::assign( const QGArray &a )
{						// shallow copy
    a.p->ref();					// avoid 'a = a'
    if ( p->deref() ) {				// delete when last reference
	if ( p->data )				// is lost
	    DELETE(p->data);
	delete p;
    }
    p = a.p;
    return *this;
}

QGArray &QGArray::assign( const char *d, uint len )
{						// shallow copy
    if ( p->count > 1 ) {			// disconnect this
	p->count--;
	p = new array_data;
	CHECK_PTR( p );
    }
    else {
	if ( p->data )
	    DELETE(p->data);
    }
    p->data = (char *)d;
    p->len = len;
    return *this;
}

QGArray &QGArray::duplicate( const QGArray &a ) // deep copy
{
    char *oldptr = 0;
    if ( p->count > 1 ) {			// disconnect this
	p->count--;
	p = new array_data;
	CHECK_PTR( p );
    }
    else					// delete after copy was made
	oldptr = p->data;
    if ( a.p->len ) {				// duplicate data
	p->data = NEW(char,a.p->len);
	CHECK_PTR( p->data );
	if ( p->data )
	    memcpy( p->data, a.p->data, a.p->len );
    }
    else
	p->data = 0;
    p->len = a.p->len;
    if ( oldptr )
	DELETE(oldptr);
    return *this;
}

QGArray &QGArray::duplicate( const char *d, uint len )
{						// deep copy
    char *oldptr = 0;
    if ( p->count > 1 ) {			// disconnect this
	p->count--;
	p = new array_data;
	CHECK_PTR( p );
    }
    else					// delete after copy was made
	oldptr = p->data;
    if ( !(d && len) ) {			// null value
	p->data = 0;
	p->len = 0;
    }
    else {					// duplicate data
	p->data = NEW(char,len);
	CHECK_PTR( p->data );
	p->len = len;
	if ( p->data )
	    memcpy( p->data, d, len );
    }
    if ( oldptr )
	DELETE(oldptr);
    return *this;
}

void QGArray::store( const char *d, uint len )
{						// store, but not deref
    resize( len );
    memcpy( p->data, d, len );
}


int QGArray::find( const char *d, uint index, uint sz ) const
{
    index *= sz;
    if ( index >= p->len ) {
#if defined(CHECK_RANGE)
	warning( "QGArray::find: Index %d out of range", index/sz );
#endif
	return -1;
    }
    register uint i;
    uint ii;
    if ( sz == 1 ) {				// 8 bit elements
	register char *x = data();
	char v = *d;
	for ( i=index; i<p->len; i++ ) {
	    if ( *x++ == v )
		break;
	}
	ii = i;
    }
    else
    if ( sz == 4 ) {				// 32 bit elements
	register INT32 *x = (INT32*)(data() + index);
	INT32 v = *((INT32*)d);
	for ( i=index; i<p->len; i+=4 ) {
	    if ( *x++ == v )
		break;
	}
	ii = i/4;
    }
    else
    if ( sz == 2 ) {				// 16 bit elements
	register INT16 *x = (INT16*)(data() + index);
	INT16 v = *((INT16*)d);
	for ( i=index; i<p->len; i+=2 ) {
	    if ( *x++ == v )
		break;
	}
	ii = i/2;
    }
    else {					// any size elements
	for ( i=index; i<p->len; i+=sz ) {
	    if ( memcmp( d, &p->data[i], sz ) == 0 )
		break;
	}
	ii = i/sz;
    }
    return i<p->len ? ii : -1;
}

int QGArray::contains( const char *d, uint sz ) const
{
    register uint i = p->len;
    int count = 0;
    if ( sz == 1 ) {				// 8 bit elements
	register char *x = data();
	char v = *d;
	while ( i-- ) {
	    if ( *x++ == v )
		count++;
	}
    }
    else
    if ( sz == 4 ) {				// 32 bit elements
	register INT32 *x = (INT32*)data();
	INT32 v = *((INT32*)d);
	i /= 4;
	while ( i-- ) {
	    if ( *x++ == v )
		count++;
	}
    }
    else
    if ( sz == 2 ) {				// 16 bit elements
	register INT16 *x = (INT16*)data();
	INT16 v = *((INT16*)d);
	i /= 2;
	while ( i-- ) {
	    if ( *x++ == v )
		count++;
	}
    }
    else {					// any size elements
	for ( i=0; i<p->len; i+=sz ) {
	    if ( memcmp( d, &p->data[i], sz ) == 0 )
		count++;
	}
    }
    return count;				// number of identical objects
}


char *QGArray::at( uint index ) const		// checked indexing
{
    if ( index >= p->len ) {
#if defined(CHECK_RANGE)
	warning( "QGArray::operator[]: Absolute index %d out of range", index);
#endif
	index = 0;				// try to recover
    }
    return &p->data[index];
}

bool QGArray::setExpand( uint index, const char *d, uint sz )
{						// set and expand if necessary
    index *= sz;
    if ( index >= p->len ) {
	if ( !resize( index+sz ) )		// no memory
	    return FALSE;
    }
    memcpy( data() + index, d, sz );
    return TRUE;
}
