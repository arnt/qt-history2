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

#include "qmovie.h"

// Notice: This class has an empty implementation. It is currently
// being refactored.

#include <qcolor.h>
#include <qimage.h>
#include <qrect.h>
#include <qpixmap.h>

QMovie::QMovie()
{
}

QMovie::QMovie(int /* bufsize */)
{
}

QMovie::QMovie(QIODevice *, int /* bufsize */)
{
}

QMovie::QMovie(const QString & /* fileName */, int /* bufsize */)
{
}

QMovie::QMovie(const QMovie &)
{
}

QMovie::~QMovie()
{
}

QMovie& QMovie::operator=(const QMovie &)
{
    static QMovie NIL;
    return NIL;
}

int QMovie::pushSpace() const
{
    return 0;
}

void QMovie::pushData(const uchar * /* data */, int /* length */)
{
}

const QColor& QMovie::backgroundColor() const
{
    static QColor NIL;
    return NIL;
}

void QMovie::setBackgroundColor(const QColor &)
{
}

const QRect& QMovie::getValidRect() const
{
    static QRect NIL;
    return NIL;
}

const QPixmap& QMovie::framePixmap() const
{
    static QPixmap NIL;
    return NIL;
}

const QImage& QMovie::frameImage() const
{
    static QImage NIL;
    return NIL;
}

bool QMovie::isNull() const
{
    return false;
}

int  QMovie::frameNumber() const
{
    return 0;
}

int  QMovie::steps() const
{
    return 0;
}

bool QMovie::paused() const
{
    return false;
}

bool QMovie::finished() const
{
    return false;
}

bool QMovie::running() const
{
    return false;
}

void QMovie::unpause()
{
}

void QMovie::pause()
{
}

void QMovie::step()
{
}

void QMovie::step(int)
{
}

void QMovie::restart()
{
}

int  QMovie::speed() const
{
    return 0;
}

void QMovie::setSpeed(int)
{
}

void QMovie::connectResize(QObject * /* receiver */, const char * /* member */)
{
}

void QMovie::disconnectResize(QObject * /* receiver */, const char * /* member */)
{
}

void QMovie::connectUpdate(QObject * /* receiver */, const char * /* member */)
{
}

void QMovie::disconnectUpdate(QObject * /* receiver */, const char * /* member */)
{
}

#ifdef Q_WS_QWS
// Temporary hack
void QMovie::setDisplayWidget(QWidget * /* w */)
{
}
#endif

void QMovie::connectStatus(QObject * /* receiver */, const char * /* member */)
{
}

void QMovie::disconnectStatus(QObject * /* receiver */, const char * /* member */)
{
}
