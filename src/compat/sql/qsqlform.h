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

#ifndef QSQLFORM_H
#define QSQLFORM_H

#include "qobject.h"
#include "qmap.h"

#ifndef QT_NO_SQL_FORM

class QSqlField;
class QSqlRecord;
class QSqlEditorFactory;
class QSqlPropertyMap;
class QWidget;
class QSqlFormPrivate;

class Q_COMPAT_EXPORT QSqlForm : public QObject
{
    Q_OBJECT
public:
    QSqlForm(QObject * parent = 0);
    ~QSqlForm();

    virtual void insert(QWidget * widget, const QString& field);
    virtual void remove(const QString& field);
    int         count() const;

    QWidget *   widget(int i) const;
    QSqlField * widgetToField(QWidget * widget) const;
    QWidget *   fieldToWidget(QSqlField * field) const;

    void        installPropertyMap(QSqlPropertyMap * map);

    virtual void setRecord(QSqlRecord* buf);

public slots:
    virtual void readField(QWidget * widget);
    virtual void writeField(QWidget * widget);
    virtual void readFields();
    virtual void writeFields();

    virtual void clear();
    virtual void clearValues();

protected:
    virtual void insert(QWidget * widget, QSqlField * field);
    virtual void remove(QWidget * widget);
    void clearMap();

private:
    Q_DISABLE_COPY(QSqlForm)

    virtual void sync();
    QSqlFormPrivate* d;
};

#endif // QT_NO_SQL
#endif // QSQLFORM_H
