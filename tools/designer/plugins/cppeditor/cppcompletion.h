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

#ifndef CPPCOMPLETION_H
#define CPPCOMPLETION_H

#include <completion.h>
#include <qguardedptr.h>

class CppEditorCompletion : public EditorCompletion
{
    Q_OBJECT

public:
    CppEditorCompletion( Editor *e );

#if defined(Q_USING)
    using EditorCompletion::doObjectCompletion;
#endif
    bool doObjectCompletion( const QString &object );
    QList<QStringList> functionParameters( const QString &func, QChar &, QString &prefix, QString &postfix );
    void setContext( QObject *this_ );

private:
    QGuardedPtr<QObject> ths;

};

#endif
