/****************************************************************************
** $Id: //depot/qt/main/src/%s#3 $
**
** Definition of ________ class.
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
#ifndef __QKEYGEN_H__
#define __QKEYGEN_H__

#include <qfile.h>
#include <qregexp.h>

#define MASK(n) ( (1 << (n)) - 1 )

/*
  Feel free to use Feature_Extra{1,2} when the need arises.
*/
enum { Feature_US = 0x1, Feature_Enterprise = 0x2, Feature_Unix = 0x4,
       Feature_Windows = 0x8, Feature_Mac = 0x10, Feature_Embedded = 0x20,
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

#endif /* __QKEYGEN_H__ */
