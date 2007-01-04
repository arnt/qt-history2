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

#include "qscriptengine_p.h"
#include "qscriptlexer_p.h"
#include "qscriptasm_p.h"
#include "qscriptcompiler_p.h"
#include "qscriptparser_p.h"
#include "qscriptobject_p.h"
#include "qscriptcontext_p.h"
#include "qscriptvalue_p.h"

#include "qscriptecmaglobal_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmanumber_p.h"
#include "qscriptecmaboolean_p.h"
#include "qscriptecmamath_p.h"
#include "qscriptecmadate_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptecmaerror_p.h"
#include "qscriptextenumeration_p.h"
#include "qscriptextvariant_p.h"
#include "qscriptextqobject_p.h"

#include <QtCore/QStringList>

Q_DECLARE_METATYPE(QScriptValue)

namespace QScript {

class EvalFunction : public QScriptFunction
{
public:
    EvalFunction(QScriptEngine *eng):
        m_compiler(eng), m_driver(QScriptEnginePrivate::get(eng))
    {
        m_compiler.setTopLevelCompiler(true);
        length = 1;
    }

    virtual ~EvalFunction() {}

    void evaluate(QScriptContext *context, QString contents)
    {
        if (! contents.endsWith(QLatin1Char('\n')))
            contents += QLatin1Char('\n'); // ### kill me

        QScript::AST::Node *program = m_driver->lookupAST(contents);

        if (! program) {
            program = m_driver->createAbstractSyntaxTree(contents);
            m_driver->enterAST(contents, program);

            if (! program) {
                context->throwError(QScriptContext::SyntaxError,
                                    m_driver->errorMessage());
                return;
            }
        }

        Q_ASSERT(program != 0);

        QScript:: CompilationUnit compilation = m_compiler.compile(program);
        if (! compilation.isValid()) {
            context->throwError(compilation.errorMessage());
            return;
        }

        QScript::Code *code = m_driver->createCompiledCode(program, compilation);

        if (QScriptContext *parentContext = context->parentContext()) {
            context->setActivationObject(parentContext->activationObject());
            context->setThisObject(parentContext->thisObject());
        } else {
            context->setActivationObject(context->engine()->globalObject());
            context->setThisObject(context->engine()->globalObject());
        }

        const QScriptInstruction *iPtr = context->instructionPointer();
        QScriptContextPrivate::get(context)->execute(code);
        context->setInstructionPointer(iPtr);
    }

    virtual void execute(QScriptContext *context)
    {
        QScriptEngine *eng = context->engine();

        if (context->argumentCount() == 0) {
            context->setReturnValue(eng->undefinedScriptValue());
        } else {
            QScriptValue arg = context->argument(0);
            if (arg.isString()) {
                QString contents = arg.toString();
                evaluate(context, contents);
            } else {
                context->setReturnValue(arg);
            }
        }
    }

private:
    QScript::Compiler m_compiler;
    QScriptEnginePrivate *m_driver;
};

class WithClassData: public QScriptClassData
{

public:
    virtual bool resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValue *base);
};

bool WithClassData::resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                            QScript::Member *member, QScriptValue *base)
{
    QScriptValue proto = object.prototype();
    Q_ASSERT(proto.isValid());
    Q_ASSERT(proto.isObject());
    return proto.impl()->resolve(nameId, member, base, QScriptValue::ResolveScope);
}


class ArgumentsClassData: public QScriptClassData
{

public:

    static inline QScript::ArgumentsObjectData *get(const QScriptValue &object)
        { return static_cast<QScript::ArgumentsObjectData*>(object.impl()->objectData().data()); }

    virtual bool resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                         QScript::Member *member, QScriptValue *base);
    virtual bool get(const QScriptValue &object, const QScript::Member &member,
                     QScriptValue *out_value);
    virtual bool put(QScriptValue *object, const QScript::Member &member,
                     const QScriptValue &value);
    virtual void mark(const QScriptValue &object, int generation);
};

bool ArgumentsClassData::resolve(const QScriptValue &object, QScriptNameIdImpl *nameId,
                                 QScript::Member *member, QScriptValue *base)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(object.engine());

    if (nameId == eng_p->idTable()->id_length) {
        member->native(nameId, /*id=*/ 0,
                       QScriptValue::Undeletable
                       | QScriptValue::ReadOnly);
        *base = object;
        return true;
    } else if (nameId == eng_p->idTable()->id_callee) {
        member->native(nameId, /*id=*/ 0,
                       QScriptValue::Undeletable
                       | QScriptValue::ReadOnly);
        *base = object;
        return true;
    }

    QString propertyName = eng_p->toString(nameId);
    bool isNumber;
    quint32 index = propertyName.toUInt(&isNumber);
    if (isNumber) {
        QScript::ArgumentsObjectData *data = ArgumentsClassData::get(object);
        if (index < data->length) {
            member->native(/*nameId=*/0, index, 0);
            *base = object;
            return true;
        }
    }

    return false;
}

