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

#ifndef QWSMOUSE_QNX4_H
#define QWSMOUSE_QNX4_H

#ifndef QT_H
#include "qwindowsystem_qws.h"
#endif // QT_H

#ifdef Q_OS_QNX4

#include "qwsevent_qws.h"
#include "qwscommand_qws.h"
#include "qwsutils_qws.h"
#include "qwsmouse_qws.h"

#include <qapplication.h>
#include <qpointarray.h>
#include <qscreen_qws.h>


#ifndef QQNX4MOUSEHANDLERPRIVATEIMPL
#define QQNX4MOUSEHANDLERPRIVATEIMPL

struct mouse_event;
class QQnx4MouseHandlerPrivate : public QWSMouseHandler {
    Q_OBJECT

    public:
        QQnx4MouseHandlerPrivate(MouseProtocol &, QString);
        ~QQnx4MouseHandlerPrivate();

        void clearCalibration();
        void calibrate();
        void getCalibration(QWSPointerCalibrationData *) const;

    private:
        QSocketNotifier *mouseNotifier;
        int mouseFD, read_in;
        mouse_event *mpack;

    private slots:
        void readMouseData(int);
};

#endif
#endif

#endif // QWSMOUSE_QNX4_H
