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

#ifndef QVALUESTACK_H
#define QVALUESTACK_H

#include "qvaluelist.h"

#ifdef QT_COMPAT
template<class T>
class QValueStack : public QValueList<T>
{
public:
    QValueStack() {}
   ~QValueStack() {}
    void  push(const T& val) { this->append(val); }
    T pop()
    {
        T elem(this->last());
        if (!this->isEmpty())
            this->remove(this->fromLast());
        return elem;
    }
    T& top() { return this->last(); }
    const T& top() const { return this->last(); }
};
#endif

#endif
