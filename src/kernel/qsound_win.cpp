/****************************************************************************
** $Id$
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 000117
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Windows may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

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
#ifdef UNICODE
#ifndef Q_OS_TEMP
    if ( qWinVersion() == Qt::WV_NT ) {
#endif
	PlaySoundW( filename.ucs2(), 0, flags );
#ifndef Q_OS_TEMP
    } else
#endif
#endif
#ifndef Q_OS_TEMP
    {
	PlaySoundA( QFile::encodeName(filename).data(), 0, flags );
    }
#endif
}

void QAuServerWindows::play(QSound* s)
{
    play(s->fileName(), s->loops() );
}

void QAuServerWindows::stop(QSound* s)
{
    if ( !s->isFinished() ) {
#ifdef UNICODE
#ifndef Q_OS_TEMP
	if ( qWinVersion() == Qt::WV_NT ) {
#endif
	    PlaySoundW( 0, 0, 0 );
#ifndef Q_OS_TEMP
	} else
#endif
#endif
#ifndef Q_OS_TEMP
	{
	    PlaySoundA( 0, 0, 0 );
	}
#endif
    }
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
