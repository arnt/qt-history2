/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgdict.cpp#8 $
**
** Implementation of QGDict and QGDictIterator classes
**
** Author  : Haavard Nord
** Created : 920529
**
** Copyright (C) 1992-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qgdict.h"
#include "qlist.h"
#if defined(DEBUG)
#include "qstring.h"
#endif
#include "qdstream.h"
#include <ctype.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/tools/qgdict.cpp#8 $";
#endif


declare(QListM,QGDictIterator);			// list of iterators (Qditlst)

// --------------------------------------------------------------------------
// Default implementation of virtual functions
//

int QGDict::hashKey( const char *key )		// make hash value
{
    register int index = 0;
    register const char *k = key;
#if defined(CHECK_NULL)
    if ( key == 0 )
	warning( "QGDict::hash: Invalid NULL key" );
#endif
    if ( cases ) {				// case sensitive
	while ( *k )
	    index = index << 1 ^ *k++;
    }
    else {					// case insensitive
	while ( *k ) {
	    index = index << 1 ^ tolower(*k);
	    k++;
	}
    }
    if ( index < 0 )				// adjust index to table size
	index = -index;
    return index;
}


QDataStream& QGDict::read( QDataStream &s, GCI &d )
{						// read item from stream
    d = 0;
    return s;
}

QDataStream& QGDict::write( QDataStream &s, GCI ) const
{						// write item to stream
    return s;
}


// --------------------------------------------------------------------------
// Qbucket class (internal hash node)
//

class Qbucket
{
public:
    char   *getKey()		{ return key; }
    char   *setKey( char *k )	{ return key = k; }
    GCI	    getData()		{ return data; }
    GCI	    setData( GCI d )	{ return data = d; }
    Qbucket *getNext()		{ return next; }
    void    setNext( Qbucket *n){ next = n; }
private:
    char   *key;
    GCI	    data;
    Qbucket *next;
};


// --------------------------------------------------------------------------
// QGDict member functions
//

QGDict::QGDict( uint sz, bool cs, bool ck, bool th )
{
    vec = new Qbucket *[vlen = sz];		// allocate hash table
    CHECK_PTR( vec );
    memset( (char*)vec, 0, vlen*sizeof(Qbucket*) );
    numItems = 0;
    cases = cs;
    copyk = ck;
    trivial = th;
    if ( trivial )				// copyk must be FALSE for
	copyk = FALSE;				//   int-hashed dicts
    iterators = 0;
}

QGDict::~QGDict()
{
    clear();					// delete everything
    delete vec;
    if ( !iterators )				// no iterators for this dict
	return;
    register QGDictIterator *i = iterators->first();
    while ( i ) {				// notify all iterators that
	i->dict = 0;				// this dict is deleted
	i = iterators->next();
    }
    delete iterators;
}


GCI QGDict::look( const char *key, GCI d, bool ins )
{						// find/insert item
    register Qbucket *n;
    int	 index;
    if ( trivial ) {				// when key is not a string
	index = (int)(long(key) % vlen);	// don't call virtual hash func
	if ( !ins ) {				// find item in list
	    for ( n=vec[index]; n; n=n->getNext() ) {
		if ( n->getKey() == key )
		    return n->getData();	// item was found
	    }
	    return 0;				// did not find the item
	}
    }
    else {					// key is a string
	index = hashKey( key ) % vlen;
	if ( !ins ) {				// find item in list
	    for ( n=vec[index]; n; n=n->getNext() ) {
		if ( (cases ? strcmp(n->getKey(),key)
			 : stricmp(n->getKey(),key)) == 0 )
		    return n->getData();	// item was found
	    }
	    return 0;				// did not find the item
	}
    }
    Qbucket *node = new Qbucket;		// insert new node
    CHECK_PTR( node );
    if ( !node )				// no memory
	return 0;
    node->setKey( (char *)(copyk ? strdup(key) : key) );
    node->setData( newItem(d) );
    CHECK_PTR( node->getData() );
    node->setNext( vec[index] );		// link node into table
    vec[index] = node;
    numItems++;
    return node->getData();
}