bool ArgumentsClassData::get(const QScriptValue &object, const QScript::Member &member,
                             QScriptValue *out_value)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(object.engine());
    QScript::ArgumentsObjectData *data = ArgumentsClassData::get(object);
    if (member.nameId() == 0) {
        QScriptObject *activation_data = data->activation.impl()->objectValue();
        *out_value = activation_data->m_objects[member.id()];
        return true;
    } else if (member.nameId() == eng_p->idTable()->id_length) {
        eng_p->newNumber(out_value, data->length);
        return true;
    } else if (member.nameId() == eng_p->idTable()->id_callee) {
        *out_value = data->callee;
        return true;
    }
    return false;
}

bool ArgumentsClassData::put(QScriptValue *object, const QScript::Member &member,
                             const QScriptValue &value)
{
    Q_ASSERT(member.nameId() == 0);
    QScript::ArgumentsObjectData *data = ArgumentsClassData::get(*object);
    QScriptObject *activation_data = data->activation.impl()->objectValue();
    activation_data->m_objects[member.id()] = value;
    return true;
}

void ArgumentsClassData::mark(const QScriptValue &object, int generation)
{
    QScript::ArgumentsObjectData *data = ArgumentsClassData::get(object);
    data->activation.mark(generation);
    data->callee.mark(generation);
}

} // namespace QScript

const qnumber QScriptEnginePrivate::D16 = 65536.0;
const qnumber QScriptEnginePrivate::D32 = 4294967296.0;

QScriptEnginePrivate::QScriptEnginePrivate()
{
}

QScriptEnginePrivate::~QScriptEnginePrivate()
{
    delete[] m_string_hash_base;
    qDeleteAll(m_stringRepository);
    qDeleteAll(m_tempStringRepository);

    qDeleteAll(m_codeCache);
    m_codeCache.clear();
    delete[] tempStackBegin;
}

QScript::AST::Node *QScriptEnginePrivate::changeAbstractSyntaxTree(QScript::AST::Node *prg)
{
    QScript::AST::Node *was = m_abstractSyntaxTree;
    m_abstractSyntaxTree = prg;
    return was;
}

QScript::AST::Node *QScriptEnginePrivate::createAbstractSyntaxTree(const QString &source, int lineNumber)
{
    m_errorMessage.clear();

    QScript::Lexer lex(q_func());
    setLexer(&lex);
    lex.setCode(source, lineNumber);

    QScriptParser parser;

    if (! parser.parse(this)) {
        m_errorMessage = parser.errorMessage();

        //### if (! m_errorMessage.isEmpty())
        //###    m_errorMessage += QLatin1String(" ");

        //### m_errorMessage += QLatin1String("line ");
        //### m_errorMessage += QString::number(lex.lineNo());
        return 0;
    }

    return abstractSyntaxTree();
}

QScript::Code *QScriptEnginePrivate::compiledCode(QScript::AST::Node *node)
{
    QHash<QScript::AST::Node*, QScript::Code*>::iterator it = m_codeCache.find(node);
    if (it == m_codeCache.end())
        return 0;

    return it.value();
}

QScript::Code *QScriptEnginePrivate::createCompiledCode(QScript::AST::Node *node, const QScript::CompilationUnit &compilation)
{
    Q_ASSERT(compilation.isValid());

    QHash<QScript::AST::Node*, QScript::Code*>::iterator it = m_codeCache.find(node);
    if (it != m_codeCache.end())
        return it.value();

    QScript::Code *code = new QScript::Code();
    code->init(compilation);

    m_codeCache.insert(node, code);
    return code;
}

void QScriptEnginePrivate::markObject(const QScriptValue &object, int generation)
{
    QScriptObject *instance = object.impl()->objectValue();
    QScript::GCBlock *block = QScript::GCBlock::get(instance);

    enum { MAX_GC_DEPTH = 32 };

    if (block->generation + 1 != generation || m_gc_depth >= MAX_GC_DEPTH)
        return;

    ++block->generation;
    ++m_gc_depth;

    if (QScriptClassData *data = object.m_class->data())
        data->mark(object, generation);

    if (isValid(instance->m_prototype) && isObject(instance->m_prototype))
        markObject(instance->m_prototype, generation);

    if (isValid(instance->m_scope) && isObject(instance->m_scope))
        markObject(instance->m_scope, generation);

    const QScriptValue &internalValue = instance->m_internalValue;

    if (isValid(internalValue)) {
        if (isObject(internalValue))
            markObject(internalValue, generation);

        else if (isString(internalValue))
            markString(internalValue.m_string_value, generation);
    }

    int garbage = 0;

    for (int i = 0; i < instance->memberCount(); ++i) {
        QScript::Member m;
        instance->member(i, &m);

        if (! m.isValid()) {
            ++garbage;
            continue;
        }

        Q_ASSERT(m.isObjectProperty());

        QScriptValue child;
        instance->get(m, &child);

        if (m.nameId())
            markString(m.nameId(), generation);

        if (! isValid(child))
            continue;

        else if (isObject(child))
            markObject(child, generation);

        else if (isString(child))
            markString(child.m_string_value, generation);
    }

    --m_gc_depth;

    if (garbage < 128) // ###
        return;

    int j = 0;
    for (int i = 0; i < instance->memberCount(); ++i) {
        QScript::Member m;
        instance->member(i, &m);

        if (! m.isValid())
            continue;

        if (i != j) {
            instance->m_members[j].object(m.nameId(), j, m.flags());
            instance->m_objects[j] = instance->m_objects[i];
        }
        ++j;
    }
    //qDebug() << "==> old:" << instance->m_members.size() << "new:" << j;
    instance->m_members.resize(j);
    instance->m_objects.resize(j);
}

