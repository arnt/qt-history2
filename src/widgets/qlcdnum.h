/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnum.h#20 $
**
** Definition of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994-1996 by Troll Tech AS.  All rights reserved.
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

    bool    smallDecimalPoint() const;

    int	    numDigits() const;
    void    setNumDigits( int nDigits );

    bool    checkOverflow( double num ) const;
    bool    checkOverflow( int	  num ) const;

    QLCDNumber::Mode mode() const;
    void    setMode( Mode );

    double  value() const;
    int     intValue() const;

public slots:
    void    display( int num );
    void    display( double num );
    void    display( const char *str );
    void    setHexMode();
    void    setDecMode();
    void    setOctMode();
    void    setBinMode();
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

#if defined(OBSOLETE)
public:
    long    longValue() const;
#endif

private:	// Disabled copy constructor and operator=
    QLCDNumber( const QLCDNumber & ) {}
    QLCDNumber &operator=( const QLCDNumber & ) { return *this; }
};

inline bool QLCDNumber::smallDecimalPoint() const
{ return (bool)smallPoint; }

inline int QLCDNumber::numDigits() const
{ return ndigits; }


#if defined(OBSOLETE)
inline long QLCDNumber::longValue() const
{
    qObsolete( "QLCDNumber", "longValue", "intValue" );
    return intValue();
}
#endif

#endif // QLCDNUM_H
