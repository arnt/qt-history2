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

#ifndef QSCRIPTENGINE_P_H
#define QSCRIPTENGINE_P_H

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

#ifndef QT_NO_QOBJECT
#include "private/qobject_p.h"
#endif
#include <QtCore/qobjectdefs.h>

#include <QtCore/QMutex>
#include <QtCore/QLinkedList>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QHash>
#include <QtCore/qnumeric.h>

#include "qscriptengine.h"
#include "qscriptobject_p.h"
#include "qscriptrepository_p.h"
#include "qscriptgc_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmaboolean_p.h"
#include "qscriptecmanumber_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptclassinfo_p.h"

#include <math.h>

class QStringList;

class QScriptContext;

namespace QScript {

namespace AST {
    class Node;
} // namespace AST

namespace Ecma {
    class Object;
    class Number;
    class Boolean;
    class String;
    class Math;
    class Date;
    class Function;
    class Array;
    class RegExp;
    class Error;
} // namespace Ecma

namespace Ext {
    class Enumeration;
    class Variant;
} // namespace Ext

class ExtQObject;

class Lexer;
class Code;
class CompilationUnit;
class DebuggerClient;

class ArgumentsObjectData: public QScriptObjectData
{
public:
    ArgumentsObjectData() {}
    virtual ~ArgumentsObjectData() {}

public: // attributes
    QScriptValue activation;
    uint length;
    QScriptValue callee;
};

class IdTable
{
public:
    QScriptNameId id_constructor;
    QScriptNameId id_false;
    QScriptNameId id_null;
    QScriptNameId id_object;
    QScriptNameId id_pointer;
    QScriptNameId id_prototype;
    QScriptNameId id_arguments;
    QScriptNameId id_this;
    QScriptNameId id_toString;
    QScriptNameId id_true;
    QScriptNameId id_undefined;
    QScriptNameId id_valueOf;
    QScriptNameId id_length;
    QScriptNameId id_callee;
    QScriptNameId id___proto__;
};

} // namespace QScript

class QScriptNameIdImpl
{
public:
    QString s;
    uint h;
    QScriptNameIdImpl *next;
    uint used: 1;
    uint persistent: 1;
    uint unique: 1;
    uint pad: 29;

    inline QScriptNameIdImpl(const QString &_s):
        s(_s), h(0), next(0), used(0), persistent(0), unique(0), pad(0) { }
};

class QScriptCustomTypeInfo
{
public:
    QScriptCustomTypeInfo() : signature(0, '\0'), marshal(0), demarshal(0)
    { prototype.invalidate(); }

    QByteArray signature;
    QScriptEngine::MarshalFunction marshal;
    QScriptEngine::DemarshalFunction demarshal;
    QScriptValue prototype;
};

class QScriptEnginePrivate
#ifndef QT_NO_QOBJECT
    : public QObjectPrivate
