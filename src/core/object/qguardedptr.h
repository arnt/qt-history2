/****************************************************************************
**
** Definition of QGuardedPtr class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QGUARDEDPTR_H
#define QGUARDEDPTR_H

#ifndef QT_H
#include "qpointer.h"
#endif // QT_H

template <class T> class QGuardedPtr : public QPointer<T>
{
public:
    inline QGuardedPtr(){}
    inline QGuardedPtr(T *obj) : QPointer<T>(obj){}
    inline QGuardedPtr(const QPointer<T> &p) : QPointer<T>(p) {}
};
#endif
