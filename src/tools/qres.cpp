/****************************************************************************
** $Id: //depot/qt/main/src/tools/qres.cpp#1 $
**
** Implementation of QResource class
**
** Author  : Haavard Nord
** Created : 920527
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** Classes used internally in QResource are:
**	QResEntry	- identifies resource entry (tag or section name)
**			  base class for QResTag and QResSection
**	QResTag		- resource tag with value string/string list
**	QResSection	- section with many resource tag objects
**
**	QResList	- list of resource entries; QList(QResEntry)
**	QResDict	- dict of resource entries; QDict(QResEntry)
*****************************************************************************/

#include "qres.h"
#include "qstrlist.h"
#include "qdict.h"
#include <stdio.h>
#include <ctype.h>

RCSTAG("$Id: //depot/qt/main/src/tools/qres.cpp#1 $")


// --------------------------------------------------------------------------
// QResEntry class (internal resource entry; tag or section name)
//

class QResEntry
{
public:
    QResEntry( const char *n )	{ entry = qstrdup(n); }
    virtual ~QResEntry()	{ delete entry; }
    char *name() const		{ return entry; }
private:
    char *entry;
};

typedef declare(QListM,QResEntry) QResList;
typedef declare(QListIteratorM,QResEntry) QResListIterator;
typedef declare(QDictM,QResEntry) QResDict;


// --------------------------------------------------------------------------
// QResTag class (internal resource tag with value string/string list)
//
// We are using a list object only when there are more than one value,
// because lists are memory expensive when there are lots of resource tags
//

class QResTag : public QResEntry
{
friend class QResSection;
public:
    QResTag( const char *n ) : QResEntry( n ) { count = 0; val.str = 0; }
   ~QResTag();
    char     *get()  const { return count > 1 ? val.list->last() : val.str; }
    void      insert( const char *value, bool single );
    QStrVec  *values( bool cases ) const;	// copy of list
private:
    int	  count;				// number of values
    union {
	char	  *str;				// single value
	QStrList  *list;			// many values
    } val;
};

QResTag::~QResTag()
{
    if ( count == 1 )
	delete val.str;				// delete value string
    else if ( count > 1 )
	delete val.list;			// delete value list
}

void QResTag::insert( const char *value, bool single )
{
    if ( count == 0 )				// no value set
	val.str = qstrdup( value );
    else
    if ( single ) {				// single value always
	delete val.str;
	val.str = qstrdup( value );
	count = 0;				// will be incremented
    }
    else {					// count > 0, never single
	if ( count == 1 ) {			// change to list storage
	    QStrList *s = new QStrList;
	    CHECK_PTR( s );
	    s->append( val.str );
	    delete val.str;
	    val.list = s;
	}
	val.list->append( value );
    }
    count++;
}

QStrVec *QResTag::values( bool cases ) const	// get all values
{
    QStrVec *v;
    if ( cases )				// case insensitive
	v = new QStrVec(count,TRUE);
    else					// case sensitive
	v = new QStrIVec(count,TRUE);
    CHECK_PTR( v );
    if ( count == 1 )				// put single value in vector
	v->insert( 0, val.str );
    else
    if ( count > 1 )				// copy all into vector
	val.list->toVector( v );
    return v;
}


// --------------------------------------------------------------------------
// QResSection class (internal section with resource tags)
//

class QResSection : public QResEntry
{
public:
    QResSection( const char *, bool, int );
   ~QResSection();
    QStrVec  *tags( bool cases ) const;		// copy of tags
    QStrVec  *values( const char *tag, bool cases ) const; // copy of values
    char     *get( const char *tag ) const;	// single value
    void      insert( const char *tag, const char *value, bool single );
    bool      remove( const char *tag );
    bool      write( FILE *, char sep ) const;
private:
    void      write( FILE *, char * ) const;
    QResList *list;				// list of tags
    QResDict *dict;				// dict of tags
};

QResSection::QResSection( const char *n, bool cs, int ds ) : QResEntry( n )
{
    list = new QResList;			// create tag list and dict
    CHECK_PTR( list );
    list->setAutoDelete( TRUE );		// master store
    dict = new QResDict( ds, cs, FALSE );
    CHECK_PTR( dict );
}

QResSection::~QResSection()
{
    delete dict;
    delete list;
}

QStrVec *QResSection::tags( bool cases ) const	// get all tags
{
    QResListIterator it( *list );
    QStrVec *v;
    if ( cases )				// case insensitive
	v = new QStrVec(it.count(),TRUE);
    else					// case sensitive
	v = new QStrIVec(it.count(),TRUE);
    CHECK_PTR( v );
    int i = 0;
    while ( it )
	v->insert( i++, it()->name() );
    return v;
}

inline QStrVec *QResSection::values( const char *tag, bool cases ) const
{						// get list of values
    QResTag *r = (QResTag*)dict->find( tag );
    return r ? r->values( cases ) : 0;
}

