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

#ifndef QSCRIPTVALUEIMPL_P_H
#define QSCRIPTVALUEIMPL_P_H

#include "qscriptclassinfo_p.h"
#include "qscriptvalue.h"

#include <QtCore/qstring.h>
#include <QtCore/qlist.h>
#include <QtCore/qshareddata.h>

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

class QScriptValueImpl;
typedef QList<QScriptValueImpl> QScriptValueImplList;

class QScriptObject;
class QScriptObjectData;
class QScriptNameIdImpl;
class QScriptClassInfo;
class QScriptFunction;

class QScriptValueImpl
{
public:
    inline QScriptValueImpl() : m_class(0) {}
    QScriptValueImpl(QScriptEnginePrivate *engine, QScriptValue::SpecialValue val);
    QScriptValueImpl(QScriptEnginePrivate *engine, bool val);
    QScriptValueImpl(QScriptEnginePrivate *engine, int val);
    QScriptValueImpl(QScriptEnginePrivate *engine, uint val);
    QScriptValueImpl(QScriptEnginePrivate *engine, qlonglong val);
    QScriptValueImpl(QScriptEnginePrivate *engine, qulonglong val);
    QScriptValueImpl(QScriptEnginePrivate *engine, qsreal val);
    QScriptValueImpl(QScriptEnginePrivate *engine, const QString &val);

#ifndef QT_NO_CAST_FROM_ASCII
    QT_ASCII_CAST_WARN_CONSTRUCTOR QScriptValueImpl(QScriptEnginePrivate *engine, const char *val);
#endif

    inline QScript::Type type() const
    {
        Q_ASSERT(isValid());
        return m_class->type();
    }

    inline QScriptEngine *engine() const
    {
        if (! m_class)
            return 0;
        return m_class->engine();
    }

    inline QScriptClassInfo *classInfo() const
    { return m_class; }

    inline void setClassInfo(QScriptClassInfo *cls)
    { m_class = cls; }

    inline QScriptNameIdImpl *stringValue() const
    {
        Q_ASSERT(isString());
        return m_string_value;
    }

    inline QScriptObject *objectValue() const
    {
        Q_ASSERT(isObject());
        return m_object_value;
    }

    inline void incr() {
        ++m_number_value;
    }

    inline void decr() {
        --m_number_value;
    }

    inline void invalidate()
    { m_class = 0; }

    inline bool isValid() const
    { return m_class && m_class->engine(); }

    inline bool isBoolean() const
    { return m_class && m_class->type() == QScript::BooleanType; }

    inline bool isNumber() const
    { return m_class && m_class->type() == QScript::NumberType; }

    inline bool isString() const
    { return m_class && m_class->type() == QScript::StringType; }

    inline bool isFunction() const
    { return m_class && (m_class->type() & QScript::FunctionBased); }

    inline bool isObject() const
    { return m_class && (m_class->type() & QScript::ObjectBased); }

    inline bool isUndefined() const
    { return m_class && m_class->type() == QScript::UndefinedType; }

    inline bool isNull() const
    { return m_class && m_class->type() == QScript::NullType; }

    inline bool isVariant() const
    { return m_class && m_class->type() == QScript::VariantType; }

    inline bool isQObject() const
    { return m_class && m_class->type() == QScript::QObjectType; }

    inline bool isReference() const
    {
        Q_ASSERT(isValid());
        return m_class->type() == QScript::ReferenceType;
    }

    bool isError() const;
    bool isArray() const;
    bool isDate() const;
    bool isRegExp() const;

    QString toString() const;
    qsreal toNumber() const;
    bool toBoolean() const;
    qsreal toInteger() const;
    qint32 toInt32() const;
    quint32 toUInt32() const;
    quint16 toUInt16() const;
    QVariant toVariant() const;
    QObject *toQObject() const;
    QScriptValueImpl toObject() const;
    QDateTime toDateTime() const;
#ifndef QT_NO_REGEXP
    QRegExp toRegExp() const;
#endif
    QScriptValueImpl toPrimitive(QScriptValue::TypeHint hint = QScriptValue::NoTypeHint) const;

    QVariant variantValue() const;
    void setVariantValue(const QVariant &v);

    bool instanceOf(const QScriptValueImpl &ctor) const;

    QScriptValueImpl prototype() const;
    void setPrototype(const QScriptValueImpl &prototype);

    QScriptValueImpl property(QScriptNameIdImpl *nameId,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal) const;
    void setProperty(QScriptNameIdImpl *nameId, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = 0);

    QScriptValueImpl property(const QString &name,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal) const;
    void setProperty(const QString &name, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = 0);

    QScriptValueImpl property(quint32 arrayIndex,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal) const;
    void setProperty(quint32 arrayIndex, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = 0);

    QScriptValueImpl call(const QScriptValueImpl &thisObject = QScriptValueImpl(),
                          const QScriptValueImplList &args = QScriptValueImplList());
    QScriptValueImpl construct(const QScriptValueImplList &args = QScriptValueImplList());

    void mark(int) const;

    operator QScriptValue() const;

    QScriptValueImpl internalValue() const;
    void setInternalValue(const QScriptValueImpl &internalValue);

    void setQObjectValue(QObject *object);

    QExplicitlySharedDataPointer<QScriptObjectData> objectData() const;
    void setObjectData(QExplicitlySharedDataPointer<QScriptObjectData> data);

    void createMember(QScriptNameIdImpl *nameId,
                      QScript::Member *member, uint flags); // ### remove me
    int memberCount() const;
    void member(int index, QScript::Member *member) const;

    bool resolve(QScriptNameIdImpl *nameId, QScript::Member *member,
                 QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const;
    bool resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const;
    void get(const QScript::Member &member, QScriptValueImpl *obj) const;
    void get_helper(const QScript::Member &member, QScriptValueImpl *obj) const;
    void get(QScriptNameIdImpl *nameId, QScriptValueImpl *out);
    void put(const QScript::Member &member, const QScriptValueImpl &object);
    void removeMember(const QScript::Member &member);
   
    QScriptValueImpl scope() const;
    void setScope(const QScriptValueImpl &scope);

    QScriptFunction *toFunction() const;

    bool lessThan(const QScriptValueImpl &other) const;
    bool equalTo(const QScriptValueImpl &other) const;
    bool strictEqualTo(const QScriptValueImpl &other) const;

    bool detectedCycle() const;

    union {
        bool m_bool_value;
        int m_int_value;
        qsreal m_number_value;
        void *m_ptr_value;
        QScriptObject *m_object_value;
        QScriptNameIdImpl *m_string_value;
    };
    QScriptClassInfo *m_class;
};

#endif
