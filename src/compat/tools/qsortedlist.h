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

#ifndef QSORTEDLIST_H
#define QSORTEDLIST_H

#ifndef QT_H
#include "qptrlist.h"
#endif // QT_H


template<class type>
class QSortedList : public QPtrList<type>
{
public:
    QSortedList() {}
    QSortedList(const QSortedList<type> &l) : QPtrList<type>(l) {}
    ~QSortedList() { this->clear(); }
    QSortedList<type> &operator=(const QSortedList<type> &l)
      { return static_cast<QSortedList<type> &>(QPtrList<type>::operator=(l)); }

    virtual int compareItems(QPtrCollection::Item s1, QPtrCollection::Item s2) {
        if (*static_cast<type*>(s1) == *static_cast<type*>(s2)) return 0;
        return (*static_cast<type*>(s1) < *static_cast<type*>(s2) ? -1 : 1); }
};

#endif
