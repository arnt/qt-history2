/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsound_win.cpp#1 $
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 000117
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsound.h"

#ifndef QT_NO_SOUND

#include <qsocknot.h>
#include <qapplication.h>
#include <qfile.h>

#include <qt_windows.h>

extern Qt::WindowsVersion qt_winver;            // in qapplication_win.cpp

class QAuServerWindows : public QAuServer {
    Q_OBJECT

public:
    QAuServerWindows(QObject* parent);
    ~QAuServerWindows();

    void play(const QString& filename);
    void play(QAuBucket* id);
    QAuBucket* newBucket(const QString& filename);
    void deleteBucket(QAuBucket* id);
    bool okay();
};

class QAuBucket {
public:
    QAuBucket( const QString& s ) : filename(s) { }
    QString filename;
};

QAuServerWindows::QAuServerWindows(QObject* parent) :
    QAuServer(parent,"Windows Audio Server")
{
}

QAuServerWindows::~QAuServerWindows()
{
}

void QAuServerWindows::play(const QString& filename)
{
#ifdef UNICODE
    if ( qt_winver == Qt::WV_NT ) {
	PlaySoundW((TCHAR*)qt_winTchar(filename,TRUE),
	    0,SND_FILENAME|SND_ASYNC);
    } else
#endif
    {
	PlaySoundA(QFile::encodeName(filename).data(),
	    0,SND_FILENAME|SND_ASYNC);
    }
}

void QAuServerWindows::play(QAuBucket* id)
{
    play(id->filename);
}

QAuBucket* QAuServerWindows::newBucket(const QString& filename)
{
    return new QAuBucket(filename);
}

void QAuServerWindows::deleteBucket(QAuBucket* id)
{
    delete id;
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
