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

#include <qfile.h>

#include <stdio.h>
#include <stdlib.h>

#include "keyinfo.h"

static void printNewKey( uint features, const QDate& expiryDate )
{
    QFile in;
    QFile out;
    QString fn;
    char block[10];

    fn.sprintf( "next.%.2x", features );
    in.setName( fn );
    if ( !in.open(IO_ReadOnly) )
	return;
    int ent = QString( in.readAll() ).toInt();
    in.close();

    fn.sprintf( "table.%.2x", features );
    in.setName( fn );
    if ( !in.open(IO_ReadOnly) )
	return;
    in.at( ent * 10 );
    in.readBlock( block, 9 );
    block[9] = '\0';
    in.close();

    ent++;
    if ( ent == (1 << NumRandomBits) )
	ent = 1; // skip first entry

    fn.sprintf( "next.%.2x", features );
    out.setName( fn );
    if ( !out.open(IO_WriteOnly) )
	return;
    QString s = QString::number( ent ) + QChar( '\n' );
    out.writeBlock( s, s.length() );
    out.close();

    if ( strlen(block) == 9 && block[4] == '-' )
	printf( "%s%s\n", block, encodedExpiryDate(expiryDate).latin1() );
}

static QString textForFeatures( uint features )
{
    QString text;

    if ( (features & Feature_US) != 0 )
	text += QString( " -us" );
    if ( (features & Feature_Enterprise) != 0 )
	text += QString( " -enterprise" );
    if ( (features & Feature_Unix) != 0 )
	text += QString( " -unix" );
    if ( (features & Feature_Windows) != 0 )
	text += QString( " -windows" );
    if ( (features & Feature_Mac) != 0 )
	text += QString( " -mac" );
    if ( (features & Feature_Embedded) != 0 )
	text += QString( " -embedded" );
    if ( (features & Feature_Extra1) != 0 )
	text += QString( " -extra1" );
    if ( (features & Feature_Extra2) != 0 )
	text += QString( " -extra2" );

    text = text.stripWhiteSpace();
    if ( text.isEmpty() )
	text = QString( "invalid key" );
    return text;
}

static QString textForDate( const QDate& date )
{
    if ( date.isValid() ) {
	return date.toString( Qt::ISODate );
    } else {
	return QString( "invalid date" );
    }
}

static void reset()
{
    for ( uint features = 0; features < (1 << NumFeatures); features++ ) {
	printf( "Resetting 'table.%.2x' and 'next.%.2x' (%s)\n", features,
		features, textForFeatures(features).latin1() );

	QFile out;
	QString fn;

	fn.sprintf( "table.%.2x", features );
	out.setName( fn );
	if ( !out.open(IO_WriteOnly) ) {
	    fprintf( stderr, "Cannot open '%s' for writing\n", fn.latin1() );
	    exit( EXIT_FAILURE );
	}

	for ( uint bits = 0; bits < (1 << NumRandomBits); bits++ ) {
	    QString k = keyForFeatures( features, bits ) + QChar( '\n' );
	    out.writeBlock( k.latin1(), k.length() );

	    /*
	      We check that the generated keys give access to the
	      correct feature sets. This accounts for most of the
	      processing time of the function.
	    */
#if 1
	    if ( featuresForKey(k) != features ) {
		fprintf( stderr, "Internal error in featuresForKey(\"%s\")\n",
			 k.stripWhiteSpace().latin1() );
		exit( EXIT_FAILURE );
	    }
	    if ( (features & ~(Feature_US | Feature_Enterprise |
			       Feature_Unix)) == 0 ) {
		if ( featuresForKeyOnUnix(k) != features ) {
		    fprintf( stderr,
			     "Internal error in featuresForKeyOnUnix(\"%s\")\n",
			     k.stripWhiteSpace().latin1() );
		    exit( EXIT_FAILURE );
		}
	    }
#endif
	}
	out.close();

	fn.sprintf( "next.%.2x", features );
	out.setName( fn );
	if ( !out.open(IO_WriteOnly) ) {
	    fprintf( stderr, "Cannot open '%s' for writing\n", fn.latin1() );
	    exit( EXIT_FAILURE );
	}
	out.writeBlock( "1\n", 2 ); // skip first key
	out.close();
    }
}

int main( int argc, char **argv )
{
    if ( argc == 1 || (strcmp(argv[1], "check") != 0 &&
		       strcmp(argv[1], "new") != 0 &&
		       strcmp(argv[1], "reset") != 0) ) {
	fprintf( stderr, "Usage:\n"
		"    keygen check <key>\n"
		"    keygen new YYYY-MM-DD [-us] [-enterprise] [-unix]\n"
		"        [-windows] [-mac] [-embedded] [-extra1] [-extra2]\n"
		"    keygen reset\n" );
	exit( EXIT_FAILURE );
    }

    if ( strcmp(argv[1], "check") == 0 ) {
	if ( argc != 3 ) {
	    fprintf( stderr, "Usage:\n"
		    "    mkcode check <key>\n" );
	    exit( EXIT_FAILURE );
	}

	QString key( argv[2] );

	printf( "Unix check: %s\n",
		textForFeatures(featuresForKeyOnUnix(QString(argv[2])))
		.latin1() );
	printf( "Full check: %s\n",
		textForFeatures(featuresForKey(QString(argv[2])))
		.latin1() );
	if ( featuresForKey(QString(argv[2])) != 0 )
	    printf( "Expiry date: %s\n",
		    textForDate(decodedExpiryDate(key.mid(9))).latin1() );
    } else if ( strcmp(argv[1], "new") == 0 ) {
	uint features = 0;

	if ( argc < 3 ) {
	    fprintf( stderr, "Usage:\n"
		     "    mkcode new YYYY-MM-DD [features]\n" );
	    exit( EXIT_FAILURE );
	}

	QDate expiryDate = QDate::fromString( QString(argv[2]), Qt::ISODate );
	if ( !expiryDate.isValid() ) {
	    fprintf( stderr, "Date '%s' not in YYYY-MM-DD format\n", argv[2] );
	    exit( EXIT_FAILURE );
	}

	for ( int i = 3; i < argc; i++ ) {
	    if ( strcmp(argv[i], "-us") == 0 ) {
		features |= Feature_US;
	    } else if ( strcmp(argv[i], "-enterprise") == 0 ) {
		features |= Feature_Enterprise;
	    } else if ( strcmp(argv[i], "-unix") == 0 ) {
		features |= Feature_Unix;
	    } else if ( strcmp(argv[i], "-windows") == 0 ) {
		features |= Feature_Windows;
	    } else if ( strcmp(argv[i], "-mac") == 0 ) {
		features |= Feature_Mac;
	    } else if ( strcmp(argv[i], "-embedded") == 0 ) {
		features |= Feature_Embedded;
	    } else if ( strcmp(argv[i], "-extra1") == 0 ) {
		features |= Feature_Extra1;
	    } else if ( strcmp(argv[i], "-extra2") == 0 ) {
		features |= Feature_Extra2;
	    } else {
		fprintf( stderr, "Unknown flag '%s'\n", argv[i] );
		exit( EXIT_FAILURE );
	    }
	}
	printNewKey( features, expiryDate );
    } else {
	reset();
    }
    return 0;
}
