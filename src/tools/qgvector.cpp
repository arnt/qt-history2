/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgvector.cpp#1 $
**
** Implementation of QGVector class
**
** Author  : Haavard Nord
** Created : 930907
**
** Copyright (C) 1993,1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The internal array is normally allocated with malloc(), deallocated with
** free() and resized with realloc(). Overloaded global new and delete oper-
** ators will not be called!
** There are two flags you can set to customize memory management.
**
** USE_MALLOC is defined by default. If you comment it, new and delete will
** be used insted of malloc() and free(). There might be a problem when using
** realloc() on memory that has been allocated with new. Problems on Mac have
** been detected.
**
** DONT_USE_REALLOC can be defined to make manual array reallocations. It only
** makes sense when USE_MALLOC is commented out.
*****************************************************************************/

#define QGVECTOR_C
#define NO_WARNINGS
#include "qgvector.h"
#include "qglist.h"
#include "qstring.h"
#include "qstream.h"
#include <stdlib.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qgvector.cpp#1 $";
#endif


#if defined(NEW) || defined(DELETE)
// #error Macro already defined
#endif


#if !defined(TRACE_FS)
#define USE_MALLOC				// comment to use new/delete
#endif

#if defined(USE_MALLOC)
#if !defined(_OS_MAC_) && !defined(VXWORKS)
#include <malloc.h>
#endif
#define NEW(type,size)	((type*)malloc(size*sizeof(type)))
#define DELETE(array)	(free((char*)array))
#else
#define NEW(type,size)	(new type[size])
#define DELETE(array)	(delete[] array)
// #define DONT_USE_REALLOC			// uncomment to avoid realloc()
#endif

#if !defined(USE_MALLOC) || defined(_OS_MAC_)
#define DONT_USE_REALLOC
#endif


// --------------------------------------------------------------------------
// Default implementation of virtual functions
//

int QGVector::compareItems( GCI d1, GCI d2 )
{
    return d1 != d2;				// compare pointers
}

QStream &QGVector::read( QStream &s, GCI &d )	// read item from stream
{
    d = 0;
    return s;
}

QStream &QGVector::write( QStream &s, GCI ) const // write item to stream
{
    return s;
}


// --------------------------------------------------------------------------
// QGVector member functions
//

QGVector::QGVector()				// create empty vector
{
    vec = 0;
    len = numItems = 0;
}

QGVector::QGVector( uint size )			// create vectors with nullptrs
{
    len = size;
    numItems = 0;
    if ( len == 0 ) {				// zero length
	vec = 0;
	return;
    }
    vec = NEW(GCI,len);
    CHECK_PTR( vec );
    memset( (void*)vec, 0, len*sizeof(GCI) );	// fill with nulls
}

QGVector::QGVector( const QGVector &a )		// make copy of other vector
{
    len = a.len;
    numItems = a.numItems;
    vec = NEW(GCI,len);
    CHECK_PTR( vec );
    for ( uint i=0; i<len; i++ ) {
	vec[i] = a.vec[i] ? newItem( a.vec[i] ) : 0;
	CHECK_PTR( vec[i] );
    }
}

QGVector::~QGVector()
{
    clear();
}


QGVector& QGVector::operator=( const QGVector &v )
{						// assign from other vector
    clear();					// first delete old vector
    len = v.len;
    numItems = v.numItems;
    vec = NEW(GCI,len);				// create new vector
    CHECK_PTR( vec );
    for ( uint i=0; i<len; i++ ) {		// copy elements
	vec[i] = v.vec[i] ? newItem( v.vec[i] ) : 0;
	CHECK_PTR( vec[i] );
    }
    return *this;
}


bool QGVector::insert( uint index, GCI d )	// insert item at index
{
#if defined(CHECK_RANGE)
    if ( index >= len ) {			// range error
	warning( "QGVector::insert: Index %d out of range", index );
	return FALSE;
    }
#endif
    if ( vec[index] ) {				// remove old item
	deleteItem( vec[index] );
	numItems--;
    }
    if ( d ) {
	vec[index] = newItem( d );
	CHECK_PTR( vec[index] );
	numItems++;
	return vec[index] != 0;
    }
    else
	vec[index] = 0;				// reset item
    return TRUE;
}

