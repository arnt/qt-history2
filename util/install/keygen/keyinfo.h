/****************************************************************************
**
** Definition of ________ class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef KEYINFO_H
#define KEYINFO_H

#include <qdatetime.h>
#include <qstring.h>

#define MASK(n) ( (1 << (n)) - 1 )

/*
  Feel free to use Feature_Extra{1,2} when the need arises.
*/
enum { Feature_US = 0x1, Feature_Enterprise = 0x2, Feature_Unix = 0x4,
       Feature_Embedded = 0x8, Feature_Windows = 0x10, Feature_Mac = 0x20,
       Feature_Extra1 = 0x40, Feature_Extra2 = 0x80,

       NumFeatures = 8,
       FeatureMask = MASK( NumFeatures ) };

enum { CheckMask = MASK( 32 - NumFeatures ) };

/*
  (1 << NumRandomBits) keys are generated per feature set.
  NumRandomBits must at most 32 - 2 * NumFeatures.
*/
enum { NumRandomBits = 12,
       RandomBitMask = MASK( NumRandomBits ) };

uint featuresForKey( const QString& key );
QString keyForFeatures( uint features, uint randomBits );
uint featuresForKeyOnUnix( const QString& key );

QString encodedExpiryDate( const QDate& date );
QDate decodedExpiryDate( const QString& encodedDate );

#endif
