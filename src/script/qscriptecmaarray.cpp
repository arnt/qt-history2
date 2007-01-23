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

#include "qscriptengine.h"
#include "qscriptengine_p.h"
#include "qscriptcontext_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

Array::ArrayClassData::ArrayClassData(QScriptClassInfo *classInfo):
    m_classInfo(classInfo)
{
}

Array::ArrayClassData::~ArrayClassData()
{
}

void Array::ArrayClassData::mark(const QScriptValue &object, int generation)
{
    Instance *instance = Instance::get(object, classInfo());
    if (! instance)
        return;

    instance->value.mark(generation);
}

bool Array::ArrayClassData::resolve(const QScriptValue &object,
                                    QScriptNameIdImpl *nameId,
                                    QScript::Member *member,
                                    QScriptValue *base)
{
    QScriptEngine *eng = object.engine();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    if (Instance::get(object, classInfo())) {

        if (nameId == eng_p->idTable()->id_length) {
            member->native(nameId, /*id=*/ 0, QScriptValue::Undeletable);
            *base = object;
            return true;
        }

        QString propertyName = eng_p->toString(nameId);
        bool isNumber;
        quint32 pos = propertyName.toUInt(&isNumber);

        if (isNumber && (eng->scriptValue(pos).toString() == propertyName)) { // ### improve me
            member->native(0, pos, QScriptValue::Undeletable);
            *base = object;
            return true;
        }
    }

    return false;
}

bool Array::ArrayClassData::get(const QScriptValue &object,
                                const QScript::Member &member,
                                QScriptValue *result)
{
    Q_ASSERT(member.isValid());

    if (! member.isNativeProperty())
        return false;

    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(object.engine());

    Instance *instance = Instance::get(object, classInfo());
    if (! instance)
        return false;

    if (member.nameId() == eng->idTable()->id_length)
        eng->newNumber(result, instance->value.count());

    else {
        quint32 pos = quint32 (member.id());

        if (pos < instance->value.count())
            *result = instance->value.at(pos);
        else
            eng->newUndefined(result);
    }

    return true;
}

bool Array::ArrayClassData::put(QScriptValue *object,
                                const QScript::Member &member,
                                const QScriptValue &value)
{
    Q_ASSERT(object != 0);
    Q_ASSERT(member.isValid());

    if (! member.isNativeProperty())
        return false;

    Instance *instance = Instance::get(*object, classInfo());
    if (! instance)
        return false;

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(object->engine());

    if (member.nameId() == eng_p->idTable()->id_length) {
        qnumber length = value.toNumber();
        quint32 len = eng_p->toUint32(length);
        instance->value.resize(len);
    }

    else if (member.nameId() == 0) {
        quint32 pos = quint32 (member.id());
        instance->value.assign(pos, value);
    }

    return true;
}

int Array::ArrayClassData::extraMemberCount(const QScriptValue &object)
{
    if (Instance *instance = Instance::get(object, classInfo())) {
        return instance->value.count();
    }
    return 0;
}

bool Array::ArrayClassData::extraMember(const QScriptValue &object,
                                        int index, QScript::Member *member)
{
    if (Instance::get(object, classInfo())) {
        member->native(/*nameId=*/ 0, index, QScriptValue::Undeletable);
        return true;
    }

    return false;
}

