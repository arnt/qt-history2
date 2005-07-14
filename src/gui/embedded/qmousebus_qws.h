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

#ifndef QMOUSEBUS_QWS_H
#define QMOUSEBUS_QWS_H

#include "QtGui/qmouse_qws.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MOUSE_BUS

class QWSBusMouseHandlerPrivate;

class QWSBusMouseHandler : public QWSMouseHandler
{
public:
    explicit QWSBusMouseHandler(const QString & = QString(),
                                const QString & = QString());
    ~QWSBusMouseHandler();

    void suspend();
    void resume();
protected:
    QWSBusMouseHandlerPrivate *d;
};

#endif

#endif // QMOUSEBUS_QWS_H
