/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qlcdnumber.cpp#5 $
**
** Implementation of QLCDNumber class
**
** Author  : Eirik Eng
** Created : 940518
**
** Copyright (C) 1994 by Troll Tech AS.	 All rights reserved.
**
*****************************************************************************/

#include "qlcdnum.h"
#include "qbitarry.h"
#include "qpainter.h"
#include <stdio.h>

#if defined(DEBUG)
static char ident[] = "$Id: //depot/qt/main/src/widgets/qlcdnumber.cpp#5 $";
#endif


static QString long2string( long num, int base, int ndigits, bool *oflow )
{
    QString s;
    bool negative;
    if ( num < 0 ) {
	negative = TRUE;
	num	 = -num;
    } else {
	negative = FALSE;
    }
    switch( base ) {
	case QLCDNumber::HEX:
	    s.sprintf( "%*lx", ndigits, num );
	    break;
	case QLCDNumber::DEC:
	    s.sprintf( "%*li", ndigits, num );
	    break;
	case QLCDNumber::OCT:
	    s.sprintf( "%*lo", ndigits, num );
	    break;
	case QLCDNumber::BIN: {
	    char buf[42];
	    char *p = &buf[41];
	    ulong n = num;
	    int len = 0;
	    *p = '\0';
	    do {
		*--p = ((n&1)+'0');
		n >>= 1;
		len++;
	    } while ( n != 0 );
	    len = ndigits - len;
	    if ( len > 0 )
	    s.fill( ' ', len );
	    s += p;
	    }
	    break;
    }
    if ( negative ) {
	for ( int i=0; i<s.length(); i++ ) {
	    if ( s[i] != ' ' ) {
		if ( i != 0 ) {
		    s[i-1] = '-';
		} else {
		    s.insert( 0, '-' );
		}
		break;
	    }
	}
    }
    if ( oflow )
	*oflow = s.length() > ndigits;
    return s;
}


QString double2string( double num, int base, int ndigits, bool *oflow )
{
    QString s;
    if ( base != QLCDNumber::DEC ) {
	bool of = num >= 2147483648.0 || num < -2147483648.0;
	if ( of ) {				// oops, 'long' overflow
	    if ( oflow )
		*oflow = TRUE;
	    return s;
	}
	s = long2string( (long)num, base, ndigits, 0 );
    } else {					// decimal base
	s.sprintf( "%*g", ndigits, num );
	int i = s.find('e');
	if ( i > 0 && s[i+1]=='+' ) {
	    s[i] = ' ';
	    s[i+1] = 'e';
	}
    }
    if ( oflow )
	*oflow = s.length() > ndigits;
    return s;
}


