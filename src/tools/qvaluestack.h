/****************************************************************************
** $Id: //depot/qt/main/src/tools/qvaluestack.h#1 $
**
** Definition of QValueStack class
**
** Created : 990925
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

#ifndef QVALUESTACK_H
#define QVALUESTACK_H

#ifndef QT_H
#include "qvaluelist.h"
#endif // QT_H


template<class T>
class Q_EXPORT QValueStack : public QValueList<T>
{
public:
    QValueStack() {}
   ~QValueStack() {}
    void  push( const T& d ) { append(d); }
    T pop()
    {
	T elem( this->last() );
	if ( !this->isEmpty() )
	    remove( this->fromLast() );
	return elem;
    }
    T& top() { return this->last(); }
    const T& top() const { return this->last(); }
};

#endif
