/****************************************************************************
** $Id$
**
** Created : 970521
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qregexp.h>

#include "keyinfo.h"

QString encodeBaseZ( uint k, int numDigits )
{
    const char alphabet[] = "X9MUEC7AJH3KS6DB4YFG2L5PQRT8VNZ";
    const int AlphabetSize = 31;
    QString str;

    for ( int i = 0; i < numDigits; i++ ) {
	str += QChar( alphabet[ k % AlphabetSize ] );
	k /= AlphabetSize;
    }
    return str;
}

uint decodeBaseZ( const QString& str )
{
    const char alphabet[] = "X9MUEC7AJH3KS6DB4YFG2L5PQRT8VNZ";
    const int AlphabetSize = 31;
    uint k = 0;
    int i = str.length();

    while ( i > 0 ) {
	i--;
	const char *p = strchr( alphabet, str[i].latin1() );
	if ( p == 0 ) {
	    return 0;
	} else {
	    k = ( k * AlphabetSize ) + ( p - alphabet );
	}
    }
    return k;
}

static inline uint shuffledBits( uint bits )
{
    const uint OddPrime = 23;
    uint shuf = 0;

    for ( int i = 0; i < 32; i++ ) {
	shuf = ( shuf << 1 ) | ( bits & 0x1 );
	bits = ( bits >> OddPrime ) | ( bits << (32 - OddPrime) );
    }
    return shuf;
}

static inline uint succ( uint x )
{
    return 3141592621u * ( x ^ 0xa5a5a5a5 ) + 1;
}

static inline uint makeCheck( uint features, uint randomBits )
{
    uint s = succ( (features << NumRandomBits) | (randomBits & RandomBitMask) );
    return shuffledBits( s ) & CheckMask;
}

static inline uint makeCode( uint features, uint check )
{
    return ( check << NumFeatures ) | features;
}

static inline uint featuresForCode( uint code )
{
    return code & FeatureMask;
}

static inline uint checkForCode( uint code )
{
    return code >> NumFeatures;
}

static QString keyForCode( uint code )
{
    QString s = encodeBaseZ( code, 7 );
    uint extra = code & ( Feature_US | Feature_Enterprise | Feature_Unix | Feature_Embedded );
    s.prepend( QChar( "23456789ABCDEFGH"[ extra ] ) );
    s.insert( 4, QChar('-') );

    return s;
}

static uint codeForKey( const QString& key )
{
    QRegExp fmt( QString("[A-Z0-9]([A-Z0-9]{3})-([A-Z0-9]{4})(?:-[A-Z0-9]{4})?") );
    QString t = key.stripWhiteSpace().upper();

    if ( fmt.exactMatch(t) ) {
	QString u = fmt.cap( 1 ) + fmt.cap( 2 );
	uint code = decodeBaseZ( u );
	uint extra = QString( t[0] ).toInt( 0, 18 ) - 2;

	if ( ((featuresForCode(code) ^ extra) &
	      (Feature_US | Feature_Enterprise | Feature_Unix)) == 0 ) {
	    return code;
	} else {
	    return 0;
	}
    } else {
	return 0;
    }
}

QString keyForFeatures( uint features, uint randomBits )
{
    uint check = makeCheck( features, randomBits );
    return keyForCode( makeCode(features, check) );
    return "";
}

uint featuresForKey( const QString& key )
{
    uint code = codeForKey( key );
    uint features = featuresForCode( code );
    uint check = checkForCode( code );

    for ( uint bits = 0; bits < (1 << NumRandomBits); bits++ ) {
	if ( check == makeCheck(features, bits) )
	    return features;
    }
    return 0;
}

uint featuresForKeyOnUnix( const QString& key )
{
    QString t = key.simplifyWhiteSpace().upper();

    if ( t.isEmpty() )
	return 0;

    uint ch = t[0].unicode();
    uint features = 0;
    if ( QString("ABCDEFGH").find(QChar(ch)) != -1 )
	features |= Feature_Embedded;
    if ( QString("6789EFGH").find(QChar(ch)) != -1 )
	features |= Feature_Unix;
    if ( QString("4589CDGH").find(QChar(ch)) != -1 )
	features |= Feature_Enterprise;
    if ( QString("3579BDFH").find(QChar(ch)) != -1 )
	features |= Feature_US;
    return features;
}

QString encodedExpiryDate( const QDate& date )
{
    const QDate StartDate( 2001, 1, 1 );
    const uint MaxDays = 4000;
    uint days = StartDate.daysTo( date );
    if ( days >= MaxDays )
	days = MaxDays - 1;

    uint x = ( days << 7 ) ^ days;
    return QString( "-" ) + encodeBaseZ( x ^ 0x0000beef, 4 );
}

QDate decodedExpiryDate( const QString& encodedDate )
{
    const QDate StartDate( 2001, 1, 1 );
    const uint MaxDays = 4000;
    QRegExp fmt( QString("-([A-Z0-9]{4})") );
    QString t = encodedDate.stripWhiteSpace().upper();
    QDate date;

    if ( fmt.exactMatch(t) ) {
	uint y = decodeBaseZ( fmt.cap(1) );
	uint x = y ^ 0x0000beef;
	uint days = ( (x >> 7) ^ x ) >> 7;

	if ( days >= MaxDays )
	    return QDate();

	date = StartDate.addDays( days );
	if ( encodedExpiryDate(date) != t )
	    return QDate();
    }
    return date;
}
