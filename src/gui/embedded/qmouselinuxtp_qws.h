/****************************************************************************
**
** Definition of Qt/Embedded mouse driver.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSELINUXTP_QWS_H
#define QMOUSELINUXTP_QWS_H

#ifndef QT_H
#include "qmouse_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_MOUSE_LINUXTP

class QWSLinuxTPMouseHandlerPrivate;

class QWSLinuxTPMouseHandler : public QWSCalibratedMouseHandler
{
public:
    QWSLinuxTPMouseHandler(const QString & = QString::null, const QString & = QString::null);
    ~QWSLinuxTPMouseHandler();

protected:
    QWSLinuxTPMouseHandlerPrivate *d;
};

#endif

#endif

