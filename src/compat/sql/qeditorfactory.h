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

#ifndef QEDITORFACTORY_H
#define QEDITORFACTORY_H

#include "qobject.h"
#include "qvariant.h"

#ifndef QT_NO_SQL_EDIT_WIDGETS

class Q_COMPAT_EXPORT QEditorFactory : public QObject
{
public:
    QEditorFactory (QObject * parent = 0);
    ~QEditorFactory();

    virtual QWidget * createEditor(QWidget * parent, const QVariant & v);

    static QEditorFactory * defaultFactory();
    static void installDefaultFactory(QEditorFactory * factory);

private:
    Q_DISABLE_COPY(QEditorFactory)
};

#endif // QT_NO_SQL
#endif // QEDITORFACTORY_H
