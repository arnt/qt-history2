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
    with QImageReader.

    \ingroup multimedia
    \module gui


   \sa QLabel, QImageReader
*/

#include "qmovie.h"

#include <qimage.h>
#include <qimagereader.h>
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

    QImageReader *reader;
    int speed;
    QMovie::MovieState movieState;
    QRect frameRect;
    QImage frameImage;
    QPixmap framePixmap;
    QColor backgroundColor;

    QTimer nextImageTimer;
};

QMoviePrivate::QMoviePrivate(QMovie *qq)
{
    Q_Q(QMovie);
    q = qq;

    reader = 0;
    movieState = QMovie::NotRunning;
    nextImageTimer.setSingleShot(true);
}

void QMoviePrivate::loadNextFrame()
{
    Q_Q(QMovie);
    if (reader->canRead()) {
        frameImage = reader->read();
        if (!frameImage.isNull()) {
            if (movieState == QMovie::NotRunning) {
                movieState = QMovie::Running;
                emit q->stateChanged(movieState);
                emit q->started();
            }

            framePixmap = frameImage;
            if (frameRect.size() != frameImage.rect().size()) {
                frameRect = frameImage.rect();
                emit q->resized(frameRect.size());
            }

            emit q->updated(frameRect);

            nextImageTimer.start(reader->nextImageDelay());
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
    d->reader = new QImageReader;
}

QMovie::QMovie(QIODevice *device, const QByteArray &format, QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->reader = new QImageReader(device, format);
}

QMovie::QMovie(const QString &fileName, const QByteArray &format, QObject *parent)
    : QObject(*new QMoviePrivate(this), parent)
{
    Q_D(QMovie);
    d->reader = new QImageReader(fileName, format);
}

QMovie::~QMovie()
{
    Q_D(QMovie);
    delete d->reader;
}

void QMovie::setDevice(QIODevice *device)
{
    Q_D(QMovie);
    d->reader->setDevice(device);
}

QIODevice *QMovie::device() const
{
    Q_D(const QMovie);
    return d->reader->device();
}

void QMovie::setFileName(const QString &fileName)
{
    Q_D(QMovie);
    d->reader->setFileName(fileName);
}

QString QMovie::fileName() const
{
    Q_D(const QMovie);
    return d->reader->fileName();
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
    return d->reader->canRead();
}

bool QMovie::isRunning() const
{
    Q_D(const QMovie);
    return d->movieState == Running;
}

int QMovie::frameCount() const
{
    Q_D(const QMovie);
    return d->reader->imageCount();
}

int QMovie::nextFrameDelay() const
{
    Q_D(const QMovie);
    return d->reader->nextImageDelay();
}

int QMovie::currentFrameNumber() const
{
    Q_D(const QMovie);
    return d->reader->currentImageNumber();
}

int QMovie::loopCount() const
{
    Q_D(const QMovie);
    return d->reader->loopCount();
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

    connect(&d->nextImageTimer, SIGNAL(timeout()), this, SLOT(loadNextFrame()));
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
    d->nextImageTimer.stop();
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
    d->nextImageTimer.start(d->reader->nextImageDelay());
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
    d->nextImageTimer.stop();
}

#define d d_func()
#include "moc_qmovie.cpp"
