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

#ifndef QDATAVIEW_H
#define QDATAVIEW_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SQL_VIEW_WIDGETS

class QSqlForm;
class QSqlRecord;
class QDataViewPrivate;

class Q_COMPAT_EXPORT QDataView : public QWidget
{
    Q_OBJECT

public:
    QDataView(QWidget* parent=0, const char* name=0, Qt::WFlags fl = 0);
    ~QDataView();

    virtual void setForm(QSqlForm* form);
    QSqlForm* form();
    virtual void setRecord(QSqlRecord* record);
    QSqlRecord* record();

public slots:
    virtual void refresh(QSqlRecord* buf);
    virtual void readFields();
    virtual void writeFields();
    virtual void clearValues();

private:
    QDataViewPrivate* d;

#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QDataView(const QDataView &);
    QDataView &operator=(const QDataView &);
#endif
};


#endif
#endif
