/****************************************************************************
**
** Implementation of QSound class and QAuServer internal class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

#include "qsocketnotifier.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include <qfile.h>

#include <qt_windows.h>

class QAuServerWindows : public QAuServer {
    Q_OBJECT

public:
    QAuServerWindows(QObject* parent);
    ~QAuServerWindows();

    void play(const QString& filename, int loop );
    void play(QSound*);
    void stop(QSound*);
    bool okay();
};

QAuServerWindows::QAuServerWindows(QObject* parent) :
    QAuServer(parent,"Windows Audio Server")
{
}

QAuServerWindows::~QAuServerWindows()
{
}

void QAuServerWindows::play( const QString& filename, int loop )
{
    DWORD flags = SND_FILENAME|SND_ASYNC;
    if ( loop > 1 || loop < 0 )
	flags |= SND_LOOP;
    QT_WA( {
	PlaySoundW( (TCHAR*)filename.ucs2(), 0, flags );
    } , {
	PlaySoundA( QFile::encodeName(filename).data(), 0, flags );
    } );
}

void QAuServerWindows::play(QSound* s)
{
    play(s->fileName(), s->loops() );
}

void QAuServerWindows::stop( QSound* )
{
    QT_WA( {
	PlaySoundW( 0, 0, 0 );
    } , {
	PlaySoundA( 0, 0, 0 );
    } );
}

bool QAuServerWindows::okay()
{
    return TRUE;
}

QAuServer* qt_new_audio_server()
{
    // #### Ask Windows if any sound
    return new QAuServerWindows(qApp);
}

#include "qsound_win.moc"

#endif // QT_NO_SOUND
