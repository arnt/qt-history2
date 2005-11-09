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

#ifndef QSCROLLAREA_P_H
#define QSCROLLAREA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#ifndef QT_NO_SCROLLAREA

#include "private/qabstractscrollarea_p.h"

class QScrollAreaPrivate: public QAbstractScrollAreaPrivate
{
    Q_DECLARE_PUBLIC(QScrollArea)

public:
    QScrollAreaPrivate(): resizable(false){}
    void updateScrollBars();
    QPointer<QWidget> widget;
    mutable QSize widgetSize;
    bool resizable;
};

#endif

#endif
