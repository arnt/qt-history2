/****************************************************************************
**
** Implementation of QSound class and QAuServer internal class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qapplication.h"

#ifndef QT_NO_SOUND

#include "qsound.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"

class QAuServerQWS : public QAuServer {
public:
    QAuServerQWS(QObject* parent);

    void play(const QString& filename)
    {
	QPaintDevice::qwsDisplay()->playSoundFile(filename);
    }
    void play(QSound* s)
    {
	QPaintDevice::qwsDisplay()->playSoundFile(s->fileName());
    }
    void stop(QSound*)
    {
	// ####
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
