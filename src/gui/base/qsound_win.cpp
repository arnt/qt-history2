/****************************************************************************
**
** Implementation of QSound class and QAuServer internal class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsound.h"

#ifndef QT_NO_SOUND

#include "qapplication.h"
#include "qapplication_p.h"
#include <qfile.h>
#include "qguardedptr.h"

#include <qt_windows.h>

class QAuServerWindows : public QAuServer {
    Q_OBJECT

public:
    QAuServerWindows(QObject* parent);
    ~QAuServerWindows();

    void playHelper(const QString &filename, int loop, QSound *snd);
    void play(const QString& filename, int loop );
    void play(QSound*);
    
    void stop(QSound*);
    bool okay();

    int decLoop(QSound *snd) { return QAuServer::decLoop(snd); }

    HANDLE current;
    HANDLE mutex;
    HANDLE event;
};

QAuServerWindows::QAuServerWindows(QObject* parent) :
    QAuServer(parent,"Windows Audio Server"), current(0)
{
    mutex = CreateMutexA(0, 0, 0);
    event = CreateEventA(0, FALSE, FALSE, 0);
}

QAuServerWindows::~QAuServerWindows()
{
    HANDLE mtx = mutex;
    WaitForSingleObject(mtx, INFINITE);
    mutex = 0;

    ReleaseMutex(mtx);
    CloseHandle(mtx); 
    CloseHandle(event);
}

struct SoundInfo
{
    SoundInfo(const QString &fn, int lp, QSound *snd, QAuServerWindows *srv)
	: sound(snd), server(srv), filename(fn), loops(lp)
    {
    }

    QSound *sound;
    QAuServerWindows *server;
    QString filename;
    int loops;
};

DWORD WINAPI SoundPlayProc(LPVOID param)
{
    SoundInfo *info = (SoundInfo*)param;

    // copy data before waking up GUI thread
    QAuServerWindows *server = info->server;
    QGuardedPtr<QSound> sound = info->sound;
    int loops = info->loops;
    QString filename(info->filename);
    HANDLE mutex = server->mutex;
    HANDLE event = server->event;
    info = 0;

    // server must not be destroyed until thread finishes
    // and all other sounds have to wait
    WaitForSingleObject(mutex, INFINITE);

    if (loops == -1) {
	server->current = 0;
	QT_WA( {
	    PlaySoundW( (TCHAR*)filename.ucs2(), 0, SND_FILENAME|SND_ASYNC|SND_LOOP );
	} , {
	    PlaySoundA( QFile::encodeName(filename).data(), 0, SND_FILENAME|SND_ASYNC|SND_LOOP );
	} );
    } 
    
    // signal GUI thread to continue - sound might be reset!
    SetEvent(event);

    if (loops != -1) {
	for (int l = 0; l < loops && server->current; ++l) {
	    QT_WA( {
		PlaySoundW( (TCHAR*)filename.ucs2(), 0, SND_FILENAME|SND_SYNC );
	    } , {
		PlaySoundA( QFile::encodeName(filename).data(), 0, SND_FILENAME|SND_SYNC );
	    } );
	    
	    if (sound)
		server->decLoop(sound);
	}
	server->current = 0;
    }
    ReleaseMutex(mutex);

    return 0;
}

void QAuServerWindows::playHelper( const QString &filename, int loop, QSound *snd )
{
    // busy?
    if (WaitForSingleObject(mutex, 0) == WAIT_TIMEOUT)
	return;
    ReleaseMutex(mutex);

    DWORD threadid = 0;
    SoundInfo info(filename, loop, snd, this);
    current = CreateThread(0, 0, SoundPlayProc, &info, 0, &threadid);

    WaitForSingleObject(event, INFINITE);
}

void QAuServerWindows::play( const QString& filename, int loop )
{
    playHelper(filename, loop, 0);
}

void QAuServerWindows::play(QSound* s)
{
    playHelper(s->fileName(), s->loops(), s);
}

void QAuServerWindows::stop( QSound* )
{
    // stop unlooped sound
    if (!current)
	PlaySound(0, 0, 0);
    // stop after loop is done
    current = 0;
}

bool QAuServerWindows::okay()
{
    // ### this should work, but returns 0
    // return auxGetNumDevs() != 0;

    return TRUE;
}

QAuServer* qt_new_audio_server()
{
    return new QAuServerWindows(qApp);
}

#include "qsound_win.moc"

#endif // QT_NO_SOUND
