/****************************************************************************
**
** Definition of Qt/Embedded keyboards.
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

#ifndef QKBDVR41XX_QWS_H
#define QKBDVR41XX_QWS_H

#ifndef QT_H
#include "qkbd_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_KBD_VR41

class QWSVr41xxKbPrivate;

class QWSVr41xxKeyboardHandler : public QWSKeyboardHandler
{
public:
    QWSVr41xxKeyboardHandler(const QString&);
    virtual ~QWSVr41xxKeyboardHandler();

private:
    QWSVr41xxKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_VR41

#endif // QKBDVR41XX_QWS_H

