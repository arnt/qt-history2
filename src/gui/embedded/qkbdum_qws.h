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

#ifndef QKBDUM_QWS_H
#define QKBDUM_QWS_H

#include <QtGui/qkbd_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_UM

class QWSUmKeyboardHandlerPrivate;

class QWSUmKeyboardHandler : public QWSKeyboardHandler
{
public:
    QWSUmKeyboardHandler(const QString &);
    virtual ~QWSUmKeyboardHandler();

private:

    QWSUmKeyboardHandlerPrivate *d;
};
#endif // QT_NO_QWS_KBD_UM

#endif // QT_NO_QWS_KEYBOARD

QT_END_NAMESPACE

QT_END_HEADER

#endif // QKBDUM_QWS_H