#endif
{
    Q_DECLARE_PUBLIC(QScriptEngine)

    enum {
        DefaultHashSize = 1021,
    };

public:
    QScriptEnginePrivate();
    ~QScriptEnginePrivate();

    void init();

    static inline QScriptEnginePrivate *get(QScriptEngine *q)
        { return q->d_func(); }

    QScript::AST::Node *createAbstractSyntaxTree(const QString &source, int lineNumber);
    QScript::AST::Node *changeAbstractSyntaxTree(QScript::AST::Node *program);

    inline QScript::AST::Node *abstractSyntaxTree() const;
    inline QString errorMessage() const;

    inline QScriptContext *context() const;
    QScriptContext *pushContext();
    void popContext();

    inline QScript::MemoryPool *nodePool();
    inline QScript::Lexer *lexer();
    inline QScriptObject *allocObject();

    QScript::Code *compiledCode(QScript::AST::Node *node);
    QScript::Code *createCompiledCode(QScript::AST::Node *node, const QScript::CompilationUnit &compilation);

    inline void maybeGC()
    {
        if (objectAllocator.blocked())
            return;

        bool do_string_gc = ((m_stringRepository.size() - m_oldStringRepositorySize) > 256);
        do_string_gc |= ((m_tempStringRepository.size() - m_oldTempStringRepositorySize) > 1024);

        if (! do_string_gc && ! objectAllocator.poll())
            return;

        maybeGC_helper(do_string_gc);
    }

    void maybeGC_helper(bool do_string_gc);

    inline bool blockGC(bool block) { return objectAllocator.blockGC(block); }

    void markObject(const QScriptValue &object, int generation);
    void markFrame(QScriptContext *context, int generation);

    inline void markString(QScriptNameIdImpl *id, int /*generation*/)
    { id->used = true; }

    QScriptValue createFunction(QScriptFunction *fun);
    QScriptValue newArray(const QScript::Array &value);

    void evaluate(QScriptContext *context, const QString &contents, int lineNumber);

    inline QScript::Code *findCode(QScript::AST::Node *node) const
    { return m_codeCache.value(node); }

    inline QScript::DebuggerClient *debuggerClient() const { return m_debuggerClient; }
    inline void setDebuggerClient(QScript::DebuggerClient *client) { m_debuggerClient = client; }

    inline void setLexer(QScript::Lexer *lexer);

    QScriptClassInfo *registerClass(const QString &pname, QScript::Type type)
    {
        if (type == -1)
            type = QScript::Type(QScript::ObjectBased | ++m_class_prev_id);

        QScriptClassInfo *oc = &*m_allocated_classes.insert(
            m_allocated_classes.end (), QScriptClassInfo());
        oc->m_engine = q_func();
        oc->m_type = type;
        oc->m_name = pname;
        oc->m_data = 0;

        m_classes[type] = oc;

        return oc;
    }

    inline QScriptClassInfo *registerClass(const QString &name)
        { return registerClass(name, QScript::Type(-1)); }

    inline QScriptValue createFunction(QScriptInternalFunctionSignature fun,
                                       int length, QScriptClassInfo *classInfo)
    {
        return createFunction(new QScript::C2Function(fun, length, classInfo));
    }

    inline QScriptValue createObject(QScriptClassInfo *classInfo)
    {
        QScriptValue v;
        newObject(&v, objectConstructor->publicPrototype, classInfo);
        return v;
    }

    inline QString toString(QScriptNameIdImpl *id) const;
    inline QString memberName(const QScript::Member &member) const;
    inline void newReference(QScriptValue *object, int mode);
    inline void newActivation(QScriptValue *object);
    inline void newBoolean(QScriptValue *object, bool b);
    inline void newNumber(QScriptValue *object, qsreal d);
    void newFunction(QScriptValue *object, QScriptFunction *function);
    void newConstructor(QScriptValue *ctor, QScriptFunction *function,
                        QScriptValue &proto);
    inline void newInteger(QScriptValue *object, int i);
    inline void newNull(QScriptValue *object);
    inline void newPointer(QScriptValue *object, void *ptr);
    inline void newNameId(QScriptValue *object, const QString &s);
    inline void newNameId(QScriptValue *object, QScriptNameIdImpl *id);
    inline void newString(QScriptValue *object, const QString &s);
    inline void newUndefined(QScriptValue *object);
    void newArguments(QScriptValue *object, const QScriptValue &activation,
                      uint length, const QScriptValue &callee);
    inline QString convertToNativeString(const QScriptValue &object);
    QString convertToNativeString_helper(const QScriptValue &object);
    inline qsreal convertToNativeDouble(const QScriptValue &object);
    qsreal convertToNativeDouble_helper(const QScriptValue &object);
    inline bool convertToNativeBoolean(const QScriptValue &object);
    bool convertToNativeBoolean_helper(const QScriptValue &object);
    inline qint32 convertToNativeInt32(const QScriptValue &object);
    QScriptFunction *convertToNativeFunction(const QScriptValue &object);

    inline static qsreal toNumber(const QString &value);
    inline static QString toString(qsreal value);
    static QString toString_helper(qsreal d);

    inline static qsreal Inf();
    inline static qsreal SNaN();
    inline static qsreal QNaN();
    inline static bool isInf(qsreal d);
    inline static bool isNaN(qsreal d);
    inline static bool isFinite(qsreal d);
    inline const QScript::IdTable *idTable() const;

    static inline QScript::Type type(const QScriptValue &v)
        { return v.m_class->type(); }
    static inline bool isValid(const QScriptValue &v)
        { return v.m_class && v.m_class->engine(); }
    static inline bool isString(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() == QScript::StringType); }
    static inline bool isObject(const QScriptValue &v)
        { return v.m_class && (v.m_class->type() & QScript::ObjectBased); }

    inline QScriptValue toObject(const QScriptValue &value)
    {
        if (!isValid(value))
            return value;

        QScriptValue result;

        switch (type(value)) {
        case QScript::UndefinedType:
        case QScript::NullType:
            break;

        case QScript::BooleanType:
            booleanConstructor->newBoolean(&result, value.m_bool_value);
            break;

        case QScript::NumberType:
            numberConstructor->newNumber(&result, value.m_number_value);
            break;

        case QScript::StringType:
            stringConstructor->newString(&result, value.m_string_value->s);
            break;

        default:
            if (isObject(value))
                result = value;
            break;
        } // switch

        return result;
    }

    inline QScriptValue toPrimitive(const QScriptValue &object,
                                    QScriptValue::TypeHint hint = QScriptValue::NoTypeHint)
    {
        Q_ASSERT(object.isValid());

        if (! isObject(object))
            return object;

        return toPrimitive_helper(object, hint);
    }

    QScriptValue toPrimitive_helper(const QScriptValue &object,
                                    QScriptValue::TypeHint hint);

    static const qsreal D16;
    static const qsreal D32;

    inline static qsreal toInteger(qsreal n);
    inline static qint32 toInt32(qsreal m);
    inline static quint32 toUint32(qsreal n);
    inline static quint16 toUint16(qsreal n);

    QDateTime toDateTime(const QScriptValue &value) const;

    inline void newArray(QScriptValue *object, const QScript::Array &value)
    { arrayConstructor->newArray(object, value); }

    inline void newObject(QScriptValue *o, const QScriptValue &proto,
                          QScriptClassInfo *oc = 0)
    {
        Q_ASSERT(o != 0);

        QScriptObject *od = allocObject();
        od->reset();

        if (isValid(proto))
            od->m_prototype = proto;
        else {
            Q_ASSERT(objectConstructor);
            od->m_prototype = objectConstructor->publicPrototype;
        }

        o->m_class = (oc ? oc : m_class_object);
        o->m_object_value = od;
    }

    inline QScriptNameIdImpl *nameId(const QString &str, bool persistent)
    {
        QScriptNameIdImpl *entry = toStringEntry(str);
        if (! entry)
            entry = insertStringEntry(str);

        Q_ASSERT(entry->unique);

        if (persistent)
            entry->persistent = true;

        return entry;
    }

    inline QScriptNameId publicNameId(const QString &str)
    {
        // ### make persistent??
        return QScriptNameId(nameId(str, /*persistent=*/false));
    }

    inline QScriptNameIdImpl *intern(const QChar *u, int s)
    {
        QString tmp(u, s);
        return nameId(tmp, /*persistent=*/ true);
    }

    inline QScriptValue valueFromVariant(const QVariant &v)
    {
        Q_Q(QScriptEngine);
        QScriptValue result = create(v.userType(), v.data());
        if (!result.isValid())
            result = q->newVariant(v);
        return result;
    }


    void rehashStringRepository(bool resize = true);
    QScriptNameIdImpl *toStringEntry(const QString &s);
    QScriptNameIdImpl *insertStringEntry(const QString &s);

    void addReference(const QScriptValue &object);
    void removeReference(const QScriptValue &object);

    QScriptValue create(int type, const void *ptr);
    bool convert(const QScriptValue &value, int type, void *ptr);

    QScriptValue arrayFromStringList(const QStringList &lst);
    QStringList stringListFromArray(const QScriptValue &arr);

    QScriptValue arrayFromVariantList(const QVariantList &lst);
    QVariantList variantListFromArray(const QScriptValue &arr);

    QScriptValue objectFromVariantMap(const QVariantMap &vmap);
    QVariantMap variantMapFromObject(const QScriptValue &obj);

    bool lessThan(const QScriptValue &lhs, const QScriptValue &rhs) const;
    bool equalTo(const QScriptValue &lhs, const QScriptValue &rhs) const;
    bool strictEqualTo(const QScriptValue &lhs, const QScriptValue &rhs) const;

