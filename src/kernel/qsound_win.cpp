/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsound_win.cpp#1 $
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

#include "qsocknot.h"
#include "qapplication.h"
#include "qapplication_p.h"
#include <qfile.h>

#include <qt_windows.h>

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
