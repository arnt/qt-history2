/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qsound_qws.cpp#1 $
**
** Implementation of QSound class and QAuServer internal class
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

#include "qapplication.h"

#ifndef QT_NO_SOUND

#include "qsound.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"

class QAuBucket {
public:
    QAuBucket(const QString& s) : name(s) { }
    QString name;
};

class QAuServerQWS : public QAuServer {
public:
    QAuServerQWS(QObject* parent);

    void play(QAuBucket* b)
    {
	QPaintDevice::qwsDisplay()->playSoundFile(b->name);
    }

    QAuBucket* newBucket(const QString& s)
    {
	return new QAuBucket(s);
    }

    void deleteBucket(QAuBucket* b)
    {
	delete b;
    }

    bool okay() { return TRUE; }
};

QAuServerQWS::QAuServerQWS(QObject* parent) :
    QAuServer(parent,0)
{
}


QAuServer* qt_new_audio_server()
{
    return new QAuServerQWS(qApp);
}

#endif // QT_NO_SOUND
