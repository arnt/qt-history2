/****************************************************************************
**
** Definition of QValueVector class.
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

    void resize( int n, const T& val = T() )
    {
	if ( n < size() )
	    erase( begin() + n, end() );
	else
	    insert( end(), n - size(), val );
    }


    T& at( int i, bool* ok = 0 )
    {
	detach();
	if ( ok )
	    *ok = ( i >= 0 && i < size() );
	return *( begin() + i );
    }

    const T&at( int i, bool* ok = 0 ) const
    {
	if ( ok )
	    *ok = ( i >= 0 && i < size() );
	return *( begin() + i );
    }
};

#define Q_DEFINED_QVALUEVECTOR
#include "qwinexport.h"
#endif // QVALUEVECTOR_H
