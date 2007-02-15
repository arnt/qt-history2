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

#ifndef QSCRIPTENGINEFWD_P_H
#define QSCRIPTENGINEFWD_P_H

#ifndef QT_NO_QOBJECT
#include "private/qobject_p.h"
#endif
#include <QtCore/qobjectdefs.h>

#include <QtCore/QHash>
#include <QtCore/QLinkedList>

#include "qscriptengine.h"
#include "qscriptrepository_p.h"
#include "qscriptmemorypool_p.h"
#include "qscriptgc_p.h"
#include "qscriptobjectfwd_p.h"
#include "qscriptclassinfo_p.h"

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

class Array;
class Lexer;
class Code;
class CompilationUnit;
class DebuggerClient;
class IdTable;

class IdTable
{
public:
    QScriptNameIdImpl *id_constructor;
    QScriptNameIdImpl *id_false;
    QScriptNameIdImpl *id_null;
    QScriptNameIdImpl *id_object;
    QScriptNameIdImpl *id_pointer;
    QScriptNameIdImpl *id_prototype;
    QScriptNameIdImpl *id_arguments;
    QScriptNameIdImpl *id_this;
    QScriptNameIdImpl *id_toString;
    QScriptNameIdImpl *id_true;
    QScriptNameIdImpl *id_undefined;
    QScriptNameIdImpl *id_valueOf;
    QScriptNameIdImpl *id_length;
    QScriptNameIdImpl *id_callee;
    QScriptNameIdImpl *id___proto__;
};

} // namespace QScript

class QScriptCustomTypeInfo
{
public:
    QScriptCustomTypeInfo() : signature(0, '\0'), marshal(0), demarshal(0)
    { prototype.invalidate(); }

    QByteArray signature;
    QScriptEngine::MarshalFunction marshal;
    QScriptEngine::DemarshalFunction demarshal;
    QScriptValueImpl prototype;
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
    inline QScriptEnginePrivate();
    ~QScriptEnginePrivate();

    void init();

    static inline QScriptEnginePrivate *get(QScriptEngine *q);

    QScript::AST::Node *createAbstractSyntaxTree(const QString &source, int &lineNumber);
    QScript::AST::Node *changeAbstractSyntaxTree(QScript::AST::Node *program);

    inline QScript::AST::Node *abstractSyntaxTree() const;
    inline QString errorMessage() const;

    inline QScriptContext *currentContext() const;
    inline QScriptContext *pushContext();
    inline void popContext();

    inline QScript::MemoryPool *nodePool();
    inline QScript::Lexer *lexer();
    inline QScriptObject *allocObject();

    inline QScript::Code *compiledCode(QScript::AST::Node *node);
    inline QScript::Code *createCompiledCode(QScript::AST::Node *node, const QScript::CompilationUnit &compilation);

    inline void maybeGC();

    void maybeGC_helper(bool do_string_gc);

    inline bool blockGC(bool block);

    void markObject(const QScriptValueImpl &object, int generation);
    void markFrame(QScriptContextPrivate *context, int generation);

    inline void markString(QScriptNameIdImpl *id, int generation);

    inline QScriptValueImpl createFunction(QScriptFunction *fun);
    inline QScriptValueImpl newArray(const QScript::Array &value);
    inline QScriptValueImpl newArray(uint length = 0);

    void evaluate(QScriptContextPrivate *context, const QString &contents, int lineNumber);

    inline QScript::Code *findCode(QScript::AST::Node *node) const;

    inline QScript::DebuggerClient *debuggerClient() const;
    inline void setDebuggerClient(QScript::DebuggerClient *client);

    inline void setLexer(QScript::Lexer *lexer);

    inline QScriptClassInfo *registerClass(const QString &pname, QScript::Type type);

    inline QScriptClassInfo *registerClass(const QString &name);

    inline QScriptValueImpl createFunction(QScriptInternalFunctionSignature fun,
                                           int length, QScriptClassInfo *classInfo);

    inline QString toString(QScriptNameIdImpl *id) const;
    inline QString memberName(const QScript::Member &member) const;
    inline void newReference(QScriptValueImpl *object, int mode);
    inline void newActivation(QScriptValueImpl *object);
    inline void newBoolean(QScriptValueImpl *object, bool b);
    inline void newNumber(QScriptValueImpl *object, qsreal d);
    inline void newFunction(QScriptValueImpl *object, QScriptFunction *function);
    inline void newConstructor(QScriptValueImpl *ctor, QScriptFunction *function,
                        QScriptValueImpl &proto);
    inline void newInteger(QScriptValueImpl *object, int i);
    inline void newNull(QScriptValueImpl *object);
    inline void newPointer(QScriptValueImpl *object, void *ptr);
    inline void newNameId(QScriptValueImpl *object, const QString &s);
    inline void newNameId(QScriptValueImpl *object, QScriptNameIdImpl *id);
    inline void newString(QScriptValueImpl *object, const QString &s);
    inline void newUndefined(QScriptValueImpl *object);
    inline void newArguments(QScriptValueImpl *object, const QScriptValueImpl &activation,
                      uint length, const QScriptValueImpl &callee);
    inline QString convertToNativeString(const QScriptValueImpl &object);
    QString convertToNativeString_helper(const QScriptValueImpl &object);
    inline qsreal convertToNativeDouble(const QScriptValueImpl &object);
    qsreal convertToNativeDouble_helper(const QScriptValueImpl &object);
    inline bool convertToNativeBoolean(const QScriptValueImpl &object);
    bool convertToNativeBoolean_helper(const QScriptValueImpl &object);
    inline qint32 convertToNativeInt32(const QScriptValueImpl &object);
    inline QScriptFunction *convertToNativeFunction(const QScriptValueImpl &object);

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