char *getSegments( char ch )			// gets list of segments for ch
{
    static char segments[30][8] =
       { { 0, 1, 2, 4, 5, 6,-1, 0},		// 0	0 / O
	 { 2, 5,-1, 0, 0, 0, 0, 0},		// 1	1
	 { 0, 2, 3, 4, 6,-1, 0, 0},		// 2	2
	 { 0, 2, 3, 5, 6,-1, 0, 0},		// 3	3
	 { 1, 2, 3, 5,-1, 0, 0, 0},		// 4	4
	 { 0, 1, 3, 5, 6,-1, 0, 0},		// 5	5 / S
	 { 0, 1, 3, 4, 5, 6,-1, 0},		// 6	6
	 { 0, 2, 5,-1, 0, 0, 0, 0},		// 7	7
	 { 0, 1, 2, 3, 4, 5, 6,-1},		// 8	8
	 { 0, 1, 2, 3, 5, 6,-1, 0},		// 9	9 / g
	 { 3,-1, 0, 0, 0, 0, 0, 0},		// 10	-
	 { 7,-1, 0, 0, 0, 0, 0, 0},		// 11	.
	 { 0, 1, 2, 3, 4, 5,-1, 0},		// 12	A
	 { 1, 3, 4, 5, 6,-1, 0, 0},		// 13	B
	 { 0, 1, 4, 6,-1, 0, 0, 0},		// 14	C
	 { 2, 3, 4, 5, 6,-1, 0, 0},		// 15	D
	 { 0, 1, 3, 4, 6,-1, 0, 0},		// 16	E
	 { 0, 1, 3, 4,-1, 0, 0, 0},		// 17	F
	 { 1, 3, 4, 5,-1, 0, 0, 0},		// 18	h
	 { 1, 2, 3, 4, 5,-1, 0, 0},		// 19	H
	 { 1, 4, 6,-1, 0, 0, 0, 0},		// 20	L
	 { 3, 4, 5, 6,-1, 0, 0, 0},		// 21	o
	 { 0, 1, 2, 3, 4,-1, 0, 0},		// 22	P
	 { 3, 4,-1, 0, 0, 0, 0, 0},		// 23	r
	 { 4, 5, 6,-1, 0, 0, 0, 0},		// 24	u
	 { 1, 2, 4, 5, 6,-1, 0, 0},		// 25	U
	 { 1, 2, 3, 5, 6,-1, 0, 0},		// 26	Y
	 { 8, 9,-1, 0, 0, 0, 0, 0},		// 27	:
	 { 0, 1, 2, 3,-1, 0, 0, 0},		// 28	'
	 {-1, 0, 0, 0, 0, 0, 0, 0} };		// 29	empty

    if (ch >= '0' && ch <= '9')
	return segments[ch - '0'];
    if (ch >= 'A' && ch <= 'F')
	return segments[ch - 'A' + 12];
    if (ch >= 'a' && ch <= 'f')
	return segments[ch - 'a' + 12];
    if (ch == '-')
	return segments[10];
    if (ch == 'O')
	return segments[0];
    if (ch == 'g')
	return segments[9];
    if (ch == '.')
	return segments[11];
    if (ch == 'h')
	return segments[18];
    if (ch == 'H')
	return segments[19];
    if (ch == 'l' || ch == 'L')
	return segments[20];
    if (ch == 'o')
	return segments[21];
    if (ch == 'p' || ch == 'P')
	return segments[22];
    if (ch == 'r' || ch == 'R')
	return segments[23];
    if (ch == 's' || ch == 'S')
	return segments[5];
    if (ch == 'u')
	return segments[24];
    if (ch == 'U')
	return segments[25];
    if (ch == 'y' || ch == 'Y')
	return segments[26];
    if (ch == ':')
	return segments[27];
    if (ch == '\'')
	return segments[28];
    return segments[29];
}


QLCDNumber::QLCDNumber( QView *parent, const char *name )
	: QWidget( parent, name )
{
    ndigits = 1;
    init();
}

QLCDNumber::QLCDNumber( uint numDigits, QView *parent, const char *name )
	: QWidget( parent, name )
{
    ndigits = numDigits;
    init();
}

void QLCDNumber::init()
{
    initMetaObject();
    base = DEC;
    smallPoint = FALSE;
    setNumDigits( ndigits );
    setBackgroundColor( lightGray );
}

QLCDNumber::~QLCDNumber()
{
}


void QLCDNumber::setNumDigits( uint numDigits )
{
    if ( numDigits > 99 ) {
#if defined(CHECK_RANGE)
	warning( "QLCDNumber: Maximum 99 digits allowed" );
#endif
	numDigits = 99;
    }
    if ( numDigits == ndigits )			// no change
	return;
    if ( digitStr.isNull() ) {
	ndigits = numDigits;
	digitStr.fill( ' ', ndigits );
	points.fill( 0, ndigits );
	digitStr[ndigits - 1] = '0';		// "0" is the default number
    }
    else {
	int i;
	int dif;
	if ( numDigits > ndigits ) {		// expand
	    dif = numDigits - ndigits;
	    QString buf;
	    buf.fill( ' ', dif );
	    digitStr.insert( 0, buf );
	    points.resize( numDigits );
	    for ( i=numDigits-1; i>=dif; i-- )
		points.setBit( i, points.testBit(i-dif) );
	    for ( i=0; i<dif; i++ )
		points.clearBit( i );
	}
	else {					// shrink
	    dif = ndigits - numDigits;
	    digitStr = digitStr.right( numDigits );	    
	    QBitArray tmpPoints = points;
	    points.resize( numDigits );
	    for ( i=0; i<numDigits; i++ )
		points.setBit( i, tmpPoints.testBit(i+dif) );
	}
	ndigits = numDigits;
	update();
    }
}