public: // attributes
    int m_gc_depth;
    QScriptValue globalObject;
    int m_oldStringRepositorySize;
    int m_oldTempStringRepositorySize;
    QVector<QScriptNameIdImpl*> m_stringRepository;
    QVector<QScriptNameIdImpl*> m_tempStringRepository;
    QScriptNameIdImpl **m_string_hash_base;
    int m_string_hash_size;
    QScript::GCAlloc<QScriptObject> objectAllocator;
    QScript::Repository<QScriptContext, QScriptContextPrivate> m_frameRepository;
    QScriptContext *m_context;
    QScriptValue *tempStackBegin;
    QScriptValue *tempStackEnd;
    QString m_errorMessage;
    QScript::AST::Node *m_abstractSyntaxTree;
    QScript::Lexer *m_lexer;
    QScript::MemoryPool m_pool;

    QScript::Ecma::Object *objectConstructor;
    QScript::Ecma::Number *numberConstructor;
    QScript::Ecma::Boolean *booleanConstructor;
    QScript::Ecma::String *stringConstructor;
    QScript::Ecma::Date *dateConstructor;
    QScript::Ecma::Function *functionConstructor;
    QScript::Ecma::Array *arrayConstructor;
    QScript::Ecma::RegExp *regexpConstructor;
    QScript::Ecma::Error *errorConstructor;
    QScript::Ext::Enumeration *enumerationConstructor;
    QScript::Ext::Variant *variantConstructor;
    QScript::ExtQObject *qobjectConstructor;

    QHash<int, QScriptCustomTypeInfo> m_customTypes;

    QHash<QScript::AST::Node*, QScript::Code*> m_codeCache;
    QScriptFunction *m_evalFunction;
    QScript::DebuggerClient *m_debuggerClient;

    QLinkedList<QScriptClassInfo> m_allocated_classes;
    QScriptClassInfo *m_class_activation;
    QScriptClassInfo *m_class_boolean;
    QScriptClassInfo *m_class_double;
    QScriptClassInfo *m_class_function;
    QScriptClassInfo *m_class_int;
    QScriptClassInfo *m_class_null;
    QScriptClassInfo *m_class_object;
    QScriptClassInfo *m_class_pointer;
    QScriptClassInfo *m_class_reference;
    QScriptClassInfo *m_class_string;
    QScriptClassInfo *m_class_undefined;
    QScriptClassInfo *m_class_variant;
    QScriptClassInfo *m_class_qobject;
    QScriptClassInfo *m_class_qclass;
    QScriptClassInfo *m_class_with;
    QScriptClassInfo *m_class_arguments;

    QHash<QScript::Type, QScriptClassInfo*> m_classes;
    int m_class_prev_id;

    QList<QScriptValue> rootObjects;
    QList<int> rootObjectRefs;
    QMutex rootMutex;

    QScript::IdTable m_id_table;

