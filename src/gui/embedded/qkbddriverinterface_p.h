/****************************************************************************
**
** Definition of Qt/Embedded Keyboard Driver Interface.
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

#ifndef QKBDDRIVERINTERFACE_P_H
#define QKBDDRIVERINTERFACE_P_H

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
////#include <private/qcom_p.h>
#endif // QT_H

#ifndef QT_NO_COMPONENT

// {C7C838EA-FC3E-4905-92AD-F479E81F1D02}
#ifndef IID_QKbdDriver
#define IID_QKbdDriver QUuid(0xc7c838ea, 0xfc3e, 0x4905, 0x92, 0xad, 0xf4, 0x79, 0xe8, 0x1f, 0x1d, 0x02)
#endif

class QWSKeyboardHandler;

struct Q_GUI_EXPORT QKbdDriverInterface : public QFeatureListInterface
{
    virtual QWSKeyboardHandler* create(const QString& driver, const QString& device) = 0;
};

#endif // QT_NO_COMPONENT

#endif // QKBDDRIVERINTERFACE_P_H
