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
#include "qscriptcontext_p.h"
#include "qscriptecmastring_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmanumber_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptecmaregexp_p.h"
#include "qscriptengine_p.h"

#include <QtCore/QStringList>
#include <QtCore/QtDebug>
#include <QtCore/qnumeric.h>

#include <limits.h>

namespace QScript { namespace Ecma {

String::StringClassData::StringClassData(QScriptClassInfo *classInfo):
    m_classInfo(classInfo)
{
}

String::StringClassData::~StringClassData()
{
}

bool String::StringClassData::resolve(const QScriptValue &object,
                                      QScriptNameIdImpl *nameId,
                                      QScript::Member *member,
                                      QScriptValue *base)
{
    if (object.impl()->classInfo() != classInfo())
        return false;

    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(object.engine());

    if (nameId == eng_p->idTable()->id_length) {
        member->native(nameId, /*id=*/ 0,
                       QScriptValue::Undeletable
                       | QScriptValue::ReadOnly);
        *base = object;
        return true;
    }

    bool ok = false;
    int index = nameId->s.toInt(&ok);

    if (ok)
        member->native(nameId, index, QScriptValue::Undeletable);

    return ok;
}

bool String::StringClassData::get(const QScriptValue &object,
                                  const QScript::Member &member,
                                  QScriptValue *result)
{
    Q_ASSERT(member.isValid());

    if (object.impl()->classInfo() != classInfo())
        return false;

    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(object.engine());
    if (! member.isNativeProperty())
        return false;

    QScriptNameIdImpl *ref = object.impl()->internalValue().impl()->stringValue();
    int len = ref->s.length();

    if (member.nameId() == eng->idTable()->id_length)
        eng->newNumber(result, len);

    else if (member.id() >= 0 && member.id() < len)
        eng->newString(result, ref->s.at(member.id()));

    else
        eng->newUndefined(result);

    return true;
}

int String::StringClassData::extraMemberCount(const QScriptValue &object)
{
    if (object.impl()->classInfo() != classInfo())
        return 0;

    QScriptNameIdImpl *ref = object.impl()->internalValue().impl()->stringValue();
    return ref->s.length();
}

bool String::StringClassData::extraMember(const QScriptValue &object,
                                          int index, Member *member)
{
    if (object.impl()->classInfo() != classInfo())
        return false;

    member->native(/*nameId=*/ 0, index, QScriptValue::Undeletable);
    return true;
}

String::String(QScriptEngine *eng):
    Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_classInfo = eng_p->registerClass(QLatin1String("String"));
    QExplicitlySharedDataPointer<QScriptClassData> data(new StringClassData(m_classInfo));
    m_classInfo->setData(data);

    publicPrototype.invalidate();
    newString(&publicPrototype, QString());

