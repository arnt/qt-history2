/****************************************************************************
**
** Definition of QValueStack class.
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

#ifndef QVALUESTACK_H
#define QVALUESTACK_H

#ifndef QT_H
#include "qvaluelist.h"
#endif // QT_H


template<class T>
class QValueStack : public QValueList<T>
{
public:
    QValueStack() {}
   ~QValueStack() {}
    void  push( const T& d ) { this->append(d); }
    T pop()
    {
	T elem( this->last() );
	if ( !this->isEmpty() )
	    this->remove( this->fromLast() );
	return elem;
    }
    T& top() { return this->last(); }
    const T& top() const { return this->last(); }
};

#endif
