/****************************************************************************
**
** Definition of QMetaObject class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#ifndef QT_H
#include "qobjectdefs.h"
#endif // QT_H

class QCoreVariant;

class Q_CORE_EXPORT QMetaMember
{
public:
    inline QMetaMember():mobj(0),handle(0){}

    const char *signature() const;
    const char *parameters() const;
    const char *type() const;
    const char *tag() const;
    enum Access { Private, Protected, Public };
    Access access() const;
    enum Attributes { Compatability = 0x08, Cloned = 0x10 };
    int attributes() const;

private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};

class Q_CORE_EXPORT QMetaEnum
{
public:
    inline QMetaEnum():mobj(0),handle(0){}

    const char *name() const;
    bool isFlag() const;

    int numKeys() const;
    const char *key(int index) const;
    int value(int index) const;

    int keyToValue(const char *key) const;
    const char* valueToKey(int value) const;
    int keysToValue(const char * keys) const;
    QByteArray valueToKeys(int value) const;

    inline operator bool() const { return name() != 0; }
private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};

class Q_CORE_EXPORT QMetaProperty
{
public:
    QMetaProperty();

    const char *name() const;
    const char *type() const;

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
    inline operator bool() const { return isReadable(); }

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

class Q_CORE_EXPORT QMetaType {
public:
    enum {
        Void, VoidStar, Long, Int, Short,
        Char, ULong, UInt, UShort, UChar, Bool,
        Float, Double, QChar, QByteArray, QString,
        User = 256
    };

    typedef void (*Destructor)(void *);
    typedef void *(*CopyConstructor)(const void *);

    static int registerType(const char *typeName, Destructor destructor, CopyConstructor copyConstructor);
    static int type(const char *typeName);
    static const char *typeName(int type);
    static bool isRegistered(int type);
    static void *copy(int type, const void *data);
    static void destroy(int type, void *data);
};

template <typename T>
void qMetaTypeDeleteHelper(T *t)
{
    delete t;
}

template <typename T>
void *qMetaTypeCopyHelper(const T *t)
{
    if (!t)
        return 0;
    return new T(*static_cast<const T*>(t));
}

template <typename T>
static int qRegisterMetaType(const char *typeName, T* = 0)
{
    typedef void*(*CopyPtr)(const T*);
    CopyPtr cptr = qMetaTypeCopyHelper<T>;
    typedef void(*DeletePtr)(T*);
    DeletePtr dptr = qMetaTypeDeleteHelper<T>;

    return QMetaType::registerType(typeName, (QMetaType::Destructor)dptr,
                                   (QMetaType::CopyConstructor)cptr);
}

#endif // QMETAOBJECT_H
