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

// for strtoll
#include <qplatformdefs.h>

#include "qscriptengine.h"
#include "qscriptcontext.h"

#include "qscriptecmaglobal_p.h"
#include "qscriptast_p.h"
#include "qscriptengine_p.h"
#include "qscriptvalue_p.h"

#include <QtCore/QVarLengthArray>
#include <QtCore/qnumeric.h>

extern Q_CORE_EXPORT qlonglong qstrtoll(const char *nptr, const char **endptr, register int base, bool *ok);

static inline char toHex(char c)
{
    static const char hexnumbers[] = "0123456789ABCDEF";
    return hexnumbers[c & 0xf];
}

static int fromHex(char c)
{
    if ((c >= '0') && (c <= '9'))
        return c - '0';
    if ((c >= 'A') && (c <= 'F'))
        return c - 'A' + 10;
    if ((c >= 'a') && (c <= 'f'))
        return c - 'a' + 10;
    return -1;
}

static QByteArray escape(const QString &input)
{
    QVarLengthArray<char> output;
    output.reserve(input.size() * 3);
    const int length = input.length();
    for (int i = 0; i < length; ++i) {
        ushort uc = input.at(i).unicode();
        if (uc < 0x100) {
            if (   (uc > 0x60 && uc < 0x7B)
                || (uc > 0x3F && uc < 0x5B)
                || (uc > 0x2C && uc < 0x3A)
                || (uc == 0x2A)
                || (uc == 0x2B)
                || (uc == 0x5F)) {
                output.append(char(uc));
            } else {
                output.append('%');
                output.append(toHex(uc >> 4));
                output.append(toHex(uc));
            }
        } else {
            output.append('%');
            output.append('u');
            output.append(toHex(uc >> 12));
            output.append(toHex(uc >> 8));
            output.append(toHex(uc >> 4));
            output.append(toHex(uc));
        }
    }
    return QByteArray(output.constData(), output.size());
}

static QString unescape(const QByteArray &input)
{
    QString result;
    int i = 0;
    const int length = input.length();
    while (i < length) {
        char c = input.at(i++);
        if ((c == '%') && (i + 1 < length)) {
            char a = input.at(i);
            if ((a == 'u') && (i + 4 < length)) {
                int d3 = fromHex(input.at(i+1));
                int d2 = fromHex(input.at(i+2));
                int d1 = fromHex(input.at(i+3));
                int d0 = fromHex(input.at(i+4));
                if ((d3 != -1) && (d2 != -1) && (d1 != -1) && (d0 != -1)) {
                    ushort uc = ushort((d3 << 12) | (d2 << 8) | (d1 << 4) | d0);
                    result.append(QChar(uc));
                    i += 5;
                } else {
                    result.append(QLatin1Char(c));
                }
            } else {
                int d1 = fromHex(a);
                int d0 = fromHex(input.at(i+1));
                if ((d1 != -1) && (d0 != -1)) {
                    c = (d1 << 4) | d0;
                    i += 2;
                }
                result.append(QLatin1Char(c));
            }
        } else {
            result.append(QLatin1Char(c));
        }
    }
    return result;
}

