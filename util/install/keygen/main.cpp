/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the network module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

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
    in.setFileName( fn );
    if ( !in.open(QFile::ReadOnly) )
	return;
    int ent = QString( in.readAll() ).toInt();
    in.close();

    fn.sprintf( "table.%.2x", features );
    in.setFileName( fn );
    if ( !in.open(QFile::ReadOnly) )
	return;
    in.seek( ent * 10 );
    in.read( block, 9 );
    block[9] = '\0';
    in.close();

    ent++;
    if ( ent == (1 << NumRandomBits) )
	ent = 1; // skip first entry

    fn.sprintf( "next.%.2x", features );
    out.setFileName( fn );
    if ( !out.open(QFile::WriteOnly) )
	return;
    QString s = QString::number( ent ) + QChar( '\n' );
    out.write( s.toLocal8Bit(), s.length() );
    out.close();

    if ( strlen(block) == 9 && block[4] == '-' )
	printf( "%s%s\n", block, qPrintable(encodedExpiryDate(expiryDate)) );
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

    text = text.trimmed();
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
		features, qPrintable(textForFeatures(features)) );

	QFile out;
	QString fn;

	fn.sprintf( "table.%.2x", features );
	out.setFileName( fn );
	if ( !out.open(QFile::WriteOnly) ) {
	    fprintf( stderr, "Cannot open '%s' for writing\n", qPrintable(fn) );
	    exit( EXIT_FAILURE );
	}

	for ( uint bits = 0; bits < (1 << NumRandomBits); bits++ ) {
	    QString k = keyForFeatures( features, bits ) + QChar( '\n' );
	    out.write( k.toLatin1(), k.length() );

	    /*
	      We check that the generated keys give access to the
	      correct feature sets. This accounts for most of the
	      processing time of the function.
	    */
#if 1
	    if ( featuresForKey(k) != features ) {
		fprintf( stderr, "Internal error in featuresForKey(\"%s\")\n",
			 qPrintable(k.trimmed()) );
		exit( EXIT_FAILURE );
	    }
	    if ( (features & ~(Feature_US | Feature_Enterprise |
			       Feature_Unix)) == 0 ) {
		if ( featuresForKeyOnUnix(k) != features ) {
		    fprintf( stderr,
			     "Internal error in featuresForKeyOnUnix(\"%s\")\n",
			     qPrintable(k.trimmed()) );
		    exit( EXIT_FAILURE );
		}
	    }
#endif
	}
	out.close();

	fn.sprintf( "next.%.2x", features );
	out.setFileName( fn );
	if ( !out.open(QFile::WriteOnly) ) {
	    fprintf( stderr, "Cannot open '%s' for writing\n", qPrintable(fn) );
	    exit( EXIT_FAILURE );
	}
	out.write( "1\n", 2 ); // skip first key
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

	printf("Unix check: %s\n",
               qPrintable(textForFeatures(featuresForKeyOnUnix(QString(argv[2])))));
	printf("Full check: %s\n",
               qPrintable(textForFeatures(featuresForKey(QString(argv[2])))));
	if ( featuresForKey(QString(argv[2])) != 0 )
	    printf("Expiry date: %s\n",
                   qPrintable(textForDate(decodedExpiryDate(key.mid(9)))));
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
