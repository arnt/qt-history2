/****************************************************************************
**
** Definition of Qt/Embedded Mouse Driver Interface.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMOUSEDRIVERINTERFACE_P_H
#define QMOUSEDRIVERINTERFACE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  This header file may
// change from version to version without notice, or even be
// removed.
//
// We mean it.
//
//

#ifndef QT_H
#include <qglobal.h>
////////#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

// {4367CF5A-F7CE-407B-8BB6-DF19AEDA2EBB}
#ifndef IID_QMouseDriver
#define IID_QMouseDriver QUuid(0x4367cf5a, 0xf7ce, 0x407b, 0x8b, 0xb6, 0xdf, 0x19, 0xae, 0xda, 0x2e, 0xbb)
#endif

class QWSMouseHandler;

struct Q_GUI_EXPORT QMouseDriverInterface : public QFeatureListInterface
{
    virtual QWSMouseHandler* create(const QString& driver, const QString &device) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QMOUSEDRIVERINTERFACE_P_H