Array::Array(QScriptEngine *eng):
    Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_classInfo = eng_p->registerClass(QLatin1String("Array"));
    QExplicitlySharedDataPointer<QScriptClassData> data(new ArrayClassData(m_classInfo));
    m_classInfo->setData(data);

    publicPrototype.invalidate();
    newArray(&publicPrototype);

    eng_p->newConstructor(&ctor, this, publicPrototype);

    QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleString"),
                                eng_p->createFunction(method_toLocaleString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("concat"),
                                eng_p->createFunction(method_concat, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("join"),
                                eng_p->createFunction(method_join, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("pop"),
                                eng_p->createFunction(method_pop, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("push"),
                                eng_p->createFunction(method_push, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("reverse"),
                                eng_p->createFunction(method_reverse, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("shift"),
                                eng_p->createFunction(method_shift, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("slice"),
                                eng_p->createFunction(method_slice, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("sort"),
                                eng_p->createFunction(method_sort, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("splice"),
                                eng_p->createFunction(method_splice, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("unshift"),
                                eng_p->createFunction(method_unshift, 1, m_classInfo), flags);
}

Array::~Array()
{
}

void Array::execute(QScriptContext *context)
{
    QScript::Array value;

    if (context->argumentCount() == 1 && context->argument(0).isNumber()) {
        qnumber size = context->argument(0).toNumber();
        quint32 isize = QScriptEnginePrivate::toUint32(size);

        if (size != qnumber(isize)) {
            context->throwError(QScriptContext::RangeError, QLatin1String("invalid array length"));
            return;
        }

        value.resize(isize);
    } else {
        for (int i = 0; i < context->argumentCount(); ++i) {
            value.assign(i, context->argument(i));
        }
    }

    newArray(&QScriptContextPrivate::get(context)->result, value);
}

void Array::newArray(QScriptValue *result, const QScript::Array &value)
{
    Instance *instance = new Instance();
    instance->value = value;

    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValue Array::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    return method_join(eng, classInfo); // ### fixme
}

QScriptValue Array::method_toLocaleString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    return method_toString(eng, classInfo);
}

QScriptValue Array::method_concat(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScript::Array result;

    if (Instance *instance = Instance::get(context->thisObject(), classInfo))
        result = instance->value;

    else {
        QString v = context->thisObject().toString();
        result.assign(0, eng->scriptValue(v));
    }

    for (int i = 0; i < context->argumentCount(); ++i) {
        quint32 k = result.size();
        QScriptValue arg = context->argument(i);

        if (Instance *elt = Instance::get(arg, classInfo))
            result.concat(elt->value);

        else
            result.assign(k, eng->scriptValue(arg.toString()));
    }

    return QScriptEnginePrivate::get(eng)->newArray(result);
}

QScriptValue Array::method_join(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue arg = context->argument(0);

    QString r4;
    if (arg.isUndefined())
        r4 = QLatin1String(",");
    else
        r4 = arg.toString();

    QScriptValue self = context->thisObject();

    QScriptValue length = self.property(QLatin1String("length"));
    qnumber r1 = length.isValid() ? length.toNumber() : 0;
    quint32 r2 = QScriptEnginePrivate::toUint32(r1);

    if (! r2)
        return eng->scriptValue(QString());

    QString R;

    QScriptValue r6 = self.property(QLatin1String("0"));
    if (r6.isValid() && !(r6.isUndefined() || r6.isNull()))
        R = r6.toString();

    for (quint32 k = 1; k < r2; ++k) {
        R += r4;

        QScriptNameId name = eng->nameId(eng->scriptValue(k).toString());
        QScriptValue r12 = self.property(name);

        if (r12.isValid() && ! (r12.isUndefined() || r12.isNull()))
            R += r12.toString();
    }

    return eng->scriptValue(R);
}

QScriptValue Array::method_pop(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QScriptValue elt = instance->value.pop();
        if (! elt.isValid())
            elt = eng->undefinedValue();

        return elt;
    }

    QScriptNameId id_length = eng->nameId(QLatin1String("length"));

    QScriptValue self = context->thisObject();
    QScriptValue length = self.property(id_length);
    qnumber r1 = length.isValid() ? length.toNumber() : 0;
    quint32 r2 = QScriptEnginePrivate::toUint32(r1);
    if (! r2) {
        self.setProperty(id_length, eng->scriptValue(0));
        return eng->undefinedValue();
    }
    QScriptNameId r6 = eng->nameId(eng->scriptValue(r2 - 1).toString());
    QScriptValue r7 = self.property(r6);
    if (! r7.isValid())
        return eng->undefinedValue();

    QScript::Member member;
    QScriptValue base;
    QScriptValueImpl::get(self)->resolve(r6, &member, &base, QScriptValue::ResolveLocal);
    Q_ASSERT(member.isValid());
    QScriptValueImpl::get(self)->removeMember(member);
    self.setProperty(id_length, eng->scriptValue(r2 - 1));
    return (r7);
}

QScriptValue Array::method_push(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        int pos = instance->value.size();
        for (int i = 0; i < context->argumentCount(); ++i) {
            instance->value.assign(pos++, context->argument(i));
        }
        return eng->scriptValue(instance->value.size());
    }

    QScriptValue self = context->thisObject();

    QScriptValue r0 = self.property(QLatin1String("length"));
    quint32 n = r0.isValid() ? QScriptEnginePrivate::toUint32(r0.toNumber()) : 0;
    for (int index = 0; index < context->argumentCount(); ++index, ++n) {
        QScriptValue r3 = context->argument(index);
        QScriptNameId name = eng->nameId(eng->scriptValue(n).toString());
        self.setProperty(name, r3);
    }
    QScriptValue r = eng->scriptValue(n);
    self.setProperty(QLatin1String("length"), r);
    return r;
}

QScriptValue Array::method_reverse(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        int lo = 0, hi = instance->value.count () - 1;

        for (; lo < hi; ++lo, --hi) {
            QScriptValue tmp = instance->value.at(lo);
            instance->value.assign(lo, instance->value.at(hi));
            instance->value.assign(hi, tmp);
        }

    } else {
        QScriptValue self = context->thisObject();

        QScriptNameId id_length = eng->nameId(QLatin1String("length"));
        QScriptEngine *eng = context->engine();

        QScriptValue lengthValue = self.property(id_length);
        quint32 length = 0;
        if (lengthValue.isValid())
            length = QScriptEnginePrivate::toUint32(lengthValue.toNumber());
        const quint32 m = length / 2;
        for (quint32 i = 0; i < m; ++i) {
            quint32 j = length - i - 1;

            QScriptNameId iid = eng->nameId(eng->scriptValue(i).toString());
            QScriptNameId jid = eng->nameId(eng->scriptValue(j).toString());

            QScript::Member imember;
            QScriptValue ibase;
            QScriptValue ival;
            bool iok = QScriptValueImpl::get(self)->resolve(iid, &imember, &ibase, QScriptValue::ResolveLocal);
            if (iok)
                QScriptValueImpl::get(ibase)->get(iid, &ival);
            else
                ival = eng->undefinedValue();

            QScript::Member jmember;
            QScriptValue jbase;
            QScriptValue jval;
            bool jok = QScriptValueImpl::get(self)->resolve(jid, &jmember, &jbase, QScriptValue::ResolveLocal);
            if (jok)
                QScriptValueImpl::get(jbase)->get(jid, &jval);
            else
                jval = eng->undefinedValue();

            if (!jok) {
                if (iok) {
                    QScriptValueImpl::get(self)->removeMember(imember);
                    self.setProperty(jid, ival);
                }
            } else if (!iok) {
                self.setProperty(iid, jval);
                QScriptValueImpl::get(self)->removeMember(jmember);
            } else {
                QScriptValueImpl::get(self)->put(imember, jval);
                QScriptValueImpl::get(self)->put(jmember, ival);
            }
        }
    }

    return context->thisObject();
}

QScriptValue Array::method_shift(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptNameId id_length = eng->nameId(QLatin1String("length"));

    quint32 length = QScriptEnginePrivate::toUint32(context->thisObject().property(id_length).toNumber());
    if (length == 0) {
        context->thisObject().setProperty(id_length, eng->scriptValue(0));
        return eng->undefinedValue();
    }

    QScriptValue self = context->thisObject();
    QScript::Member member;
    QScriptValue base;

    QScriptValue result = self.property(QLatin1String("0"));
    if (! result.isValid())
        result = eng->undefinedValue();

    for (quint32 index = 1; index < length; ++index) {
        QScriptNameId k = eng->nameId(eng->scriptValue(index).toString());
        QScriptNameId k1 = eng->nameId(eng->scriptValue(index - 1).toString());

        QScriptValue v = self.property(k);
        QScriptValue v1 = self.property(k1);

        if (v.isValid())
            self.setProperty(k1, v);

        else if (v1.isValid() && QScriptValueImpl::get(self)->resolve(k1, &member, &base, QScriptValue::ResolveLocal))
            QScriptValueImpl::get(self)->removeMember(member);
    }

    QScriptValue len = eng->scriptValue(length - 1);

    if (QScriptValueImpl::get(self)->resolve(eng->nameId(len.toString()), &member, &base, QScriptValue::ResolveLocal))
        QScriptValueImpl::get(self)->removeMember(member);

    self.setProperty(id_length, len);
    return (result);
}

QScriptValue Array::method_slice(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScript::Array result;

    QScriptValue start = context->argument(0);
    QScriptValue end = context->argument(1);

    QScriptValue self = context->thisObject();
    qnumber r2 = self.property(QLatin1String("length")).toNumber();
    quint32 r3 = QScriptEnginePrivate::toUint32(r2);
    qint32 r4 = qint32 (start.toInteger());
    quint32 r5 = r4 < 0 ? qMax(quint32(r3 + r4), quint32(0)) : qMin(quint32(r4), r3);
    quint32 k = r5;
    qint32 r7 = end.isUndefined() ? r3 : qint32 (end.toInteger());
    quint32 r8 = r7 < 0 ? qMax(quint32(r3 + r7), quint32(0)) : qMin(quint32(r7), r3);
    quint32 n = 0;
    for (; k < r8; ++k) {
        QString r11 = eng->scriptValue(k).toString();
        QScriptValue v = self.property(r11);
        if (v.isValid())
            result.assign(n++, v);
    }
    return (QScriptEnginePrivate::get(eng)->newArray(result));
}

QScriptValue Array::method_sort(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue comparefn = context->argument(0);
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        instance->value.sort(comparefn);
        return context->thisObject();
    }
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Array.prototype.sort"));
}

QScriptValue Array::method_splice(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->argumentCount() < 2)
        return eng->undefinedValue();

    QScriptValue self = context->thisObject();

    qnumber start = context->argument(0).toInteger();
    qnumber deleteCount = context->argument(1).toInteger();

    QScriptValue arrayCtor = eng->globalObject().property(QLatin1String("Array"));
    QScriptValue a = arrayCtor.call(eng->nullValue()); // ### construct()

    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QVector<QScriptValue> items;
        for (int i = 2; i < context->argumentCount(); ++i)
            items << context->argument(i);
        Instance *otherInstance = Instance::get(a, classInfo);
        instance->value.splice(start, deleteCount, items, otherInstance->value);
        return a;
    }

    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("Array.prototype.splice"));
}