static const char uriReserved[] = ";/?:@&=+$,";
static const char uriUnescaped[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_.!~*'()";

static QString encode(const QString &input, const QString &unescapedSet, bool *ok)
{
    QString output;
    const int length = input.length();
    int i = 0;
    while (i < length) {
        const QChar c = input.at(i);
        if (!unescapedSet.contains(c)) {
            ushort uc = c.unicode();
            if ((uc >= 0xDC00) && (uc <= 0xDFFF)) {
                // URIError
                break;
            }
            if (!((uc < 0xD800) || (uc > 0xDBFF))) {
                ++i;
                if (i == length) {
                    // URIError
                    break;
                }
                const ushort uc2 = input.at(i).unicode();
                if ((uc < 0xDC00) || (uc > 0xDFFF)) {
                    // URIError
                    break;
                }
                uc = ((uc - 0xD800) * 0x400) + (uc2 - 0xDC00) + 0x10000;
            }
            QString tmp(1, QChar(uc));
            QByteArray octets = tmp.toUtf8();
            for (int j = 0; j < octets.length(); ++j) {
                output.append(QLatin1Char('%'));
                output.append(QLatin1Char(toHex(octets.at(j) >> 4)));
                output.append(QLatin1Char(toHex(octets.at(j))));
            }
        } else {
            output.append(c);
        }
        ++i;
    }
    *ok = (i == length);
    return output;
}

static QString decode(const QString &input, const QString &reservedSet, bool *ok)
{
    QString output;
    const int length = input.length();
    int i = 0;
    const QChar percent = QLatin1Char('%');
    while (i < length) {
        const QChar c = input.at(i);
        if (c == percent) {
            int start = i;
            if (i + 2 >= length) {
                // URIError
                break;
            }
            int d1 = fromHex(input.at(i+1).toLatin1());
            int d0 = fromHex(input.at(i+2).toLatin1());
            if ((d1 == -1) || (d0 == -1)) {
                // URIError
                break;
            }
            int b = (d1 << 4) | d0;
            i += 2;
            if (b & 0x80) {
                int n = -1;
                while ((b << ++n) & 0x80) ;
                if ((n == 1) || (n > 4)) {
                    // URIError
                    break;
                }
                QByteArray octets;
                octets.append(b);
                if (i + (3 * (n - 1)) >= length) {
                    // URIError
                    break;
                }
                for (int j = 1; j < n; ++j) {
                    ++i;
                    if (input.at(i) != percent) {
                        // URIError
                        break;
                    }
                    d1 = fromHex(input.at(i+1).toLatin1());
                    d0 = fromHex(input.at(i+2).toLatin1());
                    if ((d1 == -1) || (d0 == -1)) {
                        // URIError
                        break;
                    }
                    b = (d1 << 4) | d0;
                    if ((b & 0xC0) != 0x80) {
                        // URIError
                        break;
                    }
                    i += 2;
                    octets.append(b);
                }
                QString tmp = QString::fromUtf8(octets);
                Q_ASSERT(tmp.length() == 1);
                uint v = tmp.at(0).unicode(); // ### need 32-bit value
                if (v < 0x10000) {
                    QChar z = QChar(ushort(v));
                    if (!reservedSet.contains(z)) {
                        output.append(z);
                    } else {
                        output.append(input.mid(start, i - start + 1));
                    }
                } else {
                    if (v > 0x10FFFF) {
                        // URIError
                        break;
                    }
                    ushort l = ushort(((v - 0x10000) & 0x3FF) + 0xDC00);
                    ushort h = ushort((((v - 0x10000) >> 10) & 0x3FF) + 0xD800);
                    output.append(QChar(l));
                    output.append(QChar(h));
                }
            } else {
                output.append(ushort(b));
            }
        } else {
            output.append(c);
        }
        ++i;
    }
    *ok = (i == length);
    return output;
}



namespace QScript {

class PrintFunction : public QScriptFunction
{
public:
    PrintFunction():
        qout(stdout, QIODevice::WriteOnly) {}

    virtual ~PrintFunction() {}

    virtual void execute(QScriptContext *context)
    {
        QScriptEngine *eng = context->engine();
        QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
        bool blocked = eng_p->blockGC(true);
        for (int i = 0; i < context->argumentCount(); ++i) {
            if (i != 0)
                qout << QLatin1String(" ");

            qout << context->argument(i).toString();
        }

        qout << endl;

        context->setReturnValue(eng->undefinedValue());
        eng_p->blockGC(blocked);
    }

    QTextStream qout;
};

} // anonumous

namespace QScript { namespace Ecma {

Global::Global(QScriptEngine *engine, QScriptClassInfo *classInfo)
    : m_engine(engine), m_classInfo(classInfo)
{
}

Global::~Global()
{
}

void Global::construct(QScriptValue *object, QScriptEngine *eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);
    QScriptClassInfo *classInfo = eng_p->registerClass(QLatin1String("global"),
                                                       QScript::ActivationType);

    // create with prototype null, since Object.prototype doesn't exist at this point
    eng_p->newObject(object, eng->nullValue(), classInfo);

    Global *instance = new Global(eng, classInfo);
    QScriptValueImpl::get(*object)->setObjectData(QExplicitlySharedDataPointer<QScriptObjectData>(instance));
}

void Global::initialize(QScriptValue *object, QScriptEngine *eng)
{
    QScriptEnginePrivate *eng_p = QScriptEnginePrivate::get(eng);

    // set the real prototype
    object->setPrototype(eng_p->objectConstructor->publicPrototype);

    const QScriptValue::PropertyFlags flags = QScriptValue::Undeletable
                                              | QScriptValue::SkipInEnumeration;

    object->setProperty(QLatin1String("NaN"), QScriptValue(eng, qSNan()), flags);
    object->setProperty(QLatin1String("Infinity"), QScriptValue(eng, qInf()), flags);
    object->setProperty(QLatin1String("undefined"), eng->undefinedValue(), flags);

    object->setProperty(QLatin1String("print"),
                        eng_p->createFunction(new PrintFunction()), flags);
    object->setProperty(QLatin1String("parseInt"),
                        eng->newFunction(method_parseInt, 2), flags);
    object->setProperty(QLatin1String("parseFloat"),
                        eng->newFunction(method_parseFloat, 1), flags);
    object->setProperty(QLatin1String("isNaN"),
                        eng->newFunction(method_isNaN, 1), flags);
    object->setProperty(QLatin1String("isFinite"),
                        eng->newFunction(method_isFinite, 1), flags);
    object->setProperty(QLatin1String("decodeURI"),
                        eng->newFunction(method_decodeURI, 1), flags);
    object->setProperty(QLatin1String("decodeURIComponent"),
                        eng->newFunction(method_decodeURIComponent, 1), flags);
    object->setProperty(QLatin1String("encodeURI"),
                        eng->newFunction(method_encodeURI, 1), flags);
    object->setProperty(QLatin1String("encodeURIComponent"),
                        eng->newFunction(method_encodeURIComponent, 1), flags);
    object->setProperty(QLatin1String("escape"),
                        eng->newFunction(method_escape, 1), flags);
    object->setProperty(QLatin1String("unescape"),
                        eng->newFunction(method_unescape, 1), flags);
    object->setProperty(QLatin1String("version"),
                        eng->newFunction(method_version, 0), flags);
    object->setProperty(QLatin1String("gc"),
                        eng->newFunction(method_gc, 0), flags);
}

QScriptValue Global::method_parseInt(QScriptContext *context,
                                     QScriptEngine *eng)
{
    if (context->argumentCount() == 0) {
        return QScriptValue(eng, qSNan());
    }
    qsreal radix = 0;
    if (context->argumentCount() > 1) {
        radix = context->argument(1).toInteger();
        if (qIsNan(radix) || (radix && (radix < 2 || radix > 36))) {
            return QScriptValue(eng, qSNan());
        }
    }
    QString str = context->argument(0).toString().trimmed();
    if (radix == 0) {
        if ((str.length() >= 2) && (str.at(0) == QLatin1Char('0'))) {
            if ((str.at(1) == QLatin1Char('x'))
                || (str.at(1) == QLatin1Char('X'))) {
                str.remove(0, 2);
                radix = 16;
            } else {
                str.remove(0, 1);
                radix = 8;
            }
        }
    }

    const char *startPtr = str.toUtf8().constData();
    qsreal result;
#if defined(Q_WS_WIN) && !defined(Q_CC_MINGW)
    const char *endPtr = 0;
    bool ok;
    result = qstrtoll(startPtr, &endPtr, int (radix), &ok);
#else
    char *endPtr = 0;
    result = strtoll(startPtr, &endPtr, int (radix));
#endif
    if (startPtr == endPtr) {
        if (str.isEmpty())
            result = qSNan();
        else if (str == QLatin1String("Infinity"))
            result = +qInf();
        else if (str == QLatin1String("+Infinity"))
            result = +qInf();
        else if (str == QLatin1String("-Infinity"))
            result = -qInf();
        else
            result = qSNan();
    }

    return QScriptValue(eng, result);
}

QScriptValue Global::method_parseFloat(QScriptContext *context,
                                       QScriptEngine *eng)
{
    if (context->argumentCount() == 0)
        return QScriptValue(eng, qSNan());

    QString str = context->argument(0).toString().trimmed();
    bool ok = false;
    qsreal result = str.toFloat(&ok);
    if (!ok) {
        if (str == QLatin1String("Infinity"))
            result = +qInf();
        else if (str == QLatin1String("+Infinity"))
            result = +qInf();
        else if (str == QLatin1String("-Infinity"))
            result = -qInf();
        else if (str.isEmpty())
            result = qSNan();
        else if (str.at(0).isNumber())
            result = 0;
        else
            result = qSNan();
    }

    return QScriptValue(eng, result);
}

QScriptValue Global::method_isNaN(QScriptContext *context,
                                  QScriptEngine *eng)
{
    qsreal v = qSNan();
    if (context->argumentCount() > 0)
        v = context->argument(0).toNumber();
    return (QScriptValue(eng, qIsNan(v)));
}

QScriptValue Global::method_isFinite(QScriptContext *context,
                                     QScriptEngine *eng)
{
    qsreal v = qInf();
    if (context->argumentCount() > 0)
        v = context->argument(0).toNumber();
    return (QScriptValue(eng, qIsFinite(v)));
}

QScriptValue Global::method_decodeURI(QScriptContext *context,
                                      QScriptEngine *eng)
{
    QScriptValue result;

    if (context->argumentCount() > 0) {
        QString str = context->argument(0).toString();
        bool ok;
        QString out = decode(str, QString::fromUtf8(uriReserved) + QString::fromUtf8("#"), &ok);
        if (ok)
            return QScriptValue(eng, out);
        else
            return context->throwError(QScriptContext::URIError,
                                         QLatin1String("malformed URI sequence"));
    }

    return eng->undefinedValue();
}

QScriptValue Global::method_decodeURIComponent(QScriptContext *context,
                                               QScriptEngine *eng)
{
    QScriptValue result;
    if (context->argumentCount() > 0) {
        QString str = context->argument(0).toString();
        bool ok;
        QString out = decode(str, QString::fromUtf8(""), &ok);
        if (ok)
            result = QScriptValue(eng, out);
        else
            result = context->throwError(QScriptContext::URIError,
                                         QLatin1String("malformed URI sequence"));
    } else {
        result = eng->undefinedValue();
    }
    return result;
}

QScriptValue Global::method_encodeURI(QScriptContext *context,
                                      QScriptEngine *eng)
{
    QScriptValue result;
    if (context->argumentCount() > 0) {
        QString str = context->argument(0).toString();
        bool ok;
        QString out = encode(str,
                             QLatin1String(uriReserved)
                             + QLatin1String(uriUnescaped)
                             + QString::fromUtf8("#"),
                             &ok);
        if (ok)
            result = QScriptValue(eng, out);
        else
            result = context->throwError(QScriptContext::URIError,
                                         QLatin1String("malformed URI sequence"));
    } else {
        result = eng->undefinedValue();
    }
    return result;
}

QScriptValue Global::method_encodeURIComponent(QScriptContext *context,
                                               QScriptEngine *eng)
{
    QScriptValue result;
    if (context->argumentCount() > 0) {
        QString str = context->argument(0).toString();
        bool ok;
        QString out = encode(str, QLatin1String(uriUnescaped), &ok);
        if (ok)
            result = QScriptValue(eng, out);
        else
            result = context->throwError(QScriptContext::URIError,
                                         QLatin1String("malformed URI sequence"));
    } else {
        result = eng->undefinedValue();
    }
    return result;
}

QScriptValue Global::method_escape(QScriptContext *context,
                                   QScriptEngine *eng)
{
    if (context->argumentCount() > 0) {
        QString str = context->argument(0).toString();
        return QScriptValue(eng, QLatin1String(escape(str)));
    }
    return QScriptValue(eng, QLatin1String("undefined"));
}

QScriptValue Global::method_unescape(QScriptContext *context,
                                     QScriptEngine *eng)
{
    if (context->argumentCount() > 0) {
        QByteArray data = context->argument(0).toString().toLatin1();
        return QScriptValue(eng, unescape(data));
    }
    return QScriptValue(eng, QLatin1String("undefined"));
}

QScriptValue Global::method_version(QScriptContext *,
                                    QScriptEngine *eng)
{
    return (QScriptValue(eng, 1));
}

QScriptValue Global::method_gc(QScriptContext *,
                               QScriptEngine *eng)
{
    QScriptEnginePrivate *envp = QScriptEnginePrivate::get(eng);
    envp->maybeGC_helper(true);
    return QScriptValue(eng, envp->objectAllocator.freeBlocks());
}


} } // namespace QScript::Ecma

