/****************************************************************************

**
** Definition of QSortedList template/macro class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
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
    QSortedList( const QSortedList<type> &l ) : QPtrList<type>(l) {}
    ~QSortedList() { this->clear(); }
    QSortedList<type> &operator=(const QSortedList<type> &l)
      { return (QSortedList<type>&)QPtrList<type>::operator=(l); }

    virtual int compareItems( QPtrCollection::Item s1, QPtrCollection::Item s2 )
      { if ( *((type*)s1) == *((type*)s2) ) return 0; return ( *((type*)s1) < *((type*)s2) ? -1 : 1 ); }
};

#endif
