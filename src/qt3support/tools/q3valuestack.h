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

#ifndef Q3VALUESTACK_H
#define Q3VALUESTACK_H

#include "Qt3Support/q3valuelist.h"

QT_MODULE(Qt3SupportLight)

template<class T>
class Q3ValueStack : public Q3ValueList<T>
{
public:
    Q3ValueStack() {}
   ~Q3ValueStack() {}
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
