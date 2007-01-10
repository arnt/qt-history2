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

typedef QList<QScriptValue> QScriptValueList;

typedef double qnumber;

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
        
        ObjectProperty      = 0x00000100,           // Stored in the member table
        NativeProperty      = 0x00000200,
        
        UserRange           = 0xffff0000            // Users may use these as they see fit.
    };
    Q_DECLARE_FLAGS(PropertyFlags, PropertyFlag)

public:
    inline QScriptValue()
        : m_class(0) {}

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
    qnumber toNumber() const;
    bool toBoolean() const;
    qnumber toInteger() const;
    qint32 toInt32() const;
    quint32 toUInt32() const;
    quint16 toUInt16() const;
    QVariant toVariant() const;
    QObject *toQObject() const;
    QScriptValue toObject() const;
    QScriptValue toPrimitive(TypeHint hint = NoTypeHint) const;

    void setVariantValue(const QVariant &v);

    bool instanceOf(const QScriptValue &ctor) const;

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

    void mark(int generation) const; // ### kill me

public: // ### private
    friend class QScriptValueImpl;
    inline QScriptValueImpl *impl() const
    { return const_cast<QScriptValueImpl*>(reinterpret_cast<const QScriptValueImpl*>(this)); }

private:
    union {
        bool m_bool_value;
        int m_int_value;
        qnumber m_number_value;
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
