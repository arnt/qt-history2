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
class QAuServerMacCallBackData;
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
    static void qt_mac_sound_callbk(QTCallBack callbk, long data);
    QAuServerMacCallBackData *callback; 
    QPixmap *offscreen;
    Movie aMovie;
    Fixed volume;
};
struct QAuServerMacCallBackData {
    QAuServerMacCallBackData(QAuServerMac *serv, QSound *q) : server(serv), qsound(q), callback(0) { }
    ~QAuServerMacCallBackData() { 	if(callback) DisposeCallBack(callback); }

    QAuServerMac *server;
    QSound *qsound;
    QTCallBack callback; //QT as in Quicktime :)
};

static int servers = 0;
static QTCallBackUPP movieCallbackProc = 0;
QAuServerMac::QAuServerMac(QObject* parent) :
    QAuServer(parent,"Mac Audio Server"), callback(0), aMovie(nil), volume(1)
{
    if(!servers++) 
	EnterMovies();
    offscreen = new QPixmap(1, 1); //what should the size be? FIXME
}

QAuServerMac::~QAuServerMac()
{
    if(!(--servers)) {
	if(movieCallbackProc)
	    DisposeQTCallBackUPP(movieCallbackProc);
	ExitMovies();
    }
}

static Movie get_movie(const QString &filename, QPixmap *offscreen) 
{
    FSSpec fileSpec;
    if(qt_mac_create_fsspec(filename, &fileSpec) != noErr) {
	qDebug("Qt: internal: bogus %d", __LINE__);
	return NULL;
    }
    Movie aMovie = nil;
    short movieResFile;
    if(OpenMovieFile(&fileSpec, &movieResFile, fsCurPerm) != noErr)
	return NULL;
    if(NewMovieFromFile(&aMovie, movieResFile, 0, 0, newMovieActive, 0) != noErr)
	return NULL;
    SetMovieGWorld(aMovie, (GWorldPtr)offscreen->handle(), 0); //just a temporary offscreen
    CloseMovieFile(movieResFile);
    return aMovie;
}

void QAuServerMac::qt_mac_sound_callbk(QTCallBack callbk, long data)
{
    qDebug("happened!!!");
    QAuServerMacCallBackData *iteration = (QAuServerMacCallBackData*)data;
    if(!iteration->server->decLoop(iteration->qsound))
	CancelCallBack(callbk);
}

void QAuServerMac::play(const QString& filename)
{
    if(!(aMovie = get_movie(filename, offscreen)))
       return;
    GoToBeginningOfMovie(aMovie);
    SetMovieVolume(aMovie, kFullVolume);
    StartMovie(aMovie);
}

void QAuServerMac::play(QSound* s)
{
    if(!s->loopsRemaining())
	return;
    if(!(aMovie = get_movie(s->fileName(), offscreen)))
       return;
    GoToBeginningOfMovie(aMovie);
    SetMovieVolume(aMovie, kFullVolume);
    if(s->loopsRemaining() > 1) {
	callback = new QAuServerMacCallBackData(this, s);
	callback->callback = NewCallBack(GetMovieTimeBase(aMovie), callBackAtExtremes);
	if(!movieCallbackProc)
	    movieCallbackProc = NewQTCallBackUPP(qt_mac_sound_callbk);
	CallMeWhen(callback->callback, movieCallbackProc, (long int)callback, triggerAtStop, 0, 0);
    }
    StartMovie(aMovie);
}

void QAuServerMac::stop(QSound*)
{
    if(callback) {
	delete callback;
	callback = 0;
    }
    StopMovie(aMovie);
    if(aMovie) {
	DisposeMovie(aMovie);
	aMovie = 0;
    }
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