#ifdef QT_NO_QOBJECT
    QScriptEngine *q_ptr;
#endif
};

inline QString QScriptEnginePrivate::toString(QScriptNameIdImpl *id) const
{
    if (! id)
        return QString();

    return id->s;
}

inline QString QScriptEnginePrivate::memberName(const QScript::Member &member) const
{
    return toString(member.nameId());
}

inline void QScriptEnginePrivate::newReference(QScriptValue *o, int mode)
{
    Q_ASSERT(o);
    o->m_class = (m_class_reference);
    o->m_int_value = (mode);
}

inline void QScriptEnginePrivate::newActivation(QScriptValue *o)
{
    Q_ASSERT(o);
    newObject(o, objectConstructor->publicPrototype, m_class_activation);
}

inline void QScriptEnginePrivate::newBoolean(QScriptValue *o, bool b)
{
    Q_ASSERT(o);
    o->m_class = (m_class_boolean);
    o->m_bool_value = (b);
}

inline void QScriptEnginePrivate::newNull(QScriptValue *o)
{
    Q_ASSERT(o);
    o->m_class = (m_class_null);
}

inline void QScriptEnginePrivate::newUndefined(QScriptValue *o)
{
    Q_ASSERT(o);
    o->m_class = (m_class_undefined);
}

inline void QScriptEnginePrivate::newPointer(QScriptValue *o, void *ptr)
{
    Q_ASSERT(o);
    o->m_class = (m_class_pointer);
    o->m_ptr_value = ptr;
}

inline void QScriptEnginePrivate::newInteger(QScriptValue *o, int i)
{
    Q_ASSERT(o);
    o->m_class = (m_class_int);
    o->m_int_value = (i);
}

inline void QScriptEnginePrivate::newNumber(QScriptValue *o, qsreal d)
{
    Q_ASSERT(o);
    o->m_class = (m_class_double);
    o->m_number_value = (d);
}

inline void QScriptEnginePrivate::newNameId(QScriptValue *o, const QString &s)
{
    Q_ASSERT(o);
    o->m_class = (m_class_string);
    o->m_string_value = (nameId(s, /*persistent=*/false));
}

inline void QScriptEnginePrivate::newString(QScriptValue *o, const QString &s)
{
    Q_ASSERT(o);
    o->m_class = (m_class_string);
    QScriptNameIdImpl *entry = new QScriptNameIdImpl(s);
    m_tempStringRepository.append(entry);
    o->m_string_value = (entry);
}

inline void QScriptEnginePrivate::newNameId(QScriptValue *o, QScriptNameIdImpl *id)
{
    Q_ASSERT(o);
    o->m_class = (m_class_string);
    o->m_string_value = (id);
}

inline const QScript::IdTable *QScriptEnginePrivate::idTable() const
{
    return &m_id_table;
}

inline QString QScriptEnginePrivate::toString(qsreal d)
{
    if (isNaN(d))
        return QLatin1String("NaN");

    else if (isInf(d))
        return QLatin1String(d < 0 ? "-Infinity" : "Infinity");

    else if (d == 0)
        return QLatin1String("0");

    return toString_helper(d);
}

