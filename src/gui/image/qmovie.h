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

class QColor;
class QIODevice;
class QMoviePrivate;
class QObject;
class QObject;
class QPixmap;
class QRect;
class QWidget;
class QImage;
class QString;

#include <QtCore/qglobal.h>

class Q_GUI_EXPORT QMovie {
public:
    QMovie();
    explicit QMovie(int bufsize);
    explicit QMovie(QIODevice*, int bufsize=1024);
    explicit QMovie(const QString &fileName, int bufsize=1024);
    QMovie(const QMovie&);
    ~QMovie();

    QMovie& operator=(const QMovie&);

    int pushSpace() const;
    void pushData(const uchar* data, int length);

    const QColor& backgroundColor() const;
    void setBackgroundColor(const QColor&);

    const QRect& getValidRect() const;
    const QPixmap& framePixmap() const;
    const QImage& frameImage() const;

    bool isNull() const;

    int  frameNumber() const;
    int  steps() const;
    bool paused() const;
    bool finished() const;
    bool running() const;

    void unpause();
    void pause();
    void step();
    void step(int);
    void restart();

    int  speed() const;
    void setSpeed(int);

    void connectResize(QObject* receiver, const char *member);
    void disconnectResize(QObject* receiver, const char *member=0);

    void connectUpdate(QObject* receiver, const char *member);
    void disconnectUpdate(QObject* receiver, const char *member=0);

#ifdef Q_WS_QWS
    // Temporary hack
    void setDisplayWidget(QWidget * w);
#endif

    enum Status { SourceEmpty=-2,
                  UnrecognizedFormat=-1,
                  Paused=1,
                  EndOfFrame=2,
                  EndOfLoop=3,
                  EndOfMovie=4,
                  SpeedChanged=5 };
    void connectStatus(QObject* receiver, const char *member);
    void disconnectStatus(QObject* receiver, const char *member=0);

private:
    QMoviePrivate *d;
};

#endif // QT_NO_MOVIE

#endif // QMOVIE_H
