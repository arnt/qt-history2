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

#ifndef QMOUSEVR41XX_QWS_H
#define QMOUSEVR41XX_QWS_H

#include "QtGui/qmouse_qws.h"

#ifndef QT_NO_QWS_MOUSE_VR41XX

class QWSVr41xxMouseHandlerPrivate;

class QWSVr41xxMouseHandler : public QWSCalibratedMouseHandler
{
public:
    explicit QWSVr41xxMouseHandler(const QString & = QString(),
                                   const QString & = QString());
    ~QWSVr41xxMouseHandler();

protected:
    QWSVr41xxMouseHandlerPrivate *d;
};

#endif

#endif // QMOUSEVR41XX_QWS_H
