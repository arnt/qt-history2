/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qsize.h#27 $
**
** Definition of QSize class
**
** Created : 931028
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QSIZE_H
#define QSIZE_H

#ifndef QT_H
#include "qpoint.h"
#endif // QT_H


class Q_EXPORT QSize
{
public:
    QSize();
    QSize( int w, int h );

    bool   isNull()	const;
    bool   isEmpty()	const;
    bool   isValid()	const;

    int	   width()	const;
    int	   height()	const;
    void   setWidth( int w );
    void   setHeight( int h );
    void   transpose();

    QSize expandedTo( const QSize & ) const;
    QSize boundedTo( const QSize & ) const;

    QCOORD &rwidth();
    QCOORD &rheight();

    QSize &operator+=( const QSize & );
    QSize &operator-=( const QSize & );
    QSize &operator*=( int c );
    QSize &operator*=( double c );
    QSize &operator/=( int c );
    QSize &operator/=( double c );

    friend inline bool	operator==( const QSize &, const QSize & );
    friend inline bool	operator!=( const QSize &, const QSize & );
    friend inline QSize operator+( const QSize &, const QSize & );
    friend inline QSize operator-( const QSize &, const QSize & );
    friend inline QSize operator*( const QSize &, int );
    friend inline QSize operator*( int, const QSize & );
    friend inline QSize operator*( const QSize &, double );
    friend inline QSize operator*( double, const QSize & );
    friend inline QSize operator/( const QSize &, int );
    friend inline QSize operator/( const QSize &, double );

private:
    static void warningDivByZero();

    QCOORD wd;
    QCOORD ht;
};


/*****************************************************************************
  QSize stream functions
 *****************************************************************************/

Q_EXPORT QDataStream &operator<<( QDataStream &, const QSize & );
Q_EXPORT QDataStream &operator>>( QDataStream &, QSize & );


/*****************************************************************************
  QSize inline functions
 *****************************************************************************/

inline QSize::QSize()
{ wd = ht = -1; }

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

inline QSize &QSize::operator*=( double c )
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

inline QSize operator*( const QSize &s, double c )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline QSize operator*( double c, const QSize &s )
{ return QSize((QCOORD)(s.wd*c), (QCOORD)(s.ht*c)); }

inline QSize &QSize::operator/=( int c )
{
#if defined(CHECK_MATH)
    if ( c == 0 )
	warningDivByZero();
#endif
    wd/=(QCOORD)c; ht/=(QCOORD)c;
    return *this;
}

inline QSize &QSize::operator/=( double c )
{
#if defined(CHECK_MATH)
    if ( c == 0.0 )
	warningDivByZero();
#endif
    wd=(QCOORD)(wd/c); ht=(QCOORD)(ht/c);
    return *this;
}

inline QSize operator/( const QSize &s, int c )
{
#if defined(CHECK_MATH)
    if ( c == 0 )
	QSize::warningDivByZero();
#endif
    return QSize(s.wd/c, s.ht/c);
}

inline QSize operator/( const QSize &s, double c )
{
#if defined(CHECK_MATH)
    if ( c == 0.0 )
	QSize::warningDivByZero();
#endif
    return QSize((QCOORD)(s.wd/c), (QCOORD)(s.ht/c));
}

inline QSize QSize::expandedTo( const QSize & otherSize ) const
{
    return QSize( QMAX(wd,otherSize.wd), QMAX(ht,otherSize.ht) );
}

inline QSize QSize::boundedTo( const QSize & otherSize ) const
{
    return QSize( QMIN(wd,otherSize.wd), QMIN(ht,otherSize.ht) );
}


#endif // QSIZE_H
