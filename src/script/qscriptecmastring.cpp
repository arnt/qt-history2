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
    if (QScriptValueImpl::get(object)->classInfo() != classInfo())
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

    if (QScriptValueImpl::get(object)->classInfo() != classInfo())
        return false;

    QScriptEnginePrivate *eng = QScriptEnginePrivate::get(object.engine());
    if (! member.isNativeProperty())
        return false;

    QScriptNameIdImpl *ref = QScriptValueImpl::get(QScriptValueImpl::get(object)->internalValue())->stringValue();
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
    if (QScriptValueImpl::get(object)->classInfo() != classInfo())
        return 0;

    QScriptNameIdImpl *ref = QScriptValueImpl::get(QScriptValueImpl::get(object)->internalValue())->stringValue();
    return ref->s.length();
}

bool String::StringClassData::extraMember(const QScriptValue &object,
                                          int index, Member *member)
{
    if (QScriptValueImpl::get(object)->classInfo() != classInfo())
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

    QScriptValue str(engine(), value);
    if (!context->calledAsConstructor()) {
        context->setReturnValue(str);
    } else {
        QScriptValue &obj = QScriptContextPrivate::get(context)->thisObject;
        QScriptValueImpl::get(obj)->setClassInfo(classInfo());
        QScriptValueImpl::get(obj)->setInternalValue(str);
        obj.setPrototype(publicPrototype);
        context->setReturnValue(obj);
    }
}

void String::newString(QScriptValue *result, const QString &value)
{
    QScriptEnginePrivate::get(engine())->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setInternalValue(QScriptValue(engine(), value));
}

QScriptValue String::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue self = context->thisObject();
    if (QScriptValueImpl::get(self)->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError, QLatin1String("String.prototype.toString"));

    return (QScriptValueImpl::get(self)->internalValue());
}

QScriptValue String::method_valueOf(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue self = context->thisObject();
    if (QScriptValueImpl::get(self)->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError, QLatin1String("String.prototype.valueOf"));

    return (QScriptValueImpl::get(self)->internalValue());
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

    return (QScriptValue(eng, result));
}

QScriptValue String::method_charCodeAt(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString str = context->thisObject().toString();

    int pos = 0;
    if (context->argumentCount() > 0)
        pos = int (context->argument(0).toInteger());

    qsreal result = qSNan();

    if (pos >= 0 && pos < str.length())
        result = str.at(pos).unicode();

    return (QScriptValue(eng, result));
}

QScriptValue String::method_concat(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();

    for (int i = 0; i < context->argumentCount(); ++i)
        value += context->argument(i).toString();

    return (QScriptValue(eng, value));
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

    return (QScriptValue(eng, index));
}

QScriptValue String::method_lastIndexOf(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();

    QString searchString;
    if (context->argumentCount() > 0)
        searchString = context->argument(0).toString();

    qsreal position = context->argument(1).toNumber();
    if (qIsNan(position))
        position = +qInf();
    else
        position = QScriptEnginePrivate::toInteger(position);

    int pos = QScriptEnginePrivate::toInt32(qMin(qMax(position, 0.0), qsreal(value.length())));
    if (!searchString.isEmpty() && pos == value.length())
        --pos;
    int index = value.lastIndexOf(searchString, pos);
    return (QScriptValue(eng, index));
}

