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

#ifndef QOBJECTDICT_H
#define QOBJECTDICT_H

#ifndef QT_H
#include "qmetaobject.h"
#include "qasciidict.h"
#endif // QT_H


//
// The object dictionary is a collection of QMetaObjects
//

class Q_COMPAT_EXPORT QObjectDictionary : public QAsciiDict<QMetaObject>
{
public:
    QObjectDictionary(int size=17,bool cs=true,bool ck=true)
        : QAsciiDict<QMetaObject>(size,cs,ck) {}
    QObjectDictionary(const QObjectDictionary &dict)
        : QAsciiDict<QMetaObject>(dict) {}
   ~QObjectDictionary() { clear(); }
    QObjectDictionary &operator=(const QObjectDictionary &dict)
        { return (QObjectDictionary&)QAsciiDict<QMetaObject>::operator=(dict);}
};

#endif // QOBJECTDICT_H