    inline QScriptValueImpl toObject(const QScriptValueImpl &value);

    inline QScriptValueImpl toPrimitive(const QScriptValueImpl &object,
                                        QScriptValue::TypeHint hint = QScriptValue::NoTypeHint);

    QScriptValueImpl toPrimitive_helper(const QScriptValueImpl &object,
                                               QScriptValue::TypeHint hint);

    static const qsreal D16;
    static const qsreal D32;

    inline static qsreal toInteger(qsreal n);
    inline static qint32 toInt32(qsreal m);
    inline static quint32 toUint32(qsreal n);
    inline static quint16 toUint16(qsreal n);

    inline QDateTime toDateTime(const QScriptValueImpl &value) const;

    inline void newArray(QScriptValueImpl *object, const QScript::Array &value);

    inline void newObject(QScriptValueImpl *o, const QScriptValueImpl &proto,
                          QScriptClassInfo *oc = 0);

    inline void newObject(QScriptValueImpl *o, QScriptClassInfo *oc = 0);

    inline QScriptValueImpl newObject();

    inline QScriptValueImpl newVariant(const QVariant &value);

#ifndef QT_NO_QOBJECT
    inline QScriptValueImpl newQObject(QObject *object);
#endif

    inline QScriptNameIdImpl *nameId(const QString &str, bool persistent = false);

    inline QScriptNameId publicNameId(const QString &str);

    inline QScriptNameIdImpl *intern(const QChar *u, int s);

    inline QScriptValueImpl valueFromVariant(const QVariant &v);

    inline QScriptValueImpl undefinedValue();

    inline QScriptValueImpl nullValue();

    inline QScriptValueImpl defaultPrototype(int metaTypeId) const;

    inline void setDefaultPrototype(int metaTypeId, const QScriptValueImpl &prototype);

    QScriptValueImpl call(const QScriptValueImpl &callee, const QScriptValueImpl &thisObject,
                          const QScriptValueImplList &args, bool asConstructor);

    void rehashStringRepository(bool resize = true);
    inline QScriptNameIdImpl *toStringEntry(const QString &s);
    QScriptNameIdImpl *insertStringEntry(const QString &s);

    QScriptValueImpl create(int type, const void *ptr);
    bool convert(const QScriptValueImpl &value, int type, void *ptr);

    QScriptValueImpl arrayFromStringList(const QStringList &lst);
    QStringList stringListFromArray(const QScriptValueImpl &arr);

    QScriptValueImpl arrayFromVariantList(const QVariantList &lst);
    QVariantList variantListFromArray(const QScriptValueImpl &arr);

    QScriptValueImpl objectFromVariantMap(const QVariantMap &vmap);
    QVariantMap variantMapFromObject(const QScriptValueImpl &obj);

    inline bool lessThan(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs) const;
    inline bool equalTo(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs) const;
    inline bool strictEqualTo(const QScriptValueImpl &lhs, const QScriptValueImpl &rhs) const;

    QScriptValuePrivate *registerValue(const QScriptValueImpl &value);

    inline void unregisterValue(QScriptValuePrivate *p);

    inline QScriptValueImpl globalObject() const;

public: // attributes
    int m_gc_depth;
    QScriptValueImpl m_globalObject;
    int m_oldStringRepositorySize;
    int m_oldTempStringRepositorySize;
    QVector<QScriptNameIdImpl*> m_stringRepository;
    QVector<QScriptNameIdImpl*> m_tempStringRepository;
    QScriptNameIdImpl **m_string_hash_base;
    int m_string_hash_size;
    QScript::GCAlloc<QScriptObject> objectAllocator;
    QScript::Repository<QScriptContext, QScriptContextPrivate> m_frameRepository;
    QScriptContext *m_context;
    QScriptValueImpl *tempStackBegin;
    QScriptValueImpl *tempStackEnd;
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

    QScript::Repository<QScriptValuePrivate, QScriptValuePrivate> m_handleRepository;
    QHash<QScriptObject*, QScriptValuePrivate*> m_objectHandles;
    QHash<QScriptNameIdImpl*, QScriptValuePrivate*> m_stringHandles;
    QVector<QScriptValuePrivate*> m_otherHandles;

    QScript::IdTable m_id_table;

#ifdef QT_NO_QOBJECT
    QScriptEngine *q_ptr;
#endif
};

#endif