void QScriptEnginePrivate::markFrame(QScriptContext *context, int generation)
{
    QScriptValue activation = context->activationObject();
    QScriptValue thisObject = context->thisObject();
    QScriptValue scopeChain = QScriptContextPrivate::get(context)->scopeChain; // ### make public?
    QScriptValue callee = QScriptContextPrivate::get(context)->callee;

    if (QScriptContextPrivate::get(context)->functionNameId)
        markString(QScriptContextPrivate::get(context)->functionNameId, generation);

    if (isValid(activation) && isObject(activation))
        markObject(activation, generation);

    if (isValid(scopeChain) && isObject(scopeChain))
        markObject(scopeChain, generation);

    if (isValid(thisObject) && isObject(thisObject))
        markObject(thisObject, generation);

    if (isValid(callee) && isObject(callee))
        markObject(callee, generation);

    if (isValid(context->returnValue())) {
        if (isObject(context->returnValue()))
            markObject(context->returnValue(), generation);

        else if (isString(context->returnValue()))
            markString(context->returnValue().m_string_value, generation);
    }

    if (context->baseStackPointer() != context->currentStackPointer()) {
        // mark the temp stack

        for (const QScriptValue *it = context->baseStackPointer(); it != (context->currentStackPointer() + 1); ++it) {
            if (! it) {
                qWarning() << "no temp stack!!!";
                break;
            }

            else if (! it->isValid()) // ### assert?
                continue;

            else if (it->isObject())
                markObject(*it, generation);

            else if (it->isString())
                markString(it->m_string_value, generation);
        }
    }
}

void QScriptEnginePrivate::maybeGC_helper(bool do_string_gc)
{
    // qDebug() << "==>" << objectAllocator.newAllocatedBlocks() << "free:" << objectAllocator.freeBlocks();

    Q_ASSERT(objectAllocator.head() == QScript::GCBlock::get(globalObject.impl()->objectValue()));

    m_gc_depth = 0;

    int generation = objectAllocator.generation(globalObject.impl()->objectValue()) + 1;

    markObject(globalObject, generation);

    for (int index = 0; index < rootObjects.count(); ++index)
        markObject(rootObjects.at(index), generation);

    QScriptContext *current = context();
    while (current != 0) {
        markFrame (current, generation);
        current = current->parentContext();
    }

    QHashIterator<QScript::AST::Node*, QScript::Code*> it(m_codeCache);
    while (it.hasNext()) {
        it.next();
        QScriptValue v = it.value()->value;
        if (isValid(v)) {
            QScriptObject *o = v.impl()->objectValue();
            QScript::GCBlock *block = QScript::GCBlock::get(o);

            if (block->generation != generation)
                it.value()->value.invalidate();
        }
    }


    objectAllocator.sweep(generation);

    //qDebug() << "free blocks:" << objectAllocator.freeBlocks();

    Q_ASSERT(objectAllocator.head() == QScript::GCBlock::get(globalObject.impl()->objectValue()));

    if (! do_string_gc)
        return;

#if 0
    qDebug() << "do_string_gc:" << do_string_gc
        << ((m_stringRepository.size() - m_oldStringRepositorySize) > 256)
        << ((m_tempStringRepository.size() - m_oldTempStringRepositorySize) > 2048);
#endif

    QVector<QScriptNameIdImpl*> compressed;
    compressed.reserve(m_stringRepository.size());

    for (int i = 0; i < m_stringRepository.size(); ++i) {
        QScriptNameIdImpl *entry = m_stringRepository.at(i);

        if (entry->used || entry->persistent) {
            compressed.append(entry);
            entry->used = false;
        }

        else {
            //qDebug() << "deleted unique:" << entry->s;
            delete entry;
      }
    }

    // qDebug() << "before:" << m_stringRepository.size() << "after:" << compressed.size() << globalObject.objectValue()->m_members.size();
    m_stringRepository = compressed;
    rehashStringRepository(/*resize=*/ false);
    m_oldStringRepositorySize = m_stringRepository.size();


    compressed.clear();
    for (int i = 0; i < m_tempStringRepository.size(); ++i) {
        QScriptNameIdImpl *entry = m_tempStringRepository.at(i);

        if (entry->used || entry->persistent) {
            compressed.append(entry);
            entry->used = false;
        }

        else {
          //qDebug() << "deleted:" << entry->s;
            delete entry;
      }
    }

    //qDebug() << "before:" << m_tempStringRepository.size() << "after:" << compressed.size();

    m_tempStringRepository = compressed;
    m_oldTempStringRepositorySize = m_tempStringRepository.size();
}

