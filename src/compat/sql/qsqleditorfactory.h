/****************************************************************************
**
** Definition of QSqlEditorFactory class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLEDITORFACTORY_H
#define QSQLEDITORFACTORY_H

#ifndef QT_H
#include "qeditorfactory.h"
#endif // QT_H

#ifndef QT_NO_SQL_EDIT_WIDGETS

class QSqlField;

class Q_COMPAT_EXPORT QSqlEditorFactory : public QEditorFactory
{
public:
    QSqlEditorFactory (QObject * parent = 0);
    ~QSqlEditorFactory();
    virtual QWidget * createEditor(QWidget * parent, const QVariant & variant);
    virtual QWidget * createEditor(QWidget * parent, const QSqlField * field);

    static QSqlEditorFactory * defaultFactory();
    static void installDefaultFactory(QSqlEditorFactory * factory);

private:
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QSqlEditorFactory(const QSqlEditorFactory &);
    QSqlEditorFactory &operator=(const QSqlEditorFactory &);
#endif
};

#endif // QT_NO_SQL
#endif // QSQLEDITORFACTORY_H
