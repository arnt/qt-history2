/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstrlist.h#30 $
**
** Definition of QStrList, QStrIList and QStrListIterator classes
**
** Created : 920730
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSTRLIST_H
#define QSTRLIST_H

#ifndef QT_H
#include "qstring.h"
#include "qlist.h"
#include "qdatastream.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QList<char>;
template class Q_EXPORT QListIterator<char>;
#endif

typedef QList<char>		QStrListBase;
typedef QListIterator<char>	QStrListIterator;


class Q_EXPORT QStrList : public QStrListBase
{
public:
    QStrList( bool deepCopies=TRUE ) { dc = deepCopies; }
    QStrList( const QStrList & );
   ~QStrList()			{ clear(); }
    QStrList& operator=( const QStrList & );

private:
    Item	  newItem( Item d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void  deleteItem( Item d )	{ if ( dc ) delete[] (char*)d; }
    int	  compareItems( Item s1, Item s2 )
				{ return strcmp((const char*)s1,
						(const char*)s2); }
    QDataStream &read( QDataStream &s, Item &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, Item d ) const
				{ return s << (const char *)d; }
    bool  dc;
};


class Q_EXPORT QStrIList : public QStrList	// case insensitive string list
{
public:
    QStrIList( bool deepCopies=TRUE ) : QStrList( deepCopies ) {}
   ~QStrIList()			{ clear(); }
private:
    int	  compareItems( Item s1, Item s2 )
				{ return stricmp((const char*)s1,
						 (const char*)s2); }
};


inline QStrList & QStrList::operator=( const QStrList &strList )
{
    clear();
    dc = strList.dc;
    QStrListBase::operator=(strList);
    return *this;
}

inline QStrList::QStrList( const QStrList &strList )
    : QStrListBase( strList )
{
    dc = FALSE;
    operator=(strList);
}


#endif // QSTRLIST_H
