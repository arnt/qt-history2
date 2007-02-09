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

#include <QtCore/QtDebug>

#include "qscriptcontext.h"
#include "qscriptasm_p.h"
#include "qscriptvalue_p.h"
#include "qscriptcontext_p.h"
#include "qscriptengine_p.h"
#include "qscriptast_p.h"
#include "qscriptengine_p.h"
#include "qscriptcompiler_p.h"
#include "qscriptprettypretty_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmanumber_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptecmaboolean_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptextenumeration_p.h"

#include <math.h> // floor & friends...

#define Q_SCRIPT_NO_PRINT_GENERATED_CODE

#define Q_SCRIPT_NO_JOINED_FUNCTION

#define CHECK_TEMPSTACK(needed) do { \
    if (stackPtr + needed >= eng->tempStackEnd) { \
        q->throwError(QLatin1String("out of memory")); \
        HandleException(); \
    } \
} while (0)

#ifndef Q_SCRIPT_NO_PRINT_GENERATED_CODE
static QTextStream qout(stderr, QIODevice::WriteOnly);
#endif

static inline void qscript_uint_to_string_helper(uint i, QString &s)
{
    switch (i) {
    case 0: case 1: case 2: case 3: case 4:
    case 5: case 6: case 7: case 8: case 9:
        s += QLatin1Char('0' + i);
        break;

    default:
        qscript_uint_to_string_helper(i / 10, s);
        s += QLatin1Char('0' + (i % 10));
    }
}

static inline void qscript_uint_to_string(qsreal i, QString &s)
{
    if (i < 0)
        return; // nothing to do

    qsreal x = ::fmod(i, 10);

    if (x != 0.0 && x != 1.0
            && x != 2.0 && x != 3.0
            && x != 4.0 && x != 5.0
            && x != 6.0 && x != 7.0
            && x != 8.0 && x != 9.0)
        return; // nothing to do

    qscript_uint_to_string_helper(uint(i), s);
}

#define BEGIN_PREFIX_OPERATOR \
    QScriptValue::ResolveFlags mode; \
    mode = static_cast<QScriptValue::ResolveFlags> (stackPtr[0].m_int_value) \
    | QScriptValue::ResolvePrototype; \
    --stackPtr; \
    const QScriptValue &object = stackPtr[-1]; \
    QScriptNameIdImpl *memberName = 0; \
    if (isString(stackPtr[0]) && stackPtr[0].m_string_value->unique) \
        memberName = stackPtr[0].m_string_value; \
    else \
        memberName = eng->nameId(stackPtr[0].toString(), /*persistent=*/false); \
    QScript::Member member; \
    QScriptValue base; \
    QScriptValue value; \
    QScriptValue getter; \
    QScriptValue setter; \
    const bool isMemberAssignment = (object.m_object_value != scopeChain.m_object_value); \
    if (object.impl()->resolve(memberName, &member, &base, mode)) { \
        base.impl()->get(member, &value); \
        if (member.isGetterOrSetter()) { \
            if (member.isGetter()) { \
                getter = value; \
                if (!member.isSetter() && !base.m_object_value->findSetter(&member)) { \
                    stackPtr -= 2; \
                    q->throwError(QLatin1String("No setter defined")); \
                    HandleException(); \
                } \
                base.impl()->get(member, &setter); \
            } else { \
                setter = value; \
                QScript::Member tmp = member; \
                if (!base.m_object_value->findGetter(&member)) { \
                    stackPtr -= 2; \
                    q->throwError(QLatin1String("No getter defined")); \
                    HandleException(); \
                } \
                base.impl()->get(member, &getter); \
                member = tmp; \
            } \
            value = getter.call(object); \
            if (engine()->hasUncaughtException()) \
                Done(); \
        } \
    } else if (!isMemberAssignment) { \
        stackPtr -= 2; \
        throwNotDefined(memberName); \
        HandleException(); \
    } else { \
        base = object; \
        base.impl()->createMember(memberName, &member, /*flags=*/0); \
        eng->newUndefined(&value); \
    }

#define END_PREFIX_OPERATOR \
    if (member.isSetter()) { \
        setter.call(object, QScriptValueList() << value); \
        if (engine()->hasUncaughtException()) { \
            stackPtr -= 2; \
            Done(); \
        } \
    } else { \
        if (isMemberAssignment && (base.m_object_value != object.m_object_value)) { \
            base = object; \
            base.impl()->createMember(memberName, &member, /*flags=*/0); \
        } \
        if (member.isWritable()) \
            base.impl()->put(member, value); \
    } \
    *--stackPtr = value; \
    ++iPtr;

#define BEGIN_INPLACE_OPERATOR \
    if (! stackPtr[-1].impl()->isReference()) { \
        stackPtr -= 2; \
        throwSyntaxError(QLatin1String("invalid assignment lvalue")); \
        HandleException(); \
    } \
    QScriptValue::ResolveFlags mode; \
    mode = static_cast<QScriptValue::ResolveFlags> (stackPtr[-1].m_int_value) \
           | QScriptValue::ResolvePrototype; \
    QScriptValue object = eng->toObject(stackPtr[-3]); \
    if (! isValid(object)) { \
        stackPtr -= 4; \
        throwTypeError(QLatin1String("not an object")); \
        HandleException(); \
    } \
    QScriptNameIdImpl *memberName = 0; \
    if (isString(stackPtr[-2]) && stackPtr[-2].m_string_value->unique) \
        memberName = stackPtr[-2].m_string_value; \
    else \
        memberName = eng->nameId(stackPtr[-2].toString(), /*persistent=*/false); \
    QScriptValue lhs; \
    QScriptValue base; \
    QScript::Member member; \
    QScriptValue getter; \
    QScriptValue setter; \
    const bool isMemberAssignment = (object.m_object_value != scopeChain.m_object_value); \
    if (object.impl()->resolve(memberName, &member, &base, mode)) { \
        base.impl()->get(member, &lhs); \
        if (member.isGetterOrSetter()) { \
            if (member.isGetter()) { \
                getter = lhs; \
                if (!member.isSetter() && !base.m_object_value->findSetter(&member)) { \
                    stackPtr -= 4; \
                    q->throwError(QLatin1String("No setter defined")); \
                    HandleException(); \
                } \
                base.impl()->get(member, &setter); \
            } else { \
                setter = lhs; \
                QScript::Member tmp = member; \
                if (!base.m_object_value->findGetter(&member)) { \
                    stackPtr -= 4; \
                    q->throwError(QLatin1String("No getter defined")); \
                    HandleException(); \
                } \
                base.impl()->get(member, &getter); \
                member = tmp; \
            } \
            lhs = getter.call(object); \
            if (engine()->hasUncaughtException()) \
                Done(); \
        } \
    } else if (!isMemberAssignment) { \
        stackPtr -= 4; \
        throwNotDefined(memberName); \
        HandleException(); \
    } else { \
        base = object; \
        base.impl()->createMember(memberName, &member, /*flags=*/0); \
        eng->newUndefined(&lhs); \
    } \
    const QScriptValue &rhs = stackPtr[0];

