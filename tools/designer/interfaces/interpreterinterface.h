/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef INTERPRETERINTERFACE_H
#define INTERPRETERINTERFACE_H

//
//  W A R N I N G  --  PRIVATE INTERFACES
//  --------------------------------------
//
// This file and the interfaces declared in the file are not
// public. It exists for internal purpose. This header file and
// interfaces may change from version to version (even binary
// incompatible) without notice, or even be removed.
//
// We mean it.
//
//

#include <private/qcom_p.h>

class QObject;

// {11cad9ec-4e3c-418b-8e90-e1b8c0c1f48f}
#ifndef IID_Interpreter
#define IID_Interpreter QUuid( 0x11cad9ec, 0x4e3c, 0x418b, 0x8e, 0x90, 0xe1, 0xb8, 0xc0, 0xc1, 0xf4, 0x8f )
#endif

struct InterpreterInterface : public QUnknownInterface
{
    virtual void setBreakPoints( QObject *obj, const QList<uint> &lst ) = 0;
};



#endif