Qbucket *QGDict::unlink( const char *key )
{
    if ( numItems == 0 )			// nothing in dictionary
	return 0;
    register Qbucket *n;
    Qbucket *prev = 0;
    int index;
    if ( trivial )
	index = (int)(long(key) % vlen);
    else
	index = hashKey( key ) % vlen;
    for ( n=vec[index]; n; n=n->getNext() ) {	// find item in list
	bool equal;
	if ( trivial )
	    equal = n->getKey() == key;
	else
	    equal = (cases ? strcmp(n->getKey(),key)
			   : stricmp(n->getKey(),key)) == 0;
	if ( equal ) {				// found key to be removed
	    if ( iterators ) {			// update iterators
		register QGDictIterator *i = iterators->first();
		while ( i ) {			// fix all iterators that
		    if ( i->curNode == n )	// refers to pending node
			i->operator++();
		    i = iterators->next();
		}
	    }
	    if ( prev )				// unlink node
		prev->setNext( n->getNext() );
	    else
		vec[index] = n->getNext();
	    numItems--;
	    return n;
	}
	prev = n;
    }
    return 0;
}

bool QGDict::remove( const char *key )		// remove item from dictionary
{
    register Qbucket *n = unlink( key );
    if ( n ) {
	if ( copyk )
	    delete n->getKey();
	deleteItem( n->getData() );
	delete n;				// delete bucket
    }
    return n != 0;
}

GCI QGDict::take( const char *key )		// take out item
{
    register Qbucket *n = unlink( key );
    if ( n ) {
	if ( copyk )
	    delete n->getKey();
	delete n;
    }
    return n;
}


void QGDict::clear()				// delete all items
{
    if ( !numItems )
	return;
    register Qbucket *n;
    numItems = 0;				// disable remove() function
    for ( uint j=0; j<vlen; j++ ) {		// destroy hash table
	n = vec[j];
	while ( n ) {
	    if ( copyk )
		delete n->getKey();
	    deleteItem( n->getData() );
	    Qbucket *next = n->getNext();
	    delete n;
	    n = next;
	}
	vec[j] = 0;
    }
    if ( !iterators )				// no iterators for this dict
	return;
    register QGDictIterator *i = iterators->first();
    while ( i ) {				// notify all iterators that
	i->curNode = 0;				// this dict is empty
	i = iterators->next();
    }
}


void QGDict::statistics() const			// show statistics
{
#if defined(DEBUG)
    QString line;
    line.fill( '-', 60 );
    double real, ideal;
    debug( line );
    debug( "DICTIONARY STATISTICS:" );
    if ( count() == 0 ) {
	debug( "Empty!" );
	debug( line );
	return;
    }
    real = 0.0;
    ideal = (float)count()/(2.0*size())*(count()+2.0*size()-1);
    uint i = 0;
    while ( i<size() ) {
	Qbucket *n = vec[i];
	int b = 0;
	while ( n ) {				// count number of buckets
	    b++;
	    n = n->getNext();
	}
	real = real + (double)b * ((double)b+1.0)/2.0;
	char buf[80], *pbuf;
	if ( b > 78 )
	    b = 78;
	pbuf = buf;
	while ( b-- )
	    *pbuf++ = '*';
	*pbuf = '\0';
	debug( buf );
	i++;
    }
    debug( "Array size = %d", size() );
    debug( "# items    = %d", count() );
    debug( "Real dist  = %g", real );
    debug( "Rand dist  = %g", ideal );
    debug( "Real/Rand  = %g", real/ideal );
    debug( line );
#endif // DEBUG
}


// --------------------------------------------------------------------------
// QGDict stream functions
//

QDataStream &operator>>( QDataStream &s, QGDict &dict )
{						// read dict
    return dict.read( s );
}

QDataStream &operator<<( QDataStream &s, const QGDict &dict )
{						// write dict
    return dict.write( s );
}

