/****************************************************************************
**
** Definition of Qt/Embedded Graphics Driver Interface.
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

#ifndef QGFXDRIVERINTERFACE_P_H
#define QGFXDRIVERINTERFACE_P_H

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
#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

// {449EC6C6-DF3E-43E3-9E57-354A3D05AB34}
#ifndef IID_QGfxDriver
#define IID_QGfxDriver QUuid(0x449ec6c6, 0xdf3e, 0x43e3, 0x9e, 0x57, 0x35, 0x4a, 0x3d, 0x05, 0xab, 0x34)
#endif

class QScreen;

struct Q_GUI_EXPORT QGfxDriverInterface : public QFeatureListInterface
{
    virtual QScreen* create(const QString& driver, int displayId) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QGFXDRIVERINTERFACE_P_H
