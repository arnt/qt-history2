/*
  set.h
*/

#ifndef SET_H
#define SET_H

#include <qlist.h>
#include <qmap.h>

template <class T>
class SetConstIterator
{
public:
    SetConstIterator()
	: useMap( true ) { }

    bool operator==( const SetConstIterator<T>& it ) const {
	return mapIter == it.mapIter && listIter == it.listIter;
    }
    bool operator!=( const SetConstIterator<T>& it ) const {
	return !operator==( it );
    }
    const T& operator*() const {
	if ( useMap ) {
	    return mapIter.key();
	} else {
	    return *listIter;
	}
    }
    SetConstIterator<T>& operator++() {
	if ( useMap ) {
	    mapIter++;
	} else {
	    listIter++;
	}
	return *this;
    }
    SetConstIterator<T> operator++( int ) {
	SetConstIterator<T> old = *this;
	operator++();
	return old;
    }
    SetConstIterator<T>& operator--() {
	if ( useMap ) {
	    mapIter--;
	} else {
	    listIter--;
	}
	return *this;
    }
    SetConstIterator<T> operator--( int ) {
	SetConstIterator<T> old = *this;
	operator--();
	return old;
    }

    SetConstIterator( const QMap<T, int>& map, const QList<T>& list )
	: useMap( list.isEmpty() ) {
	mapIter = map.end();
	mapEnd = map.end();
	listIter = list.end();
	listEnd = list.end();
    }

    bool useMap;
    typename QMap<T, int>::ConstIterator mapIter;
    typename QMap<T, int>::ConstIterator mapEnd;
    typename QList<T>::ConstIterator listIter;
    typename QList<T>::ConstIterator listEnd;
};

template <class T>
class Set
{
public:
    typedef SetConstIterator<T> ConstIterator;

    Set() { }
    Set( const QList<T>& values ) {
	typename QList<T>::ConstIterator v = values.begin();
	while ( v != values.end() ) {
	    map.insert( *v, 0 );
	    ++v;
	}
    }

    void clear() {
	map.clear();
	list.clear();
    }
    void insert( const T& item ) {
	toMap();
	map.insert( item, 0 );
    }
    void remove( const T& item ) {
	if ( list.count() > 0 && list.count() < 8 ) {
	    list.remove( item );
	} else {
	    toMap();
	    map.remove( item );
	}
    }

    Set<T>& operator<<( const T& item ) {
	insert( item );
	return *this;
    }
    Set<T>& operator+=( const Set<T>& other ) {
	*this = reunion( *this, other );
	return *this;
    }

    bool isEmpty() const {
	return map.isEmpty() && list.isEmpty();
    }
    uint count() const {
	return map.count() + list.count();
    }
    bool contains( const T& item ) const {
	if ( list.count() > 0 && list.count() < 8 ) {
	    return list.contains( item );
	} else {
	    toMap();
	    return map.contains( item );
	}
    }
    const QList<T>& asList() const {
	toList();
	return list;
    }
    ConstIterator begin() const {
	ConstIterator it( map, list );
	if ( list.isEmpty() ) {
	    it.mapIter = map.begin();
	} else {
	    it.listIter = list.begin();
	}
	return it;
    }
    ConstIterator end() const {
	return ConstIterator( map, list );
    }
    const T& first() const {
	return *begin();
    }
    const T& last() const {
	ConstIterator it = end();
	--it;
	return *it;
    }

    /*
      These are used by reunion(), intersection(), and difference(),
      which cannot be made friends of this class because of a gcc
      2.91.66 bug. The same bug is responsible for the ackward access
      to 'list'.
    */
    QMap<T, int> map;
    QList<T> list;

private:
    void toMap() const {
	if ( !list.isEmpty() ) {
	    Set<T> *that = (Set<T> *) this;
	    typename QList<T>::ConstIterator li = list.begin();
	    while ( li != list.end() ) {
		that->map.insert( *li, 0 );
		++li;
	    }
	    that->list.clear();
	}
    }
    void toList() const {
	if ( !map.isEmpty() ) {
	    Set<T> *that = (Set<T> *) this;
	    typename QMap<T, int>::ConstIterator m = map.begin();
	    while ( m != map.end() ) {
		that->list.append( m.key() );
		++m;
	    }
	    that->map.clear();
	}
    }
};

template <class T>
Set<T> reunion( const Set<T>& s, const Set<T>& t )
{
    if ( t.isEmpty() ) {
	return s;
    } else if ( s.isEmpty() ) {
	return t;
    } else {
	typename Set<T>::ConstIterator m = s.begin();
	typename Set<T>::ConstIterator n = t.begin();
	Set<T> result;

	while ( m != s.end() && n != t.end() ) {
	    if ( *m < *n ) {
		(&result)->list.append( *m );
		++m;
	    } else if ( *n < *m ) {
		(&result)->list.append( *n );
		++n;
	    } else {
		(&result)->list.append( *m );
		++m;
		++n;
	    }
	}
	while ( m != s.end() ) {
	    (&result)->list.append( *m );
	    ++m;
	}
	while ( n != t.end() ) {
	    (&result)->list.append( *n );
	    ++n;
	}
	return result;
    }
}

template <class T>
Set<T> intersection( const Set<T>& s, const Set<T>& t )
{
    if ( t.isEmpty() ) {
	return t;
    } else if ( s.isEmpty() ) {
	return s;
    } else {
	typename Set<T>::ConstIterator m = s.begin();
	typename Set<T>::ConstIterator n = t.begin();
	Set<T> result;

	while ( m != s.end() && n != t.end() ) {
	    if ( *m < *n ) {
		++m;
	    } else if ( *n < *m ) {
		++n;
	    } else {
		(&result)->list.append( *m );
		++m;
		++n;
	    }
	}
	return result;
    }
}

template <class T>
Set<T> difference( const Set<T>& s, const Set<T>& t )
{
    if ( t.isEmpty() ) {
	return s;
    } else if ( s.isEmpty() ) {
	return t;
    } else {
	typename Set<T>::ConstIterator m = s.begin();
	typename Set<T>::ConstIterator n = t.begin();
	Set<T> result;

	while ( m != s.end() && n != t.end() ) {
	    if ( *m < *n ) {
		(&result)->list.append( *m );
		++m;
	    } else if ( *n < *m ) {
		++n;
	    } else {
		++m;
		++n;
	    }
	}
	while ( m != s.end() ) {
	    (&result)->list.append( *m );
	    ++m;
	}
	return result;
    }
}

template <class T>
Set<T> operator+( const Set<T>& s, const Set<T>& t )
{
    return reunion( s, t );
}

#endif
