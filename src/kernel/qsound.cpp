/****************************************************************************
** $Id:$
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
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qsound.h"
#include "qapplication.h"
#include "qlist.h"

static QList<QAuServer> *servers=0;

QAuServer::QAuServer(QObject* parent, const char* name) :
    QObject(parent,name)
{
    if ( !servers ) {
	servers = new QList<QAuServer>;
	// ### add cleanup
    }
    servers->prepend(this);
}

QAuServer::~QAuServer()
{
    servers->remove(this);
    if ( servers->count() == 0 ) {
	delete servers;
	servers = 0;
    }
}

extern QAuServer* qt_new_audio_server();

class QSoundData {
public:
    static QAuServer& server()
    {
	if (!servers) qt_new_audio_server();
	return *servers->first();
    }

    QSoundData(const QString& fname) :
	filename(fname), bucket(0)
    {
    }

    ~QSoundData()
    {
	if ( bucket )
	    server().deleteBucket(bucket);
    }

    void play()
    {
	if ( !bucket )
	    bucket = server().newBucket(filename);
	server().play(bucket);
    }

    QString filename;

    QAuBucket* bucket;
};

/*!
  \class QSound qsound.h
  \brief Access to the platform audio facilities.
  
  \ingroup sound

  Qt provides the most commonly required audio operation in
  GUI applications: playing a sound file asynchronously
  to the user. This is most simply accomplished with a single call:
  \code
    QSound::play("mysounds/bells.wav");
  \endcode

  A second API is provided, where a QSound object is created
  from a sound file and is later be played:
  \code
    QSound bells("mysounds/bells.wav");

    bells.play();
  \endcode

  Sounds played by the second model may use more memory but play
  more immediately than sounds played using the first model, depending
  on the underlying platform audio facilities.

  On Microsoft Windows, the underlying multimedia system is used.
  On X11, the
  <a href=ftp://ftp.x.org/contrib/audio/nas/>Network Audio System</a>
  is used if available, otherwise all
  operations work silently. The availability of sound can be
  tested with QSound::available().
*/

/*!
  Play the sound in file named \a filename.
*/
void QSound::play(const QString& filename)
{
    QSoundData::server().play(filename);
}

/*!
  Constructs a sound which can quickly play the sound in file
  named \a filename.

  This can use more memory than the static \c play function.

  The \a parent and \a name arguments (default 0) are passed on to
  the QObject constructor.
*/
QSound::QSound(const QString& filename, QObject* parent, const char* name) :
    QObject(parent,name),
    d(new QSoundData(filename))
{
}

/*!
  Destructs the sound.
*/
QSound::~QSound()
{
    delete d;
}

/*!
  Starts the sound playing.  The function returns immediately.
  Depending on the platform audio facilities, other sounds may
  stop or may be mixed with the new sound.

  The sound can be played again at any time, possibly mixing or
  replacing previous plays of the sound.
*/
void QSound::play()
{
    d->play();
}

/*!
  Returns TRUE if sound facilities exist on the platform. An
  application may choose to notify the user if sound is crucial
  the the application, or operate silently without bothering
  the user.

  If no sound is available, QSound operation all work silently
  and quickly.
*/
bool QSound::available()
{
    return QSoundData::server().okay();
}