bool QLCDNumber::checkOverflow( long num ) const
{
    bool of;
    long2string( num, base, ndigits, &of );
    return of;
}

bool QLCDNumber::checkOverflow( double num ) const
{
    bool of;
    double2string( num, base, ndigits, &of );
    return of;
}

QLCDNumber::Mode QLCDNumber::mode() const
{
    return (QLCDNumber::Mode) base;
}

void QLCDNumber::display( int num )
{
    display( (long) num );
}

void QLCDNumber::display( float num )
{
    display( (double) num );
}

void QLCDNumber::display( long num )
{
    bool of;
    QString s = long2string( num, base, ndigits, &of );
    if ( of )
	emit overflow();
    else
	display( s );
}

void QLCDNumber::display( double num )
{
    bool of;
    QString s = double2string( num, base, ndigits, &of );
    if ( of )
	emit overflow();
    else
	display( s );
}


void QLCDNumber::display( const char *s )
{
    QPainter p;
    QString buffer(ndigits+1);
    int i;
    int len = strlen(s);

    p.begin( this );
    if ( !smallPoint ) {
	if ( len >= ndigits ) {			  // String too long?
	    for( i=0; i<ndigits; i++ )		  // Yes, show first chars.
		buffer[i] = s[len - ndigits + i];
	} else {
	    for( i=0; i<ndigits-len; i++ )	  // Pad with spaces.
		buffer[i] = ' ';
	    for( i=0; i<len; i++ )
		buffer[ndigits - len + i] = s[i];
	}
	drawString( buffer, p );
   } else {
	int  index = -1;
	bool lastWasPoint = TRUE;
	QBitArray newPoints(ndigits);
	newPoints.clearBit(0);
	for ( i=0; i<len; i++ ) {
	    if ( s[i] == '.' ) {
		if ( lastWasPoint ) {		// Point already set for digit?
		    if ( index == ndigits - 1 ) // No more digits?
			break;
		    index++;
		    buffer[index] = ' ';	// 2 points in a row, add space
		}
		newPoints.setBit(index);	// Set decimal point
		lastWasPoint = TRUE;
	    } else {
		if ( index == ndigits - 1 )
		    break;
		index++;
		buffer[index] = s[i];
		newPoints.clearBit(index);     // Decimal point default off
		lastWasPoint = FALSE;
	    }
	}
	if ( index < ndigits - 1 ) {
	    for( i=index; i>=0; i-- ) {
		buffer[ndigits - 1 - index + i] = buffer[i];
		newPoints.setBit( ndigits - 1 - index + i,
				   newPoints.testBit(i) );
	    }
	    for( i=0; i<ndigits-index-1; i++ ) {
		buffer[i] = ' ';
		newPoints.clearBit(i);
	    }
	}
	drawString(buffer,p,&newPoints);
    }
    p.end();
}


void QLCDNumber::setMode( Mode m )
{
    if ( base == m )
	return;
    base = m;
    display( "" );
}

void QLCDNumber::smallDecimalPoint( bool b )
{
    if ( smallPoint == b )
	return;
    smallPoint = b;
    display( "" );
}


void QLCDNumber::resizeEvent( QResizeEvent * )
{
}

