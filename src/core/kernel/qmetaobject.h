/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#include "qobjectdefs.h"
#include "qcorevariant.h"

template <typename T> class QList;

class Q_CORE_EXPORT QMetaMember
{
public:
    inline QMetaMember():mobj(0),handle(0){}

    const char *signature() const;
    const char *typeName() const;
    QList<QByteArray> parameterTypes() const;
    QList<QByteArray> parameterNames() const;
    const char *tag() const;
    enum Access { Private, Protected, Public };
    Access access() const;
    enum MemberType { Method, Signal, Slot };
    MemberType memberType() const;
    enum Attributes { Compatibility = 0x1, Cloned = 0x2, Scriptable = 0x4 };
    int attributes() const;

private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};
Q_DECLARE_TYPEINFO(QMetaMember, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QMetaEnum
{
public:
    inline QMetaEnum():mobj(0),handle(0){}

    const char *name() const;
    bool isFlag() const;

    int keyCount() const;
    const char *key(int index) const;
    int value(int index) const;

    const char *scope() const;

    int keyToValue(const char *key) const;
    const char* valueToKey(int value) const;
    int keysToValue(const char * keys) const;
    QByteArray valueToKeys(int value) const;


    inline bool isValid() const { return name() != 0; }
private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};
Q_DECLARE_TYPEINFO(QMetaEnum, Q_MOVABLE_TYPE);

class Q_CORE_EXPORT QMetaProperty
{
public:
    QMetaProperty();

    const char *name() const;
    const char *typeName() const;
    QCoreVariant::Type type() const;

    bool isReadable() const;
    bool isWritable() const;
    bool isDesignable(const QObject *obj = 0) const;
    bool isScriptable(const QObject *obj = 0) const;
    bool isStored(const QObject *obj = 0) const;
    bool isEditable(const QObject *obj = 0) const;

    bool isFlagType() const;
    bool isEnumType() const;
    QMetaEnum enumerator() const;

    QCoreVariant read(const QObject *obj) const;
    bool write(QObject *obj, const QCoreVariant &value) const;
    bool reset(QObject *obj) const;

    bool hasStdCppSet() const;
    inline bool isValid() const { return isReadable(); }

private:
    const QMetaObject *mobj[10];
    int idx[10];
    QMetaEnum menum;
    friend struct QMetaObject;
};

class Q_CORE_EXPORT QMetaClassInfo
{
public:
    inline QMetaClassInfo():mobj(0),handle(0){}
    const char *name() const;
    const char *value() const;
private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};
Q_DECLARE_TYPEINFO(QMetaMember, Q_MOVABLE_TYPE);

#endif // QMETAOBJECT_H
