/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgarray.cpp#16 $
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
** DONT_USE_REALLOC can be defined to make manual array reallocations.	This
** flag is defined when using new and delete (i.e. not USE_MALLOC).
*****************************************************************************/

#define	 QGARRAY_CPP
#include "qgarray.h"
#include <string.h>
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qgarray.cpp#16 $";
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

/*! \class QArray qarray.h

  \ingroup tools

  \brief The QArray class is a template class for arrays (even where
  the compiler does not support templates).

  It provides an array of simple types, where "simple" means a single
  word.

  \todo proper docs */

/*! \class QGArray qgarray.h

  \brief The QGArray class is a generic array, intended to be
  subclassed by real array classes such as QArray.

  It provides <em>no</em> type checking and other new-fangled luxury.
  See the entry for Real Programmer in <i>The New Hackers
  Dictionary,</i> and please consider using QArray instead.

  \sa QGCache QGDict QGList QGVector */

/*! Creates an empty QGArray. */

QGArray::QGArray()				// create empty array
{
    p = newData();
    CHECK_PTR( p );
}

/*! Dummy constructor; does nothing.

  There has to be a default constructor, but for this class, the
  default constructor has no meaning.  If you don't understand, do
  yourself a favour and use QArray.  If you do understand, you know
  why you should use QArray.

  \sa QArray */

QGArray::QGArray( int, int )			// dummy; does not alloc
{
}

/*! Creates an array with size \e size bytes. */

QGArray::QGArray( int size )			// allocate room
{
    if ( size < 0 ) {
#if defined(CHECK_RANGE)
	warning( "QGArray: Cannot allocate array with negative length" );
#endif
	size = 0;
    }
    p = newData();
    CHECK_PTR( p );
    if ( size == 0 )				// zero length
	return;
    p->data = NEW(char,size);
    CHECK_PTR( p->data );
    p->len = size;
}

/*! Creates a shallow copy of \e a */

QGArray::QGArray( const QGArray &a )		// shallow copy
{
    p = a.p;
    p->ref();
}

/*! Deletes the array, and the data too unless there are other QGArrays
  around that reference the same data. */

QGArray::~QGArray()
{
    if ( p && p->deref() ) {			// delete when last reference
	if ( p->data )				// is lost
	    DELETE(p->data);
	deleteData( p );
    }
}

/*! Returns TRUE if this array and \e a are equal.  (The comparison is
  bitwise, of course.) */


bool QGArray::isEqual( const QGArray &a ) const // is arrays equal to a
{
    if ( size() != a.size() )			// different size
	return FALSE;
    if ( data() == a.data() )			// has same data
	return TRUE;
    return (size() ? memcmp( data(), a.data(), size() ) : 0) == 0;
}

/*! Changes the size of the array to \e newsize bytes.

  \todo check out possible memory loss involving realloc() */

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

/*! Resizes the array and fills it with repeated occurences of \e d,
  which is \e sz bytes long.

  If \e len is zero or positive, the array is resized to \e len*sz
  bytes.  If \e len is negative, the array is resized to the greatest
  multiple of sz which is not greater than \e len.

  Returns TRUE if the operation succeeds, FALSE if it runs out of
  memory.

  \sa resize() */

