/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QT_CLEAN_NAMESPACE
#define QT_CLEAN_NAMESPACE
#endif

#include "qsound.h"
#include "qsound_p.h"

#ifndef QT_NO_SOUND

#include <qdir.h>
#include <qpixmap.h>
#include <qpainter.h>
#include <qapplication.h>
#include <private/qt_mac_p.h>
#undef check

/*****************************************************************************
  External functions
 *****************************************************************************/
extern OSErr qt_mac_create_fsspec(const QString &file, FSSpec *spec); //qglobal.cpp
extern GrafPtr qt_mac_qd_context(const QPaintDevice *); //qpaintdevice_mac.cpp

/*****************************************************************************
  Internal variables and functions
 *****************************************************************************/
class QAuServerMac : public QAuServer {
    Q_OBJECT

public:
    QAuServerMac(QObject* parent);
    ~QAuServerMac();

    void play(const QString& filename);
    void play(QSound *s);
    void stop(QSound*);
    bool okay() { return true; }

private:
    QPixmap *offscreen;
    friend class QAuServerMacCleanupHandler;
};
class QAuServerMacCleanupHandler : public QObject {
private:
    int loops;
    QAuServerMac *qsound_server;
    QSound *qsound;
    Movie movie;
    QTCallBack callback; //QT as in Quicktime :)
    static QTCallBackUPP movieCallbackProc;
    static QList<QAuServerMacCleanupHandler*> cleanups;
    static void movieEnd(QTCallBack, long data) {
        QAuServerMacCleanupHandler *iteration = (QAuServerMacCleanupHandler*)data;
        if(iteration->loops != -1) { //forever
            if((--iteration->loops) <= 0) {
                iteration->deleteLater();
                return;
            }
            if(iteration->qsound)
                iteration->qsound_server->decLoop(iteration->qsound);
        }
        GoToBeginningOfMovie(iteration->movie);
        CallMeWhen(iteration->callback, movieCallbackProc, (long)iteration, triggerAtStop, 0, 0);
        StartMovie(iteration->movie); //play it again Sam..
    }
    void init(int l, Movie m) {
        movie = m;
        loops = l;
        callback = NewCallBack(GetMovieTimeBase(movie), callBackAtExtremes|callBackAtInterrupt);
        if(!movieCallbackProc)
            movieCallbackProc = NewQTCallBackUPP(QTCallBackProcPtr(movieEnd));
        CallMeWhen(callback, movieCallbackProc, (long)this, triggerAtStop, 0, 0);
        cleanups.append(this);
    }

public:
    QAuServerMacCleanupHandler(QAuServerMac *ss, QSound *s, Movie m) :
        qsound_server(ss), qsound(s) { init(s->loopsRemaining(), m); }
    QAuServerMacCleanupHandler(int l, Movie m) : qsound_server(0), qsound(0) { init(l, m); }
    ~QAuServerMacCleanupHandler() {
        cleanups.removeAll(this);
        if(callback) {
            CancelCallBack(callback);
            DisposeCallBack(callback);
        }
        if(movie) {
            StopMovie(movie);
            DisposeMovie(movie);
        }
    }
    static void cleanup() {
        while(cleanups.size())
            delete cleanups.first();
        if(movieCallbackProc) {
            DisposeQTCallBackUPP(movieCallbackProc);
            movieCallbackProc = 0;
        }
    }
    static void stop(QSound *s) {
        if(!s)
            return;
        for(QList<QAuServerMacCleanupHandler*>::Iterator it = cleanups.begin(); it != cleanups.end(); ++it) {
            if((*it)->qsound == s) {
                delete (*it); //the destructor removes it..
                break;
            }
        }
    }

};
QTCallBackUPP QAuServerMacCleanupHandler::movieCallbackProc = 0;
QList<QAuServerMacCleanupHandler*> QAuServerMacCleanupHandler::cleanups;

static int servers = 0;
QAuServerMac::QAuServerMac(QObject* parent) : QAuServer(parent)
{
    if(!servers++)
        EnterMovies();
    offscreen = new QPixmap(1, 1); // Perhaps it should be a different size --Sam
}

QAuServerMac::~QAuServerMac()
{
    if(!(--servers)) {
        QAuServerMacCleanupHandler::cleanup();
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
    SetMovieGWorld(aMovie, qt_mac_qd_context(offscreen), 0); //just a temporary offscreen
    CloseMovieFile(movieResFile);
    return aMovie;
}

void QAuServerMac::play(const QString& filename)
{
    if(Movie movie = get_movie(filename, offscreen)) {
        GoToBeginningOfMovie(movie);
        SetMovieVolume(movie, kFullVolume);
        (void)new QAuServerMacCleanupHandler(1, movie); //this will handle the cleanup
        StartMovie(movie);
    }
}

void QAuServerMac::play(QSound *s)
{
    if(Movie movie = get_movie(s->fileName(), offscreen)) {
        GoToBeginningOfMovie(movie);
        SetMovieVolume(movie, kFullVolume);
        (void)new QAuServerMacCleanupHandler(this, s, movie); //this will handle the cleanup
        StartMovie(movie);
    }
}

void QAuServerMac::stop(QSound *s)
{
    QAuServerMacCleanupHandler::stop(s);
}

QAuServer* qt_new_audio_server()
{
    return new QAuServerMac(qApp);
}

#include "qsound_mac.moc"

#endif // QT_NO_SOUND
