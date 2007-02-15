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

#ifndef QSCRIPTVALUEIMPLFWD_P_H
#define QSCRIPTVALUEIMPLFWD_P_H

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
    inline QScriptValueImpl();
    inline QScriptValueImpl(QScriptEnginePrivate *engine, QScriptValue::SpecialValue val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, bool val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, int val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, uint val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, qlonglong val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, qulonglong val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, qsreal val);
    inline QScriptValueImpl(QScriptEnginePrivate *engine, const QString &val);

    inline QScript::Type type() const;
    inline QScriptEngine *engine() const;
    inline QScriptClassInfo *classInfo() const;
    inline void setClassInfo(QScriptClassInfo *cls);
    inline QScriptNameIdImpl *stringValue() const;
    inline QScriptObject *objectValue() const;
    inline void incr();
    inline void decr();

    inline void invalidate();
    inline bool isValid() const;
    inline bool isBoolean() const;
    inline bool isNumber() const;
    inline bool isString() const;
    inline bool isFunction() const;
    inline bool isObject() const;
    inline bool isUndefined() const;
    inline bool isNull() const;
    inline bool isVariant() const;
    inline bool isQObject() const;
    inline bool isReference() const;

    inline bool isError() const;
    inline bool isArray() const;
    inline bool isDate() const;
    inline bool isRegExp() const;

    inline QString toString() const;
    inline qsreal toNumber() const;
    inline bool toBoolean() const;
    inline qsreal toInteger() const;
    inline qint32 toInt32() const;
    inline quint32 toUInt32() const;
    inline quint16 toUInt16() const;
    inline QVariant toVariant() const;
    inline QObject *toQObject() const;
    inline QScriptValueImpl toObject() const;
    inline QDateTime toDateTime() const;
#ifndef QT_NO_REGEXP
    inline QRegExp toRegExp() const;
#endif
    inline QScriptValueImpl toPrimitive(QScriptValue::TypeHint hint = QScriptValue::NoTypeHint) const;

    inline QVariant variantValue() const;
    inline void setVariantValue(const QVariant &v);

    bool instanceOf(const QScriptValueImpl &ctor) const;

    inline QScriptValueImpl prototype() const;
    inline void setPrototype(const QScriptValueImpl &prototype);

    inline QScriptValueImpl property(QScriptNameIdImpl *nameId,
                                     const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal) const;
    void setProperty(QScriptNameIdImpl *nameId, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = 0);

    inline QScriptValueImpl property(const QString &name,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal) const;
    inline void setProperty(const QString &name, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = 0);

    inline QScriptValueImpl property(quint32 arrayIndex,
                              const QScriptValue::ResolveFlags &mode = QScriptValue::ResolveLocal) const;
    inline void setProperty(quint32 arrayIndex, const QScriptValueImpl &value,
                     const QScriptValue::PropertyFlags &flags = 0);

    inline QScriptValueImpl call(const QScriptValueImpl &thisObject = QScriptValueImpl(),
                          const QScriptValueImplList &args = QScriptValueImplList());
    inline QScriptValueImpl construct(const QScriptValueImplList &args = QScriptValueImplList());

    inline void mark(int) const;

    inline operator QScriptValue() const;

    inline QScriptValueImpl internalValue() const;
    inline void setInternalValue(const QScriptValueImpl &internalValue);

    inline void setQObjectValue(QObject *object);

    inline QExplicitlySharedDataPointer<QScriptObjectData> objectData() const;
    inline void setObjectData(QExplicitlySharedDataPointer<QScriptObjectData> data);

    inline void createMember(QScriptNameIdImpl *nameId,
                      QScript::Member *member, uint flags); // ### remove me
    inline int memberCount() const;
    inline void member(int index, QScript::Member *member) const;

    inline bool resolve(QScriptNameIdImpl *nameId, QScript::Member *member,
                 QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const;
    bool resolve_helper(QScriptNameIdImpl *nameId, QScript::Member *member,
                        QScriptValueImpl *object, QScriptValue::ResolveFlags mode) const;
    inline void get(const QScript::Member &member, QScriptValueImpl *obj) const;
    inline void get_helper(const QScript::Member &member, QScriptValueImpl *obj) const;
    inline void get(QScriptNameIdImpl *nameId, QScriptValueImpl *out);
    inline void put(const QScript::Member &member, const QScriptValueImpl &object);
    inline void removeMember(const QScript::Member &member);
   
    inline QScriptValueImpl scope() const;
    inline void setScope(const QScriptValueImpl &scope);

    inline QScriptFunction *toFunction() const;

    inline bool lessThan(const QScriptValueImpl &other) const;
    inline bool equalTo(const QScriptValueImpl &other) const;
    inline bool strictEqualTo(const QScriptValueImpl &other) const;

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
