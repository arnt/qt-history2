/****************************************************************************
**
** Definition of Qt/Embedded keyboards.
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

#ifndef QKBDYOPY_QWS_H
#define QKBDYOPY_QWS_H

#ifndef QT_H
#include "qkbd_qws.h"
#endif // QT_H

#ifndef QT_NO_QWS_KBD_YOPY

class QWSYopyKbPrivate;

class QWSYopyKeyboardHandler : public QWSKeyboardHandler
{
public:
    QWSYopyKeyboardHandler(const QString&);
    virtual ~QWSYopyKeyboardHandler();

private:
    QWSYopyKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_YOPY

#endif // QKBDYOPY_QWS_H
