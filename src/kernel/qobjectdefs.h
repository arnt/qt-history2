/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectdefs.h#32 $
**
** Macros and definitions related to QObject
**
** Created : 930419
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QOBJECTDEFS_H
#define QOBJECTDEFS_H

#ifndef QT_H
#include "qglobal.h"
#endif // QT_H


// The following macros are our "extensions" to C++
// They are used, strictly speaking, only by the moc.

#define slots					// slots:   in class
#define signals protected			// signals: in class
#define emit					// emit signal

#ifdef Q_PROPS
#define q_prop
#define q_properties public
#endif

/* tmake ignore Q_OBJECT */
#define Q_OBJECT							\
public:									\
    QMetaObject *metaObject() const { return metaObj; }			\
    const char  *className()  const;					\
    static QMetaObject *staticMetaObject();				\
    static QString tr(const char*);					\
protected:								\
    void	 initMetaObject();					\
private:								\
    static QMetaObject *metaObj;

/* tmake ignore Q_OBJECT */
#define Q_OBJECT_FAKE Q_OBJECT

#define Q_BUILDER( comment, pixmap, inspector )				\
private:								\
    static QObject* factory( QWidget* );

						// macro for naming members
#if defined(_OLD_CPP_)
#define METHOD(a)	"0""a"
#define SLOT(a)		"1""a"
#define SIGNAL(a)	"2""a"
#else
#define METHOD(a)	"0"#a
#define SLOT(a)		"1"#a
#define SIGNAL(a)	"2"#a
#endif

#define METHOD_CODE	0			// member type codes
#define SLOT_CODE	1
#define SIGNAL_CODE	2


// Forward declarations so you don't have to include files you don't need

class QObject;
class QMetaObject;
class QSignal;
class QConnection;
class QEvent;
struct QMetaData;
class QConnectionList;
class QConnectionListIt;
class QSignalDict;
class QSignalDictIt;
class QObjectList;
class QObjectListIt;
class QMemberDict;


#endif // QOBJECTDEFS_H
