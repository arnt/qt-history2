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

#ifndef QMOUSEVR41XX_QWS_H
#define QMOUSEVR41XX_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MOUSE_VR41XX

class QWSVr41xxMouseHandlerPrivate;

class QWSVr41xxMouseHandler : public QWSCalibratedMouseHandler
{
public:
    explicit QWSVr41xxMouseHandler(const QString & = QString(),
                                   const QString & = QString());
    ~QWSVr41xxMouseHandler();

    void resume();
    void suspend();

protected:
    QWSVr41xxMouseHandlerPrivate *d;

private:
    friend class QWSVr41xxMouseHandlerPrivate;
};

#endif // QT_NO_QWS_MOUSE_VR41XX

QT_END_NAMESPACE

QT_END_HEADER

#endif // QMOUSEVR41XX_QWS_H
