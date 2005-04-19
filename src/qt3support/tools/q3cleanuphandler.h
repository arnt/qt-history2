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

#ifndef Q3CLEANUPHANDLER_H
#define Q3CLEANUPHANDLER_H

#include <QtCore/qlist.h>

template<class T>
class Q3CleanupHandler
{
    QListData p;
public:
    inline Q3CleanupHandler()
    { p.d = 0; }
    ~Q3CleanupHandler()
    {
        if (p.d) {
            for (int i = 0; i < p.size(); ++i) {
                T** t = static_cast<T**>(*p.at(i));
                delete *t;
                *t = 0;
            }
            qFree(p.d);
            p.d = 0;
        }
    }

    T* add(T **object)
    {
        if (!p.d) {
            p.d = &QListData::shared_null;
            p.d->ref.ref();
            p.detach();
        }
        *p.prepend() =  object;
        return *object;
    }
    void remove(T **object)
    {
        if (p.d)
            for (int i = 0; i < p.size(); ++i)
                if (*p.at(i) == object)
                    p.remove(i--);
    }
};

template<class T>
class Q3SingleCleanupHandler
{
    T **object;
public:
    inline Q3SingleCleanupHandler()
    : object(0) {}
    inline ~Q3SingleCleanupHandler()
    { if (object) { delete *object; *object = 0; } }
    inline T* set(T **o)
    { object = o; return *object; }
    inline void reset() { object = 0; }
};

#endif //Q3CLEANUPHANDLER_H
