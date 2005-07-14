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

#ifndef QKBD_QWS_H
#define QKBD_QWS_H

#include "QtGui/qapplication.h"

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

class QWSKbPrivate;

class QWSKeyboardHandler
{
public:
    QWSKeyboardHandler();
    virtual ~QWSKeyboardHandler();

    virtual void processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                            bool isPress, bool autoRepeat);

protected:
    int transformDirKey(int key);
    void beginAutoRepeat(int uni, int code, Qt::KeyboardModifiers mod);
    void endAutoRepeat();

private:
    QWSKbPrivate *d;
};

#endif // QT_NO_QWS_KEYBOARD

#endif // QKBD_QWS_H
