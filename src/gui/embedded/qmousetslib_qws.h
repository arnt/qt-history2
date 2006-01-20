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

#ifndef QMOUSETSLIB_QWS_H
#define QMOUSETSLIB_QWS_H

#include <QtGui/qmouse_qws.h>

QT_MODULE(Gui)

#if !defined(QT_NO_QWS_MOUSE_TSLIB) || defined(QT_PLUGIN)

class QWSTslibMouseHandlerPrivate;

class QWSTslibMouseHandler : public QWSCalibratedMouseHandler
{
public:
    explicit QWSTslibMouseHandler(const QString &driver = QString(),
                                  const QString &device = QString());
    ~QWSTslibMouseHandler();

    void suspend();
    void resume();

    void calibrate(const QWSPointerCalibrationData *data);
    void clearCalibration();

protected:
    QWSTslibMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_TSLIB
#endif // QMOUSETSLIB_QWS_H
