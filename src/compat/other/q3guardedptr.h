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

#ifndef Q3GUARDEDPTR_H
#define Q3GUARDEDPTR_H

#include "qpointer.h"

template <class T> class Q3GuardedPtr : public QPointer<T>
{
public:
    inline Q3GuardedPtr(){}
    inline Q3GuardedPtr(T *obj) : QPointer<T>(obj){}
    inline Q3GuardedPtr(const QPointer<T> &p) : QPointer<T>(p) {}
};
#endif
