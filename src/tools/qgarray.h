/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgarray.h#11 $
**
** Definition of QGArray class
**
** Author  : Haavard Nord
** Created : 930906
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QGARRAY_H
#define QGARRAY_H

#include "qshared.h"


class QGArray					// generic array
{
friend class QBuffer;
public:
    struct array_data : QShared {		// shared array
	array_data()	{ data=0; len=0; }
	char *data;				// actual array data
	uint  len;
    };
    QGArray();
protected:
    QGArray( int, int );			// dummy; does not alloc
    QGArray( int size );			// allocate 'size' bytes
    QGArray( const QGArray &a );		// shallow copy
    virtual ~QGArray();

    virtual void detach()	{ duplicate(*this); }

    char       *data()	 const	{ return p->data; }
    uint	size()	 const	{ return p->len; }
    bool	isEqual( const QGArray &a ) const;

    bool	resize( uint newsize );

    bool	fill( const char *d, int len, uint sz );

    QGArray    &assign( const QGArray &a );
    QGArray    &assign( const char *d, uint len );
    QGArray    &duplicate( const QGArray &a );
    QGArray    &duplicate( const char *d, uint len );
    void	store( const char *d, uint len );

    QGArray    &setRawData( const char *d, uint len );
    void	resetRawData( const char *d, uint len );

    int		find( const char *d, uint index, uint sz ) const;
    int		contains( const char *d, uint sz ) const;

    char       *at( uint index ) const;

    bool	setExpand( uint index, const char *d, uint sz );

protected:
    array_data *p;

    virtual array_data *newData()		    { return new array_data; }
    virtual void	deleteData( array_data *d ) { delete d; }
};


#if !defined(CHECK_RANGE) && !defined(QGARRAY_CPP)
inline char *QGArray::at( uint index ) const
{
    return &p->data[index];
}
#endif


#endif // QGARRAY_H
