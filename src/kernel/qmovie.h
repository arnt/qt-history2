/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmovie.h#14 $
**
** Definition of movie classes
**
** Created : 970617
**
** Copyright (C) 1992-1998 Troll Tech AS.  All rights reserved.
**
** This file is part of Troll Tech's internal development tree for Qt.
**
** This header text will be replaced by an appropriate text by the
** mkdist script which generates external distributions.
**
** If you are using the Qt Professional Edition or the Qt Free Edition,
** please notify Troll Tech at <info@troll.no> if you see this text.
**
** To Troll Tech developers: This header was generated by the script
** fixcopyright-int. It has the same number of text lines as the free
** and professional editions to avoid line number inconsistency.
**
*****************************************************************************/

#ifndef QMOVIE_H
#define QMOVIE_H

#ifndef QT_H
#include "qobject.h"
#include "qpixmap.h"
#endif // QT_H

class QDataSource;
class QMoviePrivate;

class QMovie {
public:
    QMovie();
    QMovie(QDataSource*, int bufsize=1024);
    QMovie(const char* fileName, int bufsize=1024);
    QMovie(QByteArray data, int bufsize=1024);
    QMovie(const QMovie&);
    ~QMovie();

    QMovie& operator=(const QMovie&);

    const QColor& backgroundColor() const;
    void setBackgroundColor(const QColor&);

    const QRect& getValidRect() const;
    const QPixmap& framePixmap() const;

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
    void disconnectResize(QObject* receiver, const char* member);

    void connectUpdate(QObject* receiver, const char* member);
    void disconnectUpdate(QObject* receiver, const char* member);

    enum Status { SourceEmpty=-2,
	          UnrecognizedFormat=-1,
	          Paused=1,
	          EndOfFrame=2,
	          EndOfLoop=3,
	          EndOfMovie=4,
	          SpeedChanged=5 };
    void connectStatus(QObject* receiver, const char* member);
    void disconnectStatus(QObject* receiver, const char* member=0);

private:
    friend class QMoviePrivate;
    QMoviePrivate *d;
};

#endif
