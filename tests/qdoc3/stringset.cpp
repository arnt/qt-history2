/*
  stringset.cpp
*/

#include "stringset.h"

StringSet::StringSet()
    : dirty( FALSE )
{
}

StringSet::StringSet( const StringSet& ss )
    : map( ss.map ), stringl( ss.stringl ), dirty( ss.dirty )
{
}

StringSet::StringSet( const QStringList& sl )
    : dirty( TRUE )
{
    QStringList::ConstIterator s = sl.begin();
    while ( s != sl.end() ) {
	map.insert( *s, (void *) 0 );
	++s;
    }
}

StringSet& StringSet::operator=( const StringSet& ss )
{
    map = ss.map;
    stringl = ss.stringl;
    dirty = ss.dirty;
    return *this;
}

void StringSet::clear()
{
    map.clear();
    stringl.clear();
    dirty = FALSE;
}

void StringSet::insert( const QString& str )
{
    map.insert( str, (void *) 0 );
    dirty = TRUE;
}

void StringSet::remove( const QString& str )
{
    map.remove( str );
    dirty = TRUE;
}

StringSet& StringSet::operator<<( const QString& str )
{
    insert( str );
    return *this;
}

bool StringSet::contains( const QString& str ) const
{
    return map.contains( str );
}

const QStringList& StringSet::stringList() const
{
    if ( dirty ) {
	StringSet *that = (StringSet *) this;
	that->stringl.clear();
	QMap<QString, void *>::ConstIterator m = map.begin();
	while ( m != map.end() ) {
	    that->stringl.append( m.key() );
	    ++m;
	}
	that->dirty = FALSE;
    }
    return stringl;
}

QStringList StringSet::toIStringList() const
{
    QMap<QString, QString> imap;
    QMap<QString, void *>::ConstIterator s = map.begin();
    while( s != map.end() ) {
	imap.insert( s.key().lower(), s.key() );
	++s;
    }

    QStringList istringl;
    QMap<QString, QString>::ConstIterator t = imap.begin();
    while ( t != imap.end() ) {
	istringl.append( *t );
	++t;
    }
    return istringl;
}

StringSet reunion( const StringSet& ss, const StringSet& tt )
{
    QMap<QString, void *>::ConstIterator s = ss.map.begin();
    QMap<QString, void *>::ConstIterator t = tt.map.begin();
    StringSet uu;

    /*
      A clever COBOL loop.
    */
    while ( s != ss.map.end() || t != tt.map.end() ) {
	if ( s != ss.map.end() ) {
	    if ( t == tt.map.end() || s.key() < t.key() ) {
		uu.insert( s.key() );
		++s;
	    }
	}
	if ( t != tt.map.end() ) {
	    if ( s == ss.map.end() || s.key() > t.key() ) {
		uu.insert( t.key() );
		++t;
	    } else if ( s.key() == t.key() ) {
		uu.insert( s.key() );
		++s;
		++t;
	    }
	}
    }
    return uu;
}

StringSet intersection( const StringSet& ss, const StringSet& tt )
{
    QMap<QString, void *>::ConstIterator s = ss.map.begin();
    QMap<QString, void *>::ConstIterator t = tt.map.begin();
    StringSet uu;

    while ( s != ss.map.end() && t != tt.map.end() ) {
	if ( s.key() < t.key() ) {
	    ++s;
	} else if ( s.key() > t.key() ) {
	    ++t;
	} else {
	    uu.insert( s.key() );
	    ++s;
	    ++t;
	}
    }
    return uu;
}

StringSet difference( const StringSet& ss, const StringSet& tt )
{
    QMap<QString, void *>::ConstIterator s = ss.map.begin();
    QMap<QString, void *>::ConstIterator t = tt.map.begin();
    StringSet uu;

    while ( s != ss.map.end() ) {
	if ( t == tt.map.end() || s.key() < t.key() ) {
	    uu.insert( s.key() );
	    ++s;
	} else if ( s.key() > t.key() ) {
	    ++t;
	} else {
	    ++s;
	    ++t;
	}
    }
    return uu;
}
