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

#ifndef QMOVIE_H
#define QMOVIE_H

#ifndef QT_NO_MOVIE

#include <QtCore/qobject.h>

class QByteArray;
class QColor;
class QIODevice;
class QImage;
class QPixmap;
class QRect;
class QSize;

class QMoviePrivate;
class Q_GUI_EXPORT QMovie : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QMovie)
public:
    enum MovieState {
        NotRunning,
        Paused,
        Running
    };

    QMovie(QObject *parent = 0);
    explicit QMovie(QIODevice *device, const QByteArray &format = QByteArray(), QObject *parent = 0);
    explicit QMovie(const QString &fileName, const QByteArray &format = QByteArray(), QObject *parent = 0);
    ~QMovie();

    void setDevice(QIODevice *device);
    QIODevice *device() const;

    void setFileName(const QString &fileName);
    QString fileName() const;

    void setFormat(const QByteArray &format);
    QByteArray format() const;

    void setBackgroundColor(const QColor &color);
    QColor backgroundColor() const;

    MovieState state() const;

    QRect frameRect() const;
    QImage frameImage() const;
    QPixmap framePixmap() const;

    bool isValid() const;
    bool isRunning() const;

    int loopCount() const;
    int frameCount() const;
    int nextFrameDelay() const;
    int currentFrameNumber() const;

    void setPaused(bool paused);
    bool isPaused() const;

    void setSpeed(int percentSpeed);
    int speed() const;

signals:
    void started();
    void resized(const QSize &size);
    void updated(const QRect &rect);
    void stateChanged(MovieState state);
    void error();
    void finished();

public slots:
    void start();
    void pause();
    void unpause();
    void stop();

private:
    Q_DISABLE_COPY(QMovie)
    Q_PRIVATE_SLOT(d, void loadNextFrame())
};

#endif // QT_NO_MOVIE

#endif // QMOVIE_H
