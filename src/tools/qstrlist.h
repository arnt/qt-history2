/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstrlist.h#1 $
**
** Definition of QStrList, QStrIList and QStrListIterator classes
**
** Author  : Haavard Nord
** Created : 920730
**
** Copyright (C) 1992-1994 by Troll Tech as.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QStrList class provides a list of strings (i.e., char *).
*****************************************************************************/

#ifndef QSTRLIST_H
#define QSTRLIST_H

#include "qstring.h"
#include "qlist.h"
#include "qstream.h"


declare(QListM,char);
declare(QListIteratorM,char);


class QStrList : public QListM(char)		// string list
{
public:
    QStrList( bool deepCopies=TRUE ) { dc = deepCopies; }
   ~QStrList()	{ clear(); }
private:
    GCI	  newItem( GCI d )	{ return dc ? strdup( (char*)d ) : d; }
    void  deleteItem( GCI d )	{ if ( dc ) delete[] (char*)d; }
    int	  compareItems( GCI s1, GCI s2 )
				{ return strcmp((char*)s1,(char*)s2); }
    QStream& read( QStream &s, GCI &d )
				{ s >> (char *&)d; return s; }
    QStream& write( QStream &s, GCI d ) const
				{ return s << (char *)d; }
    bool  dc;
};


class QStrIList : public QStrList		// case insensitive string list
{
public:
    QStrIList( bool deepCopies=TRUE ) : QStrList( deepCopies ) {}
   ~QStrIList() { clear(); }
private:
    int	  compareItems( GCI s1, GCI s2 )
				{ return stricmp((char*)s1,(char*)s2); }
};


typedef QListIteratorM(char) QStrListIterator;	// iterator for QStrList
						// and QStrIList

#endif // QSTRLIST_H
