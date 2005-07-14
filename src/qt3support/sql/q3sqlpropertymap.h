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

#ifndef Q3SQLPROPERTYMAP_H
#define Q3SQLPROPERTYMAP_H

#include "QtCore/qvariant.h"
#include "QtCore/qstring.h"

QT_MODULE(Qt3Support)

#ifndef QT_NO_SQL_FORM

class QWidget;
class Q3SqlPropertyMapPrivate;

class Q_COMPAT_EXPORT Q3SqlPropertyMap
{
public:
    Q3SqlPropertyMap();
    virtual ~Q3SqlPropertyMap();

    QVariant      property(QWidget * widget);
    virtual void  setProperty(QWidget * widget, const QVariant & value);

    void insert(const QString & classname, const QString & property);
    void remove(const QString & classname);

    static Q3SqlPropertyMap * defaultMap();
    static void installDefaultMap(Q3SqlPropertyMap * map);

private:
    Q_DISABLE_COPY(Q3SqlPropertyMap)

    Q3SqlPropertyMapPrivate* d;
};

#endif // QT_NO_SQL_FORM

#endif // Q3SQLPROPERTYMAP_H