void QScriptEnginePrivate::evaluate(QScriptContext *context, const QString &contents)
{
    // ### try to remove cast
    static_cast<QScript::EvalFunction*>(m_evalFunction)->evaluate(context, contents);
}

QScriptFunction *QScriptEnginePrivate::convertToNativeFunction(const QScriptValue &object)
{
    if (object.isFunction())
        return static_cast<QScriptFunction*> (object.impl()->objectData().data());
    return 0;
}

qnumber QScriptEnginePrivate::convertToNativeDouble_helper(const QScriptValue &object)
{
    QScriptClassInfo *klass = object.m_class;
    Q_ASSERT(klass != 0);

    switch (klass->type()) {

    case QScript::UndefinedType:
    case QScript::PointerType:
    case QScript::FunctionType:
        break;

    case QScript::NullType:
        return 0;

    case QScript::BooleanType:
        return object.m_bool_value;

    case QScript::IntegerType:
        return object.m_int_value;

    case QScript::NumberType:
        return object.m_number_value;

    case QScript::StringType:
        return toNumber(toString(object.m_string_value));

    default: {
        QScriptValue p = toPrimitive(object, QScriptValue::NumberTypeHint);
        if (! isValid(p) || isObject(p))
            break;

        return convertToNativeDouble(p);
    } // default


    } // switch

    return SNaN();
}

bool QScriptEnginePrivate::convertToNativeBoolean_helper(const QScriptValue &object)
{
    Q_ASSERT (object.isValid());

    QScriptClassInfo *klass = object.m_class;
    Q_ASSERT(klass != 0);

    switch (klass->type()) {

    case QScript::UndefinedType:
    case QScript::PointerType:
    case QScript::NullType:
        return false;

    case QScript::FunctionType:
        return true;

    case QScript::BooleanType:
        return object.m_bool_value;

    case QScript::IntegerType:
        return object.m_int_value != 0;

    case QScript::NumberType:
        return object.m_number_value != 0 && !isNaN(object.m_number_value);

    case QScript::StringType:
        return toString(object.m_string_value).length() != 0;

    case QScript::VariantType: {
        QScriptValue p = toPrimitive(object, QScriptValue::NumberTypeHint);
        if (! isValid(p) || isObject(p))
            break;

        return convertToNativeBoolean(p);
    }

    default:
        if (isObject(object))
            return true;

        break;
    } // switch

    return false;
}

QString QScriptEnginePrivate::convertToNativeString_helper(const QScriptValue &object)
{
    QScriptClassInfo *klass = object.m_class;
    Q_ASSERT(klass != 0);

    const QScript::IdTable *ids = idTable();

    switch (klass->type()) {

    case QScript::UndefinedType:
        return toString(ids->id_undefined);

    case QScript::NullType:
        return toString(ids->id_null);

    case QScript::BooleanType:
        return toString(object.m_bool_value ? ids->id_true : ids->id_false);

    case QScript::IntegerType:
        return QString::number(object.m_int_value);

    case QScript::NumberType:
        return toString(object.m_number_value);

    case QScript::PointerType:
        return toString(ids->id_pointer);

    case QScript::StringType:
        return toString(object.m_string_value);

    default: {
        QScriptValue p = toPrimitive(object, QScriptValue::StringTypeHint);

        if (! isValid(p) || isObject(p))
            return klass->name();

        return convertToNativeString(p);
    } // default

    } // switch

    return toString(ids->id_undefined);
}

// [[defaultValue]]
QScriptValue QScriptEnginePrivate::toPrimitive_helper(const QScriptValue &object,
                                                      QScriptValue::TypeHint hint)
{
    QScriptNameIdImpl *functionIds[2];

    if ((hint == QScriptValue::NumberTypeHint)
        || (hint == QScriptValue::NoTypeHint
            && object.m_class != dateConstructor->classInfo())) { // ### date?
        functionIds[0] = idTable()->id_valueOf;
        functionIds[1] = idTable()->id_toString;
    } else {
        functionIds[0] = idTable()->id_toString;
        functionIds[1] = idTable()->id_valueOf;
    }

    for (int i = 0; i < 2; ++i) {
        QScriptValue base;
        QScript::Member member;

        if (! object.impl()->resolve(functionIds[i], &member, &base, QScriptValue::ResolvePrototype))
            return object;

        QScriptValue f_valueOf;
        base.impl()->get(member, &f_valueOf);

        if (QScriptFunction *foo = convertToNativeFunction(f_valueOf)) {
            QScriptContext *me = pushContext();
            QScriptValue activation;
            newActivation(&activation);
            activation.impl()->setScope(globalObject);
            me->setActivationObject(activation);
            me->setThisObject(object);
            foo->execute(me);
            QScriptValue result = me->returnValue();
            popContext();
            if (isValid(result) && !isObject(result))
                return result;
        }
    }

    return object;
}

