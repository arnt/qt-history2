/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTCONTEXTINFO_H
#define QSCRIPTCONTEXTINFO_H

#include <QtCore/qobjectdefs.h>

#ifndef QT_NO_SCRIPT

#include <QtCore/qstringlist.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Script)

class QScriptContext;

class QScriptContextInfoPrivate;
class Q_SCRIPT_EXPORT QScriptContextInfo
{
public:
    enum FunctionType {
        ScriptFunction,
        QtFunction,
        QtPropertyFunction,
        NativeFunction
    };

    QScriptContextInfo(QScriptContext *context);
    QScriptContextInfo(const QScriptContextInfo &other);
    QScriptContextInfo();
    ~QScriptContextInfo();

    QScriptContextInfo &operator=(const QScriptContextInfo &other);

    QString fileName() const;
    int lineNumber() const;
    int columnNumber() const;

    QString functionName() const;
    FunctionType functionType() const;

    QStringList functionParameterNames() const;

    int functionStartLineNumber() const;
    int functionEndLineNumber() const;

    int functionMetaIndex() const;

private:
    QScriptContextInfoPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptContextInfo)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QT_NO_SCRIPT

#endif
