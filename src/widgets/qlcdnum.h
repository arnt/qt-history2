/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnum.h#13 $
**
** Definition of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994,1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QLCDNUM_H
#define QLCDNUM_H

#include "qframe.h"
#include "qbitarry.h"


class QLCDNumber : public QFrame		// LCD number widget
{
    Q_OBJECT
public:
    QLCDNumber( QWidget *parent=0, const char *name=0 );
    QLCDNumber( uint numDigits, QWidget *parent=0, const char *name=0 );
   ~QLCDNumber();

    enum Mode { HEX, DEC, OCT, BIN };

    bool    smallDecimalPoint() const { return (bool)smallPoint; }

    int	    numDigits() const	{ return ndigits; }
    void    setNumDigits( int nDigits );

    bool    checkOverflow( double num ) const;
    bool    checkOverflow( long	  num ) const;

    QLCDNumber::Mode mode() const;

    double  value() const;
    long    longValue() const;
public slots:
    void    display( int num );
    void    display( long num );
    void    display( float num );
    void    display( double num );
    void    display( const char *str );
    void    setMode( Mode );
    void    setSmallDecimalPoint( bool );

signals:
    void    overflow();

protected:
    void    resizeEvent( QResizeEvent * );
    void    drawContents( QPainter * );

private:
    void    init();
    void    internalDisplay( const char * );
    void    drawString( const char *, QPainter &, QBitArray * = 0,
			bool = TRUE );
    void    drawDigit( const QPoint &, QPainter &, int, char, char = ' ' );
    void    drawSegment( const QPoint &, char, QPainter &, int, bool = FALSE );

    int	    ndigits;
    double  val;
    uint    base	: 2;
    uint    smallPoint	: 1;
    QString digitStr;
    QBitArray points;
};


#endif // QLCDNUM_H
