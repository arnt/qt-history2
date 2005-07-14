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

#ifndef QKBDUSB_QWS_H
#define QKBDUSB_QWS_H

#include "QtGui/qkbdpc101_qws.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_USB

class QWSUsbKbPrivate;

class QWSUsbKeyboardHandler : public QWSPC101KeyboardHandler
{
public:
    QWSUsbKeyboardHandler(const QString&);
    virtual ~QWSUsbKeyboardHandler();

private:
    QWSUsbKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_USB

#endif // QT_NO_QWS_KEYBOARD

#endif // QKBDUSB_QWS_H
