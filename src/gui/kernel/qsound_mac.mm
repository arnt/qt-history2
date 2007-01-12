/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <qapplication.h>
#include "qsound.h"
#include "qsound_p.h"
#include <private/qt_mac_p.h>
#include <qhash.h>
#include <qdebug.h>

#ifndef QT_NO_SOUND

#if 1
#include <AppKit/NSSound.h>

typedef QHash<QSound *, NSSound const *> Sounds;
static Sounds sounds;

class QAuServerMac : public QAuServer
{
    Q_OBJECT
public:
    QAuServerMac(QObject* parent) : QAuServer(parent) { }
    void play(const QString& filename);
    void play(QSound *s);
    void stop(QSound*);
    bool okay() { return true; }
    using QAuServer::decLoop; // promote to public.
protected:
    NSSound *createNSSound(const QString &filename, QSound *qSound);
};

@interface QMacSoundDelegate : NSObject {
    QSound *qSound; // may be null.
    QAuServerMac* server;
} 
-(id)initWithQSound:(QSound*)sound:(QAuServerMac*)server;
-(void)sound:(NSSound *)sound didFinishPlaying:(BOOL)aBool;
@end

@implementation QMacSoundDelegate
-(id)initWithQSound:(QSound*)s:(QAuServerMac*)serv {
    self = [super init];
    if(self) {
        qSound = s;
        server = serv;
    }
    return self;
}

// Delegate function that gets called each time a sound finishes.
-(void)sound:(NSSound *)sound didFinishPlaying:(BOOL)finishedOk
{
    // qSound is null if this sound was started by play(QString),
    // in which case there is no corresponding QSound object.
    if (qSound == 0) {
        [sound release];
        [self release];
        return;
    }

    // finishedOk is false if the sound cold not be played or
    // if it was interrupted by stop().
    if (finishedOk == false) {
        sounds.remove(qSound);
        [sound release];
        [self release];
        return;
    }

    // Check if the sound should loop "forever" (until stop).
    if (qSound->loops() == -1) { 
        [sound play];
        return;
    }

    const int remainingIterations = server->decLoop(qSound);
    if (remainingIterations > 0) {
        [sound play];
    } else {
        sounds.remove(qSound);
        [sound release];
        [self release];
    }
}
@end

void QAuServerMac::play(const QString &fileName)
{
    NSSound * const nsSound = createNSSound(fileName, 0);
    [nsSound play];
}

void QAuServerMac::play(QSound *qSound)
{
    NSSound * const nsSound = createNSSound(qSound->fileName(), qSound);
    [nsSound play];
    // Keep track of the nsSound object so we can find it again in stop().
    sounds[qSound] = nsSound;
}

void QAuServerMac::stop(QSound *qSound)
{
    Sounds::const_iterator it = sounds.find(qSound);
    if (it != sounds.end())
        [*it stop];
}

// Creates an NSSound object and installs a "sound finished" callack delegate on it.
NSSound *QAuServerMac::createNSSound(const QString &fileName, QSound *qSound)
{
    NSString *nsFileName = reinterpret_cast<NSString *>(QCFString::toCFStringRef(fileName));
    NSSound * const nsSound = [[NSSound alloc] initWithContentsOfFile: nsFileName byReference:YES];
    QMacSoundDelegate * const delegate = [[QMacSoundDelegate alloc] initWithQSound:qSound:this];
    [nsSound setDelegate:delegate];
    return nsSound;
}

#else //__LP64__

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
        if(iteration->qsound)
            iteration->qsound_server->decLoop(iteration->qsound);
        if(iteration->loops != -1) { //forever
            if((--iteration->loops) <= 0) {
                iteration->deleteLater();
                return;
            }
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
        for(int i = 0; i < cleanups.size(); ++i) {
            if(cleanups[i]->qsound == s) {
                delete cleanups[i]; //the destructor removes it..
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
    if(qt_mac_create_fsspec(filename, &fileSpec) != noErr)
        return 0;
    Movie aMovie = nil;
    short movieResFile;
    if(OpenMovieFile(&fileSpec, &movieResFile, fsCurPerm) != noErr)
        return 0;
    if(NewMovieFromFile(&aMovie, movieResFile, 0, 0, newMovieActive, 0) != noErr)
        return 0;
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

#endif 

QAuServer* qt_new_audio_server()
{
    return new QAuServerMac(qApp);
}


#include "qsound_mac.moc"

#endif // QT_NO_SOUND