void QScriptEnginePrivate::rehashStringRepository(bool resize)
{
    if (resize) {
        delete[] m_string_hash_base;
        m_string_hash_size <<= 1; // ### use primes

        m_string_hash_base = new QScriptNameIdImpl* [m_string_hash_size];
    }

    memset(m_string_hash_base, 0, sizeof(QScriptNameIdImpl*) * m_string_hash_size);

    for (int index = 0; index < m_stringRepository.size(); ++index) {
        QScriptNameIdImpl *entry = m_stringRepository.at(index);
        uint h = qHash(entry->s) % m_string_hash_size;
        entry->h = h;
        entry->next = m_string_hash_base[h];
        m_string_hash_base[h] = entry;
    }
}

QScriptNameIdImpl *QScriptEnginePrivate::toStringEntry(const QString &s)
{
    uint h = qHash(s) % m_string_hash_size;

    for (QScriptNameIdImpl *entry = m_string_hash_base[h]; entry && entry->h == h; entry = entry->next) {
        if (entry->s == s)
            return entry;
    }

    return 0;
}

QScriptNameIdImpl *QScriptEnginePrivate::insertStringEntry(const QString &s)
{
    QScriptNameIdImpl *entry = new QScriptNameIdImpl(s);
    entry->unique = true;
    m_stringRepository.append(entry);

    uint h = qHash(s) % m_string_hash_size;
    entry->h = h;
    entry->next = m_string_hash_base[h];
    m_string_hash_base[h] = entry;

    if (m_stringRepository.count() == m_string_hash_size)
        rehashStringRepository();

    return entry;
}

void QScriptEnginePrivate::newFunction(QScriptValue *o, QScriptFunction *function)
{
    QScriptValue proto;
    if (functionConstructor)
        proto = functionConstructor->publicPrototype;
    else {
        // creating the Function prototype object
        Q_ASSERT(objectConstructor);
        proto = objectConstructor->publicPrototype;
    }
    newObject(o, proto, m_class_function);
    o->impl()->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(function));
}

void QScriptEnginePrivate::newConstructor(QScriptValue *ctor,
                                          QScriptFunction *function,
                                          QScriptValue &proto)
{
    newFunction(ctor, function);
    ctor->setProperty(QLatin1String("prototype"), proto,
                      QScriptValue::Undeletable
                      | QScriptValue::ReadOnly
                      | QScriptValue::SkipInEnumeration);
    proto.setProperty(QLatin1String("constructor"), *ctor,
                      QScriptValue::Undeletable
                      | QScriptValue::SkipInEnumeration);
}

void QScriptEnginePrivate::newArguments(QScriptValue *object,
                                             const QScriptValue &activation,
                                             uint length,
                                             const QScriptValue &callee)
{
    QScript::ArgumentsObjectData *data = new QScript::ArgumentsObjectData();
    data->activation = activation;
    data->length = length;
    Q_ASSERT(callee.isFunction());
    data->callee = callee;

    *object = createObject(m_class_arguments);
    object->impl()->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(data));
}

QScriptContext *QScriptEnginePrivate::pushContext()
{
    QScriptContext *context = m_frameRepository.get();
    QScriptContextPrivate::get(context)->init(m_context);
    m_context = context;
    return m_context;
}

extern double qstrtod(const char *s00, const char **se, bool *ok);
extern char *qdtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve, char **digits_str);

QString QScriptEnginePrivate::toString_helper(qnumber d)
{
    QByteArray buf;
    buf.reserve(80);

    int ndigits;
    int sign;
    char *result = 0;
    (void) qdtoa(d, 0, 0, &ndigits, &sign, 0, &result);

    if (! result)
        return QString();

    else if (ndigits <= 0 && ndigits > -6) {

        buf.fill('0', -ndigits + 2 + sign);

        if (sign) // fix the sign.
            buf[0] = '-';

        buf[sign + 1] = '.';
        buf += result;
    }

    else {
        if (sign)
            buf += '-';

        buf += result;
        int length = buf.length() - sign;

        if (ndigits <= 21 && ndigits > 0) {
            if (length <= ndigits)
                buf += QByteArray().fill('0', ndigits - length);
            else
                buf.insert(ndigits + sign, '.');
        }

        else if (result[0] >= '0' && result[0] <= '9') {
            if (length > 1)
                buf.insert(1, '.');

            buf += 'e';
            buf += (ndigits >= 0) ? '+' : '-';

            int e = ndigits - 1;

            if (e < 0)
                e = -e;

            if (e >= 100)
                buf += '0' + e / 100;

            if (e >= 10)
                buf += '0' + (e % 100) / 10;

            buf += '0' + e % 10;
        }
    }

    free(result);

    return QString::fromLatin1(buf);
}

