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
#include "qpointer.h"

#include <qt_windows.h>

class QAuServerWindows : public QAuServer {
    Q_OBJECT

public:
    QAuServerWindows(QObject* parent);
    ~QAuServerWindows();

    void playHelper(const QString &filename, int loop, QSound *snd);
    void play(const QString& filename, int loop);
    void play(QSound*);

    void stop(QSound*);
    bool okay();

    int decLoop(QSound *snd) { return QAuServer::decLoop(snd); }

    HANDLE current;
    HANDLE mutex;
    HANDLE event;
};

QAuServerWindows::QAuServerWindows(QObject* parent) :
    QAuServer(parent), current(0)
{
    mutex = CreateMutexA(0, 0, 0);
    event = CreateEventA(0, false, false, 0);
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
    QSound *sound = info->sound;
    int loops = info->loops;
    QString filename(info->filename);
    HANDLE mutex = server->mutex;
    HANDLE event = server->event;
    info = 0;

    // server must not be destroyed until thread finishes
    // and all other sounds have to wait
    WaitForSingleObject(mutex, INFINITE);

    if (loops <= 1) {
        server->current = 0;
        int flags = SND_FILENAME|SND_ASYNC;
        if (loops == -1)
            flags |= SND_LOOP;

        QT_WA({
            PlaySoundW((TCHAR*)filename.utf16(), 0, flags);
        } , {
            PlaySoundA(QFile::encodeName(filename).data(), 0, flags);
        });
	if (sound && loops == 1)
	    server->decLoop(sound);

        // GUI thread continues, but we are done as well.
        SetEvent(event);
    } else {
        // signal GUI thread to continue - sound might be reset!
        QPointer<QSound> guarded_sound = sound;
        SetEvent(event);

	for (int l = 0; l < loops && server->current; ++l) {
	    QT_WA( {
		PlaySoundW( (TCHAR*)filename.utf16(), 0, SND_FILENAME|SND_SYNC );
	    } , {
		PlaySoundA( QFile::encodeName(filename).data(), 0,
		    SND_FILENAME|SND_SYNC );
	    } );

	    if (guarded_sound)
		server->decLoop(guarded_sound);
	}
	server->current = 0;
    }
    ReleaseMutex(mutex);

    return 0;
}

void QAuServerWindows::playHelper(const QString &filename, int loop, QSound *snd)
{
    if (loop == 0)
	return;
    // busy?
    if (WaitForSingleObject(mutex, 0) == WAIT_TIMEOUT)
        return;
    ReleaseMutex(mutex);

    DWORD threadid = 0;
    SoundInfo info(filename, loop, snd, this);
    current = CreateThread(0, 0, SoundPlayProc, &info, 0, &threadid);
    CloseHandle(current);

    WaitForSingleObject(event, INFINITE);
}

void QAuServerWindows::play(const QString& filename, int loop)
{
    playHelper(filename, loop, 0);
}

void QAuServerWindows::play(QSound* s)
{
    playHelper(s->fileName(), s->loops(), s);
}

void QAuServerWindows::stop(QSound*)
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

    return true;
}

QAuServer* qt_new_audio_server()
{
    return new QAuServerWindows(qApp);
}

#include "qsound_win.moc"

#endif // QT_NO_SOUND
