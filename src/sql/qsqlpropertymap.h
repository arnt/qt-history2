/****************************************************************************
**
** Definition of QSqlPropertyMap class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLPROPERTYMAP_H
#define QSQLPROPERTYMAP_H

#ifndef QT_H
#include "qvariant.h"
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_SQL_FORM

class QWidget;
class QSqlPropertyMapPrivate;

class Q_SQL_EXPORT QSqlPropertyMap {
public:
    QSqlPropertyMap();
    virtual ~QSqlPropertyMap();

    QVariant      property( QWidget * widget );
    virtual void  setProperty( QWidget * widget, const QVariant & value );

    void insert( const QString & classname, const QString & property );
    void remove( const QString & classname );

    static QSqlPropertyMap * defaultMap();
    static void installDefaultMap( QSqlPropertyMap * map );

private:	// Disabled copy constructor and operator=
#if defined(Q_DISABLE_COPY)
    QSqlPropertyMap( const QSqlPropertyMap & );
    QSqlPropertyMap &operator=( const QSqlPropertyMap & );
#endif
    QSqlPropertyMapPrivate* d;

};

#endif // QT_NO_SQL_FORM
#endif // QSQLPROPERTYMAP_H
