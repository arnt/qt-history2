/****************************************************************************
** $Id$
**
** ...
**
** Copyright (C) 2001-$THISYEAR$ Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QCLEANUPHANDLER_H
#define QCLEANUPHANDLER_H

#ifndef QT_H
#include <qlist.h>
#endif // QT_H

template<class T>
class QCleanupHandler
{
    QListData p;
public:
    inline QCleanupHandler()
    { p.d = 0; }
    ~QCleanupHandler()
    {
        if (p.d) {
            for (int i = 0; i < p.size(); ++i) {
                T** t = (T**) *p.at(i);
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
            ++p.d->ref;
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
class QSingleCleanupHandler
{
    T **object;
public:
    inline QSingleCleanupHandler()
    : object(0) {}
    inline ~QSingleCleanupHandler()
    { if (object) { delete *object; *object = 0; } }
    inline T* set(T **o)
    { object = o; return *object; }
    inline void reset() { object = 0; }
};


// ###### remove before release!
template<class T>
class QSharedCleanupHandler
{
    T **object;
public:
    inline QSharedCleanupHandler() : object(0) {}
    inline ~QSharedCleanupHandler()
    {
        if (object) {
            if ((*object)->deref()) delete *object;
            *object = 0;
        }
    }
    inline T* set(T **o) { object = o; return *object; }
    inline void reset() { object = 0; }
};

#endif //QCLEANUPHANDLER_H