    eng_p->newConstructor(&ctor, this, publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("valueOf"),
                                eng_p->createFunction(method_valueOf, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("charAt"),
                                eng_p->createFunction(method_charAt, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("charCodeAt"),
                                eng_p->createFunction(method_charCodeAt, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("concat"),
                                eng_p->createFunction(method_concat, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("indexOf"),
                                eng_p->createFunction(method_indexOf, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("lastIndexOf"),
                                eng_p->createFunction(method_lastIndexOf, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("localeCompare"),
                                eng_p->createFunction(method_localeCompare, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("match"),
                                eng_p->createFunction(method_match, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("replace"),
                                eng_p->createFunction(method_replace, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("search"),
                                eng_p->createFunction(method_search, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("slice"),
                                eng_p->createFunction(method_slice, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("split"),
                                eng_p->createFunction(method_split, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("substring"),
                                eng_p->createFunction(method_substring, 2, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLowerCase"),
                                eng_p->createFunction(method_toLowerCase, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleLowerCase"),
                                eng_p->createFunction(method_toLocaleLowerCase, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toUpperCase"),
                                eng_p->createFunction(method_toUpperCase, 0, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toLocaleUpperCase"),
                                eng_p->createFunction(method_toLocaleUpperCase, 0, m_classInfo), flags);

    ctor.setProperty(QLatin1String("fromCharCode"),
                     eng_p->createFunction(method_fromCharCode, 1, m_classInfo), flags);
}

String::~String()
{
}

void String::execute(QScriptContext *context)
{
    QString value;

    if (context->argumentCount() > 0)
        value = context->argument(0).toString();

    QScriptValue str = engine()->scriptValue(value);
    if (!context->calledAsConstructor()) {
        context->setReturnValue(str);
    } else {
        QScriptValue &obj = QScriptContextPrivate::get(context)->thisObject;
        obj.impl()->setClassInfo(classInfo());
        obj.impl()->setInternalValue(str);
        obj.setPrototype(publicPrototype);
    }
}

void String::newString(QScriptValue *result, const QString &value)
{
    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    result->impl()->setInternalValue(engine()->scriptValue(value));
}

QScriptValue String::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError, QLatin1String("String.prototype.toString"));

    return (context->thisObject().impl()->internalValue());
}

QScriptValue String::method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (context->thisObject().impl()->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError, QLatin1String("String.prototype.valueOf"));

    return (context->thisObject().impl()->internalValue());
}

QScriptValue String::method_charAt(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString str = context->thisObject().toString();

    int pos = 0;
    if (context->argumentCount() > 0)
        pos = int (context->argument(0).toInteger());

    QString result;
    if (pos >= 0 && pos < str.length())
        result += str.at(pos);

    return (eng->scriptValue(result));
}

QScriptValue String::method_charCodeAt(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString str = context->thisObject().toString();

    int pos = 0;
    if (context->argumentCount() > 0)
        pos = int (context->argument(0).toInteger());

    qnumber result = qSNan();

    if (pos >= 0 && pos < str.length())
        result = str.at(pos).unicode();

    return (eng->scriptValue(result));
}

QScriptValue String::method_concat(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();

    for (int i = 0; i < context->argumentCount(); ++i)
        value += context->argument(i).toString();

    return (eng->scriptValue(value));
}

QScriptValue String::method_indexOf(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();

    QString searchString;
    if (context->argumentCount() > 0)
        searchString = context->argument(0).toString();

    int pos = 0;
    if (context->argumentCount() > 1)
        pos = int (context->argument(1).toInteger());

    int index = -1;
    if (! value.isEmpty())
        index = value.indexOf(searchString, qMax(pos, 0));

    return (eng->scriptValue(index));
}

QScriptValue String::method_lastIndexOf(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();

    QString searchString;
    if (context->argumentCount() > 0)
        searchString = context->argument(0).toString();

    qnumber position = context->argument(1).toNumber();
    if (qIsNan(position))
        position = +qInf();
    else
        position = QScriptEnginePrivate::toInteger(position);

    int pos = QScriptEnginePrivate::toInt32(qMin(qMax(position, 0.0), qnumber(value.length())));
    if (!searchString.isEmpty() && pos == value.length())
        --pos;
    int index = value.lastIndexOf(searchString, pos);
    return (eng->scriptValue(index));
}

QScriptValue String::method_localeCompare(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    return QScriptContextPrivate::get(context)->throwNotImplemented(
        QLatin1String("String.prototype.localeCompare"));
}

QScriptValue String::method_match(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QScriptValue pattern = context->argument(0);

    if (! eng_p->regexpConstructor->get(pattern))
        eng_p->regexpConstructor->newRegExp(&pattern, context->argument(0).toString(), QString());

    QScriptValue rx_exec = pattern.property(QLatin1String("exec"), QScriptValue::ResolvePrototype);
    if (! (rx_exec.isValid() && rx_exec.isFunction()))
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("String.prototype.match"));

    QScriptValueList args;
    args << context->thisObject();

    QScriptValue global = pattern.property(QLatin1String("global"));
    if (! (global.isValid() && global.toBoolean()))
        return (rx_exec.call(pattern, args));

    QScript::Array result;

    QScriptNameId lastIndexId = eng->nameId(QLatin1String("lastIndex"));
    QScriptNameId zeroId = eng->nameId(QLatin1String("0"));

    pattern.setProperty(lastIndexId, eng->scriptValue(0));
    int n = 0;
    while (true) {
        qnumber lastIndex = pattern.property(lastIndexId).toNumber();
        QScriptValue r = rx_exec.call(pattern, args);
        if (r.isNull())
            break;
        qnumber newLastIndex = pattern.property(lastIndexId).toNumber();
        if (newLastIndex == lastIndex)
            pattern.setProperty(lastIndexId, eng->scriptValue(lastIndex + 1));
        result.assign(n++, r.property(zeroId));
    }

    return (QScriptEnginePrivate::get(eng)->newArray(result));
}

QScriptValue String::method_replace(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QString input = context->thisObject().toString();
    QScriptValue searchValue = context->argument(0);
    QScriptValue replaceValue = context->argument(1);

    QString output;
    if (searchValue.impl()->classInfo() == eng_p->regexpConstructor->classInfo()) {
        // searchValue is a RegExp
        QScriptValue rx_exec = searchValue.property(QLatin1String("exec"), QScriptValue::ResolvePrototype);
        if (!rx_exec.isFunction())
            return context->throwError(QScriptContext::TypeError,
                                       QLatin1String("String.prototype.replace"));
        QVector<QScriptValue> occurrences;
        QScriptValue global = searchValue.property(QLatin1String("global"));
        QScriptValueList args;
        args << eng->scriptValue(input);
        if (!global.toBoolean()) {
            QScriptValue r = rx_exec.call(searchValue, args);
            if (!r.isNull())
                occurrences.append(r);
        } else {
            QScriptNameId lastIndexId = eng->nameId(QLatin1String("lastIndex"));
            searchValue.setProperty(lastIndexId, eng->scriptValue(0));
            while (true) {
                qnumber lastIndex = searchValue.property(lastIndexId).toNumber();
                QScriptValue r = rx_exec.call(searchValue, args);
                if (r.isNull())
                    break;
                qnumber newLastIndex = searchValue.property(lastIndexId).toNumber();
                if (newLastIndex == lastIndex)
                    searchValue.setProperty(lastIndexId, eng->scriptValue(lastIndex + 1));
                occurrences.append(r);
            }
        }
        int pos = 0;
        if (replaceValue.isFunction()) {
            QScriptNameId indexId = eng->nameId(QLatin1String("index"));
            QScriptNameId lengthId = eng->nameId(QLatin1String("length"));
            for (int i = 0; i < occurrences.count(); ++i) {
                QScriptValue needle = occurrences.at(i);
                int index = int (needle.property(indexId).toInteger());
                uint length = eng_p->toUint32(needle.property(lengthId).toNumber());
                output += input.mid(pos, index - pos);
                args.clear();
                args << eng->scriptValue(index);
                args << eng->scriptValue(input);
                for (uint j = 0; j < length; ++j)
                    args << needle.property(eng->scriptValue(j).toString());
                QScriptValue ret = replaceValue.call(eng->nullScriptValue(), args);
                output += ret.toString();
                pos = index + args[0].toString().length();
            }
        } else {
            // use string representation of replaceValue
            const QString replaceString = replaceValue.toString();
            const QLatin1Char dollar = QLatin1Char('$');
            QScriptNameId indexId = eng->nameId(QLatin1String("index"));
            QScriptNameId zeroId = eng->nameId(QLatin1String("0"));
            for (int i = 0; i < occurrences.count(); ++i) {
                QScriptValue needle = occurrences.at(i);
                int index = int (needle.property(indexId).toInteger());
                output += input.mid(pos, index - pos);
                int j = 0;
                while (j < replaceString.length()) {
                    const QChar c = replaceString.at(j++);
                    if ((c == dollar) && (j < replaceString.length())) {
                        const QChar nc = replaceString.at(j);
                        if (nc == dollar) {
                            ++j;
                        } else if (nc == QLatin1Char('`')) {
                            ++j;
                            output += input.left(index);
                            continue;
                        } else if (nc == QLatin1Char('\'')) {
                            ++j;
                            output += input.mid(index + needle.property(zeroId).toString().length());
                            continue;
                        } else if (nc.isDigit()) {
                            ++j;
                            int cap = nc.toLatin1() - '0';
                            if ((j < replaceString.length()) && replaceString.at(j).isDigit()) {
                                cap = cap * 10;
                                cap = replaceString.at(j++).toLatin1() - '0';
                            }
                            output += needle.property(eng->scriptValue(cap).toString()).toString();
                            continue;
                        }
                    }
                    output += c;
                }
                pos = index + needle.property(zeroId).toString().length();
            }
            output += input.mid(pos);
        }
    } else {
        // use string representation of searchValue
        const QString searchString = searchValue.toString();
        int pos = 0;
        if (replaceValue.isFunction()) {
            QScriptValueList args;
            args << eng->scriptValue(searchString);
            args << eng->undefinedScriptValue(); // dummy
            args << eng->scriptValue(input);
            int index = input.indexOf(searchString, pos);
            while (index != -1) {
                output += input.mid(pos, index - pos);
                args[1] = eng->scriptValue(index);
                QScriptValue ret = replaceValue.call(eng->nullScriptValue(), args);
                output += ret.toString();
                pos = index + searchString.length();
                index = input.indexOf(searchString, pos);
            }
            output += input.mid(pos);
        } else {
            // use string representation of replaceValue
            const QString replaceString = replaceValue.toString();
            const QLatin1Char dollar = QLatin1Char('$');
            int index = input.indexOf(searchString, pos);
            while (index != -1) {
                output += input.mid(pos, index - pos);
                int j = 0;
                while (j < replaceString.length()) {
                    const QChar c = replaceString.at(j++);
                    if ((c == dollar) && (j < replaceString.length())) {
                        const QChar nc = replaceString.at(j);
                        if (nc == dollar) {
                            ++j;
                        } else if (nc == QLatin1Char('`')) {
                            output += input.left(index);
                            ++j;
                            continue;
                        } else if (nc == QLatin1Char('\'')) {
                            output += input.mid(index + searchString.length());
                            ++j;
                            continue;
                        }
                    }
                    output += c;
                }
                pos = index + searchString.length();
                index = input.indexOf(searchString, pos);
            }
            output += input.mid(pos);
        }
    }
    return eng->scriptValue(output);
}

QScriptValue String::method_search(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QScriptValue pattern = context->argument(0);

    Ecma::RegExp::Instance *rx_data = 0;
    if (0 == (rx_data = eng_p->regexpConstructor->get(pattern))) {
        eng_p->regexpConstructor->newRegExp(&pattern, context->argument(0).toString(), QString());
        rx_data = eng_p->regexpConstructor->get(pattern);
    }

    QString value = context->thisObject().toString();
#ifndef QT_NO_REGEXP
    return (eng->scriptValue(value.indexOf(rx_data->value)));
#else
    return eng->createNull();
#endif
}

QScriptValue String::method_slice(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    if (context->argumentCount() != 2)
        return context->throwError(QLatin1String("invalid argument"));

    QString text = context->thisObject().toString();
    int length = text.length();

    int start = int (context->argument(0).toInteger());
    int end = int (context->argument(1).toInteger());

    if (start < 0)
        start = qMax(length + start, 0);
    else
        start = qMin(start, length);

    if (end < 0)
        end = qMax(length + end, 0);
    else
        end = qMin(end, length);

    int count = qMax(0, end - start);
    return (eng->scriptValue(text.mid(start, count)));
}

QScriptValue String::method_split(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue l = context->argument(1);
    quint32 lim = l.isUndefined() ? UINT_MAX : QScriptEnginePrivate::toUint32(l.toNumber());

    if (lim == 0)
        return eng->newArray();

    QString S = context->thisObject().toString();
    QScriptValue separator = context->argument(0);

    QScript::Array A;
    // the argumentCount() check is for compatibility with spidermonkey;
    // it is not according to ECMA-262
    if (separator.isUndefined() && (context->argumentCount() == 0)) {
        A.assign(0, eng->scriptValue(S));
    } else {
        QStringList matches;
#ifndef QT_NO_REGEXP
        QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
        if (Ecma::RegExp::Instance *rx = eng_p->regexpConstructor->get(separator)) {
            matches = S.split(rx->value, rx->value.pattern().isEmpty()
                              ? QString::SkipEmptyParts : QString::KeepEmptyParts);
        } else
#endif // QT_NO_REGEXP
        {
            QString sep = separator.toString();
            matches = S.split(sep, sep.isEmpty()
                              ? QString::SkipEmptyParts : QString::KeepEmptyParts);
        }
        uint count = qMin(lim, uint(matches.count()));
        for (uint i = 0; i < count; ++i)
            A.assign(i, eng->scriptValue(matches.at(i)));
    }

    return QScriptEnginePrivate::get(eng)->newArray(A);
}

QScriptValue String::method_substring(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    int length = value.length();

    qnumber start = 0;
    qnumber end = length;

    if (context->argumentCount() > 0)
        start = context->argument(0).toNumber();

    if (context->argumentCount() > 1)
        end = context->argument(1).toNumber();

    if (qIsNan(start) || start < 0)
        start = 0;

    if (qIsNan(end) || end < 0)
        end = 0;

    if (start > length)
        start = length;

    if (end > length)
        end = length;

    if (start > end) {
        qnumber was = start;
        start = end;
        end = was;
    }

    qint32 x = QScriptEnginePrivate::toInt32(start);
    qint32 y = QScriptEnginePrivate::toInt32(end - start);

    return (eng->scriptValue(value.mid(x, y)));
}

QScriptValue String::method_toLowerCase(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    return (eng->scriptValue(value.toLower()));
}

QScriptValue String::method_toLocaleLowerCase(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    return method_toLowerCase(eng, classInfo); // ### check me
}

QScriptValue String::method_toUpperCase(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    return (eng->scriptValue(value.toUpper()));
}

QScriptValue String::method_toLocaleUpperCase(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    return method_toUpperCase(eng, classInfo); // ### check me
}

QScriptValue String::method_fromCharCode(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString str;
    for (int i = 0; i < context->argumentCount(); ++i) {
        QChar c(context->argument(i).toUInt16());
        str += c;
    }
    return (eng->scriptValue(str));
}

} } // namespace QScript::Ecma
