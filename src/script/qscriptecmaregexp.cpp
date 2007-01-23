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
#include "qscriptecmaregexp_p.h"
#include "qscriptecmaarray_p.h"
#include "qscriptecmanumber_p.h"
#include "qscriptecmaobject_p.h"
#include "qscriptecmafunction_p.h"
#include "qscriptengine_p.h"

#include <QtCore/QStringList>
#include <QtCore/QRegExp>
#include <QtCore/QtDebug>

namespace QScript { namespace Ecma {

RegExp::RegExp(QScriptEngine *eng):
    Core(eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    m_classInfo = eng_p->registerClass(QLatin1String("RegExp"));

    publicPrototype.invalidate();
    newRegExp(&publicPrototype, QString(), QString());

    eng_p->newConstructor(&ctor, this, publicPrototype);

    QScriptValue::PropertyFlags flags = QScriptValue::SkipInEnumeration;

    publicPrototype.setProperty(QLatin1String("exec"),
                                eng_p->createFunction(method_exec, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("test"),
                                eng_p->createFunction(method_test, 1, m_classInfo), flags);
    publicPrototype.setProperty(QLatin1String("toString"),
                                eng_p->createFunction(method_toString, 1, m_classInfo), flags);
}

RegExp::~RegExp()
{
}

RegExp::Instance *RegExp::Instance::get(const QScriptValue &object, QScriptClassInfo *klass)
{
    if (! klass || klass == QScriptValueImpl::get(object)->classInfo())
        return QExplicitlySharedDataPointer<Instance> (static_cast<Instance*> (QScriptValueImpl::get(object)->objectData().data()));
    
    return 0;
}

void RegExp::execute(QScriptContext *context)
{
    QString source;
    QString flags;

    if (context->argumentCount() > 0)
        source = context->argument(0).toString();

    if (context->argumentCount() > 1) {
        flags = context->argument(1).toString();
        if (!flags.isEmpty()) {
            // check that the flags are OK
            const QString legalFlags = QLatin1String("gim");
            for (int i = 0; i < flags.length(); ++i) {
                if (legalFlags.indexOf(flags.at(i)) == -1) {
                    context->throwError(
                        QScriptContext::SyntaxError,
                        QString::fromUtf8("invalid regular expression flag '%0'")
                        .arg(flags.at(i)));
                    return;
                }
            }
        }
    }

    newRegExp(&QScriptContextPrivate::get(context)->result, source, flags);
}

void RegExp::newRegExp(QScriptValue *result, const QString &pattern, const QString &flags)
{
#ifndef QT_NO_REGEXP
    bool ignoreCase = flags.contains(QLatin1Char('i'));

    QRegExp rx(pattern,
        (ignoreCase ? Qt::CaseInsensitive: Qt::CaseSensitive)
#if QT_VERSION >= 0x040200
        , QRegExp::RegExp2
#endif
        );

    newRegExp_helper(result, rx, flags);
#else
    Instance *instance = new Instance();
    instance->pattern = pattern;
    instance->flags = flags;

    QScriptEngine *eng = engine();

    QScriptEnginePrivate::get(eng)->newObject(result, publicPrototype, classInfo());
    result->impl()->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
    result->setProperty(QLatin1String("source"), eng->scriptValue(pattern), QScriptValue::ReadOnly);
#endif // QT_NO_REGEXP
}

#ifndef QT_NO_REGEXP
void RegExp::newRegExp(QScriptValue *result, const QRegExp &rx)
{
    bool ignoreCase = rx.caseSensitivity() == Qt::CaseInsensitive;
    QString flags;
    if (ignoreCase)
        flags += QLatin1String("i");

    newRegExp_helper(result, rx, flags);
}

void RegExp::newRegExp_helper(QScriptValue *result, const QRegExp &rx,
                              const QString &flags)
{
    bool global = flags.contains(QLatin1Char('g'));
    bool ignoreCase = flags.contains(QLatin1Char('i'));
    bool multiline = flags.contains(QLatin1Char('m'));

    Instance *instance = new Instance();
    instance->value = rx;
    instance->flags = flags;

    QScriptEngine *eng = engine();

    QScriptEnginePrivate::get(eng)->newObject(result, publicPrototype, classInfo());
    QScriptValueImpl::get(*result)->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));

    QScriptValue::PropertyFlags propertyFlags = QScriptValue::SkipInEnumeration
                                                | QScriptValue::Undeletable
                                                | QScriptValue::ReadOnly;

    result->setProperty(QLatin1String("global"), eng->scriptValue(global),
                        propertyFlags);
    result->setProperty(QLatin1String("ignoreCase"), eng->scriptValue(ignoreCase),
                        propertyFlags);
    result->setProperty(QLatin1String("multiline"), eng->scriptValue(multiline),
                        propertyFlags);
    result->setProperty(QLatin1String("source"), eng->scriptValue(rx.pattern()),
                        propertyFlags);
    result->setProperty(QLatin1String("lastIndex"), eng->scriptValue(0),
                        propertyFlags & ~QScriptValue::ReadOnly);
}
#endif // QT_NO_REGEXP

QScriptValue RegExp::method_exec(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    QScriptValue self = context->thisObject();
    if (QScriptValueImpl::get(self)->classInfo() != classInfo)
        return context->throwError(QScriptContext::TypeError,
                                   QLatin1String("RegExp.prototype.exec"));

    Instance *rx_data = Instance::get(self, classInfo);
    Q_ASSERT(rx_data != 0);

    QString S = context->argument(0).toString();
    int length = S.length();
    QScriptValue lastIndex = self.property(QLatin1String("lastIndex"));

    int i = lastIndex.isValid() ? int (lastIndex.toInteger()) : 0;
    bool global = self.property(QLatin1String("global")).toBoolean();

    if (! global)
        i = 0;

    if (i < 0 || i >= length)
        return (eng->nullValue());

#ifndef QT_NO_REGEXP
    int index = rx_data->value.indexIn(S, i);
    if (index == -1)
#endif // QT_NO_REGEXP
        return eng->nullValue();

#ifndef QT_NO_REGEXP
    int e = index + rx_data->value.matchedLength();

    if (global)
        self.setProperty(QLatin1String("lastIndex"), eng->scriptValue(e));

    QScript::Array elts;
    QStringList capturedTexts = rx_data->value.capturedTexts();
    for (int i = 0; i < capturedTexts.count(); ++i)
        elts.assign(i, eng->scriptValue(capturedTexts.at(i)));

    QScriptValue r = QScriptEnginePrivate::get(eng)->newArray(elts);

    r.setProperty(QLatin1String("index"), eng->scriptValue(index));
    r.setProperty(QLatin1String("input"), eng->scriptValue(S));

    return r;
#endif // QT_NO_REGEXP
}

QScriptValue RegExp::method_test(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();

    method_exec(eng, classInfo);
    return (eng->scriptValue(! context->returnValue().isNull()));
}

QScriptValue RegExp::method_toString(QScriptEngine *eng, QScriptClassInfo *classInfo)
{
    QScriptContext *context = eng->currentContext();
    if (Instance *instance = Instance::get(context->thisObject(), classInfo)) {
        QString pattern;
        pattern += QLatin1Char('/');
#ifndef QT_NO_REGEXP
        pattern += instance->value.pattern(); // ### quote
#endif
        pattern += QLatin1Char('/');
        pattern += instance->flags;
        return (eng->scriptValue(pattern));
    }

    return context->throwError(QScriptContext::TypeError,
                               QLatin1String("RegExp.prototype.toString"));
}

} } // namespace QScript::Ecma
