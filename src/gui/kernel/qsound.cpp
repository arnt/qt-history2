/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qsound.h"

#ifndef QT_NO_SOUND

#include "qlist.h"
#include <private/qobject_p.h>
#include "qsound_p.h"

static QList<QAuServer*> *servers=0;

QAuServer::QAuServer(QObject* parent)
    : QObject(parent)
{
    if (!servers)
        servers = new QList<QAuServer*>;
    servers->prepend(this);
}

QAuServer::~QAuServer()
{
    servers->removeAll(this);
    if (servers->count() == 0) {
        delete servers;
        servers = 0;
    }
}

void QAuServer::play(const QString& filename)
{
    QSound s(filename);
    play(&s);
}

extern QAuServer* qt_new_audio_server();

static QAuServer& server()
{
    if (!servers) qt_new_audio_server();
    return *servers->first();
}

class QSoundPrivate : public QObjectPrivate
{
public:
    QSoundPrivate(const QString& fname)
        : filename(fname), bucket(0), looprem(0), looptotal(1)
    {
    }

    ~QSoundPrivate()
    {
        delete bucket;
    }

    QString filename;
    QAuBucket* bucket;
    int looprem;
    int looptotal;
};

/*!
    \class QSound
    \brief The QSound class provides access to the platform audio facilities.

    \ingroup multimedia
    \mainclass

    Qt provides the most commonly required audio operation in GUI
    applications: asynchronously playing a sound file. This is most
    easily accomplished with a single call:
    \code
        QSound::play("mysounds/bells.wav");
    \endcode

    A second API is provided in which a QSound object is created from
    a sound file and is played later:
    \code
        QSound bells("mysounds/bells.wav");

        bells.play();
    \endcode

    Sounds played using the second model may use more memory but play
    more immediately than sounds played using the first model,
    depending on the underlying platform audio facilities.

    On Microsoft Windows, the underlying multimedia system is used;
    only WAVE format sound files are supported.

    On X11, the \l{ftp://ftp.x.org/contrib/audio/nas/}{Network Audio System}
    is used if available, otherwise all operations work silently. NAS
    supports WAVE and AU files.

    On Mac OS X, we use \l{http://quicktime.apple.com/}{QuickTime} for sound.
    All QuickTime formats are supported by Qt/Mac.

    In Qtopia Core, a built-in mixing sound server is used, which
    accesses \c /dev/dsp directly. Only the WAVE format is supported.

    The availability of sound can be tested with
    QSound::isAvailable().

    Note that QSound does not support \l{resources.html}{resources}.
    This might be fixed in a future Qt version.
*/

/*!
    Plays the sound in a file called \a filename.
*/
void QSound::play(const QString& filename)
{
    server().play(filename);
}

/*!
    Constructs a QSound that can quickly play the sound in a file
    named \a filename.

    This may use more memory than the static \c play function.

    The \a parent argument (default 0) is passed on to the QObject
    constructor.
*/
QSound::QSound(const QString& filename, QObject* parent)
    : QObject(*new QSoundPrivate(filename), parent)
{
    server().init(this);
}

#ifdef QT3_SUPPORT
/*!
  \obsolete
    Constructs a QSound that can quickly play the sound in a file
    named \a filename.

    This may use more memory than the static \c play function.

    The \a parent and \a name arguments (default 0) are passed on to
    the QObject constructor.
*/
QSound::QSound(const QString& filename, QObject* parent, const char* name)
    : QObject(*new QSoundPrivate(filename), parent)
{
    setObjectName(name);
    server().init(this);
}
#endif

/*!
    Destroys the sound object. If the sound is not finished playing stop() is called on it.

    \sa stop() isFinished()
*/
QSound::~QSound()
{
    if (!isFinished())
        stop();
}

/*!
    Returns true if the sound has finished playing; otherwise returns false.

    \warning On Windows this function always returns true for unlooped sounds.
*/
bool QSound::isFinished() const
{
    Q_D(const QSound);
    return d->looprem == 0;
}

/*!
    \overload

    Starts the sound playing. The function returns immediately.
    Depending on the platform audio facilities, other sounds may stop
    or may be mixed with the new sound.

    The sound can be played again at any time, possibly mixing or
    replacing previous plays of the sound.
*/
void QSound::play()
{
    Q_D(QSound);
    d->looprem = d->looptotal;
    server().play(this);
}

/*!
    Returns the number of times the sound will play.
*/
int QSound::loops() const
{
    Q_D(const QSound);
    return d->looptotal;
}

/*!
    Returns the number of times the sound will loop. This value
    decreases each time the sound loops.
*/
int QSound::loopsRemaining() const
{
    Q_D(const QSound);
    return d->looprem;
}

/*!
    Sets the sound to repeat \a n times when it is played. Passing the
    value -1 will cause the sound to loop indefinitely.

    \sa loops()
*/
void QSound::setLoops(int n)
{
    Q_D(QSound);
    d->looptotal = n;
}

/*!
    Returns the filename associated with the sound.
*/
QString QSound::fileName() const
{
    Q_D(const QSound);
    return d->filename;
}

/*!
    Stops the sound playing.

    On Windows the current loop will finish if a sound is played
    in a loop.

    \sa play()
*/
void QSound::stop()
{
    Q_D(QSound);
    server().stop(this);
    d->looprem = 0;
}


/*!
    Returns true if sound facilities exist on the platform; otherwise
    returns false. An application may choose either to notify the user
    if sound is crucial to the application or to operate silently
    without bothering the user.

    If no sound is available, all QSound operations work silently and
    quickly.

    Note: On Windows this always returns true because some sound card drivers
    do not implement a way to find out whether it is available or not.
*/
bool QSound::isAvailable()
{
    return server().okay();
}

/*!
    Sets the internal bucket record of sound \a s to \a b, deleting
    any previous setting.
*/
void QAuServer::setBucket(QSound* s, QAuBucket* b)
{
    delete s->d_func()->bucket;
    s->d_func()->bucket = b;
}

/*!
    Returns the internal bucket record of sound \a s.
*/
QAuBucket* QAuServer::bucket(QSound* s)
{
    return s->d_func()->bucket;
}

/*!
    Decrements the QSound::loopRemaining() value for sound \a s,
    returning the result.
*/
int QAuServer::decLoop(QSound* s)
{
    if (s->d_func()->looprem > 0)
        --s->d_func()->looprem;
    return s->d_func()->looprem;
}

/*!
    Initializes the sound. The default implementation does nothing.
*/
void QAuServer::init(QSound*)
{
}

QAuBucket::~QAuBucket()
{
}
/*!
    \fn bool QSound::available()

    Use isAvailable() instead.
*/


#endif // QT_NO_SOUND
