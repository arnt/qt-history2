/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QKBD_QWS_H
#define QKBD_QWS_H

#include <QtGui/qapplication.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

class QWSKbPrivate;

class  Q_GUI_EXPORT QWSKeyboardHandler
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

QT_END_NAMESPACE

QT_END_HEADER

#endif // QKBD_QWS_H
