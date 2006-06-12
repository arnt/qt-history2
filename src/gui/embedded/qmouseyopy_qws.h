/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSEYOPY_QWS_H
#define QMOUSEYOPY_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MOUSE_YOPY

// YOPY touch panel support based on changes contributed by Ron Victorelli
// (victorrj at icubed.com) to Custom TP driver.

class QWSYopyMouseHandlerPrivate;

class QWSYopyMouseHandler : public QWSMouseHandler
{
public:
    explicit QWSYopyMouseHandler(const QString & = QString(),
                                 const QString & = QString());
    ~QWSYopyMouseHandler();

    void resume();
    void suspend();

protected:
    QWSYopyMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_YOPY

QT_END_HEADER

#endif // QMOUSEYOPY_QWS_H
