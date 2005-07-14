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

#ifndef QKBDTTY_QWS_H
#define QKBDTTY_QWS_H

#include "QtGui/qkbdpc101_qws.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_TTY

class QWSTtyKbPrivate;

class QWSTtyKeyboardHandler : public QWSPC101KeyboardHandler
{
public:
    explicit QWSTtyKeyboardHandler(const QString&);
    virtual ~QWSTtyKeyboardHandler();

protected:
    virtual void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                                bool isPress, bool autoRepeat);

private:
    QWSTtyKbPrivate *d;
};

#endif

#endif // QT_NO_QWS_KEYBOARD

#endif // QKBDTTY_QWS_H
