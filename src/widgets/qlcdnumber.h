/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnumber.h#43 $
**
** Definition of QLCDNumber class
**
** Created : 940518
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

#ifndef QLCDNUMBER_H
#define QLCDNUMBER_H

#ifndef QT_H
#include "qframe.h"
#include "qbitarray.h"
#endif // QT_H


class QLCDNumberPrivate;

class Q_EXPORT QLCDNumber : public QFrame		// LCD number widget
{
    Q_OBJECT
public:
    QLCDNumber( QWidget *parent=0, const char *name=0 );
    QLCDNumber( uint numDigits, QWidget *parent=0, const char *name=0 );
   ~QLCDNumber();

    enum Mode { Hex, HEX=Hex, Dec, DEC=Dec, Oct, OCT=Oct, Bin, BIN=Bin };
    enum SegmentStyle { Outline, Filled, Flat };

    bool    smallDecimalPoint() const;

    int	    numDigits() const;
    virtual void setNumDigits( int nDigits );

    bool    checkOverflow( double num ) const;
    bool    checkOverflow( int	  num ) const;

    Mode mode() const;
    virtual void setMode( Mode );

    SegmentStyle segmentStyle() const;
    virtual void setSegmentStyle( SegmentStyle );

    double  value() const;
    int	    intValue() const;

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

public slots:
    void    display( int num );
    void    display( double num );
    void    display( const QString &str );
    virtual void setHexMode();
    virtual void setDecMode();
    virtual void setOctMode();
    virtual void setBinMode();
    virtual void setSmallDecimalPoint( bool );

signals:
    void    overflow();

protected:
    void    drawContents( QPainter * );

private:
    void    init();
    void    internalDisplay( const QString &);
    void    internalSetString( const QString& s, const QBitArray* newPoints );
    void    drawString( const QString &, QPainter &, QBitArray * = 0 ) const;
    void    drawDigit( const QPoint &, QPainter &, int, char ) const;
    void    drawSegment( const QPoint &, char, QPainter &, int ) const;

    int	    ndigits;
    double  val;
    uint    base	: 2;
    uint    smallPoint	: 1;
    uint    fill	: 1;
    uint    shadow	: 1;
    QString digitStr;
    QBitArray points;
    QLCDNumberPrivate * d;

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QLCDNumber( const QLCDNumber & );
    QLCDNumber &operator=( const QLCDNumber & );
#endif
};

inline bool QLCDNumber::smallDecimalPoint() const
{ return (bool)smallPoint; }

inline int QLCDNumber::numDigits() const
{ return ndigits; }


#endif // QLCDNUMBER_H
