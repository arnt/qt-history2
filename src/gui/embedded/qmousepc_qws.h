/****************************************************************************
**
** Definition of Qt/Embedded mouse driver.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSEPC_QWS_H
#define QMOUSEPC_QWS_H

#ifndef QT_H
#include "qmouse_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_MOUSE_PC

class QWSPcMouseHandlerPrivate;

class QWSPcMouseHandler : public QWSMouseHandler
{
public:
    QWSPcMouseHandler( const QString & = QString::null, const QString & = QString::null );
    ~QWSPcMouseHandler();

protected:
    QWSPcMouseHandlerPrivate *d;
};

#endif

#endif

