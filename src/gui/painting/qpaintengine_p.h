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

#ifndef QPAINTENGINE_P_H
#define QPAINTENGINE_P_H

// Used to get Q_DECLARE_PUBLIC
#include "private/qobject_p.h"
#include "qpainter.h"

class QPaintEngine;
class QPaintDevice;

class QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QPaintEngine)
public:
    QPaintEnginePrivate() : pdev(0), q_ptr(0), renderhints(0) { }
    QPaintDevice *pdev;
    QPaintEngine *q_ptr;
    QPainter::RenderHints renderhints;
};

#endif // QPAINTENGINE_P_H
