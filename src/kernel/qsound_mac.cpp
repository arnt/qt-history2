/****************************************************************************
**
** Implementation of QSound class and QAuServer internal class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREEPROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#define QT_CLEAN_NAMESPACE

#include "qsound.h"

#ifndef QT_NO_SOUND

#include <qdir.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qapplication.h>
#include "qt_mac.h"
#undef check

/*****************************************************************************
  External functions
 *****************************************************************************/
OSErr qt_mac_create_fsspec(const QString &file, FSSpec *spec); //qglobal.cpp


/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
class QAuServerMac : public QAuServer {
    Q_OBJECT

public:
    QAuServerMac(QObject* parent);
    ~QAuServerMac();

    void play(const QString& filename);
    void play(QSound*);
    void stop(QSound*);
    bool okay();

private:
    QPixmap *offscreen;
    Movie aMovie;
    Fixed volume;
};

static int servers = 0;
QAuServerMac::QAuServerMac(QObject* parent) :
    QAuServer(parent,"Mac Audio Server"), aMovie(nil), volume(1)
{
    if(!servers++)
	EnterMovies();
    offscreen = new QPixmap(1, 1); //what should the size be? FIXME
}

QAuServerMac::~QAuServerMac()
{
    if(!(--servers))
	ExitMovies();
}

static Movie get_movie(const QString &filename, QPixmap *offscreen) 
{
    FSSpec fileSpec;
    if(qt_mac_create_fsspec(filename, &fileSpec) != noErr) {
	qDebug("Qt: internal: bogus %d", __LINE__);
	return NULL;
    }

    short movieResFile;
    Movie aMovie = nil;
    if(OpenMovieFile(&fileSpec, &movieResFile, fsRdPerm) != noErr)
	return NULL;

    short           movieResID = 0;         /* want first movie */
    Str255          movieName;
    Boolean         wasChanged;
    if(NewMovieFromFile(&aMovie, movieResFile, &movieResID, movieName, newMovieActive, &wasChanged) != noErr)
	return NULL;

    // If a movie soundtrack is played then the movie will be played on
    // the current graphics port. So create an offscreen graphics port.
    SetMovieGWorld(aMovie, (GWorldPtr)offscreen->handle(), 0);
    CloseMovieFile(movieResFile);
    return aMovie;
}

void QAuServerMac::play(const QString& filename)
{
    if(!(aMovie = get_movie(filename, offscreen)))
       return;
    SetMovieVolume(aMovie, kFullVolume);
    GoToBeginningOfMovie(aMovie);
    StartMovie(aMovie);
}

/*
void
QAuServerMac::setVolume(int x) 
{  
    volume = x;
    if(aMovie) 
	MCDoAction(aMovie, mcActionSetVolume, &volume); 
} 
void
QAuServerMac::playLoop(const QString &filename) 
{
    if(!(aMovie = get_movie(filename, offscreen)))
       return;
    Boolean loop = true;
    MCDoAction(aMovie, mcActionSetLooping, &loop);
    MCDoAction(aMovie, mcActionSetVolume, &volume); 

    //play
    Fixed rate = 1;
    MCDoAction(aMovie, mcActionPlay, &raterate);
}
*/

void QAuServerMac::play(QSound* s)
{
    play(s->fileName());
}

void QAuServerMac::stop(QSound*)
{
    StopMovie(aMovie);
}

bool QAuServerMac::okay()
{
    return TRUE;
}

QAuServer* qt_new_audio_server()
{
    return new QAuServerMac(qApp);
}

#include "qsound_mac.moc"

#endif // QT_NO_SOUND