inline qsreal QScriptEnginePrivate::toNumber(const QString &repr)
{
    bool converted = false;
    qsreal v;

    if (repr.length() > 2 && repr.at(0) == QLatin1Char('0') && repr.at(1).toUpper() == QLatin1Char('X'))
        v = repr.mid(2).toLongLong(&converted, 16);
    else
        v = repr.toDouble(&converted); // ### fixme

    if (converted)
        return v;

    if (repr.isEmpty())
        return 0;

    if (repr == QLatin1String("Infinity"))
        return +Inf();

    if (repr == QLatin1String("+Infinity"))
        return +Inf();

    if (repr == QLatin1String("-Infinity"))
        return -Inf();

    {
        QString trimmed = repr.trimmed();
        if (trimmed.length() < repr.length())
            return toNumber(trimmed);
    }

    return SNaN();
}

inline qsreal QScriptEnginePrivate::convertToNativeDouble(const QScriptValue &object)
{
    Q_ASSERT (object.isValid());

    if (object.isNumber())
        return object.m_number_value;

    return convertToNativeDouble_helper(object);
}

inline qint32 QScriptEnginePrivate::convertToNativeInt32(const QScriptValue &object)
{
    Q_ASSERT (object.isValid());

    return toInt32 (convertToNativeDouble(object));
}


inline bool QScriptEnginePrivate::convertToNativeBoolean(const QScriptValue &object)
{
    Q_ASSERT (object.isValid());

    if (type(object) == QScript::BooleanType)
        return object.m_bool_value;

    return convertToNativeBoolean_helper(object);
}

inline QString QScriptEnginePrivate::convertToNativeString(const QScriptValue &object)
{
    Q_ASSERT (object.isValid());

    if (isString(object))
        return object.m_string_value->s;

    return convertToNativeString_helper(object);
}

inline qsreal QScriptEnginePrivate::Inf()
{
    return qInf();
}

// Signaling NAN
inline qsreal QScriptEnginePrivate::SNaN()
{
    return qSNan();
}

// Quiet NAN
inline qsreal QScriptEnginePrivate::QNaN()
{
    return qQNan();
}

inline bool QScriptEnginePrivate::isInf(qsreal d)
{
    return qIsInf(d);
}

inline bool QScriptEnginePrivate::isNaN(qsreal d)
{
    return qIsNan(d);
}

inline bool QScriptEnginePrivate::isFinite(qsreal d)
{
    return qIsFinite(d);
}

inline qsreal QScriptEnginePrivate::toInteger(qsreal n)
{
    if (isNaN(n))
        return 0;

    if (n == 0 || isInf(n))
        return n;

    int sign = n < 0 ? -1 : 1;
    return sign * ::floor(::fabs(n));
}

inline qint32 QScriptEnginePrivate::toInt32(qsreal n)
{
    if (isNaN(n))
        return 0;

    double sign = (n < 0) ? -1.0 : 1.0;
    qsreal abs_n = fabs(n);
    if (! abs_n || isInf(abs_n))
        return 0;

    n = ::fmod(sign * ::floor(abs_n), D32);
    const double D31 = D32 / 2.0;

    if (sign == -1 && n < -D31)
        n += D32;

    else if (sign != -1 && n >= D31)
        n -= D32;

    return qint32 (n);
}

inline quint32 QScriptEnginePrivate::toUint32(qsreal n)
{
    qsreal d = qRound64(n);
    qsreal d32 = ::fmod(d, D32);

    return quint32 (d32);
}

inline quint16 QScriptEnginePrivate::toUint16(qsreal n)
{
    qsreal d = qRound64(n);
    qsreal d16 = ::fmod(d, D16);

    return quint16 (d16);
}

inline QScript::AST::Node *QScriptEnginePrivate::abstractSyntaxTree() const
{
    return m_abstractSyntaxTree;
}

inline QScript::MemoryPool *QScriptEnginePrivate::nodePool()
{
    return &m_pool;
}

inline QScript::Lexer *QScriptEnginePrivate::lexer()
{
    return m_lexer;
}

inline void QScriptEnginePrivate::setLexer(QScript::Lexer *lexer)
{
    m_lexer = lexer;
}

inline QString QScriptEnginePrivate::errorMessage() const
{
    return m_errorMessage;
}

inline QScriptObject *QScriptEnginePrivate::allocObject()
{
    return objectAllocator();
}

inline QScriptContext *QScriptEnginePrivate::context() const
{
    return m_context;
}

#endif

