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

/*! \class QMovie

    \brief The QMovie class is a convenience class for playing movies
    with QImageIO.

    \ingroup multimedia
    \module gui


   \sa QLabel, QImageIO
*/

#include "qmovie.h"

#include <qimage.h>
#include <qimageio.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qtimer.h>
#include <private/qobject_p.h>

class QMoviePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QMovie)
public:
    QMoviePrivate(QMovie *qq);

    // private slots
    void loadNextFrame();

    QImageIO *io;
    int speed;
    QMovie::MovieState movieState;
    QRect frameRect;
    QImage frameImage;
    QPixmap framePixmap;
    QColor backgroundColor;

    QTimer nextFrameTimer;
};

QMoviePrivate::QMoviePrivate(QMovie *qq)
{
    Q_Q(QMovie);
    q = qq;

    io = 0;
    movieState = QMovie::NotRunning;
    nextFrameTimer.setSingleShot(true);
}

void QMoviePrivate::loadNextFrame()
{
    Q_Q(QMovie);
    if (io->hasNextFrame()) {
        if (io->load()) {
            if (movieState == QMovie::NotRunning) {
                movieState = QMovie::Running;
                emit q->stateChanged(movieState);
                emit q->started();
            }

            frameImage = io->image();
            framePixmap = frameImage;
            if (frameRect.size() != frameImage.rect().size()) {
                frameRect = frameImage.rect();
                emit q->resized(frameRect.size());
            }

            emit q->updated(frameRect);

            nextFrameTimer.start(io->nextFrameDelay());
        } else {
            emit q->error();
            if (movieState != QMovie::NotRunning) {
                movieState = QMovie::NotRunning;
                emit q->stateChanged(movieState);
                emit q->finished();
            }
        }
        return;
    }

    movieState = QMovie::NotRunning;
    emit q->stateChanged(movieState);
    emit q->finished();
}

QMovie::QMovie(QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->io = new QImageIO;
}

QMovie::QMovie(QIODevice *device, const QByteArray &format, QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->io = new QImageIO(device, format);
}

QMovie::QMovie(const QString &fileName, const QByteArray &format, QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->io = new QImageIO(fileName, format);
}

QMovie::~QMovie()
{
    Q_D(QMovie);
    delete d->io;
}

void QMovie::setDevice(QIODevice *device)
{
    Q_D(QMovie);
    d->io->setDevice(device);
}

QIODevice *QMovie::device() const
{
    Q_D(const QMovie);
    return d->io->device();
}

void QMovie::setFileName(const QString &fileName)
{
    Q_D(QMovie);
    d->io->setFileName(fileName);
}

QString QMovie::fileName() const
{
    Q_D(const QMovie);
    return d->io->fileName();
}

void QMovie::setBackgroundColor(const QColor &color)
{
    Q_D(QMovie);
    d->backgroundColor = color;
}

QColor QMovie::backgroundColor() const
{
    Q_D(const QMovie);
    return d->backgroundColor;
}

QMovie::MovieState QMovie::state() const
{
    Q_D(const QMovie);
    return d->movieState;
}

QRect QMovie::frameRect() const
{
    Q_D(const QMovie);
    return d->frameRect;
}

QPixmap QMovie::framePixmap() const
{
    Q_D(const QMovie);
    return d->framePixmap;
}

QImage QMovie::frameImage() const
{
    Q_D(const QMovie);
    return d->frameImage;
}

bool QMovie::isValid() const
{
    Q_D(const QMovie);
    return d->io->hasNextFrame();
}

bool QMovie::isRunning() const
{
    Q_D(const QMovie);
    return d->movieState == Running;
}

int QMovie::frameCount() const
{
    Q_D(const QMovie);
    return d->io->frameCount();
}

int QMovie::nextFrameDelay() const
{
    Q_D(const QMovie);
    return d->io->nextFrameDelay();
}

int QMovie::currentFrameNumber() const
{
    Q_D(const QMovie);
    return d->io->currentFrameNumber();
}

int QMovie::loopCount() const
{
    Q_D(const QMovie);
    return d->io->loopCount();
}

void QMovie::setPaused(bool paused)
{
    if (paused)
        pause();
    else
        unpause();
}

bool QMovie::isPaused() const
{
    Q_D(const QMovie);
    return d->movieState == Paused;
}

void QMovie::setSpeed(int percentSpeed)
{
    Q_D(QMovie);
    d->speed = percentSpeed;
}

int QMovie::speed() const
{
    Q_D(const QMovie);
    return d->speed;
}

void QMovie::start()
{
    Q_D(QMovie);
    if (d->movieState == Running) {
        qWarning("QMovie::start() called when state is Running");
        return;
    }

    connect(&d->nextFrameTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
    d->loadNextFrame();
}

void QMovie::pause()
{
    Q_D(QMovie);
    if (d->movieState == NotRunning) {
        qWarning("QMovie::pause() called when state is NotRunning");
        return;
    }
    emit stateChanged(Paused);
    d->movieState = Paused;
    d->nextFrameTimer.stop();
}

void QMovie::unpause()
{
    Q_D(QMovie);
    if (d->movieState == Running) {
        qWarning("QMovie::unpause() called when state is NotRunning");
        return;
    }
    emit stateChanged(Running);
    d->movieState = Running;
    d->nextFrameTimer.start(d->io->nextFrameDelay());
}

void QMovie::stop()
{
    Q_D(QMovie);
    if (d->movieState == NotRunning) {
        qWarning("QMovie::stop() called when state is NotRunning");
        return;
    }
    emit stateChanged(NotRunning);
    d->movieState = NotRunning;
    d->nextFrameTimer.stop();
}

#define d d_func()
#include "moc_qmovie.cpp"
