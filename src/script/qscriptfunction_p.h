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

#include "qscriptobject_p.h"
#include "qscriptglobals_p.h"

#ifndef QT_NO_QOBJECT
#include <QtCore/QPointer>
#include <QtCore/QMetaMethod>
#endif

class QScriptContext;

class QScriptFunction: public QScriptObjectData
{
public:
    enum Type {
        Unknown,
        C,
        C2,
        Qt
    };

    QScriptFunction(int len = 0)
        : length(len)
        { }
    virtual ~QScriptFunction();

    virtual void execute(QScriptContext *context) = 0;
    virtual QString toString(QScriptContext *context) const;

    virtual Type type() const { return Unknown; }

public: // ### private
    int length;
    QList<QScriptNameIdImpl*> formals; // ### mark the formals
};

namespace QScript {

class CFunction: public QScriptFunction
{
public:
    CFunction(QScriptFunctionSignature funPtr, int length)
        : QScriptFunction(length), m_funPtr(funPtr)
        { }

    virtual ~CFunction() { }

    virtual void execute(QScriptContext *context);

    virtual Type type() const { return QScriptFunction::C; }

private:
    QScriptFunctionSignature m_funPtr;
};

class C2Function: public QScriptFunction
{
public:
    C2Function(QScriptInternalFunctionSignature funPtr, int length,
               QScriptClassInfo *classInfo)
        : QScriptFunction(length), m_funPtr(funPtr),
          m_classInfo(classInfo)
        { }

    virtual ~C2Function() {}

    virtual void execute(QScriptContext *context);

    virtual Type type() const { return QScriptFunction::C2; }

private:
    QScriptInternalFunctionSignature m_funPtr;
    QScriptClassInfo *m_classInfo;
};

} // namespace QScript

#endif

