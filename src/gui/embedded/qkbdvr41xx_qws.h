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

#ifndef QKBDVR41XX_QWS_H
#define QKBDVR41XX_QWS_H

#include "QtGui/qkbd_qws.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KBD_VR41XX

class QWSVr41xxKbPrivate;

class QWSVr41xxKeyboardHandler : public QWSKeyboardHandler
{
public:
    explicit QWSVr41xxKeyboardHandler(const QString&);
    virtual ~QWSVr41xxKeyboardHandler();

private:
    QWSVr41xxKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_VR41XX

#endif // QKBDVR41XX_QWS_H