void QLCDNumber::paintEvent( QPaintEvent * )
{
    QPainter p;
    QColor tc, bc;

    p.begin(this);
    tc = backgroundColor().dark();
    bc = backgroundColor().light();
    p.drawShadePanel( clientRect(), tc, bc, 1, 1 );
    p.pen().setColor( black );
    if ( smallPoint )
	drawString( digitStr, p, &points, FALSE );
    else
	drawString( digitStr, p, 0, FALSE );
    p.end();
}

void QLCDNumber::drawString( const char *s, QPainter &p,
			     QBitArray *newPoints, bool newString )
{
    QPoint  pos;

    int digitSpace = smallPoint ? 2 : 1;
    int xSegLen	   = clientWidth()*5/
			 (ndigits*(5 + digitSpace) + digitSpace);
    int ySegLen	   = clientHeight()*5/12;
    int segLen	   = ySegLen > xSegLen ? xSegLen : ySegLen;
    int xAdvance   = segLen*( 5 + digitSpace )/5;
    int xOffset	   = ( clientWidth() - ndigits*xAdvance + segLen/5 )/2;
    int yOffset	   = ( clientHeight() - segLen*2 )/2;

    for ( int i=0;  i<ndigits; i++ ) {
	pos = QPoint( xOffset + xAdvance*i, yOffset );
	if ( newString )
	    drawDigit( pos, p, segLen, s[i], digitStr[i] );
	else
	    drawDigit( pos, p, segLen, s[i]);
	if ( newPoints ) {
	    char newPoint = newPoints->testBit(i) ? '.' : ' ';
	    if ( newString ) {
		char oldPoint = points.testBit(i) ? '.' : ' ';
		drawDigit( pos, p, segLen, newPoint, oldPoint );
	    } else {
		drawDigit( pos, p, segLen, newPoint );
	    }
	}
    }
    if ( newString ) {
	digitStr = s;
	if ( digitStr.length() > ndigits )
	    digitStr.resize( ndigits );
	if ( newPoints )
	    points = *newPoints;
    }
}

void QLCDNumber::drawDigit( const QPoint &pos, QPainter &p, int segLen,
			    char newCh, char oldCh )
{
/* Draws and/or erases segments to change display of a single digit
   from oldCh to newCh */

    char updates[18][2]; // Can hold 2 times number of segments, only
			 // first 9 used if segment table is correct.
    int	 nErases;
    int	 nUpdates;
    char *segs;
    int	 i,j;

    const int erase	 = 0;
    const int draw	 = 1;
    const int leaveAlone = 2;

    segs = getSegments(oldCh);
    for ( nErases=0; segs[nErases] != -1; nErases++ ) {
	updates[nErases][0] = erase;		// get segments to erase to
	updates[nErases][1] = segs[nErases];	// remove old char.
    }
    nUpdates = nErases;
    segs = getSegments(newCh);
    for(i = 0 ; segs[i] != -1 ; i++) {
	for ( j=0;  j<nErases; j++ )
	    if ( segs[i] == updates[j][1] ) {	// same segment ?
		updates[j][0] = leaveAlone;	// yes, already on screen
		break;
	    }
	if ( j == nErases ) {			// if not already on screen
	    updates[nUpdates][0] = draw;
	    updates[nUpdates][1] = segs[i];
	    nUpdates++;
	}
    }
    for ( i=0; i<nUpdates; i++ ) {
	if ( updates[i][0] == draw )
	    drawSegment( pos, updates[i][1], p, segLen );
	if (updates[i][0] == erase)
	    drawSegment( pos, updates[i][1], p, segLen, TRUE );
    }
}