inline char *QResSection::get( const char *tag ) const
{
    QResTag *r = (QResTag*)dict->find( tag );
    return r ? r->get() : 0;
}

void QResSection::insert( const char *tag, const char *value, bool single )
{						// insert tag and value
    QResTag *r = (QResTag*)dict->find( tag );
    if ( !r ) {					// create new tag
	r = new QResTag( tag );
	CHECK_PTR( r );
	list->append( r );			// store in list
	dict->insert( r->name(), r );		// put reference in dict
    }
    r->insert( value, single );
}

bool QResSection::remove( const char *tag )	// remove tag from section
{
    QResEntry *r = dict->find( tag );		// get ref to object
    if ( !r )					// no such tag
	return FALSE;
    dict->remove( tag );			// deletes only reference
#if defined(CHECK_STATE)
    if ( !list->removeRef(r) ) {
	warning( "QResSection::remove: Inconsistent internal data structure" );
	return FALSE;
    }
    return TRUE;
#else
    return list->removeRef();
#endif
}

void QResSection::write( FILE *fh, char *s ) const
{						// write value string to file
    while ( *s ) {
	if ( *s == '\n' ) {			// expand nl to \ + tab
	    putc( '\\', fh );
	    putc( '\n', fh );
	    putc( '\t', fh );
	}
	else
	    putc( *s, fh );
	s++;
    }
    putc( '\n', fh );
}

bool QResSection::write( FILE *fh, char sep ) const
{						// write tags to file
    bool is_local = name() && *name();
    char spaces[40];
    if ( is_local ) {				// write section tag
	fprintf( fh, "\n[%s]\n", name() );
	memset( spaces, ' ', 4 );
	spaces[4] = '\0';
    }
    else {					// global section
	if ( list->count() == 0 )		// nothing to write
	    return TRUE;
	putc( '\n', fh );
	spaces[0] = '\0';
    }
    for ( QResTag *t=(QResTag*)list->first(); t; t=(QResTag*)list->next() ) {
	if ( t->count == 0 )			// no values
	    fprintf( fh, "%s%s%c\n", spaces, t->name(), sep );
	else
	if ( t->count == 1 ) {			// one value
	    fprintf( fh, "%s%s%c", spaces, t->name(), sep );
	    write( fh, t->val.str );
	}
	else {					// many values
	    QStrListIt it( *t->val.list );
	    while ( it ) {
		fprintf( fh, "%s%s%c ", spaces, t->name(), sep );
		write( fh, it() );
	    }
	}
    }
    return TRUE;
}


// --------------------------------------------------------------------------
// QResource member functions
//

QResource::QResource( bool caseSensitivity, bool multipleValues,
		      char separator, int dictSize, int subDictSize )
{
    list = new QResList;			// create section list and dict
    CHECK_PTR( list );
    list->setAutoDelete( TRUE );		// master store
    dict = new QResDict( dictSize, caseSensitivity, FALSE );
    CHECK_PTR( dict );
    tsize = subDictSize;			// size of section dict
    cases = caseSensitivity;
    multi = multipleValues;
    sep = separator;
}

QResource::~QResource()
{
    delete dict;				// delete section dict and list
    delete list;
}


inline bool is_white( char c )			// check if white space
{
    return isspace( c );			// use standard K&R
}

inline char *skip_white( char *s )		// skip leading white space
{
    while ( is_white(*s) )
	s++;
    return s;
}

