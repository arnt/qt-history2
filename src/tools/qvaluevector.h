/****************************************************************************
** $Id$
**
** Definition of QValueVector class
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QVALUEVECTOR_H
#define QVALUEVECTOR_H

#ifndef QT_H
#include "qvector.h"
#endif // QT_H

#ifndef QT_NO_STL
#include <vector>
#endif


template <typename T>
class QValueVector : public QVector<T>
{
public:

    inline QValueVector() : QVector<T>() {}
    inline QValueVector( const QValueVector<T>& v ) : QVector<T>(v) {}
    inline QValueVector( typename QValueVector<T>::size_type n,
			 const T& val = T() ) : QVector<T>(n, val) {}

#ifndef QT_NO_STL
    inline QValueVector( const std::vector<T>& v ) : QVector<T>()
	{ resize(v.size()); qCopy( v.begin(), v.end(), begin() ); }
#endif

    QValueVector<T>& operator= ( const QValueVector<T>& v )
	{ QVector<T>::operator=(v); return *this; }

#ifndef QT_NO_STL
    QValueVector<T>& operator= ( const std::vector<T>& v )
    {
	clear();
	resize( v.size() );
	qCopy( v.begin(), v.end(), begin() );
	return *this;
    }
#endif

    T &at( typename QValueVector<T>::size_type i)
    {
	Q_ASSERT(i >= 0 && i < size());
	detach();
	return *( begin() + i );
    }
};

#define Q_DEFINED_QVALUEVECTOR
#include "qwinexport.h"
#endif // QVALUEVECTOR_H