bool QGArray::fill( const char *d, int len, uint sz )
{						// resize and fill array
    if ( len < 0 )
	len = p->len/sz;			// default: use array length
    else if ( !resize( len*sz ) )
	return FALSE;
    if ( sz == 1 )				// 8 bit elements
	memset( data(), *d, len );
    else if ( sz == 4 ) {			// 32 bit elements
	register INT32 *x = (INT32*)data();
	INT32 v = *((INT32*)d);
	while ( len-- )
	    *x++ = v;
    }
    else if ( sz == 2 ) {			// 16 bit elements
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

/*! Makes this array into a shallow copy of \e a.

  \sa duplicate() operator= */

QGArray &QGArray::assign( const QGArray &a )
{						// shallow copy
    a.p->ref();					// avoid 'a = a'
    if ( p->deref() ) {				// delete when last reference
	if ( p->data )				// is lost
	    DELETE(p->data);
	deleteData( p );
    }
    p = a.p;
    return *this;
}

/*! Makes this array into a shallow copy of the \e len bytes at
  address \e d.

  \sa duplicate() operator= */

QGArray &QGArray::assign( const char *d, uint len )
{						// shallow copy
    if ( p->count > 1 ) {			// disconnect this
	p->count--;
	p = newData();
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

/*! Makes this array into a deep copy of \e a.

  \sa assign() operator= */

QGArray &QGArray::duplicate( const QGArray &a ) // deep copy
{
    if ( a.p == p ) {				// a.duplicate(a) !
	if ( p->count > 1 ) {
	    p->count--;
	    register array_data *n = newData();
	    CHECK_PTR( n );
	    if ( (n->len=p->len) ) {
		n->data = NEW(char,n->len);
		CHECK_PTR( n->data );
		if ( n->data )
		    memcpy( n->data, p->data, n->len );
	    }
	    else
		n->data = 0;
	    p = n;
	}
	return *this;
    }
    char *oldptr = 0;
    if ( p->count > 1 ) {			// disconnect this
	p->count--;
	p = newData();
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

/*! Makes this array into a deep copy of the \e len bytes at
  address \e d.

  \sa assign() operator= */

QGArray &QGArray::duplicate( const char *d, uint len )
{						// deep copy
    bool overlap = d >= p->data && d < p->data + p->len;
    char *oldptr = 0;
    bool null = !(d && len);
    if ( p->count > 1 ) {			// disconnect this
	p->count--;
	p = newData();
	CHECK_PTR( p );
    }
    else {					// just a single reference
	if ( len == p->len && !null ) {		// same size; copy the data
	    if ( overlap )
		memmove( p->data, d, len );
	    else
		memcpy( p->data, d, len );
	    return *this;
	}
	if ( overlap )
	    oldptr = p->data;
	else if ( p->data )
	    DELETE(p->data);
    }
    if ( null ) {				// null value
	p->data = 0;
	p->len  = 0;
    }
    else {					// duplicate data
	p->data = NEW(char,len);
	CHECK_PTR( p->data );
	p->len = len;
	if ( p->data ) {
	    if ( overlap )
		memmove( p->data, d, len );
	    else
		memcpy( p->data, d, len );
	}
    }
    if ( oldptr )
	DELETE(oldptr);
    return *this;
}

/*! Resizes this array to \e len bytes and copies the \e len bytes at
  address \e into it.

  \warning This function disregards the reference count mechanism.  If
  other QGArrays reference the same data as this, all will be updated.

  */

void QGArray::store( const char *d, uint len )
{						// store, but not deref
    resize( len );
    memcpy( p->data, d, len );
}

/*! Searches for and returns the address of the first occurence of the
  \e sz bytes at \e d at or after position \e index of this array.

  Note that \e index is given in units of \e sz, not bytes.

  This function only compares whole cells, not bytes.  It is not like
  strstr.  If sz is 4, the raw contents of the array is "bananana",
  and you search for "nana", you will get a match at position 1, which
  translates to byte 4. */

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
    return i<p->len ? (int)ii : -1;
}

/*! Returns the number of occurences of the \e sz bytes at \e d in
  this array.

  IF you have an 144-byte array containing only null bytes and ask for
  the number of occurences of the 32-bit word 0 (ie. \e d points to 4
  null bytes and \e sz is 4) you will get 36. */

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

/*! Returns a pointer to the byte at offset \e index of this array. */

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

/*! Expand the array if necessary, and copies (the first part of) its
  contents from the \e index*zx bytes at \e d.

  Returns TRUE if the operation succeeds, FALSE if it runs out of
  memory.

  \warning This function disregards the reference count mechanism.  If
  other QGArrays reference the same data as this, all will be changed.

  */

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