void QScriptEnginePrivate::popContext()
{
    Q_ASSERT(m_context != 0);

    QScriptContext *context = m_context;
    m_context = context->parentContext();
    if (m_context) {
        // propagate the state
        QScriptContextPrivate::get(m_context)->result = QScriptContextPrivate::get(context)->result;
        QScriptContextPrivate::get(m_context)->state = QScriptContextPrivate::get(context)->state;
        QScriptContextPrivate::get(m_context)->errorLineNumber = QScriptContextPrivate::get(context)->errorLineNumber;
    }
    m_frameRepository.release(context);
}

QScriptValue QScriptEnginePrivate::createFunction(QScriptFunction *fun)
{
    QScriptValue v;
    newFunction(&v, fun);
    return v;
}

QScriptValue QScriptEnginePrivate::newArray(const QScript::Array &value)
{
    QScriptValue v;
    newArray(&v, value);
    return v;
}

void QScriptEnginePrivate::addRootObject(const QScriptValue &object)
{
    if (! object.isObject())
        return;

    QMutexLocker locker(&rootMutex);

    QScriptObject *instance = object.impl()->objectValue();
    for (int index = 0; index < rootObjects.count(); ++index) {
        const QScriptValue &v = rootObjects.at(index);

        if (v.impl()->objectValue() == instance) {
            ++rootObjectRefs[index];
            return;
        }
    }

    rootObjects.append(object);
    rootObjectRefs.append(1);
}

void QScriptEnginePrivate::removeRootObject(const QScriptValue &object)
{
    if (! object.isObject())
        return;

    QMutexLocker locker(&rootMutex);

    QScriptObject *instance = object.impl()->objectValue();

    for (int index = 0; index < rootObjects.count(); ++index) {
        const QScriptValue &v = rootObjects.at(index);

        if (v.impl()->objectValue() == instance) {
            if (--rootObjectRefs[index] == 0) {
                rootObjects.removeAt(index);
                rootObjectRefs.removeAt(index);
                break;
            }
        }
    }
}

QScriptValue QScriptEnginePrivate::arrayFromStringList(const QStringList &lst)
{
    Q_Q(QScriptEngine);
    QScriptValue arr = q->newArray(lst.size());
    for (int i = 0; i < lst.size(); ++i) {
        QString itemName = q->scriptValue(i).toString();
        arr.setProperty(itemName, q->scriptValue(lst.at(i)));
    }
    return arr;
}

QStringList QScriptEnginePrivate::stringListFromArray(const QScriptValue &arr)
{
    Q_Q(QScriptEngine);
    QStringList lst;
    uint len = arr.property(QLatin1String("length")).toUInt32();
    for (uint i = 0; i < len; ++i) {
        QString itemName = q->scriptValue(i).toString();
        lst.append(arr.property(itemName).toString());
    }
    return lst;
}

QScriptValue QScriptEnginePrivate::create(int type, const void *ptr)
{
    Q_Q(QScriptEngine);
    QScriptValue result;
    QScriptCustomTypeInfo info = m_customTypes.value(type);
    if (info.marshall) {
        result = info.marshall(q, ptr);
    } else {
        // check if it's one of the types we know
        switch (QMetaType::Type(type)) {
        case QMetaType::Bool:
            result = q->scriptValue(*reinterpret_cast<const bool*>(ptr));
            break;
        case QMetaType::Int:
            result = q->scriptValue(*reinterpret_cast<const int*>(ptr));
            break;
        case QMetaType::UInt:
            result = q->scriptValue(*reinterpret_cast<const uint*>(ptr));
            break;
        case QMetaType::Double:
            result = q->scriptValue(*reinterpret_cast<const double*>(ptr));
            break;
        case QMetaType::QString:
            result = q->scriptValue(*reinterpret_cast<const QString*>(ptr));
            break;
        case QMetaType::Float:
            result = q->scriptValue(*reinterpret_cast<const float*>(ptr));
            break;
        case QMetaType::Short:
            result = q->scriptValue(*reinterpret_cast<const short*>(ptr));
            break;
        case QMetaType::UShort:
            result = q->scriptValue(*reinterpret_cast<const unsigned short*>(ptr));
            break;
        case QMetaType::Char:
            result = q->scriptValue(*reinterpret_cast<const char*>(ptr));
            break;
        case QMetaType::UChar:
            result = q->scriptValue(*reinterpret_cast<const unsigned char*>(ptr));
            break;
        case QMetaType::QStringList:
            result = arrayFromStringList(*reinterpret_cast<const QStringList *>(ptr));
            break;
#ifndef QT_NO_QOBJECT
        case QMetaType::QObjectStar:
        case QMetaType::QWidgetStar:
            result = q->scriptValueFromQObject(*reinterpret_cast<QObject* const *>(ptr));
            break;
#endif
        default:
            if (type == qMetaTypeId<QScriptValue>())
                result = *reinterpret_cast<const QScriptValue*>(ptr);
            else
                result = q->scriptValueFromVariant(QVariant(type, ptr));
        }
    }
    if (isObject(result) && isValid(info.prototype))
        result.setPrototype(info.prototype);
    return result;
}

