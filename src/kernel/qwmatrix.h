/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwmatrix.h#6 $
**
** Definition of QWMatrix class
**
** Created : 941020
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QWMATRIX_H
#define QWMATRIX_H

#include "qwindefs.h"
#include "qpntarry.h"
#include "qrect.h"


class QWMatrix					// 2D transform matrix
{
public:
    QWMatrix();
    QWMatrix( float m11, float m12, float m21, float m22,
	      float dx,	 float dy );

    void	setMatrix( float m11, float m12, float m21, float m22,
			   float dx,  float dy );

    float	m11() const { return _m11; }
    float	m12() const { return _m12; }
    float	m21() const { return _m21; }
    float	m22() const { return _m22; }
    float	dx()  const { return _dx; }
    float	dy()  const { return _dy; }

    void	map( int x, int y, int *tx, int *ty )	      const;
    void	map( float x, float y, float *tx, float *ty ) const;
    QPoint	map( const QPoint & )	const;
    QRect	map( const QRect & )	const;
    QPointArray map( const QPointArray & ) const;

    void	reset();

    QWMatrix   &translate( float dx, float dy );
    QWMatrix   &scale( float sx, float sy );
    QWMatrix   &shear( float sh, float sv );
    QWMatrix   &rotate( float a );

    QWMatrix	invert( bool * = 0 ) const;

    bool	operator==( const QWMatrix & ) const;
    bool	operator!=( const QWMatrix & ) const;
    QWMatrix   &operator*=( const QWMatrix & );

private:
    QWMatrix   &bmul( const QWMatrix & );
    float	_m11, _m12;
    float	_m21, _m22;
    float	_dx,  _dy;
};


QWMatrix operator*( const QWMatrix &, const QWMatrix & );


/*****************************************************************************
  QWMatrix stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QWMatrix & );
QDataStream &operator>>( QDataStream &, QWMatrix & );


#endif // QWMATRIX_H