void QLCDNumber::drawSegment( const QPoint &pos, char segmentNo, QPainter &p,
			      int segLen, bool erase )
{
    QPoint pt = pos;
    QColor lightColor,darkColor;
    if ( erase ){
	lightColor = backgroundColor();
	darkColor  = lightColor;
    } else {
	lightColor = backgroundColor().light();
	darkColor  = backgroundColor().dark();
    }
    int width = segLen/5;

#define LINETO(X,Y) p.lineTo(QPoint(pt.x() + (X),pt.y() + (Y)))
#define LIGHT p.pen().setColor(lightColor)
#define DARK  p.pen().setColor(darkColor)

    switch ( segmentNo ) {
	case 0 :
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(segLen - 1,0);
	    DARK;
	    LINETO(segLen - width - 1,width);
	    LINETO(width,width);
	    LINETO(0,0);
	    break;
	case 1 :
	    pt.ry()++;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,width);
	    DARK;
	    LINETO(width,segLen - width/2 - 2);
	    LINETO(0,segLen - 2);
	    LIGHT;
	    LINETO(0,0);
	    break;
	case 2 :
	    pt.rx() += segLen - 1;
	    pt.ry()++;
	    p.moveTo(pt);
	    DARK;
	    LINETO(0,segLen - 2);
	    LINETO(-width,segLen - width/2 - 2);
	    LIGHT;
	    LINETO(-width,width);
	    LINETO(0,0);
	    break;
	case 3 :
	    pt.ry() += segLen;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,-width/2);
	    LINETO(segLen - width - 1,-width/2);
	    LINETO(segLen - 1,0);
	    DARK;
	    if (width & 1) {	// Adjust for integer division error.
		LINETO(segLen - width - 3,width/2 + 1);
		LINETO(width + 2,width/2 + 1);
	    } else {
		LINETO(segLen - width - 1,width/2);
		LINETO(width,width/2);
	    }
	    LINETO(0,0);
	    break;
	case 4 :
	    pt.ry() += segLen + 1;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,width/2);
	    DARK;
	    LINETO(width,segLen - width - 2);
	    LINETO(0,segLen - 2);
	    LIGHT;
	    LINETO(0,0);
	    break;
	case 5 :
	    pt.rx() += segLen - 1;
	    pt.ry() += segLen + 1;
	    p.moveTo(pt);
	    DARK;
	    LINETO(0,segLen - 2);
	    LINETO(-width,segLen - width - 2);
	    LIGHT;
	    LINETO(-width,width/2);
	    LINETO(0,0);
	    break;
	case 6 :
	    pt.ry() += 2*segLen;
	    p.moveTo(pt);
	    LIGHT;
	    LINETO(width,-width);
	    LINETO(segLen - width - 1,-width);
	    LINETO(segLen - 1,0);
	    DARK;
	    LINETO(0,0);
	    break;
	case 7 :
	    if ( smallPoint )  // If smallpoint place'.' between other digits
		pt.rx() += segLen + width/2;
	    else
		pt.rx() += segLen/2;
	    pt.ry() += 2*segLen;
	    p.moveTo(pt);
	    DARK;
	    LINETO(width,0);
	    LINETO(width,-width);
	    LIGHT;
	    LINETO(0,-width);
	    LINETO(0,0);
	    break;
	case 8 :
	    pt.ry() += segLen/2 + width;
	    pt.rx() += segLen/2 - width/2 + 1;
	    p.moveTo(pt);
	    DARK;
	    LINETO(width,0);
	    LINETO(width,-width);
	    LIGHT;
	    LINETO(0,-width);
	    LINETO(0,0);
	    break;
	case 9 :
	    pt.ry() += 3*segLen/2 + width;
	    pt.rx() += segLen/2 - width/2 + 1;
	    p.moveTo(pt);
	    DARK;
	    LINETO(width,0);
	    LINETO(width,-width);
	    LIGHT;
	    LINETO(0,-width);
	    LINETO(0,0);
	    break;
#if defined(CHECK_RANGE)
	default :
	    warning( "QLCDNumber::drawSegment: Internal error."
		     "	Illegal segment id: %s\n", segmentNo );
#endif
    }
#undef LINETO
#undef LIGHT
#undef DARK
}
