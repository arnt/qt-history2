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

#ifndef Q3SQLEDITORFACTORY_H
#define Q3SQLEDITORFACTORY_H

#include "q3editorfactory.h"

#ifndef QT_NO_SQL_EDIT_WIDGETS

class QSqlField;

class Q_COMPAT_EXPORT Q3SqlEditorFactory : public Q3EditorFactory
{
public:
    Q3SqlEditorFactory (QObject * parent = 0);
    ~Q3SqlEditorFactory();
    virtual QWidget * createEditor(QWidget * parent, const QVariant & variant);
    virtual QWidget * createEditor(QWidget * parent, const QSqlField * field);

    static Q3SqlEditorFactory * defaultFactory();
    static void installDefaultFactory(Q3SqlEditorFactory * factory);

private:
    Q_DISABLE_COPY(Q3SqlEditorFactory)
};

#endif // QT_NO_SQL
#endif // QSQLEDITORFACTORY_H
