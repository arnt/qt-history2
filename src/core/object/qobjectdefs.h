/****************************************************************************
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

#ifndef QOBJECTDEFS_H
#define QOBJECTDEFS_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H

class QString;

class QByteArray;

// The following macros are our "extensions" to C++
// They are used, strictly speaking, only by the moc.

#ifndef QT_MOC_CPP
# define slots
# define signals protected
#ifndef QT_NO_EMIT
# define emit
#endif
#define Q_CLASSINFO( name, value )
#define Q_PROPERTY( text )
#define QDOC_PROPERTY( text )
#define Q_OVERRIDE( text )
#define Q_ENUMS( x )
#define Q_FLAGS( x )
#ifdef QT_COMPAT
# define Q_SETS( x )
#endif

#ifndef QT_NO_TRANSLATION
# ifndef QT_NO_TEXTCODEC
// full set of tr functions
#  define QT_TR_FUNCTIONS \
    static inline QString tr(const char *s, const char *c = 0) \
	{ return staticMetaObject.tr(s, c); } \
    static inline QString trUtf8(const char *s, const char *c = 0) \
	{ return staticMetaObject.trUtf8(s, c); }
# else
// no QTextCodec, no utf8
#  define QT_TR_FUNCTIONS \
    static inline QString tr(const char *s, const char *c = 0) \
	{ return staticMetaObject.tr(s, c); }
# endif
#else
// inherit the ones from QObject
# define QT_TR_FUNCTIONS
#endif

/* tmake ignore Q_OBJECT */
#define Q_OBJECT \
public: \
    virtual const QMetaObject *metaObject() const; \
    static const QMetaObject staticMetaObject; \
    virtual void *qt_metacast(const char *) const; \
    QT_TR_FUNCTIONS \
    virtual int qt_metacall(QMetaObject::Call, int, void **); \
private:
/* tmake ignore Q_OBJECT */
#define Q_OBJECT_FAKE Q_OBJECT
#else
#define slots slots
#define signals signals
#define Q_CLASSINFO( name, value ) Q_CLASSINFO( name, value )
#define Q_PROPERTY( text ) Q_PROPERTY( text )
#define QDOC_PROPERTY( text ) QDOC_PROPERTY( text )
#define Q_OVERRIDE( text ) Q_OVERRIDE( text )
#define Q_ENUMS( x ) Q_ENUMS( x )
#define Q_FLAGS( x ) Q_FLAGS( x )
#ifdef QT_COMPAT
# define Q_SETS( x ) Q_SETS( x )
#endif
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

#define METHOD(a)	"0"#a
#define SLOT(a)		"1"#a
#define SIGNAL(a)	"2"#a

#ifdef QT_COMPAT
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

struct Q_CORE_EXPORT QMetaObject
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

    int	slotCount() const;
    int	signalCount() const;
    int	enumeratorCount() const;
    int	propertyCount() const;
    int	classInfoCount() const;

    int	indexOfSlot(const char *slot) const;
    int	indexOfSignal(const char *signal) const;
    int indexOfEnumerator(const char *name) const;
    int indexOfProperty(const char *name) const;
    int indexOfClassInfo(const char *name) const;

    QMetaMember slot(int index) const;
    QMetaMember signal(int index) const;
    QMetaEnum enumerator(int index) const;
    QMetaProperty property(int index) const;
    QMetaClassInfo classInfo(int index) const;

    static bool checkConnectArgs(const char *signal, const char *member);
    static QByteArray normalizedSignature(const char *member);

    // internal index-based connect
    static bool connect(const QObject *sender,
			int signal_index,
			const QObject *receiver,
			int membcode, int member_index,
			int type = 0, int *types = 0);
    // internal index-based disconnect
    static bool disconnect(const QObject *sender,
			   int signal_index,
			   const QObject *receiver,
			   int membcode, int member_index);
    // internal slot-name based connect
    static void connectSlotsByName(const QObject *o);

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

#ifdef QT_COMPAT
    QT_COMPAT const char *superClassName() const;
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

#ifdef QT_COMPAT
inline const char *QMetaObject::superClassName() const
{ return d.superdata ? d.superdata->className() : 0; }
#endif

#define Q_DECLARE_PRIVATE(Class) \
    inline Class##Private* d_func() { return (Class##Private *)d_ptr; } \
    inline const Class##Private* d_func() const { return (const Class##Private *)d_ptr; } \
    inline Class* q_func() { return this; } \
    inline const Class* q_func() const { return this; } \
    friend class Class##Private

#endif // QOBJECTDEFS_H
