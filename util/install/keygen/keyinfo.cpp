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

#include "keyinfo.h"

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

/*
  We avoid '0', '1', 'O' and 'I' because they are confusing. We avoid
  one more letter, 'W', so that our alphabet size is not a power of
  two.

  The alphabet is quite visible in the binary of the program. It
  doesn't matter; the goal is to make the keys look more random, not
  to prevent cracking.
*/
static const char alphabet[] = "X9MUEC7AJH3KS6DB4YFG2L5PQRT8VNZ";
static const int AlphabetSize = 31;

static QString keyForCode( uint code )
{
    QString s;
    uint k = code;

    for ( int i = 0; i < 7; i++ ) {
	s += QChar( alphabet[k % AlphabetSize] );
	k /= AlphabetSize;
    }

    uint extra = code & ( Feature_US | Feature_Enterprise | Feature_Unix );
    s.prepend( QChar('A' + extra) );
    s.insert( 4, QChar('-') );

    return s;
}

static uint codeForKey( const QString& key )
{
    QRegExp fmt( QString("[A-H]([A-Z0-9]{3})-([A-Z0-9]{4})") );
    QString t;

    for ( int i = 0; i < (int) key.length(); i++ ) {
	QChar ch = key[i];
	if ( !ch.isSpace() )
	    t += ch.upper();
    }

    if ( fmt.exactMatch(t) ) {
	QString u = fmt.cap( 1 ) + fmt.cap( 2 );
	uint code = 0;
	uint extra = t[0].unicode() - 'A';

	int i = 7;
	while ( i > 0 ) {
	    i--;
	    const char *p = strchr( alphabet, u[i].latin1() );
	    if ( p == 0 ) {
		return 0;
	    } else {
		code = ( code * AlphabetSize ) + ( p - alphabet );
	    }
	}

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

    if ( t.isEmpty() || QString("ABCDEFGH").find(t[0]) == -1 )
	return 0;

    uint ch = t[0].unicode();
    uint features = 0;
    if ( QString("EFGH").find(QChar(ch)) != -1 ) {
	features |= Feature_Unix;
	ch -= 4;
    }
    if ( QString("CD").find(QChar(ch)) != -1 ) {
	features |= Feature_Enterprise;
	ch -= 2;
    }
    if ( QChar(ch) == QChar('B') )
	features |= Feature_US;
    return features;
}