QDataStream &QGDict::read( QDataStream &s )	// read dict from stream
{
    uint num;
    s >> num;					// read number of items
    clear();					// clear dict
    while ( num-- ) {				// read all items
	GCI d;
	char *k;
	if ( trivial ) {
	    long k_triv;
	    s >> k_triv;			// key is long int
	    k = (char *)k_triv;
	}
	else
	    s >> k;				// key is string
	read( s, d );				// read data
	look( k, d, TRUE );
    }
    return s;
}

QDataStream& QGDict::write( QDataStream &s ) const
{						// write dict to stream
    s << count();				// write number of items
    uint i = 0;
    while ( i<size() ) {
	Qbucket *n = vec[i];
	while ( n ) {				// write all buckets
	    if ( trivial )
		s << (long)n->getKey();		// write key as long int
	    else
		s << n->getKey();		// write key as string
	    write( s, n->getData() );		// write data
	    n = n->getNext();
	}
	i++;
    }
    return s;
}


// --------------------------------------------------------------------------
// QGDictIterator member functions
//

QGDictIterator::QGDictIterator( const QGDict &d )
{
    dict = (QGDict *)&d;			// get reference to dict
    toFirst();					// set to first noe
    if ( !dict->iterators ) {
	dict->iterators = new Qditlst;		// create iterator list
	CHECK_PTR( dict->iterators );
    }
    dict->iterators->append( this );		// notify dict about iterator
}

QGDictIterator::~QGDictIterator()
{
    if ( dict )
	dict->iterators->remove( this );	// remove iterator from dict
}


GCI QGDictIterator::toFirst()			// move to first item
{
    if ( !dict ) {
#if defined(CHECK_NULL)
	warning( "QGDictIterator::toFirst: Dictionary has been deleted" );
#endif
	return 0;
    }
    if ( dict->count() == 0 ) {			// empty dictionary
	curNode = 0;
	return 0;
    }
    register uint i = 0;
#if defined(PARANOID_TEST)
    while ( !dict->vec[i] && i < dict->size() )	// paranoid test
	i++;
    if ( i == dict->size() ) {			// nothing found!?
	debug( "QGDictIterator::toFirst: Internal error" );
	return 0;
    }
#else
    register Qbucket **v = dict->vec;
    while ( !(*v++) )
	i++;
#endif
    curNode = dict->vec[i];
    curIndex = i;
    return curNode->getData();
}


GCI QGDictIterator::get() const			// get current item
{
    return curNode ? curNode->getData() : 0;
}

const char *QGDictIterator::getKey() const	// get current key
{
    return curNode ? curNode->getKey() : 0;
}


GCI QGDictIterator::operator()()		// get current and move to next
{
    if ( !dict ) {
#if defined(CHECK_NULL)
	warning( "QGDictIterator::operator(): Dictionary has been deleted" );
#endif
	return 0;
    }
    if ( !curNode )
	return 0;
    GCI d = curNode->getData();
    this->operator++();
    return d;
}

GCI QGDictIterator::operator++()		// move to next item (prefix)
{
    if ( !dict ) {
#if defined(CHECK_NULL)
	warning( "QGDictIterator::operator++: Dictionary has been deleted" );
#endif
	return 0;
    }
    if ( !curNode )
	return 0;
    curNode = curNode->getNext();
    if ( !curNode ) {				// no next bucket
	register uint i = curIndex + 1;		// look from next vec element
	register Qbucket **v = &dict->vec[i];
	while ( i < dict->size() && !(*v++) )
	    i++;
	if ( i == dict->size() ) {		// nothing found
	    curNode = 0;
	    return 0;
	}
	curNode = dict->vec[i];
	curIndex = i;
    }
    return curNode->getData();
}

GCI QGDictIterator::operator+=( uint jumps )	// move n positions forward
{
    while ( curNode && jumps-- )
	operator++();
    return curNode ? curNode->getData() : 0;
}
