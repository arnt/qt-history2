/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmovie.h#6 $
**
** Definition of movie classes
**
** Created : 970617
**
** Copyright (C) 1994-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#ifndef QMOVIE_H
#define QMOVIE_H

#include "qobject.h"
#include "qpixmap.h"

class QIODevice;
class QMoviePrivate;

class QMovie {
public:
    QMovie();
    QMovie(const char* srcfile, int bufsize=1024);
    QMovie(QByteArray data, int bufsize=1024);
    QMovie(const QMovie&);
    ~QMovie();

    const QMovie& operator=(const QMovie&);

    const QColor& backgroundColor() const;
    void setBackgroundColor(const QColor&);

    const QRect& getValidRect() const;
    const QPixmap& currentFrame() const;

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

    void connectResize(QObject* receiver, const char* member);
    void disconnectResize(QObject* receiver, const char* member=0);

    void connectUpdate(QObject* receiver, const char* member);
    void disconnectUpdate(QObject* receiver, const char* member=0);

    enum {  SourceEmpty=-2,
	    UnrecognizedFormat=-1,
	    Paused=1,
	    EndOfFrame=2,
	    EndOfLoop=3,
	    EndOfMovie=4,
	    SpeedChanged=5 } Status;
    void connectStatus(QObject* receiver, const char* member);
    void disconnectStatus(QObject* receiver, const char* member=0);

private:
    friend QMoviePrivate;
    QMoviePrivate *d;
};

#endif
