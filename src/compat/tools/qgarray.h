/****************************************************************************
**
** Definition of QGArray class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGARRAY_H
#define QGARRAY_H

#ifndef QT_H
#include "qshared.h"
#endif // QT_H


class Q_COMPAT_EXPORT QGArray					// generic array
{
friend class QBuffer;
public:
    // do not use this, even though this is public
    // ### make protected or private in Qt 4.0 beta?
    struct array_data : public QShared {	// shared array
	array_data():data(0),len(0)
#ifdef QT_QGARRAY_SPEED_OPTIM
		    ,maxl(0)
#endif
	    {}
	char *data;				// actual array data
	uint  len;
#ifdef QT_QGARRAY_SPEED_OPTIM
	uint maxl;
#endif
    };
    QGArray();
    enum Optimization { MemOptim, SpeedOptim };
protected:
    QGArray( int, int );			// dummy; does not alloc
    QGArray( int size );			// allocate 'size' bytes
    QGArray( const QGArray &a );		// shallow copy
    virtual ~QGArray();

    QGArray    &operator=( const QGArray &a ) { return assign( a ); }

    virtual void detach()	{ duplicate(*this); }

    // ### Qt 4.0: maybe provide two versions of data(), at(), etc.
    char       *data()	 const	{ return shd->data; }
    uint	nrefs()	 const	{ return shd->count; }
    uint	size()	 const	{ return shd->len; }
    bool	isEqual( const QGArray &a ) const;

    bool	resize( uint newsize, Optimization optim );
    bool	resize( uint newsize );

    bool	fill( const char *d, int len, uint sz );

    QGArray    &assign( const QGArray &a );
    QGArray    &assign( const char *d, uint len );
    QGArray    &duplicate( const QGArray &a );
    QGArray    &duplicate( const char *d, uint len );
    void	store( const char *d, uint len );

    array_data *sharedBlock()	const		{ return shd; }
    void	setSharedBlock( array_data *p ) { shd=(array_data*)p; }

    QGArray    &setRawData( const char *d, uint len );
    void	resetRawData( const char *d, uint len );

    int		find( const char *d, uint index, uint sz ) const;
    int		contains( const char *d, uint sz ) const;

    void	sort( uint sz );
    int		bsearch( const char *d, uint sz ) const;

    char       *at( uint index ) const;

    bool	setExpand( uint index, const char *d, uint sz );

protected:
    virtual array_data *newData();
    virtual void deleteData( array_data *p );

private:
    array_data *shd;
};


inline char *QGArray::at( uint index ) const
{
    Q_ASSERT(index < size());
    return &shd->data[index];
}


#endif // QGARRAY_H
