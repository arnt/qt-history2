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

#ifndef QPAINTENGINE_QWS_P_H
#define QPAINTENGINE_QWS_P_H

#include <private/qpaintengine_p.h>

class QGfx;

class QWSPaintEnginePrivate : public QPaintEnginePrivate
{
    Q_DECLARE_PUBLIC(QWSPaintEngine)
public:
    QWSPaintEnginePrivate() :gfx(0), clipChildren(true) {}
    QGfx *gfx;
    bool clipChildren;
};

#endif
