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

#ifndef QSQLPROPERTYMAP_H
#define QSQLPROPERTYMAP_H

#include "qvariant.h"
#include "qstring.h"

#ifndef QT_NO_SQL_FORM

class QWidget;
class QSqlPropertyMapPrivate;

class Q_COMPAT_EXPORT QSqlPropertyMap
{
public:
    QSqlPropertyMap();
    virtual ~QSqlPropertyMap();

    QVariant      property(QWidget * widget);
    virtual void  setProperty(QWidget * widget, const QVariant & value);

    void insert(const QString & classname, const QString & property);
    void remove(const QString & classname);

    static QSqlPropertyMap * defaultMap();
    static void installDefaultMap(QSqlPropertyMap * map);

private:
    Q_DISABLE_COPY(QSqlPropertyMap)

    QSqlPropertyMapPrivate* d;
};

#endif // QT_NO_SQL_FORM
#endif // QSQLPROPERTYMAP_H
