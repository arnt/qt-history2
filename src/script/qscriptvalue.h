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

#ifndef QSCRIPTVALUE_H
#define QSCRIPTVALUE_H

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>

QT_BEGIN_HEADER

QT_MODULE(Script)

class QScriptValue;
class QScriptEngine;
class QScriptObject;
class QScriptNameIdImpl;
class QScriptNameId;
class QScriptClassInfo;
class QVariant;
class QObject;
class QDateTime;
#ifndef QT_NO_REGEXP
class QRegExp;
#endif

typedef QList<QScriptValue> QScriptValueList;

typedef double qsreal;

class QScriptValueImpl;

class Q_SCRIPT_EXPORT QScriptValue
{
public:
    enum TypeHint {
        NoTypeHint,
        NumberTypeHint,
        StringTypeHint
    };

    enum ResolveFlag {
        ResolveLocal        = 0x00,
        ResolvePrototype    = 0x01,
        ResolveScope        = 0x02,
        ResolveFull         = ResolvePrototype | ResolveScope
    };

    Q_DECLARE_FLAGS(ResolveFlags, ResolveFlag)

    enum PropertyFlag {
        ReadOnly            = 0x00000001,
        Undeletable         = 0x00000002,
        SkipInEnumeration   = 0x00000004,

        UninitializedConst  = 0x00000080,

        ObjectProperty      = 0x00000100,           // Stored in the member table
        NativeProperty      = 0x00000200,

        PropertyGetter      = 0x00000400,
        PropertySetter      = 0x00000800,

        UserRange           = 0xffff0000            // Users may use these as they see fit.
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

    enum SpecialValue {
        NullValue,
        UndefinedValue,
    };

public:
    inline QScriptValue()
        : m_class(0) {}

    QScriptValue(QScriptEngine *engine, SpecialValue val);
    QScriptValue(QScriptEngine *engine, bool val);
    QScriptValue(QScriptEngine *engine, int val);
    QScriptValue(QScriptEngine *engine, uint val);
    QScriptValue(QScriptEngine *engine, qlonglong val);
    QScriptValue(QScriptEngine *engine, qulonglong val);
    QScriptValue(QScriptEngine *engine, qsreal val);
    QScriptValue(QScriptEngine *engine, const QString &val);

#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN_CONSTRUCTOR QScriptValue(QScriptEngine *engine, const char *val);
#endif

    bool isValid() const;
    void invalidate();

    QScriptEngine *engine() const;

    bool isBoolean() const;
    bool isNumber() const;
    bool isFunction() const;
    bool isNull() const;
    bool isString() const;
    bool isUndefined() const;
    bool isVariant() const;
    bool isQObject() const;
    bool isObject() const;
    bool isDate() const;
    bool isRegExp() const;
    bool isArray() const;
    bool isError() const;

    QString toString() const;
    qsreal toNumber() const;
    bool toBoolean() const;
    qsreal toInteger() const;
    qint32 toInt32() const;
    quint32 toUInt32() const;
    quint16 toUInt16() const;
    QVariant toVariant() const;
    QObject *toQObject() const;
    QScriptValue toObject() const;
    QDateTime toDateTime() const;
#ifndef QT_NO_REGEXP
    QRegExp toRegExp() const;
#endif
    QScriptValue toPrimitive(TypeHint hint = NoTypeHint) const;

    void setVariantValue(const QVariant &v);

    bool instanceOf(const QScriptValue &ctor) const;

    bool lessThan(const QScriptValue &other) const;
    bool equalTo(const QScriptValue &other) const;
    bool strictEqualTo(const QScriptValue &other) const;

    QScriptValue prototype() const;
    void setPrototype(const QScriptValue &prototype);

    QScriptValue property(const QScriptNameId &nameId,
                          const ResolveFlags &mode = ResolveLocal) const;
    void setProperty(const QScriptNameId &nameId, const QScriptValue &value,
                     const PropertyFlags &flags = 0);

    QScriptValue property(const QString &name,
                          const ResolveFlags &mode = ResolveLocal) const;
    void setProperty(const QString &name, const QScriptValue &value,
                     const PropertyFlags &flags = 0);

    QScriptValue property(quint32 arrayIndex,
                          const ResolveFlags &mode = ResolveLocal) const;
    void setProperty(quint32 arrayIndex, const QScriptValue &value,
                     const PropertyFlags &flags = 0);

    QScriptValue call(const QScriptValue &thisObject = QScriptValue(),
                      const QScriptValueList &args = QScriptValueList());

    void ref() const;
    void deref() const;

    void mark(int generation) const; // ### kill me

private:
    friend class QScriptValueImpl;
    inline QScriptValueImpl *impl() const
    { return const_cast<QScriptValueImpl*>(reinterpret_cast<const QScriptValueImpl*>(this)); }

private:
    union {
        bool m_bool_value;
        int m_int_value;
        qsreal m_number_value;
        void *m_ptr_value;
        QScriptObject *m_object_value;
        QScriptNameIdImpl *m_string_value;
    };
    QScriptClassInfo *m_class;

    friend class QScriptContextPrivate;
    friend class QScriptEnginePrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptValue::ResolveFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(QScriptValue::PropertyFlags)

QT_END_HEADER

#endif
