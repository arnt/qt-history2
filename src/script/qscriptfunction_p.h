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

#ifndef QSCRIPTFUNCTION_P_H
#define QSCRIPTFUNCTION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qscriptobjectdata_p.h"

#ifndef QT_NO_SCRIPT

#include "qscriptglobals_p.h"
#include "qscriptcontext_p.h"
#include "qscriptnodepool_p.h"

#include <QtCore/QList>
#ifndef QT_NO_QOBJECT
#include <QtCore/QPointer>
#include <QtCore/QMetaMethod>
#endif

class QScriptContext;
class QScriptNameIdImpl;

class QScriptFunction: public QScriptObjectData
{
public:
    enum Type {
        Unknown,
        Script,
        C,
        C2,
        Qt,
        QtProperty
    };

    QScriptFunction(int len = 0)
        : length(len)
        { }
    virtual ~QScriptFunction();

    virtual void execute(QScriptContextPrivate *context) = 0;
    virtual QString toString(QScriptContextPrivate *context) const;

    virtual Type type() const { return Unknown; }

    // name of the file the function is defined in
    virtual QString fileName() const;

    virtual QString functionName() const;

public: // ### private
    int length;
    QList<QScriptNameIdImpl*> formals; // ### mark the formals
};

namespace QScript {

// public API function
class CFunction: public QScriptFunction
{
public:
    CFunction(QScriptFunctionSignature funPtr, int length)
        : QScriptFunction(length), m_funPtr(funPtr)
        { }

    virtual ~CFunction() { }

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::C; }

    virtual QString functionName() const;

private:
    QScriptFunctionSignature m_funPtr;
};

// internal API function
class C2Function: public QScriptFunction
{
public:
    C2Function(QScriptInternalFunctionSignature funPtr, int length,
               QScriptClassInfo *classInfo)
        : QScriptFunction(length), m_funPtr(funPtr),
          m_classInfo(classInfo)
        { }

    virtual ~C2Function() {}

    virtual void execute(QScriptContextPrivate *context);

    virtual Type type() const { return QScriptFunction::C2; }

    virtual QString functionName() const;

private:
    QScriptInternalFunctionSignature m_funPtr;
    QScriptClassInfo *m_classInfo;
};

namespace AST {
    class FunctionExpression;
}

// implemented in qscriptcontext_p.cpp
class ScriptFunction: public QScriptFunction
{
public:
    ScriptFunction(AST::FunctionExpression *definition, NodePool *astPool):
        m_definition(definition), m_astPool(astPool), m_compiledCode(0) {}

    virtual ~ScriptFunction() {}

    virtual void execute(QScriptContextPrivate *context);

    virtual QString toString(QScriptContextPrivate *context) const;

    virtual Type type() const
    { return QScriptFunction::Script; }

    virtual QString fileName() const;

    virtual QString functionName() const;

private:
    AST::FunctionExpression *m_definition;
    QExplicitlySharedDataPointer<NodePool> m_astPool;
    Code *m_compiledCode;
};

} // namespace QScript

#endif // QT_NO_SCRIPT
#endif

