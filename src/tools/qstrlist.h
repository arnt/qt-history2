/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstrlist.h#14 $
**
** Definition of QStrList, QStrIList and QStrListIterator classes
**
** Created : 920730
**
** Copyright (C) 1992-1996 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QStrList class provides a list of strings (i.e., char *).
*****************************************************************************/

#ifndef QSTRLIST_H
#define QSTRLIST_H

#include "qstring.h"
#include "qlist.h"
#include "qdstream.h"


#if defined(DEFAULT_TEMPLATECLASS)
typedef QListT<char>			QStrListBase;
typedef QListIteratorT<char>		QStrListIterator;
#else
typedef Q_DECLARE(QListM,char)		QStrListBase;
typedef Q_DECLARE(QListIteratorM,char)	QStrListIterator;
#endif


class QStrList : public QStrListBase
{
public:
    QStrList( bool deepCopies=TRUE ) { dc = deepCopies; }
    QStrList( const QStrList& other ) { dc = other.dc; *this = other; }
   ~QStrList()	{ clear(); }
private:
    GCI	  newItem( GCI d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void  deleteItem( GCI d )	{ if ( dc ) delete[] (char*)d; }
    int	  compareItems( GCI s1, GCI s2 )
				{ return strcmp((const char*)s1,
						(const char*)s2); }
    QDataStream &read( QDataStream &s, GCI &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, GCI d ) const
				{ return s << (const char *)d; }
    bool  dc;
};


class QStrIList : public QStrList		// case insensitive string list
{
public:
    QStrIList( bool deepCopies=TRUE ) : QStrList( deepCopies ) {}
   ~QStrIList() { clear(); }
private:
    int	  compareItems( GCI s1, GCI s2 )
				{ return stricmp((const char*)s1,
						 (const char*)s2); }
};


#endif // QSTRLIST_H
