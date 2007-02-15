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

#include "qscriptecmaarray_p.h"
#include "qscriptengine_p.h"
#include "qscriptvalueimpl_p.h"
#include "qscriptcontext_p.h"
#include "qscriptmember_p.h"
#include "qscriptobject_p.h"

#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

Array::ArrayClassData::ArrayClassData(QScriptClassInfo *classInfo):
    m_classInfo(classInfo)
{
}

Array::ArrayClassData::~ArrayClassData()
{
}

void Array::ArrayClassData::mark(const QScriptValueImpl &object, int generation)
{
    Instance *instance = Instance::get(object, classInfo());
    if (! instance)
        return;

    instance->value.mark(generation);
}

bool Array::ArrayClassData::resolve(const QScriptValueImpl &object,
                                    QScriptNameIdImpl *nameId,
                                    QScript::Member *member,
                                    QScriptValueImpl *base)
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

        if (isNumber && (QScriptValue(eng, pos).toString() == propertyName)) { // ### improve me
            member->native(0, pos, QScriptValue::Undeletable);
            *base = object;
            return true;
        }
    }

    return false;
}

bool Array::ArrayClassData::get(const QScriptValueImpl &object,
                                const QScript::Member &member,
                                QScriptValueImpl *result)
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

bool Array::ArrayClassData::put(QScriptValueImpl *object,
                                const QScript::Member &member,
                                const QScriptValueImpl &value)
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
        qsreal length = value.toNumber();
        quint32 len = eng_p->toUint32(length);
        instance->value.resize(len);
    }

    else if (member.nameId() == 0) {
        quint32 pos = quint32 (member.id());
        instance->value.assign(pos, value);
    }

    return true;
}

int Array::ArrayClassData::extraMemberCount(const QScriptValueImpl &object)
{
    if (Instance *instance = Instance::get(object, classInfo())) {
        return instance->value.count();
    }
    return 0;
}

bool Array::ArrayClassData::extraMember(const QScriptValueImpl &object,
                                        int index, QScript::Member *member)
{
    if (Instance::get(object, classInfo())) {
        member->native(/*nameId=*/ 0, index, QScriptValue::Undeletable);
        return true;
    }

    return false;
}