QScriptValue Array::method_unshift(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue self = context->thisObject();

    QScriptValue r1 = self.property(QLatin1String("length"));
    quint32 r2 = r1.isValid() ? QScriptEnginePrivate::toUint32(r1.toNumber()) : 0;
    quint32 r3 = quint32 (context->argumentCount());
    quint32 k = r2;
    for (; k != 0; --k) {
        QScriptNameId r6 = eng->nameId(eng->scriptValue(k - 1).toString());
        QScriptNameId r7 = eng->nameId(eng->scriptValue(k + r3 - 1).toString());
        QScriptValue r8 = self.property(r6);
        if (r8.isValid())
            self.setProperty(r7, r8);

        else {
            QScript::Member member;
            QScriptValue base;

            if (QScriptValueImpl::get(self)->resolve(r7, &member, &base, QScriptValue::ResolveLocal))
                QScriptValueImpl::get(self)->removeMember(member);
        }
    }

    for (k = 0; k < r3; ++k) {
        QScriptValue r16 = context->argument(k);
        QScriptNameId r17 = eng->nameId(eng->scriptValue(k).toString());
        self.setProperty(r17, r16);
    }
    QScriptValue r = eng->scriptValue(r2 + r3);
    self.setProperty(QLatin1String("length"), r);
    return (r);
}

Array::Instance *Array::Instance::get(const QScriptValue &object, QScriptClassInfo *klass)
{
    if (! klass || klass == QScriptValueImpl::get(object)->classInfo())
        return static_cast<Instance*> (QScriptValueImpl::get(object)->objectData().data());
    
    return 0;
}

} } // namespace QScript::Ecma
