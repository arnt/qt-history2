/****************************************************************************
**
** Definition of QMetaObject class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

#ifndef Q_MOC_OUTPUT_REVISION
#define Q_MOC_OUTPUT_REVISION 45
#endif

class QKernelVariant;

class Q_KERNEL_EXPORT QMetaMember
{
public:
    inline QMetaMember():mobj(0),handle(0){}

    const char *signature() const;
    const char *parameters() const;
    const char *type() const;
    const char *tag() const;
    enum Access { Private, Protected, Public };
    Access access() const;

private:
    const QMetaObject *mobj;
    uint handle;
    friend struct QMetaObject;
};

class Q_KERNEL_EXPORT QMetaEnum
{
public:
    inline QMetaEnum():mobj(0),handle(0){}

    const char *name() const;
    bool isSet() const;

    int numKeys() const;
    const char *key(int index) const;

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

class Q_KERNEL_EXPORT QMetaProperty
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

    bool isSetType() const;
    bool isEnumType() const;
    QMetaEnum enumerator() const;

    QKernelVariant read(const QObject *obj) const;
    bool write(QObject *obj, const QKernelVariant &value) const;
    bool reset(QObject *obj) const;

    bool hasStdCppSet() const;
    inline operator bool() const { return isReadable(); }

private:
    const QMetaObject *mobj[10];
    int idx[10];
    QMetaEnum menum;
    friend struct QMetaObject;
};

class Q_KERNEL_EXPORT QMetaClassInfo
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


#endif // QMETAOBJECT_H
