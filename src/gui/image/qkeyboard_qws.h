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

#ifndef QKEYBOARD_QWS_H
#define QKEYBOARD_QWS_H

#include "qobject.h"

#ifndef QT_NO_QWS_KEYBOARD
class QWSKeyboardHandler : public QObject {
    Q_OBJECT
public:
    QWSKeyboardHandler();
    virtual ~QWSKeyboardHandler();

protected:
    virtual void processKeyEvent(int unicode, int keycode, int modifiers,
                            bool isPress, bool autoRepeat);
};
#endif

#endif
