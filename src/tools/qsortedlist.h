/****************************************************************************
** $Id: //depot/qt/main/src/tools/qsortedlist.h#3 $
**
** Definition of QList template/macro class
**
** Created : 920701
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSORTEDLIST_H
#define QSORTEDLIST_H

#ifndef QT_H
#include "qlist.h"
#endif // QT_H


template<class type> class Q_EXPORT QSortedList : public QList<type>
{
public:
    QSortedList() {}
    QSortedList( const QSortedList<type> &l ) : QList<type>(l) {}
    ~QSortedList() { clear(); }
    QSortedList<type> &operator=(const QSortedList<type> &l)
      { return (QSortedList<type>&)QList<type>::operator=(l); }

    virtual int compareItems( QCollection::Item s1, QCollection::Item s2 )
      { if ( *((type*)s1) == *((type*)s2) ) return 0; return ( *((type*)s1) < *((type*)s2) ? -1 : 1 ); }
};

#endif