bool QGVector::remove( uint index )		// remove item at index
{
#if defined(CHECK_RANGE)
    if ( index >= len ) {			// range error
	warning( "QGVector::remove: Index %d out of range", index );
	return FALSE;
    }
#endif
    if ( vec[index] ) {				// valid item
	deleteItem( vec[index] );		// delete it
	vec[index] = 0;				// reset pointer
	numItems--;
    }
    return TRUE;
}

GCI QGVector::take( uint index )		// take out item
{
#if defined(CHECK_RANGE)
    if ( index >= len ) {			// range error
	warning( "QGVector::take: Index %d out of range", index );
	return 0;
    }
#endif
    GCI d = vec[index];				// don't delete item
    if ( d )
	numItems--;
    vec[index] = 0;
    return d;
}


void QGVector::clear()				// clear vector
{
    if ( vec ) {
	for ( uint i=0; i<len; i++ ) {		// delete each item
	    if ( vec[i] )
		deleteItem( vec[i] );
	}
	DELETE(vec);
	vec = 0;
	len = numItems = 0;
    }
}

bool QGVector::resize( uint newsize )		// resize array
{
    if ( newsize == len )			// nothing to do
	return TRUE;
    if ( vec ) {				// existing data
	if ( newsize < len ) {			// shrink vector
	    uint i = newsize;
	    while ( i < len ) {			// delete lost items
		if ( vec[i] ) {
		    deleteItem( vec[i] );
		    numItems--;
		}
		i++;
	    }
	}
	if ( newsize == 0 ) {			// vector becomes empty
	    DELETE(vec);
	    vec = 0;
	    len = numItems = 0;
	    return TRUE;
	}
#if defined(DONT_USE_REALLOC)
	GCI *newvec = NEW(GCI,newsize);		// manual realloc
	memcpy( newvec, vec, (len < newsize ? len : newsize)*sizeof(GCI) );
	DELETE(vec);
	vec = newvec;
#else
	vec = (GCI*)realloc( (char *)vec, newsize*sizeof(GCI) );
#endif
    }
    else {					// create new vector
	vec = NEW(GCI,newsize);
	len = numItems = 0;
    }
    CHECK_PTR( vec );
    if ( !vec )					// no memory
	return FALSE;
    if ( newsize > len )			// init extra space added
	memset( (void*)&vec[len], 0, (newsize-len)*sizeof(GCI) );
    len = newsize;
    return TRUE;
}


bool QGVector::fill( GCI d, int flen )		// resize and fill vector
{
    if ( flen < 0 )
	flen = len;				// default: use vector length
    else
    if ( !resize( flen ) )
	return FALSE;
    for ( uint i=0; i<(uint)flen; i++ )		// insert d at every index
	insert( i, d );
    return TRUE;
}


void QGVector::sort()				// sort vector
{
    int i = len - 1;
    while ( i > 0 && vec[i] == 0 )		// skip nulls items at end
	i--;
    if ( i > 0 )
	qsort( &vec[0], &vec[i+1] );
}

void QGVector::qsort( GCI *n1, GCI *n2 )	// quicksorts vector
{
    register GCI *i;
    register GCI *j;
    GCI tmp;
    while ( n2 > n1 + 1 ) {
	i = n1;
	j = n2;
	while ( TRUE ) {
	    if ( !*n1 )				// take null items into account
		i = n2;
	    else
	    while ( ++i < n2 ) {
		if ( !*i || compareItems( *i, *n1 ) > 0 )
		    break;
	    }
	    if ( !*n1 ) {
		while ( --j > n1 && !*j )
		    ;
	    }
	    else
	    while ( --j > n1 ) {		// same as above; reversed
		if ( !*j )
		    continue;
		else
		if ( compareItems( *j, *n1 ) < 0 )
		    break;
	    }
	    if ( i >= j )
		break;
	    tmp = *i;				// swap items
	    *i = *j;
	    *j = tmp;
	}
	if ( j == n1 ) {
	    n1++;
	    continue;
	}
	tmp = *n1;				// swap items
	*n1 = *j;
	*j = tmp;
	if ( j - n1 < n2 - j - 1 ) {
	    if ( j - n1 > 1 )
		qsort( n1, j );			// quicksort left items
	    n1 = j + 1;
	}
	else {
	    if ( n2 > j + 2 )
		qsort( j + 1, n2 );		// quicksort right items
	    n2 = j;
	}
    }
}