#define END_INPLACE_OPERATOR \
    if (member.isSetter()) { \
        setter.call(object, QScriptValueList() << *stackPtr); \
        if (engine()->hasUncaughtException()) \
            Done(); \
    } else { \
        if (isMemberAssignment && (base.m_object_value != object.m_object_value)) { \
            base = object; \
            base.impl()->createMember(memberName, &member, /*flags=*/0); \
        } \
        if (member.isWritable()) \
            base.impl()->put(member, *stackPtr); \
    } \
    ++iPtr;

namespace QScript {

class ScriptFunction: public QScriptFunction // ### rename
{
public:
    ScriptFunction(QScript::AST::Node *functionBody):
        m_functionBody(functionBody), m_compiledCode(0) {}

    virtual ~ScriptFunction() {}

    virtual void execute(QScriptContext *context);

    virtual QString toString(QScriptContext *context) const {
        QScriptEngine *eng = context->engine();
        QString str;
        QTextStream out(&str, QIODevice::WriteOnly);
        out << QLatin1String("function (");
        for (int i = 0; i < formals.count(); ++i) {
            if (i != 0)
                out << QLatin1String(", ");
            out << QScriptEnginePrivate::get(eng)->toString(formals.at(i));
        }
        out << QLatin1String(")") << endl << QLatin1String("{");
        QScript::PrettyPretty pp(eng, out);
        pp(m_functionBody, /*indent=*/ 1);
        out << endl << QLatin1String("}") << endl;
        return str;
    }

private:
    QScript::AST::Node *m_functionBody;
    QScript::Code *m_compiledCode;
};

void ScriptFunction::execute(QScriptContext *context)
{
    if (! m_compiledCode) {
        QScriptEngine *eng = context->engine();
        QScript::Compiler compiler(eng);

        QScript::CompilationUnit unit = compiler.compile(m_functionBody, formals);
        if (! unit.isValid()) {
            context->throwError(unit.errorMessage());
            return;
        }

        m_compiledCode = QScriptEnginePrivate::get(eng)->createCompiledCode(m_functionBody, unit);
        m_compiledCode->value = context->callee();
        QScriptContextPrivate::get(context)->abstractSyntaxTree = m_functionBody;
    }

    QScriptContextPrivate::get(context)->execute(m_compiledCode);
}

} // namespace QScript

bool QScriptContextPrivate::resolveField(QScriptValue *stackPtr)
{
    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(engine());

    const QScriptValue &m = stackPtr[0];
    QScriptValue &object = stackPtr[-1];

    if (! isObject(object))
        object = eng->toObject(object);

    if (! isValid(object))
        return false;

    QScriptValue undefined;
    eng->newUndefined(&undefined);

    if (QScript::Ecma::Array::Instance *arrayInstance = eng->arrayConstructor->get(object)) {
        qint32 pos = -1;

        if (isNumber(m))
            pos = eng->convertToNativeInt32(m);

        else if (isString(m)) {
            QByteArray bytes = m.m_string_value->s.toUtf8();
            char *eptr;
            pos = strtoul(bytes.constData(), &eptr, 10);
            if (eptr != bytes.constData() + bytes.size())
                pos = -1;
        }

        if (pos != -1) {
            *stackPtr = arrayInstance->value.at(pos);

            if (! stackPtr->isValid())
                eng->newUndefined(&stackPtr[0]);

            return true;
        }
    }

    QScriptNameIdImpl *nameId = isString(m) ? m.m_string_value : 0;

    if (! nameId || ! nameId->unique)
        nameId = eng->nameId(eng->convertToNativeString(m), /*persistent=*/false); // ### slow!

    QScript::Member member;
    QScriptValue base;

    if (! object.impl()->resolve(nameId, &member, &base, QScriptValue::ResolveFull)) // ### ...
        return false;

    if (base.m_class == eng->globalObject.m_class)
        stackPtr[-1] = base;
    else if (object.m_class == eng->m_class_with)
        stackPtr[-1] = object.prototype();

    base.impl()->get(member, stackPtr);

    return true;
}

