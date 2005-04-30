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

#ifndef QMOUSELINUXTP_QWS_H
#define QMOUSELINUXTP_QWS_H

#include "QtGui/qmouse_qws.h"

#ifndef QT_NO_QWS_MOUSE_LINUXTP

class QWSLinuxTPMouseHandlerPrivate;

class QWSLinuxTPMouseHandler : public QWSCalibratedMouseHandler
{
public:
    explicit QWSLinuxTPMouseHandler(const QString & = QString(),
                                    const QString & = QString());
    ~QWSLinuxTPMouseHandler();

    void suspend();
    void resume();
protected:
    QWSLinuxTPMouseHandlerPrivate *d;
};

#endif

#endif // QMOUSELINUXTP_QWS_H