int QGVector::bsearch( GCI d ) const		// binary search; when sorted
{
    if ( !len )
	return -1;
    if ( !d ) {
#if defined(CHECK_NULL)
	warning( "QGVector::bsearch: Cannot search for null object" );
#endif
	return -1;
    }
    int n1 = 0;
    int n2 = len - 1;
    int mid;
    while ( n1 <= n2 ) {
	int  res;
	mid = (n1 + n2)/2;
	if ( vec[mid] == 0 )			// null item greater
	    res = -1;
	else
	    res = ((QGVector*)this)->compareItems( d, vec[mid] );
	if ( res < 0 )
	    n2 = mid - 1;
	else
	if ( res > 0 )
	    n1 = mid + 1;
	else					// found it
	    return mid;
    }
    return -1;
}


int QGVector::findRef( GCI d, uint index) const // find exact item in vector
{
#if defined(CHECK_RANGE)
    if ( index >= len ) {			// range error
	warning( "QGVector::findRef: Index %d out of range", index );
	return 0;
    }
#endif
    for ( uint i=index; i<len; i++ ) {
	if ( vec[i] == d )
	    return i;
    }
    return -1;
}

int QGVector::find( GCI d, uint index ) const	// find equal item in vector
{
#if defined(CHECK_RANGE)
    if ( index >= len ) {			// range error
	warning( "QGVector::find: Index %d out of range", index );
	return 0;
    }
#endif
    for ( uint i=index; i<len; i++ ) {
	if ( vec[i] == 0 && d == 0 )		// found null item
	    return i;
	if ( vec[i] && ((QGVector*)this)->compareItems( vec[i], d ) == 0 )
	    return i;
    }
    return -1;
}

uint QGVector::containsRef( GCI d ) const	// get number of exact matches
{
    uint count = 0;
    for ( uint i=0; i<len; i++ ) {
	if ( vec[i] == d )
	    count++;
    }
    return count;
}

uint QGVector::contains( GCI d ) const		// get number of equal matches
{
    uint count = 0;
    for ( uint i=0; i<len; i++ ) {
	if ( vec[i] == 0 && d == 0 )		// count null items
	    count++;
	if ( vec[i] && ((QGVector*)this)->compareItems( vec[i], d ) == 0 )
	    count++;
    }
    return count;
}


GCI QGVector::at( uint index ) const		// checked indexing
{
    if ( index >= len ) {
#if defined(CHECK_RANGE)
	warning( "QGVector::operator[]: Index %d out of range", index );
#endif
	index = 0;
    }
    return vec[index];
}

bool QGVector::insertGrow( uint index, GCI d )	// insert and grow if necessary
{
    if ( index >= len ) {
	if ( !resize( index+1 ) )		// no memory
	    return FALSE;
    }
    insert( index, d );
    return TRUE;
}


void QGVector::asList( QGList &list ) const	// store items in list
{
    list.clear();
    for ( uint i=0; i<len; i++ ) {
	if ( vec[i] )
	    list.append( vec[i] );
    }
}


int QGVector::apply( GCF f, void *x ) const	// apply function to all items
{
    int res;
    for ( uint i=0; i<len; i++ ) {
	if ( res = (f)( vec[i], x ) )
	    return res;				// exit if f returns FALSE
    }
    return 0;
}


// --------------------------------------------------------------------------
// QGVector stream functions
//

QStream &operator>>( QStream &s, QGVector &vec )// read vector
{
    return vec.read( s );
}

QStream &operator<<( QStream &s, const QGVector &vec )
{						// write vector
    return vec.write( s );
}

QStream &QGVector::read( QStream &s )		// read vector from stream
{
    uint num;
    s >> num;					// read number of items
    clear();					// clear vector
    resize( num );
    for (uint i=0; i<num; i++) {		// read all items
	GCI d;
	read( s, d );
	CHECK_PTR( d );
	if ( !d )				// no memory
	    break;
	vec[i] = d;
    }
    return s;
}

QStream &QGVector::write( QStream &s ) const	// write vector to stream
{
    uint num = count();
    s << num;					// number of items to write
    num = size();
    for (uint i=0; i<num; i++) {		// write non-null items
	if ( vec[i] )
	    write( s, vec[i] );
    }
    return s;
}
