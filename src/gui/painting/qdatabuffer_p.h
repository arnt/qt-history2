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
**/

#ifndef QDATABUFFER_P_H
#define QDATABUFFER_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of other Qt classes.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtCore/qbytearray.h>

template <typename Type> class QDataBuffer
{
public:
    QDataBuffer()
    {
        capacity = 64;
        buffer = (Type*) qMalloc(capacity * sizeof(Type));
        siz = 0;
    }

    ~QDataBuffer()
    {
        qFree(buffer);
    }

    void reset() { siz = 0; }

    bool isEmpty() const { return siz==0; }

    int size() const { return siz; }
    Type *data() const { return buffer; }

    const Type &at(int i) const { Q_ASSERT(i >= 0 && i < siz); return buffer[i]; }
    const Type &last() const { Q_ASSERT(!isEmpty()); return buffer[siz-1]; }
    const Type &first() const { Q_ASSERT(!isEmpty()); return buffer[0]; }

    void add(const Type &t) {
        if (siz >= capacity) {
            capacity *= 2;
            buffer = (Type*) qRealloc(buffer, capacity * sizeof(Type));
        }
        buffer[siz] = t;
        ++siz;
    }

private:
    int capacity;
    int siz;
    Type *buffer;
};

#endif // QDATABUFFER_P_H