bool QScriptEnginePrivate::convert(const QScriptValue &value,
                                   int type, void *ptr)
{
    QScriptCustomTypeInfo info = m_customTypes.value(type);
    if (info.demarshall) {
        info.demarshall(value, ptr);
        return true;
    }

    // check if it's one of the types we know
    switch (QMetaType::Type(type)) {
    case QMetaType::Bool:
        *reinterpret_cast<bool*>(ptr) = value.toBoolean();
        return true;
    case QMetaType::Int:
        *reinterpret_cast<int*>(ptr) = value.toInt32();
        return true;
    case QMetaType::UInt:
        *reinterpret_cast<uint*>(ptr) = value.toUInt32();
        return true;
    case QMetaType::Double:
        *reinterpret_cast<double*>(ptr) = value.toNumber();
        return true;
    case QMetaType::QString:
        *reinterpret_cast<QString*>(ptr) = value.toString();
        return true;
    case QMetaType::Float:
        *reinterpret_cast<float*>(ptr) = value.toNumber();
        return true;
#ifndef QT_NO_QOBJECT
    case QMetaType::QObjectStar:
    case QMetaType::QWidgetStar:
        *reinterpret_cast<QObject* *>(ptr) = value.toQObject();
        return true;
#endif
    case QMetaType::QStringList:
        if (value.isArray()) {
            *reinterpret_cast<QStringList *>(ptr) = stringListFromArray(value);
            return true;
        }
        // fallthrough
    default:
    ;
    }

    QByteArray name = QMetaType::typeName(type);
#ifndef QT_NO_QOBJECT
    if (value.isQObject() && name.endsWith('*')) {
        QByteArray className = name.left(name.size()-1);
        QObject *qobject = value.toQObject();
        if (qobject->inherits(className)) {
            *reinterpret_cast<QObject* *>(ptr) = qobject;
            return true;
        }
    }
#endif
    if (type == qMetaTypeId<QScriptValue>()) {
        *reinterpret_cast<QScriptValue*>(ptr) = value;
        return true;
    }
#if 0
    if (!name.isEmpty()) {
        qWarning("QScriptEngine::convert: unable to convert value to type `%s'",
                 name.constData());
    }
#endif
    return false;
}

