/****************************************************************************
**
** Definition of QEditorFactory class.
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

#ifndef QEDITORFACTORY_H
#define QEDITORFACTORY_H

#ifndef QT_H
#include "qobject.h"
#include "qvariant.h"
#endif // QT_H

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
#if defined(Q_DISABLE_COPY) // Disabled copy constructor and operator=
    QEditorFactory(const QEditorFactory &);
    QEditorFactory &operator=(const QEditorFactory &);
#endif
};

#endif // QT_NO_SQL
#endif // QEDITORFACTORY_H
