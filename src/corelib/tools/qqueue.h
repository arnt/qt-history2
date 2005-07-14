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

#ifndef QQUEUE_H
#define QQUEUE_H

#include "QtCore/qlist.h"

QT_MODULE(Core)

template <class T>
class QQueue : public QList<T>
{
public:
    inline QQueue() {}
    inline ~QQueue() {}
    inline void enqueue(const T &t) { QList<T>::append(t); }
    inline T dequeue() { return QList<T>::takeFirst(); }
    inline T &head() { return QList<T>::first(); }
    inline const T &head() const { return QList<T>::first(); }
};

#endif // QQUEUE_H