void QScriptContextPrivate::execute(QScript::Code *code)
{
    Q_Q(QScriptContext);
#ifndef Q_SCRIPT_NO_PRINT_GENERATED_CODE
    qout << QLatin1String("function:") << endl;
    for (QScriptInstruction *current = code->firstInstruction; current != code->lastInstruction; ++current) {
        qout << int(current - code->firstInstruction) << QLatin1String(":\t");
        current->print(qout);
        qout << endl;
    }
    qout << endl;
#endif

    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(engine());

    // set up the temp stack
    if (! tempStack)
        stackPtr = tempStack = eng->tempStackBegin;

    QScriptValue undefined;
    eng->newUndefined(&undefined);

    catching = false;
    result = undefined;
    firstInstruction = code->firstInstruction;
    lastInstruction = code->lastInstruction;
    iPtr = code->firstInstruction; // ### kill iPtr

    scopeChain = activation;

#ifdef Q_SCRIPT_DEBUGGER_CLIENT
    QScript::DebuggerClient *dbg = eng->debuggerClient();
    if (dbg)
        dbg->frameEntry(q);
#endif


#ifndef Q_SCRIPT_DIRECT_CODE

#  define I(opc) case QScriptInstruction::OP_##opc
#  define Next() goto Lfetch
#  define Done() goto Ldone
#  define HandleException() goto Lhandle_exception

Lfetch:


#else

#  define I(opc) qscript_execute_##opc
#  define Next() goto *iPtr->code
#  define Done() goto Ldone
#  define HandleException() goto Lhandle_exception

    static void *jump_table [] = {

#  define Q_SCRIPT_DEFINE_OPERATOR(op) &&I(op),
#  include "instruction.table"
#  undef Q_SCRIPT_DEFINE_OPERATOR
    }; // jump_table


    if (!code->optimized) {
        for (QScriptInstruction *current = code->firstInstruction; current != code->lastInstruction; ++current) {
            current->code = jump_table[current->op];
        }

        code->optimized = true;
    }

#endif

Ltop:
#ifndef Q_SCRIPT_DIRECT_CODE
    switch (iPtr->op) {
#else
    goto *iPtr->code;
#endif

    I(LoadUndefined):
    {
        CHECK_TEMPSTACK(1);
        eng->newUndefined(++stackPtr);
        ++iPtr;
    }   Next();

    I(LoadTrue):
    {
        CHECK_TEMPSTACK(1);
        eng->newBoolean(++stackPtr, true);
        ++iPtr;
    }   Next();

    I(LoadFalse):
    {
        CHECK_TEMPSTACK(1);
        eng->newBoolean(++stackPtr, false);
        ++iPtr;
    }   Next();

    I(LoadThis):
    {
        CHECK_TEMPSTACK(1);
        Q_ASSERT(isObject(thisObject));
        *++stackPtr = thisObject;
        ++iPtr;
    }   Next();

    I(LoadNull):
    {
        CHECK_TEMPSTACK(1);
        eng->newNull(++stackPtr);
        ++iPtr;
    }   Next();

    I(LoadNumber):
    {
        CHECK_TEMPSTACK(1);
        *++stackPtr = iPtr->operand[0];
        ++iPtr;
    }   Next();


    I(LoadString):
    {
        CHECK_TEMPSTACK(1);
        *++stackPtr = iPtr->operand[0];
        ++iPtr;
    }   Next();

    I(NewString):
    {
        CHECK_TEMPSTACK(1);
        eng->newNameId(++stackPtr, iPtr->operand[0].m_string_value);
        ++iPtr;
    }   Next();

    I(Duplicate):
    {
        CHECK_TEMPSTACK(1);
        ++stackPtr;
        *stackPtr = stackPtr[-1];
        ++iPtr;
    }   Next();


    I(Receive):
    {
        int n = iPtr->operand[0].m_int_value;

        if (n >= argc) {
            q->throwError(QLatin1String("invalid argument"));
            HandleException();
        }

        CHECK_TEMPSTACK(1);
        *++stackPtr = argument(n);
        ++iPtr;
    }   Next();

    I(Fetch):
    {
        CHECK_TEMPSTACK(1);

        QScriptNameIdImpl *memberName = iPtr->operand[0].m_string_value;

        QScriptValue base;
        QScript::Member member;

        QScriptObject *instance = scopeChain.m_object_value;
        if (instance->findMember(memberName, &member)) {
            instance->get(member, ++stackPtr);
        } else {
            if (scopeChain.impl()->resolve_helper(memberName, &member, &base, QScriptValue::ResolveFull)) {
                base.impl()->get(member, ++stackPtr);
                if (member.isGetterOrSetter()) {
                    // call the getter function
                    QScriptValue getter;
                    if (member.isGetter()) {
                        getter = *stackPtr;
                    } else {
                        if (!base.m_object_value->findGetter(&member)) {
                            q->throwError(QLatin1String("No getter defined"));
                            HandleException();
                        }
                        base.impl()->get(member, &getter);
                    }
                    if (scopeChain.m_class == eng->m_class_with)
                        *stackPtr = getter.call(scopeChain.prototype());
                    else
                        *stackPtr = getter.call(scopeChain);
                    if (engine()->hasUncaughtException())
                        Done();
                }
            } else {
                throwNotDefined(memberName);
                HandleException();
            }
        }
        ++iPtr;
    }   Next();

    I(Resolve):
    {
        Q_ASSERT(isString(iPtr->operand[0]));

        CHECK_TEMPSTACK(2);
        *++stackPtr = scopeChain;
        *++stackPtr = iPtr->operand[0];
        eng->newReference(++stackPtr, QScriptValue::ResolveScope);
        ++iPtr;
    }   Next();

    I(PutField):
    {
        Q_ASSERT(stackPtr[-1].impl()->isReference());

        const QScriptValue &object = stackPtr[-3];
        QScriptNameIdImpl *memberName = stackPtr[-2].m_string_value;
        const QScriptValue &value = stackPtr[0];

        QScript::Member member;
        QScriptValue base;

        if (! object.impl()->resolve(memberName, &member, &base, QScriptValue::ResolveLocal)) {
            base = object;
            base.impl()->createMember(memberName, &member, /*flags=*/0);
        }

        base.impl()->put(member, value);
        stackPtr -= 4;
        ++iPtr;
    }   Next();

    I(Call):
    {
        int argc = iPtr->operand[0].m_int_value;
        QScriptValue *argp = stackPtr - argc;

        QScriptValue base;
        QScriptValue callee;

        bool isReference = argp[0].impl()->isReference();

        if (! isReference) { // we have a value
            base = eng->globalObject;
            callee = argp[0];
        } else if (resolveField(&argp[-1])) {
            base = argp[-2];
            callee = argp[-1];
        } else {
            stackPtr = argp - 1;
            if (isReference)
                stackPtr -= 2;

            throwNotDefined(QLatin1String("function")); // ### name
            HandleException();
        }

        Q_ASSERT(isValid(base));
        Q_ASSERT(isValid(callee));

        QScriptFunction *function = eng->convertToNativeFunction(callee);
        if (! function) {
            throwTypeError(QLatin1String("not a function")); // ### name
            HandleException();
        }

        QScriptContext *nested = eng->pushContext();
        QScriptContextPrivate *nested_data = nested->d_func();
        nested_data->thisObject = base;
        nested_data->callee = callee;
        nested_data->functionNameId = 0; //member.nameId(); ### fixme

        // create the activation
        eng->newActivation(&nested_data->activation);
        QScriptObject *activation_data = nested_data->activation.m_object_value;

        int formalCount = function->formals.count();
        int mx = qMax(formalCount, argc);
        activation_data->m_members.resize(mx);
        activation_data->m_objects.resize(mx);
        for (int i = 0; i < mx; ++i) {
            QScriptNameIdImpl *nameId = 0;
            if (i < formalCount)
                nameId = function->formals.at(i);

            activation_data->m_members[i].object(nameId, i,
                                                 QScriptValue::Undeletable
                                                 | QScriptValue::SkipInEnumeration);
            activation_data->m_objects[i] = (i < argc) ? argp[i + 1] : undefined;
        }

        nested_data->argc = argc;
        if (callee.m_object_value->m_scope.isValid())
            activation_data->m_scope = callee.m_object_value->m_scope;
        else
            activation_data->m_scope = eng->globalObject;
        nested_data->tempStack = stackPtr;
        nested_data->args = &argp[1];

        function->execute(nested);

        stackPtr = argp - 1;
        if (isReference)
            stackPtr -= 2;

        CHECK_TEMPSTACK(1);
        *++stackPtr = nested_data->result;

        eng->popContext();

        if (nested_data->state == QScriptContext::ExceptionState)
            Done();

        ++iPtr;
    }   Next();


    I(NewArray):
    {
        CHECK_TEMPSTACK(1);
        eng->arrayConstructor->newArray(++stackPtr);
        ++iPtr;
    }   Next();

    I(NewRegExp):
    {
        CHECK_TEMPSTACK(1);

        QString pattern = eng->toString(iPtr->operand[0].m_string_value);
        QString flags;

        if (isValid(iPtr->operand[1]))
            flags = eng->toString(iPtr->operand[1].m_string_value);

        eng->regexpConstructor->newRegExp(++stackPtr, pattern, flags);
        ++iPtr;
    }   Next();

    I(NewObject):
    {
        CHECK_TEMPSTACK(1);
        eng->objectConstructor->newObject(++stackPtr);
        ++iPtr;
    }   Next();

    I(New):
    {
        int argc = iPtr->operand[0].m_int_value;
        QScriptValue *argp = stackPtr - argc;

        // QScriptValue base;
        QScriptValue callee;

        bool isReference = argp[0].impl()->isReference();

        if (! isReference) { // we have a value
            // base = eng->globalObject;
            callee = argp[0];
        } else if (resolveField(&argp[-1])) {
            // base = argp[-2];
            callee = argp[-1];
        } else {
            stackPtr = argp - 1;
            if (isReference)
                stackPtr -= 2;

            throwTypeError(QLatin1String("not a constructor"));
            HandleException();
        }

        // Q_ASSERT(isValid(base));
        Q_ASSERT(isValid(callee));

        QScriptFunction *function = eng->convertToNativeFunction(callee);
        if (! function) {
            throwTypeError(QLatin1String("not a constructor"));
            HandleException();
        }

        QScriptContext *nested = eng->pushContext();
        QScriptContextPrivate *nested_data = nested->d_func();
        nested_data->callee = callee;
        nested_data->functionNameId = 0; //member.nameId(); ### FIXME
        nested_data->calledAsConstructor = true;

        // create the activation
        eng->newActivation(&nested_data->activation);
        QScriptObject *activation_data = nested_data->activation.m_object_value;

        int formalCount = function->formals.count();
        int mx = qMax(formalCount, argc);
        activation_data->m_members.resize(mx);
        activation_data->m_objects.resize(mx);
        for (int i = 0; i < mx; ++i) {
            QScriptNameIdImpl *nameId = 0;
            if (i < formalCount)
                nameId = function->formals.at(i);

            activation_data->m_members[i].object(nameId, i,
                                                 QScriptValue::Undeletable
                                                 | QScriptValue::SkipInEnumeration);
            activation_data->m_objects[i] = (i < argc) ? argp[i + 1] : undefined;
        }

        eng->objectConstructor->newObject(&nested_data->thisObject);
        nested_data->argc = argc;
        if (callee.m_object_value->m_scope.isValid())
            activation_data->m_scope = callee.m_object_value->m_scope;
        else
            activation_data->m_scope = eng->globalObject;
        nested_data->tempStack = stackPtr;
        eng->newUndefined(&nested_data->result);

        QScriptObject *instance = nested_data->thisObject.m_object_value;

        // set [[prototype]]
        QScriptValue dummy;
        QScript::Member proto;
        if (callee.impl()->resolve(eng->idTable()->id_prototype, &proto, &dummy, QScriptValue::ResolveLocal))
            callee.impl()->get(proto, &instance->m_prototype);
        if (!isObject(instance->m_prototype))
            instance->m_prototype = eng->objectConstructor->publicPrototype;

        function->execute(nested);

        stackPtr = argp - 1;
        if (isReference)
            stackPtr -= 2;

        if (! isValid(nested_data->result))
            eng->newUndefined(&nested_data->result);
        else if (! isObject(nested_data->result))
            nested_data->result = nested_data->thisObject;

        if (nested_data->state == QScriptContext::ExceptionState) {
            eng->popContext();
            Done();
        }

        CHECK_TEMPSTACK(1);

        *++stackPtr = nested_data->result;

        eng->popContext();

        ++iPtr;
    }   Next();

    I(FetchField):
    {
        QScriptValue object = eng->toObject(stackPtr[-1]);
        if (! isValid(object)) {
            stackPtr -= 2;
            throwTypeError(QLatin1String("not an object"));
            HandleException();
        }

        const QScriptValue &m = stackPtr[0];

        QScript::Ecma::Array::Instance *arrayInstance = 0;
        if (object.m_class == eng->arrayConstructor->classInfo())
            arrayInstance = static_cast<QScript::Ecma::Array::Instance *> (object.m_object_value->m_data.data());

        if (arrayInstance) {
            qint32 pos = -1;
            bool ok = isNumber(m);

            if (ok)
                pos = QScriptEnginePrivate::toUint32(eng->convertToNativeDouble(m));
            else if (isString(m)) {
                QByteArray bytes = m.m_string_value->s.toUtf8();
                char *eptr;
                pos = strtoul(bytes.constData(), &eptr, 10);
                ok = (eptr == bytes.constData() + bytes.size());
            }

            if (ok) {
                *--stackPtr = arrayInstance->value.at(pos);

                if (! stackPtr->isValid())
                    eng->newUndefined(&stackPtr[0]);

                ++iPtr;
                Next();
            }
        }

        QScriptNameIdImpl *nameId = isString(m) ? m.m_string_value : 0;

        if (! nameId || ! nameId->unique) {
            QString str;

            if (isNumber(m))
                qscript_uint_to_string(m.m_number_value, str);

            if (str.isEmpty())
                str = eng->convertToNativeString(m);

            nameId = eng->nameId(str, /*persistent=*/false);
        }

        QScript::Member member;
        QScriptValue base;

        if (object.impl()->resolve(nameId, &member, &base, QScriptValue::ResolvePrototype)) {
            base.impl()->get(member, --stackPtr);
            if (member.isGetterOrSetter()) {
                // call the getter function
                QScriptValue getter;
                if (member.isGetter()) {
                    getter = *stackPtr;
                } else {
                    if (!base.m_object_value->findGetter(&member)) {
                        q->throwError(QLatin1String("No getter defined"));
                        HandleException();
                    }
                    base.impl()->get(member, &getter);
                }
                *stackPtr = getter.call(object);
                if (engine()->hasUncaughtException())
                    Done();
            }
        } else {
            eng->newUndefined(--stackPtr);
        }

        ++iPtr;
    }   Next();

    I(FetchArguments):
    {
        CHECK_TEMPSTACK(1);
        if (activation.impl()->objectValue() == thisObject.impl()->objectValue())
            eng->newUndefined(++stackPtr); // ### arguments array parsed from command line
        else
            eng->newArguments(++stackPtr, activation, argc, callee);
        ++iPtr;
    }   Next();

    I(DeclareLocal):
    {
        QScriptValueImpl *act = activation.impl();

        QScriptNameIdImpl *memberName = iPtr->operand[0].m_string_value;
        bool readOnly = iPtr->operand[1].m_int_value != 0;
        QScript::Member member;
        QScriptValue object;

        if (! act->resolve(memberName, &member, &object, QScriptValue::ResolveLocal)) {
            QScriptValue::PropertyFlags flags = QScriptValue::Undeletable;
            if (readOnly)
                flags |= QScriptValue::UninitializedConst | QScriptValue::ReadOnly;
            act->createMember(memberName, &member, flags);
            act->put(member, undefined);
        }
        ++iPtr;
    }   Next();

    I(Assign):
    {
        if (! stackPtr[-1].impl()->isReference()) {
            stackPtr -= 2;
            throwSyntaxError(QLatin1String("invalid assignment lvalue"));
            HandleException();
        }

        QScriptValue::ResolveFlags mode;
        mode = static_cast<QScriptValue::ResolveFlags> (stackPtr[-1].m_int_value)
               | QScriptValue::ResolvePrototype;

        QScriptValue object = eng->toObject(stackPtr[-3]);
        if (! isValid(object)) {
            stackPtr -= 4;
            throwTypeError(QLatin1String("invalid assignment lvalue"));
            HandleException();
        }

        const QScriptValue &m = stackPtr[-2];
        QScriptValue &value = stackPtr[0];

        qint32 pos = -1;

        QScript::Ecma::Array::Instance *arrayInstance = eng->arrayConstructor->get(object);
        if (arrayInstance && isNumerical(m)) {
            pos = eng->convertToNativeInt32(m);
        }

        stackPtr -= 3;

        if (pos >= 0)
            arrayInstance->value.assign(pos, value);

        else {
            QScriptNameIdImpl *memberName;

            if (isString(m) && m.m_string_value->unique)
                memberName = m.m_string_value;
            else
                memberName = eng->nameId(eng->convertToNativeString(m), /*persistent=*/false);

            QScriptValue base;
            QScript::Member member;

            const bool isMemberAssignment = (object.m_object_value != scopeChain.m_object_value);
            if (! object.impl()->resolve(memberName, &member, &base, mode)) {
                if (isMemberAssignment)
                    base = object;
                else
                    base = eng->globalObject;

                base.impl()->createMember(memberName, &member, /*flags=*/0);
            }

            if (isString(value) && ! value.m_string_value->unique)
                eng->newNameId(&value, value.m_string_value->s);
            if (object.m_class == eng->m_class_with)
                object = object.prototype();
            if (member.isGetterOrSetter()) {
                // find and call setter(value)
                QScriptValue setter;
                if (!member.isSetter()) {
                    if (!base.m_object_value->findSetter(&member)) {
                        q->throwError(QLatin1String("no setter defined"));
                        HandleException();
                    }
                }
                base.impl()->get(member, &setter);
                value = setter.call(object, QScriptValueList() << value);
                if (engine()->hasUncaughtException())
                    Done();
            } else {
                if (isMemberAssignment && (base.m_object_value != object.m_object_value)) {
                    base = object;
                    base.impl()->createMember(memberName, &member, /*flags=*/0);
                }

                if (member.isWritable())
                    base.impl()->put(member, value);

                else if (member.isUninitializedConst()) {
                    base.impl()->put(member, value);
                    if (member.isObjectProperty()) {
                        base.m_object_value->m_members[member.id()]
                            .unsetFlags(QScriptValue::UninitializedConst);
                    }
                }
            }
        }

        *stackPtr = value;
        ++iPtr;
    }   Next();

    I(BitAnd):
    {
        qint32 v1 = eng->convertToNativeInt32(stackPtr[-1]);
        qint32 v2 = eng->convertToNativeInt32(stackPtr[0]);
        eng->newNumber(--stackPtr, v1 & v2);
        ++iPtr;
    }   Next();

    I(BitOr):
    {
        qint32 v1 = eng->convertToNativeInt32(stackPtr[-1]);
        qint32 v2 = eng->convertToNativeInt32(stackPtr[0]);
        eng->newNumber(--stackPtr, v1 | v2);
        ++iPtr;
    }   Next();

    I(BitXor):
    {
        qint32 v1 = eng->convertToNativeInt32(stackPtr[-1]);
        qint32 v2 = eng->convertToNativeInt32(stackPtr[0]);
        eng->newNumber(--stackPtr, v1 ^ v2);
        ++iPtr;
    }   Next();

    I(BitNot):
    {
        qint32 v1 = eng->convertToNativeInt32(stackPtr[0]);
        eng->newNumber(stackPtr, ~v1);
        ++iPtr;
    }   Next();

    I(Not):
    {
        bool v1 = eng->convertToNativeBoolean(stackPtr[0]);
        eng->newBoolean(stackPtr, !v1);
        ++iPtr;
    }   Next();

    I(LeftShift):
    {
        qint32 v1 = eng->convertToNativeInt32(stackPtr[-1]);
        qint32 v2 = eng->convertToNativeInt32(stackPtr[0]) & 0x1f;
        eng->newNumber(--stackPtr, v1 << v2);
        ++iPtr;
    } Next();

    I(Mod):
    {
        qsreal v1 = eng->convertToNativeDouble(stackPtr[-1]);
        qsreal v2 = eng->convertToNativeDouble(stackPtr[0]);

        eng->newNumber(--stackPtr, ::fmod(v1, v2));
        ++iPtr;
    }   Next();

    I(RightShift):
    {
        qint32 v1 = eng->convertToNativeInt32(stackPtr[-1]);
        quint32 v2 = QScriptEnginePrivate::toUint32 (eng->convertToNativeDouble(stackPtr[0])) & 0x1f;
        eng->newNumber(--stackPtr, v1 >> v2);
        ++iPtr;
    }   Next();

    I(URightShift):
    {
        quint32 v1 = QScriptEnginePrivate::toUint32 (eng->convertToNativeDouble(stackPtr[-1]));
        qint32 v2 = eng->convertToNativeInt32(stackPtr[0]) & 0x1f;
        eng->newNumber(--stackPtr, v1 >> v2);
        ++iPtr;
    }   Next();

    I(InstanceOf):
    {
        QScriptValue object = stackPtr[-1];
        QScriptValue ctor = stackPtr[0];
        bool result = false;

        if (!isObject(ctor)) {
            stackPtr -= 2;
            throwTypeError(QLatin1String("invalid 'instanceof' operand"));
            HandleException();
        }

        // ### fixme, this is not according to spec
        // only Function implements [[hasInstance]]
        if (isObject(object)) {
            QScriptValue prototype = ctor.property(eng->idTable()->id_prototype);
            if (!(isValid(prototype) && isObject(prototype))) {
                stackPtr -= 2;
                throwTypeError(QLatin1String("instanceof: 'prototype' property is not an object"));
                HandleException();
            }
            result = object.instanceOf(prototype);
        }

        eng->newBoolean(--stackPtr, result);
        ++iPtr;
    }   Next();

    I(In):
    {
        QScriptValue object = stackPtr[0];
        if (!isObject(object)) {
            stackPtr -= 2;
            throwTypeError(QLatin1String("invalid 'in' operand"));
            HandleException();
        }
        QString propertyName = eng->convertToNativeString(stackPtr[-1]);
        bool result = object.property(propertyName, QScriptValue::ResolvePrototype).isValid(); // ### hasProperty()
        eng->newBoolean(--stackPtr, result);
        ++iPtr;
    }   Next();

    I(Add):
    {
        QScriptValue lhs = eng->toPrimitive(stackPtr[-1], QScriptValue::NoTypeHint);
        QScriptValue rhs = eng->toPrimitive(stackPtr[0], QScriptValue::NoTypeHint);

        if (isString(lhs) || isString(rhs)) {
            QString tmp = eng->convertToNativeString(lhs);
            tmp += eng->convertToNativeString(rhs);
            eng->newNameId(--stackPtr, tmp);
        } else {
            qsreal tmp = eng->convertToNativeDouble(lhs);
            tmp += eng->convertToNativeDouble(rhs);
            eng->newNumber(--stackPtr, tmp);
        }

        ++iPtr;
    }   Next();

    I(Div):
    {
        qsreal v1 = eng->convertToNativeDouble(stackPtr[-1]);
        qsreal v2 = eng->convertToNativeDouble(stackPtr[0]);
        eng->newNumber(--stackPtr, v1 / v2);
        ++iPtr;
    }   Next();

    I(Equal):
    {
        bool v = eq_cmp(stackPtr[-1], stackPtr[0]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(GreatOrEqual):
    {
        bool v = le_cmp(stackPtr[0], stackPtr[-1]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(GreatThan):
    {
        bool v = lt_cmp(stackPtr[0], stackPtr[-1]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(LessOrEqual):
    {
        bool v = le_cmp(stackPtr[-1], stackPtr[0]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(LessThan):
    {
        bool v = lt_cmp(stackPtr[-1], stackPtr[0]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(NotEqual):
    {
        bool v = ! eq_cmp(stackPtr[-1], stackPtr[0]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(Mul):
    {
        qsreal v1 = eng->convertToNativeDouble(stackPtr[-1]);
        qsreal v2 = eng->convertToNativeDouble(stackPtr[0]);
        eng->newNumber(--stackPtr, v1 * v2);
        ++iPtr;
    }   Next();

    I(StrictEqual):
    {
        bool v = strict_eq_cmp(stackPtr[-1], stackPtr[0]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(StrictNotEqual):
    {
        bool v = ! strict_eq_cmp(stackPtr[-1], stackPtr[0]);
        eng->newBoolean(--stackPtr, v);
        ++iPtr;
    }   Next();

    I(Sub):
    {
        qsreal v1 = eng->convertToNativeDouble(stackPtr[-1]);
        qsreal v2 = eng->convertToNativeDouble(stackPtr[0]);
        eng->newNumber(--stackPtr, v1 - v2);
        ++iPtr;
    }   Next();

    I(UnaryMinus):
    {
        qsreal v1 = eng->convertToNativeDouble(*stackPtr);
        eng->newNumber(stackPtr, -v1);
        ++iPtr;
    }   Next();

    I(UnaryPlus):
    {
        qsreal v1 = eng->convertToNativeDouble(*stackPtr);
        eng->newNumber(stackPtr, +v1);
        ++iPtr;
    }   Next();

    I(Branch):
    {
        iPtr += iPtr->operand[0].m_int_value;
    }   Next();

    I(BranchFalse):
    {
        if (! eng->convertToNativeBoolean(*stackPtr--))
            iPtr += iPtr->operand[0].m_int_value;
        else
            ++iPtr;
    }   Next();

    I(BranchTrue):
    {
        if (eng->convertToNativeBoolean(*stackPtr--))
            iPtr += iPtr->operand[0].m_int_value;
        else
            ++iPtr;
    }   Next();

    I(NewClosure):
    {
        CHECK_TEMPSTACK(1);

        QScript::AST::FormalParameterList *formals = static_cast<QScript::AST::FormalParameterList *> (iPtr->operand[0].m_ptr_value);
        QScript::AST::Node *functionBody = static_cast<QScript::AST::Node*> (iPtr->operand[1].m_ptr_value);

#ifndef Q_SCRIPT_NO_JOINED_FUNCTION
        if (QScript::Code *code = eng->findCode(functionBody)) {
            QScriptValue value = code->value;

            if (isValid(value)) {
                QScriptObject *instance = value.m_object_value;
                Q_ASSERT(instance != 0);

                if (instance->m_scope.m_object_value == scopeChain.m_object_value)
                {
                    *++stackPtr = value;
                    ++iPtr;
                    Next();
                }
            }
        }
#endif

        QScript::ScriptFunction *function = new QScript::ScriptFunction(functionBody); // ### add the AST

        // update the formals
        for (QScript::AST::FormalParameterList *it = formals; it != 0; it = it->next) {
            function->formals.append(it->name);
        }
        function->length = function->formals.count();

        eng->functionConstructor->newFunction(++stackPtr, function);

        QScriptObject *instance = stackPtr->m_object_value;
        // initialize [[scope]]
        instance->m_scope = scopeChain;

        // create and initialize `prototype'
        QScriptValue proto;
        eng->objectConstructor->newObject(&proto);

        QScript::Member member;
        proto.impl()->createMember(eng->idTable()->id_constructor, &member,
                                   QScriptValue::Undeletable
                                   | QScriptValue::ReadOnly
                                   | QScriptValue::SkipInEnumeration);
        proto.impl()->put(member, *stackPtr);

        stackPtr->impl()->createMember(eng->idTable()->id_prototype, &member,
                                       QScriptValue::Undeletable);
        stackPtr->impl()->put(member, proto);

        ++iPtr;
    }   Next();

    I(Incr):
    {
        if (! stackPtr[0].impl()->isReference()) {
            stackPtr -= 1;
            throwSyntaxError(QLatin1String("invalid increment operand"));
            HandleException();
        }

        BEGIN_PREFIX_OPERATOR

        qsreal x = eng->convertToNativeDouble(value);
        eng->newNumber(&value, x + 1);

        END_PREFIX_OPERATOR
    }   Next();

    I(Decr):
    {
        if (! stackPtr[0].impl()->isReference()) {
            stackPtr -= 1;
            throwSyntaxError(QLatin1String("invalid decrement operand"));
            HandleException();
        }

        BEGIN_PREFIX_OPERATOR

        qsreal x = eng->convertToNativeDouble(value);
        eng->newNumber(&value, x - 1);

        END_PREFIX_OPERATOR
    }   Next();

    I(PostIncr):
    {
        if (! stackPtr[0].impl()->isReference()) {
            stackPtr -= 1;
            throwSyntaxError(QLatin1String("invalid increment operand"));
            HandleException();
        }

        QScriptValue::ResolveFlags mode;
        mode = static_cast<QScriptValue::ResolveFlags> (stackPtr[0].m_int_value)
               | QScriptValue::ResolvePrototype;

        --stackPtr;

        const QScriptValue &object = stackPtr[-1];
        QScriptNameIdImpl *memberName = 0;
        if (isString(stackPtr[0]) && stackPtr[0].m_string_value->unique)
            memberName = stackPtr[0].m_string_value;
        else
            memberName = eng->nameId(stackPtr[0].toString(), /*persistent=*/false);

        QScript::Member member;
        QScriptValue base;
        QScriptValue value;
        QScriptObject *instance = object.m_object_value;
        const bool isMemberAssignment = (instance != scopeChain.m_object_value);
        if (instance->findMember(memberName, &member)) {
            if (!member.isGetterOrSetter()) {
                QScriptValue &r = instance->reference(member);
                if (isNumber(r)) {
                    eng->newNumber(--stackPtr, r.m_number_value);
                    r.impl()->incr();
                    ++iPtr;
                    Next();
                }
            }
            base = object;
        } else if (!object.impl()->resolve_helper(memberName, &member, &base, mode)) {
            if (!isMemberAssignment) {
                stackPtr -= 2;
                throwNotDefined(memberName);
                HandleException();
            }
            base = object;
            base.impl()->createMember(memberName, &member, /*flags=*/0);
            base.impl()->put(member, undefined);
        }

        QScriptValue getter;
        QScriptValue setter;
        base.impl()->get(member, &value);
        if (member.isGetterOrSetter()) {
            if (member.isGetter()) {
                getter = value;
                if (!member.isSetter() && !base.m_object_value->findSetter(&member)) {
                    stackPtr -= 2;
                    q->throwError(QLatin1String("No setter defined"));
                    HandleException();
                }
                base.impl()->get(member, &setter);
            } else {
                setter = value;
                QScript::Member tmp = member;
                if (!base.m_object_value->findGetter(&member)) {
                    stackPtr -= 2;
                    q->throwError(QLatin1String("No getter defined"));
                    HandleException();
                }
                base.impl()->get(member, &getter);
                member = tmp;
            }
            value = getter.call(object);
            if (engine()->hasUncaughtException()) {
                stackPtr -= 2;
                Done();
            }
        }

        qsreal x = eng->convertToNativeDouble(value);

        eng->newNumber(&value, x + 1);

        if (member.isSetter()) {
            setter.call(object, QScriptValueList() << value);
            if (engine()->hasUncaughtException()) {
                stackPtr -= 2;
                Done();
            }
        } else {
            if (isMemberAssignment && (base.m_object_value != object.m_object_value)) {
                base = object;
                base.impl()->createMember(memberName, &member, /*flags=*/0);
            }
            if (member.isWritable())
                base.impl()->put(member, value);
        }

        eng->newNumber(--stackPtr, x);

        ++iPtr;
    }   Next();

    I(PostDecr):
    {
        if (! stackPtr[0].impl()->isReference()) {
            stackPtr -= 1;
            throwSyntaxError(QLatin1String("invalid decrement operand"));
            HandleException();
        }

        QScriptValue::ResolveFlags mode = static_cast<QScriptValue::ResolveFlags> (stackPtr[0].m_int_value);

        --stackPtr;

        const QScriptValue &object = stackPtr[-1];
        QScriptNameIdImpl *memberName = 0;
        if (isString(stackPtr[0]) && stackPtr[0].m_string_value->unique)
            memberName = stackPtr[0].m_string_value;
        else
            memberName = eng->nameId(stackPtr[0].toString(), /*persistent=*/false);

        QScript::Member member;
        QScriptValue base;
        QScriptValue value;
        QScriptObject *instance = object.m_object_value;
        const bool isMemberAssignment = (instance != scopeChain.m_object_value);
        if (instance->findMember(memberName, &member)) {
            if (!member.isGetterOrSetter()) {
                QScriptValue &r = instance->reference(member);
                if (isNumber(r)) {
                    eng->newNumber(--stackPtr, r.m_number_value);
                    r.impl()->decr();
                    ++iPtr;
                    Next();
                }
            }
            base = object;
        } else {
            if (! object.impl()->resolve_helper(memberName, &member, &base, mode)) {
                if (!isMemberAssignment) {
                    stackPtr -= 2;
                    throwNotDefined(memberName);
                    HandleException();
                }
                base = object;
                base.impl()->createMember(memberName, &member, /*flags=*/0);
                base.impl()->put(member, undefined);
            }
        }

        QScriptValue getter;
        QScriptValue setter;
        base.impl()->get(member, &value);
        if (member.isGetterOrSetter()) {
            if (member.isGetter()) {
                getter = value;
                if (!member.isSetter() && !base.m_object_value->findSetter(&member)) {
                    stackPtr -= 2;
                    q->throwError(QLatin1String("No setter defined"));
                    HandleException();
                }
                base.impl()->get(member, &setter);
            } else {
                setter = value;
                QScript::Member tmp = member;
                if (!base.m_object_value->findGetter(&member)) {
                    stackPtr -= 2;
                    q->throwError(QLatin1String("No getter defined"));
                    HandleException();
                }
                base.impl()->get(member, &getter);
                member = tmp;
            }
            value = getter.call(object);
            if (engine()->hasUncaughtException()) {
                stackPtr -= 2;
                Done();
            }
        }

        qsreal x = eng->convertToNativeDouble(value);

        eng->newNumber(&value, x - 1);

        if (member.isSetter()) {
            setter.call(object, QScriptValueList() << value);
            if (engine()->hasUncaughtException()) {
                stackPtr -= 2;
                Done();
            }
        } else {
            if (isMemberAssignment && (base.m_object_value != object.m_object_value)) {
                base = object;
                base.impl()->createMember(memberName, &member, /*flags=*/0);
            }
            if (member.isWritable())
                base.impl()->put(member, value);
        }

        eng->newNumber(--stackPtr, x);

        ++iPtr;
    }   Next();

    I(InplaceAdd):
    {
        BEGIN_INPLACE_OPERATOR

        if (isString(lhs) || isString(rhs)) {
            if (isString(lhs) && !lhs.m_string_value->unique) {
                lhs.m_string_value->s += eng->convertToNativeString(rhs);
                stackPtr -= 3;
                *stackPtr = lhs;
            } else {
                QString tmp = eng->convertToNativeString(lhs);
                tmp += eng->convertToNativeString(rhs);
                stackPtr -= 3;
                eng->newString(stackPtr, tmp);
            }
        } else {
            qsreal tmp = eng->convertToNativeDouble(lhs);
            tmp += eng->convertToNativeDouble(rhs);
            stackPtr -= 3;
            eng->newNumber(stackPtr, tmp);
        }

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceSub):
    {
        BEGIN_INPLACE_OPERATOR

        qsreal v1 = eng->convertToNativeDouble(lhs);
        qsreal v2 = eng->convertToNativeDouble(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 - v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceAnd):
    {
        BEGIN_INPLACE_OPERATOR

        qint32 v1 = eng->convertToNativeInt32(lhs);
        qint32 v2 = eng->convertToNativeInt32(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 & v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceDiv):
    {
        BEGIN_INPLACE_OPERATOR

        qsreal v1 = eng->convertToNativeDouble(lhs);
        qsreal v2 = eng->convertToNativeDouble(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 / v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceLeftShift):
    {
        BEGIN_INPLACE_OPERATOR

        qint32 v1 = eng->convertToNativeInt32(lhs);
        qint32 v2 = eng->convertToNativeInt32(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 << v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceMod):
    {
        BEGIN_INPLACE_OPERATOR

        qsreal v1 = eng->convertToNativeDouble(lhs);
        qsreal v2 = eng->convertToNativeDouble(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, fmod (v1, v2));

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceMul):
    {
        BEGIN_INPLACE_OPERATOR

        qsreal v1 = eng->convertToNativeDouble(lhs);
        qsreal v2 = eng->convertToNativeDouble(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 * v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceOr):
    {
        BEGIN_INPLACE_OPERATOR

        qint32 v1 = eng->convertToNativeInt32(lhs);
        qint32 v2 = eng->convertToNativeInt32(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 | v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceRightShift):
    {
        BEGIN_INPLACE_OPERATOR

        qint32 v1 = eng->convertToNativeInt32(lhs);
        qint32 v2 = eng->convertToNativeInt32(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 >> v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceURightShift):
    {
        BEGIN_INPLACE_OPERATOR

        quint32 v1 = QScriptEnginePrivate::toUint32 (eng->convertToNativeDouble(lhs));
        qint32 v2 = eng->convertToNativeInt32(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 >> v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(InplaceXor):
    {
        BEGIN_INPLACE_OPERATOR

        qint32 v1 = eng->convertToNativeInt32(lhs);
        qint32 v2 = eng->convertToNativeInt32(rhs);

        stackPtr -= 3;
        eng->newNumber(stackPtr, v1 ^ v2);

        END_INPLACE_OPERATOR
    }   Next();

    I(MakeReference):
    {
        CHECK_TEMPSTACK(1);
        eng->newReference(++stackPtr, QScriptValue::ResolveLocal);
        ++iPtr;
    }   Next();

    I(TypeOf):
    {
        QScriptValue object;

        bool isReference = stackPtr[0].impl()->isReference();

        if (! isReference) { // we have a value
            object = stackPtr[0];
        } else if (resolveField(&stackPtr[-1])) {
            object = stackPtr[-1];
            stackPtr -= 2;
        } else {
            object = undefined;
            stackPtr -= 2;
        }

        QString typeName;

        switch (type(object)) {

        case QScript::UndefinedType:
            typeName = QLatin1String("undefined");
            break;

        case QScript::NullType:
            typeName = QLatin1String("object");
            break;

        case QScript::ActivationType: // ### checkme
            typeName = QLatin1String("global");
            break;

        case QScript::BooleanType:
            typeName = QLatin1String("boolean");
            break;

        case QScript::IntegerType:
        case QScript::NumberType:
            typeName = QLatin1String("number");
            break;

        case QScript::StringType:
            typeName = QLatin1String("string");
            break;

        case QScript::FunctionType:
            typeName = QLatin1String("function");
            break;

        case QScript::VariantType:
            typeName = QLatin1String("variant");
            break;

        default:
            if (!stackPtr->isObject())
                typeName = QLatin1String("undefined");
            else
                typeName = QLatin1String("object");
            break;
        }

        eng->newString(stackPtr, typeName);
        ++iPtr;
    }   Next();

    I(Line):
    {
        eng->maybeGC();
        currentLine = iPtr->operand[0].m_int_value;
        currentColumn = iPtr->operand[1].m_int_value;

#ifdef Q_SCRIPT_DEBUGGER_CLIENT
        if (dbg)
            dbg->lineChange(engine(), currentLine, currentColumn);
#endif

        ++iPtr;
    }   Next();

    I(Delete):
    {
        bool result;
        if (! stackPtr[0].impl()->isReference())
            result = true;

        else {
            QScriptValue object = stackPtr[-2];
            if (!isObject(object))
                object = eng->toObject(object);

            QScriptNameIdImpl *nameId = 0;
            if (isString(stackPtr[-1]))
                nameId = stackPtr[-1].m_string_value;
            else
                nameId = eng->nameId(eng->convertToNativeString(stackPtr[-1]),
                                     /*persistent=*/false);

            QScriptValue base;
            QScript::Member member;

            if (object.impl()->resolve(nameId, &member, &base, QScriptValue::ResolveScope)) {
                result = member.isDeletable();
                if (result)
                    base.impl()->removeMember(member);
            } else {
                result = true; // doesn't have the property ==> return true
            }
            stackPtr -= 2;
        }

        eng->newBoolean(stackPtr, result);

        ++iPtr;
    }   Next();


    I(NewEnumeration): {
        QScriptValue e;
        QScriptValue object = eng->toObject(stackPtr[0]);
        if (! isValid(object)) {
            stackPtr -= 1;
            throwTypeError(QLatin1String("QScript.VM.NewEnumeration"));
            HandleException();
        }
        eng->enumerationConstructor->newEnumeration(&e, object);
        *stackPtr = e;
        ++iPtr;
    }   Next();


    I(ToFirstElement): {
        QScript::Ext::Enumeration::Instance *e = eng->enumerationConstructor->get(stackPtr[0]);
        Q_ASSERT(e != 0);
        e->toFirst();
        --stackPtr;
        ++iPtr;
    }   Next();


    I(HasNextElement): {
        QScript::Ext::Enumeration::Instance *e = eng->enumerationConstructor->get(stackPtr[0]);
        Q_ASSERT(e != 0);
        e->hasNext(q, stackPtr);
        ++iPtr;
    }   Next();


    I(NextElement): {
        // the Enumeration should be located below the result of I(Resolve)
        if (! stackPtr[0].impl()->isReference()) {
            throwTypeError(QLatin1String("QScript.VM.NextElement"));
            HandleException();
        }

        QScript::Ext::Enumeration::Instance *e = eng->enumerationConstructor->get(stackPtr[-3]);
        if (! e) {
            throwTypeError(QLatin1String("QScript.VM.NextElement"));
            HandleException();
        }
        e->next(q, ++stackPtr);
        ++iPtr;
    }   Next();


    I(Pop):
    {
        --stackPtr;
        ++iPtr;
    }   Next();

    I(Sync):
    {
        result = *stackPtr;
        --stackPtr;
        ++iPtr;
    }   Next();

    I(Throw):
    {
        Q_ASSERT(stackPtr->isValid());
        result = *stackPtr--;
        state = QScriptContext::ExceptionState;
    }   HandleException();

    I(Ret):
    {
//        if (eng->context()->activationObject().m_object_value == eng->globalObject.m_object_value) {
//            throwSyntaxError("return not in function");
//            HandleException();
//        }
        Q_ASSERT(stackPtr->isValid());
        result = *stackPtr--;
        state = QScriptContext::NormalState;

        ++iPtr;
    }   Done();

    I(Halt):
    {
        state = QScriptContext::NormalState;

        ++iPtr;
    }   Done();

    I(EnterWith):
    {
        QScriptValue object = eng->toObject(*stackPtr--);
        if (! isValid(object)) {
            throwTypeError(QLatin1String("value has no properties"));
            HandleException();
        }
        QScriptValue withObject;
        eng->newObject(&withObject, object, eng->m_class_with);
        withObject.m_object_value->m_scope = scopeChain;
        scopeChain = withObject;
        ++iPtr;
    }   Next();

    I(LeaveWith):
    {
        QScriptValue withObject = scopeChain;
        scopeChain = withObject.m_object_value->m_scope;
        ++iPtr;
    }   Next();

    I(BeginCatch):
    {
        // result contains the thrown object
        QScriptValue object;
        eng->newObject(&object, undefined); // ### prototype
        QScript::Member member;
        object.impl()->createMember(iPtr->operand[0].m_string_value, &member, /*flags=*/0);
        object.impl()->put(member, result);
        // make catch-object head of scopechain
        object.m_object_value->m_scope = scopeChain;
        scopeChain = object;

        catching = true;
        ++iPtr;
    }   Next();

    I(EndCatch):
    {
        // remove catch-object from scopechain
        QScriptValue object = scopeChain;
        scopeChain = object.m_object_value->m_scope;

        catching = false;
        ++iPtr;
    }   Next();

#ifndef Q_SCRIPT_DIRECT_CODE
    } // end switch
#endif

Lhandle_exception:
    errorLineNumber = currentLine;

Ldone:
    Q_ASSERT(isValid(result));

    if (state == QScriptContext::ExceptionState) {
        if (catching) {
            // exception thrown in catch -- clean up scopechain
            QScriptValue object = scopeChain;
            scopeChain = object.m_object_value->m_scope;
            catching = false;
        }
        // see if we have an exception handler in this context
        int offset = iPtr - code->firstInstruction;
        for (int i = 0; i < code->exceptionHandlers.count(); ++i) {
            QScript::ExceptionHandlerDescriptor e = code->exceptionHandlers.at(i);
            if (offset >= e.startInstruction() && offset <= e.endInstruction()) {
                // go to the handler
                iPtr = code->firstInstruction + e.handlerInstruction();
                recover();
                goto Ltop;
            }
        }
    }

#ifdef Q_SCRIPT_DEBUGGER_CLIENT
    if (dbg)
        dbg->frameExit(q);
#endif
}

QScriptValue QScriptContextPrivate::throwNotImplemented(const QString &name)
{
    return throwTypeError(QString::fromUtf8("%1 is not implemented").arg(name));
}

QScriptValue QScriptContextPrivate::throwNotDefined(const QString &name)
{
    Q_Q(QScriptContext);
    return q->throwError(QScriptContext::ReferenceError,
                         QString::fromUtf8("%1 is not defined").arg(name));
}

QScriptValue QScriptContextPrivate::throwNotDefined(QScriptNameIdImpl *nameId)
{
    return throwNotDefined(QScriptEnginePrivate::get(engine())->toString(nameId));
}
