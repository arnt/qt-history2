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

#ifndef QIOENGINE_P_H
#define QIOENGINE_P_H

class QIOEngine;
class Q_CORE_EXPORT QIOEnginePrivate
{
protected:
    QIOEngine *q_ptr;
    Q_DECLARE_PUBLIC(QIOEngine)
protected:
    inline QIOEnginePrivate() : q_ptr(0) { }
    virtual ~QIOEnginePrivate() { q_ptr = 0; }
};

#endif // QIOENGINE_P_H
