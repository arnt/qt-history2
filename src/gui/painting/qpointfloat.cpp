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

#include "qpointfloat.h"
#include <qdebug.h>

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug d, const QPointFloat &p)
{
    d << "QPointFloat(" << p.x() << ", " << p.y() << ")";
    return d;
}
#endif
