/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3STRLIST_H
#define Q3STRLIST_H

#include "qstring.h"
#include "q3ptrlist.h"
#include "qdatastream.h"
#include "qlist.h"

#if defined(Q_QDOC)
class Q3StrListIterator : public Q3PtrListIterator<char>
{
};
#else
typedef Q3PtrListIterator<char> Q3StrListIterator;
#endif

class Q_COMPAT_EXPORT Q3StrList : public Q3PtrList<char>
{
public:
    Q3StrList( bool deepCopies=true ) { dc = deepCopies; del_item = deepCopies; }
    Q3StrList( const Q3StrList & );
    ~Q3StrList()			{ clear(); }
    Q3StrList& operator=( const Q3StrList & );
    Q3StrList(const QList<QByteArray> &list) {
        for (int i = 0; i < list.size(); ++i)
            append(list.at(i));
    }

    Q3StrList &operator =(const QList<QByteArray> &list) {
        clear();
        for (int i = 0; i < list.size(); ++i)
            append(list.at(i));
        return *this;
    }

    operator QList<QByteArray>() const {
        QList<QByteArray> list;
        for (Q3PtrListStdIterator<char> it = begin(); it != end(); ++it)
            list.append(QByteArray(*it));
        return list;
    }

private:
    Q3PtrCollection::Item newItem( Q3PtrCollection::Item d ) { return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( Q3PtrCollection::Item d ) { if ( del_item ) delete[] (char*)d; }
    int compareItems( Q3PtrCollection::Item s1, Q3PtrCollection::Item s2 ) { return qstrcmp((const char*)s1,
							 (const char*)s2); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read( QDataStream &s, Q3PtrCollection::Item &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, Q3PtrCollection::Item d ) const
				{ return s << (const char *)d; }
#endif
    bool  dc;
};


class Q_COMPAT_EXPORT Q3StrIList : public Q3StrList	// case insensitive string list
{
public:
    Q3StrIList( bool deepCopies=true ) : Q3StrList( deepCopies ) {}
    ~Q3StrIList()			{ clear(); }
private:
    int	  compareItems( Q3PtrCollection::Item s1, Q3PtrCollection::Item s2 )
				{ return qstricmp((const char*)s1,
						    (const char*)s2); }
};


inline Q3StrList & Q3StrList::operator=( const Q3StrList &strList )
{
    clear();
    dc = strList.dc;
    del_item = dc;
    Q3PtrList<char>::operator=( strList );
    return *this;
}

inline Q3StrList::Q3StrList( const Q3StrList &strList )
    : Q3PtrList<char>( strList )
{
    dc = false;
    operator=( strList );
}

#endif // Q3STRLIST_H
