/****************************************************************************
** $Id:$
**
** Definition of QSound class and QAuServer internal class
**
** Created : 000117
**
** Copyright (C) 1999-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/
#ifndef QSOUND_H
#define QSOUND_H

#ifndef QT_H
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_SOUND

class QSoundData;
class QAuServer;
class QAuBucket;

class Q_EXPORT QSound : public QObject {
    Q_OBJECT
public:
    static bool available();
    static void play(const QString& filename);

    QSound(const QString& filename, QObject* parent=0, const char* name=0);
    ~QSound();

    /* Coming soon...
	?
    QSound(int hertz, Type type=Mono);
    void play(const ushort* data, int samples);
    bool full();
    bool done();
    signal void notFull();
	?
    */

public slots:
    void play();

private:
    QSoundData* d;
};


/*
  QAuServer is an INTERNAL class.  If you wish to provide support for
  additional audio servers, you can make a subclass of QAuServer to do
  so, HOWEVER, your class may need to be re-engineered to some degree
  with each new Qt release, including minor releases.

  QAuBucket is whatever you want.
*/
class QAuServer : public QObject {
    Q_OBJECT

public:
    QAuServer(QObject* parent, const char* name);
    ~QAuServer();

    virtual void play(const QString& filename);
    virtual void play(QAuBucket* id)=0;
    virtual QAuBucket* newBucket(const QString& filename)=0;
    virtual void deleteBucket(QAuBucket* id)=0;
    virtual bool okay()=0;
};

#endif // QT_NO_SOUND

#endif
