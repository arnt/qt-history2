/****************************************************************************
** $Id: //depot/qt/main/src/tools/qgarray.h#28 $
**
** Definition of QGArray class
**
** Created : 930906
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QGARRAY_H
#define QGARRAY_H

#ifndef QT_H
#include "qshared.h"
#endif // QT_H


class Q_EXPORT QGArray					// generic array
{
friend class QBuffer;
public:
    // DO NOT USE THIS.  IT IS PUBLIC BUT DO NOT USE IT IN NEW CODE.
    struct array_data : public QShared {	// shared array
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

    QGArray    &operator=( const QGArray &a ) { return assign( a ); }

    virtual void detach()	{ duplicate(*this); }

    char       *data()	 const	{ return shd->data; }
    uint	nrefs()	 const	{ return shd->count; }
    uint	size()	 const	{ return shd->len; }
    bool	isEqual( const QGArray &a ) const;

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

    char       *at( uint index ) const;

    bool	setExpand( uint index, const char *d, uint sz );

protected:
    virtual array_data *newData();
    virtual void deleteData( array_data *p );

private:
    static void msg_index( uint );
    array_data *shd;
};


inline char *QGArray::at( uint index ) const
{
#if defined(CHECK_RANGE)
    if ( index >= size() ) {
	msg_index( index );
	index = 0;
    }
#endif
    return &shd->data[index];
}


#endif // QGARRAY_H