Array::Array(QScriptEnginePrivate *eng):
    Core(eng)
{
    m_classInfo = eng->registerClass(QLatin1String("Array"));
    QExplicitlySharedDataPointer<QScriptClassData> data(new ArrayClassData(m_classInfo));
    m_classInfo->setData(data);

    publicPrototype.invalidate();
    newArray(&publicPrototype);

    eng->newConstructor(&ctor, this, publicPrototype);

    QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleString"),
                                eng->createFunction(method_toLocaleString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("concat"),
                                eng->createFunction(method_concat, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("join"),
                                eng->createFunction(method_join, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("pop"),
                                eng->createFunction(method_pop, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("push"),
                                eng->createFunction(method_push, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("reverse"),
                                eng->createFunction(method_reverse, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("shift"),
                                eng->createFunction(method_shift, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("slice"),
                                eng->createFunction(method_slice, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("sort"),
                                eng->createFunction(method_sort, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("splice"),
                                eng->createFunction(method_splice, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("unshift"),
                                eng->createFunction(method_unshift, 1, m_classInfo), flags);
}

Array::~Array()
{
}

void Array::execute(QScriptContextPrivate *context)
{
    QScript::Array value;

    if (context->argumentCount() == 1 && context->argument(0).isNumber()) {
        qsreal size = context->argument(0).toNumber();
        quint32 isize = QScriptEnginePrivate::toUint32(size);

        if (size != qsreal(isize)) {
            context->throwError(QScriptContext::RangeError, QLatin1String("invalid array length"));
            return;
        }

        value.resize(isize);
    } else {
        for (int i = 0; i < context->argumentCount(); ++i) {
            value.assign(i, context->argument(i));
        }
    }

    newArray(&context->m_result, value);
}

void Array::newArray(QScriptValueImpl *result, const QScript::Array &value)
{
    Instance *instance = new Instance();
    instance->value = value;

    engine()->newObject(result, publicPrototype, classInfo());
    result->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

QScriptValueImpl Array::method_toString(QScriptContextPrivate *context,
                                        QScriptEnginePrivate *eng,
                                        QScriptClassInfo *classInfo)
{
    return method_join(context, eng, classInfo); // ### fixme
}

QScriptValueImpl Array::method_toLocaleString(QScriptContextPrivate *context,
                                              QScriptEnginePrivate *eng,
                                              QScriptClassInfo *classInfo)
{
    return method_toString(context, eng, classInfo);
}

QScriptValueImpl Array::method_concat(QScriptContextPrivate *context,
                                      QScriptEnginePrivate *eng,
                                      QScriptClassInfo *classInfo)
{
    QScript::Array result;

    if (Instance *instance = Instance::get(context->thisObject(), classInfo))
        result = instance->value;

    else {
        QString v = context->thisObject().toString();
        result.assign(0, QScriptValueImpl(eng, v));
    }

    for (int i = 0; i < context->argumentCount(); ++i) {
        quint32 k = result.size();
        QScriptValueImpl arg = context->argument(i);

        if (Instance *elt = Instance::get(arg, classInfo))
            result.concat(elt->value);

        else
            result.assign(k, QScriptValueImpl(eng, arg.toString()));
    }

    return eng->newArray(result);
}

QScriptValueImpl Array::method_join(QScriptContextPrivate *context,
                                    QScriptEnginePrivate *eng,
                                    QScriptClassInfo *)
{
    QScriptValueImpl arg = context->argument(0);

    QString r4;
    if (arg.isUndefined())
        r4 = QLatin1String(",");
    else
        r4 = arg.toString();

    QScriptValueImpl self = context->thisObject();

    QScriptValueImpl length = self.property(QLatin1String("length"));
    qsreal r1 = length.isValid() ? length.toNumber() : 0;
    quint32 r2 = QScriptEnginePrivate::toUint32(r1);

    if (! r2)
        return QScriptValueImpl(eng, QString());

    QString R;

    QScriptValueImpl r6 = self.property(QLatin1String("0"));
    if (r6.isValid() && !(r6.isUndefined() || r6.isNull()))
        R = r6.toString();

    for (quint32 k = 1; k < r2; ++k) {
        R += r4;

        QScriptNameIdImpl *name = eng->nameId(QScriptValueImpl(eng, k).toString());
        QScriptValueImpl r12 = self.property(name);

        if (r12.isValid() && ! (r12.isUndefined() || r12.isNull()))
            R += r12.toString();
    }

    return QScriptValueImpl(eng, R);
}

QScriptValueImpl Array::method_pop(QScriptContextPrivate *context,
                                   QScriptEnginePrivate *eng,
                                   QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QScriptValueImpl elt = instance->value.pop();
        if (! elt.isValid())
            elt = eng->undefinedValue();

        return elt;
    }

    QScriptNameIdImpl *id_length = eng->nameId(QLatin1String("length"));

    QScriptValueImpl self = context->thisObject();
    QScriptValueImpl length = self.property(id_length);
    qsreal r1 = length.isValid() ? length.toNumber() : 0;
    quint32 r2 = QScriptEnginePrivate::toUint32(r1);
    if (! r2) {
        self.setProperty(id_length, QScriptValueImpl(eng, 0));
        return eng->undefinedValue();
    }
    QScriptNameIdImpl *r6 = eng->nameId(QScriptValueImpl(eng, r2 - 1).toString());
    QScriptValueImpl r7 = self.property(r6);
    if (! r7.isValid())
        return eng->undefinedValue();

    QScript::Member member;
    QScriptValueImpl base;
    self.resolve(r6, &member, &base, QScriptValue::ResolveLocal);
    Q_ASSERT(member.isValid());
    self.removeMember(member);
    self.setProperty(id_length, QScriptValueImpl(eng, r2 - 1));
    return (r7);
}

QScriptValueImpl Array::method_push(QScriptContextPrivate *context,
                                    QScriptEnginePrivate *eng,
                                    QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        int pos = instance->value.size();
        for (int i = 0; i < context->argumentCount(); ++i) {
            instance->value.assign(pos++, context->argument(i));
        }
        return QScriptValueImpl(eng, instance->value.size());
    }

    QScriptValueImpl self = context->thisObject();

    QScriptValueImpl r0 = self.property(QLatin1String("length"));
    quint32 n = r0.isValid() ? QScriptEnginePrivate::toUint32(r0.toNumber()) : 0;
    for (int index = 0; index < context->argumentCount(); ++index, ++n) {
        QScriptValueImpl r3 = context->argument(index);
        QScriptNameIdImpl *name = eng->nameId(QScriptValueImpl(eng, n).toString());
        self.setProperty(name, r3);
    }
    QScriptValueImpl r(eng, n);
    self.setProperty(QLatin1String("length"), r);
    return r;
}

QScriptValueImpl Array::method_reverse(QScriptContextPrivate *context,
                                       QScriptEnginePrivate *eng,
                                       QScriptClassInfo *classInfo)
{
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        int lo = 0, hi = instance->value.count () - 1;

        for (; lo < hi; ++lo, --hi) {
            QScriptValueImpl tmp = instance->value.at(lo);
            instance->value.assign(lo, instance->value.at(hi));
            instance->value.assign(hi, tmp);
        }

    } else {
        QScriptValueImpl self = context->thisObject();

        QScriptNameIdImpl *id_length = eng->nameId(QLatin1String("length"));

        QScriptValueImpl lengthValue = self.property(id_length);
        quint32 length = 0;
        if (lengthValue.isValid())
            length = QScriptEnginePrivate::toUint32(lengthValue.toNumber());
        const quint32 m = length / 2;
        for (quint32 i = 0; i < m; ++i) {
            quint32 j = length - i - 1;

            QScriptNameIdImpl *iid = eng->nameId(QScriptValueImpl(eng, i).toString());
            QScriptNameIdImpl *jid = eng->nameId(QScriptValueImpl(eng, j).toString());

            QScript::Member imember;
            QScriptValueImpl ibase;
            QScriptValueImpl ival;
            bool iok = self.resolve(iid, &imember, &ibase, QScriptValue::ResolveLocal);
            if (iok)
                ibase.get(iid, &ival);
            else
                ival = eng->undefinedValue();

            QScript::Member jmember;
            QScriptValueImpl jbase;
            QScriptValueImpl jval;
            bool jok = self.resolve(jid, &jmember, &jbase, QScriptValue::ResolveLocal);
            if (jok)
                jbase.get(jid, &jval);
            else
                jval = eng->undefinedValue();

            if (!jok) {
                if (iok) {
                    self.removeMember(imember);
                    self.setProperty(jid, ival);
                }
            } else if (!iok) {
                self.setProperty(iid, jval);
                self.removeMember(jmember);
            } else {
                self.put(imember, jval);
                self.put(jmember, ival);
            }
        }
    }

    return context->thisObject();
}

QScriptValueImpl Array::method_shift(QScriptContextPrivate *context,
                                     QScriptEnginePrivate *eng,
                                     QScriptClassInfo *)
{
    QScriptNameIdImpl *id_length = eng->nameId(QLatin1String("length"));

    quint32 length = QScriptEnginePrivate::toUint32(context->thisObject().property(id_length).toNumber());
    if (length == 0) {
        context->thisObject().setProperty(id_length, QScriptValueImpl(eng, 0));
        return eng->undefinedValue();
    }

    QScriptValueImpl self = context->thisObject();
    QScript::Member member;
    QScriptValueImpl base;

    QScriptValueImpl result = self.property(QLatin1String("0"));
    if (! result.isValid())
        result = eng->undefinedValue();

    for (quint32 index = 1; index < length; ++index) {
        QScriptNameIdImpl *k = eng->nameId(QScriptValueImpl(eng, index).toString());
        QScriptNameIdImpl *k1 = eng->nameId(QScriptValueImpl(eng, index - 1).toString());

        QScriptValueImpl v = self.property(k);
        QScriptValueImpl v1 = self.property(k1);

        if (v.isValid())
            self.setProperty(k1, v);

        else if (v1.isValid() && self.resolve(k1, &member, &base, QScriptValue::ResolveLocal))
            self.removeMember(member);
    }

    QScriptValueImpl len = QScriptValueImpl(eng, length - 1);

    if (self.resolve(eng->nameId(len.toString()), &member, &base, QScriptValue::ResolveLocal))
        self.removeMember(member);

    self.setProperty(id_length, len);
    return (result);
}

QScriptValueImpl Array::method_slice(QScriptContextPrivate *context,
                                     QScriptEnginePrivate *eng,
                                     QScriptClassInfo *)
{
    QScript::Array result;

    QScriptValueImpl start = context->argument(0);
    QScriptValueImpl end = context->argument(1);

    QScriptValueImpl self = context->thisObject();
    qsreal r2 = self.property(QLatin1String("length")).toNumber();
    quint32 r3 = QScriptEnginePrivate::toUint32(r2);
    qint32 r4 = qint32 (start.toInteger());
    quint32 r5 = r4 < 0 ? qMax(quint32(r3 + r4), quint32(0)) : qMin(quint32(r4), r3);
    quint32 k = r5;
    qint32 r7 = end.isUndefined() ? r3 : qint32 (end.toInteger());
    quint32 r8 = r7 < 0 ? qMax(quint32(r3 + r7), quint32(0)) : qMin(quint32(r7), r3);
    quint32 n = 0;
    for (; k < r8; ++k) {
        QString r11 = QScriptValueImpl(eng, k).toString();
        QScriptValueImpl v = self.property(r11);
        if (v.isValid())
            result.assign(n++, v);
    }
    return eng->newArray(result);
}

QScriptValueImpl Array::method_sort(QScriptContextPrivate *context,
                                    QScriptEnginePrivate *,
                                    QScriptClassInfo *classInfo)
{
    QScriptValueImpl comparefn = context->argument(0);
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        instance->value.sort(comparefn);
        return context->thisObject();
    }
    return context->throwNotImplemented(QLatin1String("Array.prototype.sort"));
}

QScriptValueImpl Array::method_splice(QScriptContextPrivate *context,
                                      QScriptEnginePrivate *eng,
                                      QScriptClassInfo *classInfo)
{
    if (context->argumentCount() < 2)
        return eng->undefinedValue();

    QScriptValueImpl self = context->thisObject();

    qsreal start = context->argument(0).toInteger();
    qsreal deleteCount = context->argument(1).toInteger();

    QScriptValueImpl arrayCtor = eng->globalObject().property(QLatin1String("Array"));
    QScriptValueImpl a = arrayCtor.construct();

    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QVector<QScriptValueImpl> items;
        for (int i = 2; i < context->argumentCount(); ++i)
            items << context->argument(i);
        Instance *otherInstance = Instance::get(a, classInfo);
        instance->value.splice(start, deleteCount, items, otherInstance->value);
        return a;
    }

    return context->throwNotImplemented(QLatin1String("Array.prototype.splice"));
}

QScriptValueImpl Array::method_unshift(QScriptContextPrivate *context,
                                       QScriptEnginePrivate *eng,
                                       QScriptClassInfo *)
{
    QScriptValueImpl self = context->thisObject();

    QScriptValueImpl r1 = self.property(QLatin1String("length"));
    quint32 r2 = r1.isValid() ? QScriptEnginePrivate::toUint32(r1.toNumber()) : 0;
    quint32 r3 = quint32 (context->argumentCount());
    quint32 k = r2;
    for (; k != 0; --k) {
        QScriptNameIdImpl *r6 = eng->nameId(QScriptValueImpl(eng, k - 1).toString());
        QScriptNameIdImpl *r7 = eng->nameId(QScriptValueImpl(eng, k + r3 - 1).toString());
        QScriptValueImpl r8 = self.property(r6);
        if (r8.isValid())
            self.setProperty(r7, r8);

        else {
            QScript::Member member;
            QScriptValueImpl base;

            if (self.resolve(r7, &member, &base, QScriptValue::ResolveLocal))
                self.removeMember(member);
        }
    }

    for (k = 0; k < r3; ++k) {
        QScriptValueImpl r16 = context->argument(k);
        QScriptNameIdImpl *r17 = eng->nameId(QScriptValueImpl(eng, k).toString());
        self.setProperty(r17, r16);
    }
    QScriptValueImpl r(eng, r2 + r3);
    self.setProperty(QLatin1String("length"), r);
    return (r);
}

Array::Instance *Array::Instance::get(const QScriptValueImpl &object, QScriptClassInfo *klass)
{
    if (! klass || klass == object.classInfo())
        return static_cast<Instance*> (object.objectData().data());
    
    return 0;
}

} } // namespace QScript::Ecma