void QScriptEnginePrivate::init()
{
    Q_Q(QScriptEngine);

    qMetaTypeId<QScriptValue>();

    m_oldStringRepositorySize = 0;
    m_oldTempStringRepositorySize = 0;
    m_context = 0;
    m_abstractSyntaxTree = 0;
    m_lexer = 0;
    m_debuggerClient = 0;

    objectConstructor = 0;
    numberConstructor = 0;
    booleanConstructor = 0;
    stringConstructor = 0;
    dateConstructor = 0;
    functionConstructor = 0;
    arrayConstructor = 0;
    regexpConstructor = 0;
    errorConstructor = 0;
    enumerationConstructor = 0;
    variantConstructor = 0;
    qobjectConstructor = 0;

    m_stringRepository.reserve(DefaultHashSize);
    m_string_hash_size = DefaultHashSize;
    m_string_hash_base = new QScriptNameIdImpl* [m_string_hash_size];
    memset(m_string_hash_base, 0, sizeof(QScriptNameIdImpl*) * m_string_hash_size);

    m_class_prev_id = QScript::CustomType;
    m_class_activation = registerClass(QLatin1String("activation"), QScript::ActivationType);
    m_class_boolean = registerClass(QLatin1String("boolean"), QScript::BooleanType);
    m_class_double = registerClass(QLatin1String("qnumber"), QScript::NumberType);
    m_class_function = registerClass(QLatin1String("Function"), QScript::FunctionType);
    m_class_int = registerClass(QLatin1String("integer"), QScript::IntegerType);
    m_class_null = registerClass(QLatin1String("null"), QScript::NullType);
    m_class_object = registerClass(QLatin1String("Object"), QScript::ObjectType);
    m_class_pointer = registerClass(QLatin1String("pointer"), QScript::PointerType);
    m_class_reference = registerClass(QLatin1String("reference"), QScript::ReferenceType);
    m_class_string = registerClass(QLatin1String("string"), QScript::StringType);
    m_class_undefined = registerClass(QLatin1String("undefined"), QScript::UndefinedType);
    m_class_variant = registerClass(QLatin1String("variant"), QScript::VariantType);
    m_class_qobject = registerClass(QLatin1String("qobject"), QScript::QObjectType);
    m_class_qclass = registerClass(QLatin1String("qclass"), QScript::FunctionType);
#ifndef QT_NO_QOBJECT
    QExplicitlySharedDataPointer<QScriptClassData> data(new QScript::ExtQClassData());
    m_class_qclass->setData(data);
#endif

    m_class_arguments = registerClass(QLatin1String("arguments"), QScript::ObjectType);
    QExplicitlySharedDataPointer<QScriptClassData> data2(new QScript::ArgumentsClassData());
    m_class_arguments->setData(data2);

    m_class_with = registerClass(QLatin1String("__qsa_internal_with"), QScript::ObjectType);
    QExplicitlySharedDataPointer<QScriptClassData> data3(new QScript::WithClassData());
    m_class_with->setData(data3);

    // public name ids
    m_id_table.id_constructor = nameId(QLatin1String("constructor"), true);
    m_id_table.id_false       = nameId(QLatin1String("false"), true);
    m_id_table.id_null        = nameId(QLatin1String("null"), true);
    m_id_table.id_object      = nameId(QLatin1String("object"), true);
    m_id_table.id_pointer     = nameId(QLatin1String("pointer"), true);
    m_id_table.id_prototype   = nameId(QLatin1String("prototype"), true);
    m_id_table.id_arguments   = nameId(QLatin1String("arguments"), true);
    m_id_table.id_this        = nameId(QLatin1String("this"), true);
    m_id_table.id_toString    = nameId(QLatin1String("toString"), true);
    m_id_table.id_true        = nameId(QLatin1String("true"), true);
    m_id_table.id_undefined   = nameId(QLatin1String("undefined"), true);
    m_id_table.id_valueOf     = nameId(QLatin1String("valueOf"), true);
    m_id_table.id_length      = nameId(QLatin1String("length"), true);
    m_id_table.id_callee      = nameId(QLatin1String("callee"), true);
    m_id_table.id___proto__   = nameId(QLatin1String("__proto__"), true);

    const int TEMP_STACK_SIZE = 10 * 1024;
    tempStackBegin = new QScriptValue[TEMP_STACK_SIZE];
    tempStackEnd = tempStackBegin + TEMP_STACK_SIZE;
    newUndefined(&tempStackBegin[0]);

    objectAllocator.blockGC(true);

    // GC requires that GlobalObject is the first object created
    QScript::Ecma::Global::construct(&globalObject, q);

    // create the prototypes first...
    objectConstructor = new QScript::Ecma::Object(q, m_class_object);
    functionConstructor = new QScript::Ecma::Function(q, m_class_function);
    // ... then we can initialize
    functionConstructor->initialize();
    objectConstructor->initialize();

    numberConstructor = new QScript::Ecma::Number(q);
    booleanConstructor = new QScript::Ecma::Boolean(q);
    stringConstructor = new QScript::Ecma::String(q);
    dateConstructor = new QScript::Ecma::Date(q);
    arrayConstructor = new QScript::Ecma::Array(q);
    regexpConstructor = new QScript::Ecma::RegExp(q);
    errorConstructor = new QScript::Ecma::Error(q);

    QScript::Ecma::Global::initialize(&globalObject, q);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;

    globalObject.setProperty(QLatin1String("Object"),
                             objectConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("Function"),
                             functionConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("Number"),
                             numberConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("Boolean"),
                             booleanConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("String"),
                             stringConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("Date"),
                             dateConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("Array"),
                             arrayConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("RegExp"),
                             regexpConstructor->ctor, flags);
    globalObject.setProperty(QLatin1String("Error"),
                             errorConstructor->ctor, flags);

    globalObject.setProperty(QLatin1String("EvalError"),
                             errorConstructor->evalErrorCtor, flags);
    globalObject.setProperty(QLatin1String("RangeError"),
                             errorConstructor->rangeErrorCtor, flags);
    globalObject.setProperty(QLatin1String("ReferenceError"),
                             errorConstructor->referenceErrorCtor, flags);
    globalObject.setProperty(QLatin1String("SyntaxError"),
                             errorConstructor->syntaxErrorCtor, flags);
    globalObject.setProperty(QLatin1String("TypeError"),
                             errorConstructor->typeErrorCtor, flags);
    globalObject.setProperty(QLatin1String("URIError"),
                             errorConstructor->uriErrorCtor, flags);

    QScriptValue tmp; // ### fixme
    m_evalFunction = new QScript::EvalFunction(q);
    functionConstructor->newFunction(&tmp, m_evalFunction);
    globalObject.setProperty(QLatin1String("eval"), tmp, flags);

    QScriptValue mathObject;
    QScript::Ecma::Math::construct(&mathObject, q);
    globalObject.setProperty(QLatin1String("Math"), mathObject, flags);

    enumerationConstructor = new QScript::Ext::Enumeration(q);
    globalObject.setProperty(QLatin1String("Enumeration"),
                             enumerationConstructor->ctor, flags);

    variantConstructor = new QScript::Ext::Variant(q, m_class_variant);
    globalObject.setProperty(QLatin1String("Variant"),
                             variantConstructor->ctor, flags);

#ifndef QT_NO_QOBJECT
    qobjectConstructor = new QScript::ExtQObject(q, m_class_qobject);
    globalObject.setProperty(QLatin1String("QObject"),
                             qobjectConstructor->ctor, flags);
#endif

    objectAllocator.blockGC(false);

    QScriptContext *context = pushContext();
    QScriptValue activation;
    newActivation(&activation);
    activation.impl()->setScope(globalObject);
    context->setActivationObject(activation);
    context->setThisObject(globalObject);
}
