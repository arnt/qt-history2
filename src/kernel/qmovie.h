/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmovie.h#20 $
**
** Definition of movie classes
**
** Created : 970617
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QMOVIE_H
#define QMOVIE_H

#ifndef QT_H
#include "qobject.h"
#include "qpixmap.h"
#include "qlist.h"
#endif // QT_H

class QDataSource;
class QMovieFilePrivate;
class QMovieFramePrivate;

class Q_EXPORT QMovieFrame {
public:
    QMovieFrame( QPixmap& pix, int dx, int dy, int dtime );

    QPixmap& pixmap() const;
    int xOffset() const;
    int yOffset() const;
    int timeOffset() const;

    void set( QPixmap& pix, int dx, int dy, int dtime );

    bool operator==( const QMovieFrame& );
    bool operator<( const QMovieFrame& );

private:
    int x_offset;
    int y_offset;
    int time_offset;
    QPixmap mypixmap;
};

typedef QList<QMovieFrame> QMovieFrames;

QDataStream& operator>>(QDataStream&, QMovieFrame&);
QDataStream& operator<<(QDataStream&, QMovieFrame&);
QDataStream& operator>>(QDataStream&, QMovieFrames&);
QDataStream& operator<<(QDataStream&, QMovieFrames&);

class Q_EXPORT QMovie {
public:
    QMovie();
    QMovie(QDataSource*, int bufsize=1024);
    QMovie(const QString &fileName, int bufsize=1024);
    QMovie(QByteArray data, int bufsize=1024);
    QMovie(QList<QMovieFrame> &frames);
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

    void connectResize(QObject* receiver, const char *member);
    void disconnectResize(QObject* receiver, const char *member=0);

    void connectUpdate(QObject* receiver, const char *member);
    void disconnectUpdate(QObject* receiver, const char *member=0);

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
    friend class QMoviePrivate;
    QMovieFilePrivate *d;
    QMovieFramePrivate* f;
};

#endif
