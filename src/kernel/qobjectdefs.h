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
#include "qstring.h"
#endif // QT_H

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
    virtual void *qt_metacast(const char *); \
    static inline QString tr(const char *s, const char *c = 0) \
	{ return staticMetaObject.tr(s, c); } \
    static inline QString trUtf8(const char *s, const char *c = 0) \
	{ return staticMetaObject.trUtf8(s, c); } \
    virtual int qt_metacall(int _f, int _id, void **_o); \
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

    int	numSlots(bool super = false) const;
    int	numSignals(bool super = false) const;
    int	numEnumerators(bool super = false) const;
    int	numProperties(bool super = false) const;
    int	numClassInfo(bool super = false) const;

    int	findSlot(const char *slot, bool super = false) const;
    int	findSignal(const char *signal, bool super = false) const;
    int findEnumerator(const char *name, bool super = false) const;
    int findProperty(const char *name, bool super = false) const;
    int findClassInfo(const char *name, bool super = false) const;

    QMetaMember slot(int index, bool super = false) const;
    QMetaMember signal(int index, bool super = false) const;
    QMetaEnum enumerator(int index, bool super = false) const;
    QMetaProperty property(int index, bool super = false) const;
    QMetaClassInfo classInfo(int index, bool super = false) const;

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

#ifndef QT_NO_COMPAT
    const char *superClassName() const;
    bool inherits(const char* classname) const;
#endif


    struct { // private data
	const QMetaObject *superdata;
	const char *stringdata;
	const int *data;
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

//### TODO: add interface support using metacast. Also use proper
//### partial template specialization on systems that support it to
//### get nicer error messages.
template <class Type>
Q_INLINE_TEMPLATES Type qt_cast(const QObject *object)
{ Type t = 0; return (Type) t->staticMetaObject.cast(object); }


#endif // QOBJECTDEFS_H
