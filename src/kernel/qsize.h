/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.h#18 $
**
** Definition of QSize class
**
** Created : 931028
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QSIZE_H
#define QSIZE_H

#ifndef QT_H
#include "qpoint.h"
#endif // QT_H

#if defined(QSIZE_C) || defined(DEBUG)
#define QSIZE_DEBUG
#endif


class QSize
{
public:
    QSize()	{ wd = ht = -1; }
    QSize( int w, int h );

    bool   isNull()	const;
    bool   isEmpty()	const;
    bool   isValid()	const;

    int	   width()	const;
    int	   height()	const;
    void   setWidth( int w );
    void   setHeight( int h );
    void   transpose();

    QSize max( const QSize & );

    QCOORD &rwidth();
    QCOORD &rheight();

    QSize &operator+=( const QSize & );
    QSize &operator-=( const QSize & );
    QSize &operator*=( int c );
    QSize &operator*=( float c );
    QSize &operator/=( int c );
    QSize &operator/=( float c );

    friend inline bool	operator==( const QSize &, const QSize & );
    friend inline bool	operator!=( const QSize &, const QSize & );
    friend inline QSize operator+( const QSize &, const QSize & );
    friend inline QSize operator-( const QSize &, const QSize & );
    friend inline QSize operator*( const QSize &, int );
    friend inline QSize operator*( int, const QSize & );
    friend inline QSize operator*( const QSize &, float );
    friend inline QSize operator*( float, const QSize & );
#if defined(QSIZE_DEBUG)
    friend	  QSize operator/( const QSize &, int );
    friend	  QSize operator/( const QSize &, float );
#else
    friend inline QSize operator/( const QSize &, int );
    friend inline QSize operator/( const QSize &, float );
#endif

private:
    QCOORD wd;
    QCOORD ht;
};


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

QDataStream &operator<<( QDataStream &, const QSize & );
QDataStream &operator>>( QDataStream &, QSize & );


/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

inline QSize::QSize( int w, int h )
{ wd=(QCOORD)w; ht=(QCOORD)h; }

inline bool QSize::isNull() const
{ return wd==0 && ht==0; }

inline bool QSize::isEmpty() const
{ return wd<1 || ht<1; }

inline bool QSize::isValid() const
{ return wd>=0 && ht>=0; }

inline int QSize::width() const
{ return wd; }

inline int QSize::height() const
{ return ht; }

inline void QSize::setWidth( int w )
{ wd=(QCOORD)w; }

inline void QSize::setHeight( int h )
{ ht=(QCOORD)h; }

inline QCOORD &QSize::rwidth()
{ return wd; }

inline QCOORD &QSize::rheight()
{ return ht; }

inline QSize &QSize::operator+=( const QSize &s )
{ wd+=s.wd; ht+=s.ht; return *this; }

inline QSize &QSize::operator-=( const QSize &s )
{ wd-=s.wd; ht-=s.ht; return *this; }

inline QSize &QSize::operator*=( int c )
{ wd*=(QCOORD)c; ht*=(QCOORD)c; return *this; }

inline QSize &QSize::operator*=( float c )
{ wd=(QCOORD)(wd*c); ht=(QCOORD)(ht*c); return *this; }

inline bool operator==( const QSize &s1, const QSize &s2 )
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=( const QSize &s1, const QSize &s2 )
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline QSize operator+( const QSize & s1, const QSize & s2 )
{ return QSize(s1.wd+s2.wd, s1.ht+s2.ht); }

inline QSize operator-( const QSize &s1, const QSize &s2 )
{ return QSize(s1.wd-s2.wd, s1.ht-s2.ht); }

inline QSize operator*( const QSize &s, int c )
{ return QSize(s.wd*c, s.ht*c); }

inline QSize operator*( int c, const QSize &s )
{  return QSize(s.wd*c, s.ht*c); }

inline QSize operator*( const QSize &s, float c )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline QSize operator*( float c, const QSize &s )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

#if !defined(QSIZE_DEBUG)

inline QSize &QSize::operator/=( int c )
{ wd/=(QCOORD)c; ht/=(QCOORD)c; return *this; }

inline QSize &QSize::operator/=( float c )
{ wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c); return *this; }

inline QSize operator/( const QSize &s, int c )
{ return QSize(s.wd/c, s.ht/c); }

inline QSize operator/( const QSize &s, float c )
{ return QSize((QCOORD)(s.wd/c), (QCOORD)(s.ht/c)); }

#endif // no-debug functions


inline QSize QSize::max( const QSize & otherSize )
{
    return QSize( QMAX( wd, otherSize.wd ), QMAX( ht, otherSize.ht ) );
}

#endif // QSIZE_H
