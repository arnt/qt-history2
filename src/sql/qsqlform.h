/****************************************************************************
**
** Definition of QSqlForm class
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#ifndef QSQLFORM_H
#define QSQLFORM_H

#ifndef QT_H
#include "qwidget.h"
#include "qmap.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlField;
class QSqlRecord;
class QSqlCursor;
class QSqlEditorFactory;

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
template class Q_EXPORT QMap<QString,QString>;
template class Q_EXPORT QMap<QWidget*, QSqlField*>;
// MOC_SKIP_END
#endif

class Q_EXPORT QSqlPropertyMap {
public:
    QSqlPropertyMap();
    virtual ~QSqlPropertyMap();

    QVariant      property( QWidget * widget );
    virtual void  setProperty( QWidget * widget, const QVariant & value );

    void insert( const QString & classname, const QString & property );
    void remove( const QString & classname );

    static QSqlPropertyMap * defaultMap();
    static void installDefaultMap( QSqlPropertyMap * map );

private:
    QMap< QString, QString > propertyMap;
};

class Q_EXPORT QSqlFormMap
{
public:
    QSqlFormMap();
    virtual ~QSqlFormMap();

    virtual void insert( QWidget * widget, QSqlField * field );
    virtual void remove( QWidget * widget );
    virtual void clear();
    virtual void clearValues();
    uint         count() const;

    QWidget *   widget( uint i ) const;
    QSqlField * widgetToField( QWidget * widget ) const;
    QWidget *   fieldToWidget( QSqlField * field ) const;

    void        readFields();
    void        writeFields();

    void        installPropertyMap( QSqlPropertyMap * map );

private:
    QMap< QWidget *, QSqlField * > map;
    QSqlPropertyMap * m;
};

class Q_EXPORT QSqlForm : public QObject
{
    Q_OBJECT
public:
    QSqlForm( QObject * parent = 0, const char * name = 0 );
    QSqlForm( QWidget * widget, QSqlRecord * fields, uint columns = 1,
	      QObject * parent = 0, const char * name = 0 );
    ~QSqlForm();

    virtual void associate( QWidget * widget, QSqlField * field );
    virtual void populate( QWidget * widget, QSqlRecord * fields, 
			   uint columns = 1 );

    void setReadOnly( bool enable );
    bool isReadOnly() const;

    void installEditorFactory( QSqlEditorFactory * factory );
    void installPropertyMap( QSqlPropertyMap * map );

public slots:
    virtual void readFields();
    virtual void writeFields();
    virtual void clear();
    virtual void clearValues();

private:
    bool readOnly;
    QSqlFormMap map;
    QSqlEditorFactory * factory;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
