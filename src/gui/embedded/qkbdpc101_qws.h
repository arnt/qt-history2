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

#ifndef QKBDPC101_QWS_H
#define QKBDPC101_QWS_H

#include <QtGui/qkbd_qws.h>

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_PC101

typedef struct QWSKeyMap {
    uint key_code;
    ushort unicode;
    ushort shift_unicode;
    ushort ctrl_unicode;
};

class QWSPC101KeyboardHandler : public QWSKeyboardHandler
{
public:
    explicit QWSPC101KeyboardHandler(const QString&);
    virtual ~QWSPC101KeyboardHandler();

    virtual void doKey(uchar scancode);
    virtual const QWSKeyMap *keyMap() const;

protected:
    bool shift;
    bool alt;
    bool ctrl;
    bool caps;
#if defined(QT_QWS_IPAQ)
    uint ipaq_return_pressed:1;
#endif
    uint extended:2;
    Qt::KeyboardModifiers modifiers;
    int prevuni;
    int prevkey;
};

#endif // QT_NO_QWS_KBD_PC101

#endif // QT_NO_QWS_KEYBOARD

#endif // QKBDPC101_QWS_H
