/****************************************************************************
** $Id:  qt/qmetaobject.h   3.0.0-beta5   edited Sep 12 10:50 $
**
** Definition of QMetaObject class
**
** Created : 930419
**
** Copyright (C) 1992-2003 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QMETAOBJECT_H
#define QMETAOBJECT_H

#ifndef QT_H
#include "qobjectdefs.h"
#endif // QT_H

#ifndef Q_MOC_OUTPUT_REVISION
#define Q_MOC_OUTPUT_REVISION 41
#endif

class QVariant;

class QMetaMember
{
public:
    inline QMetaMember():mobj(0),handle(0){}

    const char *signature() const;
    const char *parameters() const;
    const char *type() const;
    enum Access { Private, Protected, Public };
    Access access() const;

private:
    const QMetaObject *mobj;
    int handle;
    friend struct QMetaObject;
};

class QMetaEnum
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
    int handle;
    friend struct QMetaObject;
};

class QMetaProperty
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

    bool isSetType() const;
    bool isEnumType() const;
    QMetaEnum enumerator() const;

    QVariant read(const QObject *obj) const;
    bool write(QObject *obj, const QVariant &value) const;
    bool reset(QObject *obj) const;

    bool hasStdCppSet() const;
    inline operator bool() const { return isReadable(); }

private:
    const QMetaObject *mobj[6];
    int idx[6];
    QMetaEnum menum;
    friend struct QMetaObject;
};

class QMetaClassInfo
{
public:
    inline QMetaClassInfo():mobj(0),handle(0){}
    const char *name() const;
    const char *value() const;
private:
    const QMetaObject *mobj;
    int handle;
    friend struct QMetaObject;
};


#endif // QMETAOBJECT_H
