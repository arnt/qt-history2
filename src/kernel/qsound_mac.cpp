/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsound_mac.cpp
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 001019
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

#define QT_CLEAN_NAMESPACE

#include "qsound.h"

#ifndef QT_NO_SOUND

#include <qpixmap.h>
#include <qpainter.h>
#include <qapplication.h>
#include "qt_mac.h"

class QAuServerMac : public QAuServer {
    Q_OBJECT

public:
    QAuServerMac(QObject* parent);
    ~QAuServerMac();

    void play(const QString& filename);
    void play(QAuBucket* id);
    QAuBucket* newBucket(const QString& filename);
    void deleteBucket(QAuBucket* id);
    bool okay();

private:
    QPixmap *offscreen;
};

class QAuBucket {
public:
    QAuBucket( const QString& s ) : filename(s) { }
    QString filename;
};

QAuServerMac::QAuServerMac(QObject* parent) :
    QAuServer(parent,"Mac Audio Server")
{
    EnterMovies();
    offscreen = new QPixmap( 1, 1 );
}

QAuServerMac::~QAuServerMac()
{
    ExitMovies();
}


// The FSpLocationFromFullPath function is descended from Apple Source Code,
// but changes have been made.
QMAC_PASCAL OSErr FSpLocationFromFullPath( short fullPathLength,
				      const void *fullPath,
				      FSSpec *spec)
{
    AliasHandle alias;
    OSErr result;
    Boolean wasChanged;
    Str32 nullString;
	
    /* Create a minimal alias from the full pathname */
    nullString[0] = 0;	/* null string to indicate no zone or server name */
    result = NewAliasMinimalFromFullPath( fullPathLength, fullPath, nullString,
					  nullString, &alias );
    if ( result == noErr ) {
        qDebug( "first 1" );
	/* Let the Alias Manager resolve the alias. */
	result = ResolveAlias( NULL, alias, spec, &wasChanged );
		
	/* work around Alias Mgr sloppy volume matching bug */
	if ( spec->vRefNum == 0 )
	{
	    /* invalidate wrong FSSpec */
	    spec->parID = 0;
	    spec->name[0] =  0;
	    result = nsvErr;
	}
	DisposeHandle( (Handle)alias );	/* Free up memory used */
    }
    return result;
}

void QAuServerMac::play( const QString& filename )
{
    OSErr err;
    FSSpec fileSpec;
    Movie aMovie = nil;
    short movieResFile;

    QString macFilename = filename.mid( 1 );
    while ( macFilename.find( "/" ) != -1 )
	macFilename.replace( macFilename.find( "/" ), 1, ":" );
    //FIXME: prepend the volume name to the macFilename 

    err = FSpLocationFromFullPath( macFilename.length(), macFilename.latin1(),
				   &fileSpec ); 
    if ( err != noErr )
	return;

    // If a movie soundtrack is played then the movie will be played on
    // the current graphics port. So create an offscreen graphics port.
    QPainter p( offscreen );

    err = OpenMovieFile ( &fileSpec, &movieResFile, fsRdPerm );
    if ( err != noErr )
	return;

    short           movieResID = 0;         /* want first movie */
    Str255          movieName;
    Boolean         wasChanged;
    err = NewMovieFromFile (&aMovie, movieResFile,
			    &movieResID,
			    movieName, 
			    newMovieActive,         /* flags */
			    &wasChanged);
    if ( err != noErr )
	return;

    CloseMovieFile (movieResFile);
    StartMovie( aMovie );
}

void QAuServerMac::play(QAuBucket* id)
{
    play( id->filename );
}

QAuBucket* QAuServerMac::newBucket(const QString& filename)
{
    return new QAuBucket( filename );
}

void QAuServerMac::deleteBucket(QAuBucket* id)
{
    delete id;
}

bool QAuServerMac::okay()
{
    return TRUE;
}

QAuServer* qt_new_audio_server()
{
    return new QAuServerMac( qApp );
}

#include "qsound_mac.moc"

#endif // QT_NO_SOUND
