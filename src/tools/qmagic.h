#ifndef QMAGIC_H
#define QMAGIC_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


/*
QMagic pollutes the global namespace with three new keywords: foreach,
removeall and forever. All three are not strictly necessary, but add
syntatic sugar that can improve the readability of your programs.

We advise against including qmagic.h in headerfiles as it relies on
preprocessor macros.


forever: a new loop construct, used like

    forever {
	...
	if ( cond )
	    break;
	...
    }

forever is semantically identically to while(1) or for(;;)


foreach: a new loop construct to iterate over containers. Example:

    std::list<int> list;
    ...
    int i;
    foreach( i, list )
           cout << i;

The code is semanitically identical to the standard construct:

    std::list<int> list;
    ...
    std::list<int>::const_iterator it = list.begin;
    int i;
    while( it != list.end() ) {
	i = *it;
	++it;
	cout << i;
     }

removeall: a new loop construct to remove certain elements from a
container. Example:

    std::list<int> list;
    ...
    int i;
    removeall( i, list ) where ( i < 10 );

The code is semantically identical to the the standard construct:

    std::list<int> list;
    ...
    std::list<int>::const_iterator it = list.begin;
    int i;
    while( it != list.end() ) {
	i = *it;
	if ( i < 10 )
	    it = list.erase( it );
	else
	    ++it;
     }

foreach and removeall operate on standard STL containers like
std::list, as well as as on Qt's container classes QValueList, QMap
and QPtrList.

*/


#define forever for(;;)
#define foreach(element,container) for(QForEach foreach_##element(&container); foreach_##element(element,&container);)
#define removeall(element,container) for(QRemoveAll where; where.iterate(element,&container);)

class QForEach
{
public:
    template <class C> inline QForEach(const C* c) {
	typedef typename C::const_iterator const_iterator;
	const_iterator* i = (const_iterator*) &ptr;
	*i = c->begin();
    }
    template <class E, class C> inline bool operator()( E& e, const C* c) {
	typedef typename C::const_iterator const_iterator;
	const_iterator* i = (const_iterator*) &ptr;
	if ( *i == c->end() )
	    return FALSE;
 	e = **i;
	++(*i);
	return TRUE;
    }
    void* ptr;
};

class QRemoveAll
{
public:
    inline QRemoveAll():ptr(0), init(0),rm(0){}
    inline void operator()( bool remove ) { rm = remove; }
    template <class E, class C> inline bool iterate( E& e, C* c) {
	typedef typename C::iterator iterator;
	iterator* i = (iterator*) &ptr;
	if ( rm )
	    *i = c->erase( *i );
	else if ( !init )
	    *i = c->begin();
	else
	    ++(*i);
	if ( *i == c->end() )
	    return FALSE;
	init = 1;
 	e = **i;
	return TRUE;
    }
    void* ptr;
    uint init : 1;
    uint rm : 1;
};


#endif // QMAGIC_H
