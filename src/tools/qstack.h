/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstack.h#16 $
**
** Definition of QStack template/macro class
**
** Created : 920917
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the tools
** module and therefore may only be used if the tools module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSTACK_H
#define QSTACK_H

#ifndef QT_H
#include "qglist.h"
#endif // QT_H


template<class type> class QStack : private QGList
{
public:
    QStack()				{}
    QStack( const QStack<type> &s ) : QGList(s) {}
   ~QStack()				{ clear(); }
    QStack<type> &operator=(const QStack<type> &s)
			{ return (QStack<type>&)QGList::operator=(s); }
    bool  autoDelete() const		{ return QCollection::autoDelete(); }
    void  setAutoDelete( bool del )	{ QCollection::setAutoDelete(del); }
    uint  count()   const		{ return QGList::count(); }
    bool  isEmpty() const		{ return QGList::count() == 0; }
    void  push( const type *d )		{ QGList::insertAt(0,Item(d)); }
    type *pop()				{ return (type *)QGList::takeFirst(); }
    bool  remove()			{ return QGList::removeFirst(); }
    void  clear()			{ QGList::clear(); }
    type *top()	    const		{ return (type *)QGList::cfirst(); }
	  operator type *() const	{ return (type *)QGList::cfirst(); }
    type *current() const		{ return (type *)QGList::cfirst(); }
private:
    void  deleteItem( Item d ) { if ( del_item ) delete (type *)d; }
};


#endif // QSTACK_H
