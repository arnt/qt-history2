/****************************************************************************
** $Id$
**
** Macros and definitions related to QObject
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

#ifndef QOBJECTDEFS_H
#define QOBJECTDEFS_H

#ifndef QT_H
#include "qglobal.h"
#endif
class QString;
class QByteArray;

// The following macros are our "extensions" to C++
// They are used, strictly speaking, only by the moc.

#ifndef QT_MOC_CPP
#define slots
#define signals protected
#ifndef QT_NO_EMIT
#define emit
#endif
#define Q_CLASSINFO( name, value )
#define Q_PROPERTY( text )
#define Q_OVERRIDE( text )
#define Q_ENUMS( x )
#define Q_SETS( x )
/* tmake ignore Q_OBJECT */
#define Q_OBJECT \
public: \
    virtual const QMetaObject *metaObject() const; \
    static const QMetaObject staticMetaObject; \
    virtual void *qt_metacast(const char *) const; \
    static inline QString tr(const char *s, const char *c = 0) \
	{ return staticMetaObject.tr(s, c); } \
    static inline QString trUtf8(const char *s, const char *c = 0) \
	{ return staticMetaObject.trUtf8(s, c); } \
    virtual int qt_metacall(QMetaObject::Call _c, int _id, void **_o); \
private:
/* tmake ignore Q_OBJECT */
#define Q_OBJECT_FAKE Q_OBJECT
#else
#define slots slots
#define signals signals
#define Q_CLASSINFO( name, value ) Q_CLASSINFO( name, value )
#define Q_PROPERTY( text ) Q_PROPERTY( text )
#define Q_OVERRIDE( text ) Q_OVERRIDE( text )
#define Q_ENUMS( x ) Q_ENUMS( x )
#define Q_SETS( x ) Q_SETS( x )
 /* tmake ignore Q_OBJECT */
#define Q_OBJECT Q_OBJECT
 /* tmake ignore Q_OBJECT */
#define Q_OBJECT_FAKE Q_OBJECT_FAKE
#endif

// macro for naming members
#ifdef METHOD
#undef METHOD
#endif
#ifdef SLOT
#undef SLOT
#endif
#ifdef SIGNAL
#undef SIGNAL
#endif

#if defined(_OLD_CPP_)
#define METHOD(a)	"0""a"
#define SLOT(a)		"1""a"
#define SIGNAL(a)	"2""a"
#else
#define METHOD(a)	"0"#a
#define SLOT(a)		"1"#a
#define SIGNAL(a)	"2"#a
#endif

#ifndef QT_CLEAN_NAMESPACE
#define METHOD_CODE	0			// member type codes
#define SLOT_CODE	1
#define SIGNAL_CODE	2
#endif

#define QMETHOD_CODE	0			// member type codes
#define QSLOT_CODE	1
#define QSIGNAL_CODE	2

class QObject;
class QMetaMember;
class QMetaEnum;
class QMetaProperty;
class QMetaClassInfo;

struct QMetaObject
{
    const char *className() const;
    const QMetaObject *superClass() const;

    QObject *cast(const QObject *obj) const;

#ifndef QT_NO_TRANSLATION
    QString tr(const char *s, const char *c) const;
    QString trUtf8(const char *s, const char *c) const;
#endif // QT_NO_TRANSLATION

    int	slotOffset() const;
    int	signalOffset() const;
    int	enumeratorOffset() const;
    int	propertyOffset() const;
    int	classInfoOffset() const;

    int	numSlots() const;
    int	numSignals() const;
    int	numEnumerators() const;
    int	numProperties() const;
    int	numClassInfo() const;

    int	findSlot(const char *slot) const;
    int	findSignal(const char *signal) const;
    int findEnumerator(const char *name) const;
    int findProperty(const char *name) const;
    int findClassInfo(const char *name) const;

    QMetaMember slot(int index) const;
    QMetaMember signal(int index) const;
    QMetaEnum enumerator(int index) const;
    QMetaProperty property(int index) const;
    QMetaClassInfo classInfo(int index) const;

    static bool checkConnectArgs(const char *signal, const char *member);
    static QByteArray normalizeSignature(const char *member);

    // internal index-based connect
    static bool connect(const QObject *sender,
			int signal_index,
			const QObject *receiver,
			int membcode, int member_index);
    // internal index-based disconnect
    static bool disconnect(const QObject *sender,
			   int signal_index,
			   const QObject *receiver,
			   int membcode, int member_index);
    // internal index-based signal activation
    static void activate(QObject *obj, int signal_index, void **argv);
    static void activate(QObject *obj, const QMetaObject *, int local_signal_index, void **argv);
    // internal guarded pointers
    static void addGuard(QObject **ptr);
    static void removeGuard(QObject **ptr);
    static void changeGuard(QObject **ptr, QObject *o);

    enum Call {
	InvokeSlot = QSLOT_CODE,
	EmitSignal = QSIGNAL_CODE,
	ReadProperty,
	WriteProperty,
	ResetProperty,
	QueryPropertyDesignable,
	QueryPropertyScriptable,
	QueryPropertyStored,
	QueryPropertyEditable
    };

#ifndef QT_NO_COMPAT
    const char *superClassName() const;
    bool inherits(const char* classname) const;
#endif

    struct { // private data
	const QMetaObject *superdata;
	const char *stringdata;
	const uint *data;
    } d;
};

inline const char *QMetaObject::className() const
{ return d.stringdata; }

inline const QMetaObject *QMetaObject::superClass() const
{ return d.superdata; }

#ifndef QT_NO_COMPAT
inline const char *QMetaObject::superClassName() const
{ return d.superdata ? d.superdata->className() : 0; }
#endif

#endif // QOBJECTDEFS_H
