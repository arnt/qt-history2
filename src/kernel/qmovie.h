/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qmovie.h#24 $
**
** Definition of movie classes
**
** Created : 970617
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
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

class Q_EXPORT QMovie {
public:
    QMovie();
    QMovie(QDataSource*, int bufsize=1024);
    QMovie(const QString &fileName, int bufsize=1024);
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

    void connectResize(QObject* receiver, const char *member);
    void disconnectResize(QObject* receiver, const char *member=0);

    void connectUpdate(QObject* receiver, const char *member);
    void disconnectUpdate(QObject* receiver, const char *member=0);

#ifdef _WS_QWS_
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
    friend class QMoviePrivate;
    QMovieFilePrivate *d;
};

#endif
