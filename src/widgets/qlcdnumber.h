/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnumber.h#2 $
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

    enum Mode {HEXADECIMAL, DECIMAL, OCTAL, BINARY};

    bool    overflow( double num ) const;
    bool    overflow( long   num ) const;
    
    QLCDNumber::Mode mode() const { return base; }

slots:						/* methods!!! */
    void    display( int num )	  { display((long)num); }
    void    display( long num );
    void    display( float num )  { display((double)num); }
    void    display( double num );
    void    display( const char *str );
    void    setMode( Mode );
    void    smallDecimalPoint( bool );

protected:
    void    resizeEvent( QResizeEvent * );
    void    paintEvent( QPaintEvent * );

private:
    void    drawString( const char *, QPainter &, QBitArray * = NULL,
			bool = TRUE );
    void    drawDigit( const QPoint &, char, QPainter &, QPen & );
    void    drawDigit( const QPoint &, char, char, QPainter &, QPen & );
    void    drawSegment( const QPoint &, int, QPainter &, QPen &, bool= FALSE);
    int	    digitLength()     const;
    int	    segmentXLength()  const;
    int	    segmentYLength()  const;
    int	    xSpace()	      const;
    int	    xOffset()	      const;
    int	    yOffset()	      const;

    int	     nDigits;
    QString  digits;
    QBitArray points;
    bool     smallPoint;
    Mode     base;
};


#endif // QLCDNUM_H
