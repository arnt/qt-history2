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

#ifndef QSTRLIST_H
#define QSTRLIST_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qptrlist.h"
#include "qdatastream.h"
#endif // QT_H

#if defined(qdoc)
class QStrListIterator : public QPtrListIterator<char>
{
};
#else
typedef QPtrListIterator<char> QStrListIterator;
#endif

class Q_COMPAT_EXPORT QStrList : public QPtrList<char>
{
public:
    QStrList(bool deepCopies=true) { dc = deepCopies; del_item = deepCopies; }
    QStrList(const QStrList &);
    ~QStrList()                        { clear(); }
    QStrList& operator=(const QStrList &);

    QStringList toStringList() const;
private:
    QPtrCollection::Item newItem(QPtrCollection::Item d) { return dc ? qstrdup(static_cast<const char*>(d)) : d; }
    void deleteItem(QPtrCollection::Item d) { if (del_item) delete[] static_cast<char*>(d); }
    int compareItems(QPtrCollection::Item s1, QPtrCollection::Item s2) {
        return qstrcmp(static_cast<const char*>(s1), static_cast<const char*>(s2)); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read(QDataStream &s, QPtrCollection::Item &d)
                                { s >> reinterpret_cast<char *&>(d); return s; }
    QDataStream &write(QDataStream &s, QPtrCollection::Item d) const
                                { return s << static_cast<const char *>(d); }
#endif
    bool  dc;
};


class Q_COMPAT_EXPORT QStrIList : public QStrList        // case insensitive string list
{
public:
    QStrIList(bool deepCopies=true) : QStrList(deepCopies) {}
    ~QStrIList()                        { clear(); }
private:
    int          compareItems(QPtrCollection::Item s1, QPtrCollection::Item s2)
        { return qstricmp(static_cast<const char*>(s1), static_cast<const char*>(s2)); }
};


inline QStrList & QStrList::operator=(const QStrList &strList)
{
    clear();
    dc = strList.dc;
    del_item = dc;
    QPtrList<char>::operator=(strList);
    return *this;
}

inline QStrList::QStrList(const QStrList &strList)
    : QPtrList<char>(strList)
{
    dc = false;
    operator=(strList);
}

#endif // QSTRLIST_H
