/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsound_mac.cpp
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 001019
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Unix/X11 may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#define QT_CLEAN_NAMESPACE

#include "qsound.h"

#ifndef QT_NO_SOUND

#include <qsocknot.h>
#include <qapplication.h>


#ifdef QT_NAS_SUPPORT

class QAuServerNAS : public QAuServer {
    Q_OBJECT

public:
    QAuServerNAS(QObject*);
    ~QAuServerNAS();

    void play(const QString&);
    void play(QAuBucket* );
    QAuBucket* newBucket(const QString& );
    void deleteBucket(QAuBucket* );
    bool okay();

public slots:
    void dataReceived();
};

class QAuBucket { }; // Just an ID - we cast to and from AuBucketID

QAuServerNAS::QAuServerNAS(QObject* ) :
    QAuServer(parent,"Network Audio System")
{
    qDebug( "QAuServerNAS::QAuServerNAS" );
}

QAuServerNAS::~QAuServerNAS()
{
    qDebug( "QAuServerNAS::~QAuServerNAS" );
}

void QAuServerNAS::play(const QString& )
{
    qDebug( "QAuServerNAS::play" );
}

void QAuServerNAS::play(QAuBucket* id)
{
    qDebug( "QAuServerNAS::play" );
}

QAuBucket* QAuServerNAS::newBucket(const QString& )
{
    qDebug( "QAuServerNAS::newBucket" );
    return 0
}

void QAuServerNAS::deleteBucket(QAuBucket* )
{
    qDebug( "QAuServerNAS::deleteBucket" );
}

bool QAuServerNAS::okay()
{
    qDebug( "QAuServerNAS::okay" );
    return false;
}

void QAuServerNAS::dataReceived()
{
    qDebug( "QAuServerNAS::dataReceived" );
}

#include "qsound_mac.moc"

#endif


class QAuServerNull : public QAuServer {
public:
    QAuServerNull(QObject* parent);

    void play(const QString&) { }
    void play(QAuBucket*) { }
    QAuBucket* newBucket(const QString&) { return 0; }
    void deleteBucket(QAuBucket*) { }
    bool okay() { return FALSE; }
};

QAuServerNull::QAuServerNull(QObject* parent) :
    QAuServer(parent,"Null Audio Server")
{
    qDebug( "QAuServerNull::QAuServerNull" );
}


QAuServer* qt_new_audio_server()
{
    qDebug( "qt_new_audio_server" );
    return new QAuServerNull(qApp);
}

#endif // QT_NO_SOUND