QScriptValue String::method_localeCompare(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    QString that = context->argument(0).toString();
    return QScriptValue(eng, QString::localeAwareCompare(value, that));
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

    pattern.setProperty(lastIndexId, QScriptValue(eng, 0));
    int n = 0;
    while (true) {
        qsreal lastIndex = pattern.property(lastIndexId).toNumber();
        QScriptValue r = rx_exec.call(pattern, args);
        if (r.isNull())
            break;
        qsreal newLastIndex = pattern.property(lastIndexId).toNumber();
        if (newLastIndex == lastIndex)
            pattern.setProperty(lastIndexId, QScriptValue(eng, lastIndex + 1));
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
    if (QScriptValueImpl::get(searchValue)->classInfo() == eng_p->regexpConstructor->classInfo()) {
        // searchValue is a RegExp
        QScriptValue rx_exec = searchValue.property(QLatin1String("exec"), QScriptValue::ResolvePrototype);
        if (!rx_exec.isFunction())
            return context->throwError(QScriptContext::TypeError,
                                       QLatin1String("String.prototype.replace"));
        QVector<QScriptValue> occurrences;
        QScriptValue global = searchValue.property(QLatin1String("global"));
        QScriptValueList args;
        args << QScriptValue(eng, input);
        if (!global.toBoolean()) {
            QScriptValue r = rx_exec.call(searchValue, args);
            if (!r.isNull())
                occurrences.append(r);
        } else {
            QScriptNameId lastIndexId = eng->nameId(QLatin1String("lastIndex"));
            searchValue.setProperty(lastIndexId, QScriptValue(eng, 0));
            while (true) {
                qsreal lastIndex = searchValue.property(lastIndexId).toNumber();
                QScriptValue r = rx_exec.call(searchValue, args);
                if (r.isNull())
                    break;
                qsreal newLastIndex = searchValue.property(lastIndexId).toNumber();
                if (newLastIndex == lastIndex)
                    searchValue.setProperty(lastIndexId, QScriptValue(eng, lastIndex + 1));
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
                args << QScriptValue(eng, index);
                args << QScriptValue(eng, input);
                for (uint j = 0; j < length; ++j)
                    args << needle.property(QScriptValue(eng, j).toString());
                QScriptValue ret = replaceValue.call(eng->nullValue(), args);
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
                            output += needle.property(QScriptValue(eng, cap).toString()).toString();
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
            args << QScriptValue(eng, searchString);
            args << eng->undefinedValue(); // dummy
            args << QScriptValue(eng, input);
            int index = input.indexOf(searchString, pos);
            while (index != -1) {
                output += input.mid(pos, index - pos);
                args[1] = QScriptValue(eng, index);
                QScriptValue ret = replaceValue.call(eng->nullValue(), args);
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
    return QScriptValue(eng, output);
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
    return (QScriptValue(eng, value.indexOf(rx_data->value)));
#else
    return eng->createNull();
#endif
}

QScriptValue String::method_slice(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString text = context->thisObject().toString();
    int length = text.length();

    int start = int (context->argument(0).toInteger());
    int end = context->argument(1).isUndefined()
              ? length : int (context->argument(1).toInteger());

    if (start < 0)
        start = qMax(length + start, 0);
    else
        start = qMin(start, length);

    if (end < 0)
        end = qMax(length + end, 0);
    else
        end = qMin(end, length);

    int count = qMax(0, end - start);
    return (QScriptValue(eng, text.mid(start, count)));
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
        A.assign(0, QScriptValue(eng, S));
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
            A.assign(i, QScriptValue(eng, matches.at(i)));
    }

    return QScriptEnginePrivate::get(eng)->newArray(A);
}

QScriptValue String::method_substring(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    int length = value.length();

    qsreal start = 0;
    qsreal end = length;

    if (context->argumentCount() > 0)
        start = context->argument(0).toInteger();

    if (context->argumentCount() > 1)
        end = context->argument(1).toInteger();

    if (qIsNan(start) || start < 0)
        start = 0;

    if (qIsNan(end) || end < 0)
        end = 0;

    if (start > length)
        start = length;

    if (end > length)
        end = length;

    if (start > end) {
        qsreal was = start;
        start = end;
        end = was;
    }

    qint32 x = QScriptEnginePrivate::toInt32(start);
    qint32 y = QScriptEnginePrivate::toInt32(end - start);

    return (QScriptValue(eng, value.mid(x, y)));
}

QScriptValue String::method_toLowerCase(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    return (QScriptValue(eng, value.toLower()));
}

QScriptValue String::method_toLocaleLowerCase(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    return method_toLowerCase(eng, classInfo); // ### check me
}

QScriptValue String::method_toUpperCase(QScriptEngine *eng, QScriptClassInfo *)
{
    QScriptContext *context = eng->currentContext();
    QString value = context->thisObject().toString();
    return (QScriptValue(eng, value.toUpper()));
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
    return (QScriptValue(eng, str));
}

} } // namespace QScript::Ecma
