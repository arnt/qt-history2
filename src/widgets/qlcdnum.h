/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnum.h#3 $
**
** Definition of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#ifndef QLCDNUM_H
#define QLCDNUM_H

#include "qwidget.h"
#include "qbitarry.h"


class QLCDNumber : public QWidget		// LCD number widget
{
    Q_OBJECT
public:
    QLCDNumber( QView *p, int noOfDigits = 1 );
   ~QLCDNumber();

    enum Mode {HEX, DEC, OCT, BIN};

    void    setDigits( int noOfDigits );
    int	    digits();
    bool    checkOverflow( double num ) const;
    bool    checkOverflow( long	  num ) const;

    QLCDNumber::Mode mode() const;

slots:						/* methods!!! */
    void    display( int num );
    void    display( long num );
    void    display( float num );
    void    display( double num );
    void    display( const char *str );
    void    setMode( Mode );
    void    smallDecimalPoint( bool );
signals:
    void overflow();
protected:
    void    resizeEvent( QResizeEvent * );
    void    paintEvent( QPaintEvent * );

private:
    void    drawString( const char *, QPainter &, QBitArray * = NULL,
			bool = TRUE );
    void    drawDigit( const QPoint &, QPainter &, int, char, char = ' ' );
    void    drawSegment( const QPoint &, char, QPainter &, int, bool = FALSE );

    uint      nDigits	 : 8;
    uint      base	 : 2;
    uint      smallPoint : 1;
    QString   digitStr;
    QBitArray points;
};


#endif // QLCDNUM_H
