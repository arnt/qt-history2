/****************************************************************************
**
** Definition of QDataView class.
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

#ifndef QDATAVIEW_H
#define QDATAVIEW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_COMPAT_EXPORT_SQL
#else
#define QM_COMPAT_EXPORT_SQL Q_COMPAT_EXPORT
#endif

#ifndef QT_NO_SQL_VIEW_WIDGETS

class QSqlForm;
class QSqlRecord;
class QDataViewPrivate;

class QM_COMPAT_EXPORT_SQL QDataView : public QWidget
{
    Q_OBJECT

public:
    QDataView( QWidget* parent=0, const char* name=0, WFlags fl = 0 );
    ~QDataView();

    virtual void setForm( QSqlForm* form );
    QSqlForm* form();
    virtual void setRecord( QSqlRecord* record );
    QSqlRecord* record();

public slots:
    virtual void refresh( QSqlRecord* buf );
    virtual void readFields();
    virtual void writeFields();
    virtual void clearValues();

private:
    QDataViewPrivate* d;

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDataView( const QDataView & );
    QDataView &operator=( const QDataView & );
#endif
};


#endif
#endif
