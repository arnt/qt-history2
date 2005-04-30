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

#ifndef QMOUSEYOPY_QWS_H
#define QMOUSEYOPY_QWS_H

#include "QtGui/qmouse_qws.h"

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

protected:
    QWSYopyMouseHandlerPrivate *d;
};

#endif

#endif // QMOUSEYOPY_QWS_H