int QResource::load( const char *filename )	// load a resource file
{
    char buffer[Res_MaxLineLength];		// parse buffer
    char *p;
    int	 line_num;				// line number
    int	 nerrors;				// number of errors

    FILE *fh = fopen( filename, "r" );
    if ( !fh ) {				// cannot open file
#if defined(CHECK_NULL)
	warning( "QResource::load: Cannot open file %s", filename );
#endif
	return -1;
    }

    line_num = nerrors = 0;
    QString section  = (char*)0;		// current section
    QString multiln  = (char*)0;		// multi line value string
    QString multitag = (char*)0;		// multi line tag kept

    while ( !feof( fh ) ) {			// read until EOF
	char *tag, *value;			// read one line
	p = fgets( buffer, Res_MaxLineLength, fh );
	if ( !p )				// end of file
	    break;
	line_num++;

	p = skip_white( buffer );		// skip leading white space
	if ( !*p || *p == '#' || *p == ';' )	// skip blank line/comment
	    continue;

	if ( (const char*)multiln ) {		// multi-line value string
	    value = p;
	    p = &value[ strlen(value) - 1 ];
	    while ( is_white(*p) )		// find end of string
		p--;
	    if ( *p == '\\' ) {			// '\'-terminated line
		*p++ = '\n';
		*p = '\0';
		multiln += value;
	    }
	    else {				// last of multi lines
		*++p = '\0';
		multiln += value;
		insert( section, multitag, multiln );
		multiln = (char*)0;
		multitag = (char*)0;
	    }
	    continue;
	}

	nerrors++;				// assume error from now on

	bool parsing_section = FALSE;		// parsing section
	char end_tag = sep;			// indicates end of tag

	if ( *p == '[' ) {			// section tag
	    p = skip_white( p + 1 );
	    parsing_section = TRUE;
	    end_tag = ']';			// look for end of section
	}
	tag = p;				// start of tag string
	while ( *p && *p != end_tag )		// find end of tag
	    p++;
	if ( !*p ) {
#if defined(CHECK_NULL)
	    warning( "QResource::load: Missing '%c' in line %d", end_tag,
		     line_num );
#endif
	    break;
	}

	value = skip_white( p + 1 );		// start of value string

	p--;
	while ( p >= buffer && is_white( *p ) ) // find end of tag
	    p--;
	if ( p < buffer || *p == '[') {
#if defined(CHECK_NULL)
	    warning( "QResource::load: Missing tag in line %d", line_num );
#endif
	    break;
	}
	*++p = '\0';				// terminate tag string

	if ( parsing_section ) {
#if defined(CHECK_NULL)
	    if ( *value )
		warning( "QResource::load: Unexpected characters after ']'" );
#endif
	    nerrors--;
	    insert( section = tag, 0, 0 );	// create empty section
	    continue;
	}

	if ( *value ) {				// non-empty value string
	    p = &value[ strlen(value) - 1 ];
	    while ( is_white(*p) )		// find end of string
		p--;
	    if ( *p == '\\' ) {			// '\'-terminated line
		*p++ = '\n';
		*p = '\0';
		multiln = value;
		multitag = tag;
	    }
	    else				// normal line
		*++p = '\0';
	}

	nerrors--;				// assume no more real errors
	if ( multiln.isNull() )
	    insert( section, tag, value );
    }
    fclose( fh );

#if defined(CHECK_STATE)
    if ( !multiln.isNull() )
	warning( "QResource::load: Unexpected '\\' near end of file" );
#endif
    return nerrors ? line_num : 0;		// return error line
}

bool QResource::save( const char *filename, const char *header ) const
{						// save to file
    FILE *fh = fopen( filename, "w" );		// create ASCII file
    if ( !fh )
	return FALSE;
    if ( header ) {
	fprintf( fh, header );			// write header to file
	fprintf( fh, "\n" );
    }
    QResListIterator it( *list );
    while ( it )				// for all sections...
	((QResSection*)it())->write( fh, sep );
    fclose( fh );
    return TRUE;
}


QStrVec *QResource::getAllSections() const	// get all sections
{
    QResListIterator it( *list );
    QStrVec *v;
    if ( cases )				// case sensitive
	v = new QStrVec(it.count(),TRUE);
    else					// case insensitive
	v = new QStrIVec(it.count(),TRUE);
    CHECK_PTR( v );
    int i = 0;
    while ( it )
	v->insert( i++, ((QResSection*)it())->name() );
    return v;
}

QStrVec *QResource::getAllTags( const char *section ) const
{						// get all tags for section
    if ( section == 0 )				// global section
	section = "";
    QResSection *r = (QResSection*)dict->find( section );
    return r ? r->tags( cases ) : 0;
}

QStrVec *QResource::getAllValues( const char *section, const char *tag) const
{						// get all values for section
    if ( section == 0 )				// global section
	section = "";
    QResSection *r = (QResSection*)dict->find( section );
    return r ? r->values( tag, cases ) : 0;
}

char *QResource::get( const char *section, const char *tag ) const
{						// get single/last tag value
    if ( section == 0 )				// global section
	section = "";
    QResSection *r = (QResSection*)dict->find( section );
    return r ? r->get( tag ) : 0;
}

void QResource::insert(const char *section, const char *tag, const char *value)
{						// insert section and tag
    if ( !tag )
	return;
    if ( section == 0 )				// global section
	section = "";
    QResSection *r = (QResSection*)dict->find( section );
    if ( !r ) {					// add new section
	r = new QResSection( section, cases, tsize );
	CHECK_PTR( r );
	if ( *section == '\0' )			// global section
	    list->insert( 0, r );			// store in list, at head
	else
	    list->append( r );			// store in list
	dict->insert( r->name(), r );		// put reference in dict
    }
    r->insert( tag, value, !multi );
}

bool QResource::remove( const char *section )	// remove section
{
    QResEntry *r = dict->find( section );	// get ref to object
    if ( !r )					// no such section
	return FALSE;
    dict->remove( section );			// deletes only reference
#if defined(CHECK_STATE)
    if ( !list->removeRef(r) ) {
	warning( "QResource::remove: Inconsistent internal data structure" );
	return FALSE;
    }
    return TRUE;
#else
    return list->removeRef();
#endif
}

bool QResource::remove( const char *section, const char *tag )
{						// remove tag in section
    if ( section == 0 )				// global section
	section = "";
    QResSection *r = (QResSection*)dict->find( section );
    if ( !r )
	return FALSE;
    return r->remove( tag );
}

void QResource::clear()				// remove everything
{
    dict->clear();
    list->clear();
}
