/****************************************************************************
** $Id$
**
** Definition of Qt/Embedded keyboards
**
** Created : 20020218
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QKBDPC101_QWS_H
#define QKBDPC101_QWS_H

#include "qkbd_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBDPC101

typedef struct QWSKeyMap {
    ushort key_code;
    ushort unicode;
    ushort shift_unicode;
    ushort ctrl_unicode;
};

class QWSPC101KeyboardHandler : public QWSKeyboardHandler
{
public:
    QWSPC101KeyboardHandler();
    virtual ~QWSPC101KeyboardHandler();

    virtual void doKey(uchar scancode);
    virtual const QWSKeyMap *keyMap() const;

protected:
    bool shift;
    bool alt;
    bool ctrl;
    bool caps;
    bool extended;
    int modifiers;
    int prevuni;
    int prevkey;
};

#endif // QT_NO_QWS_KBDPC101

#endif // QT_NO_QWS_KEYBOARD

#endif // QKBDTTY_QWS_H

