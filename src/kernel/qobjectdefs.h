/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qobjectdefs.h#24 $
**
** Macros and definitions related to QObject
**
** Created : 930419
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** This file contains ugly macros and definitions that are necessary to make
** the QObjects work.
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

/* tmake ignore Q_OBJECT */
#define Q_OBJECT							      \
public:									      \
    QMetaObject *metaObject() const { return metaObj; }			      \
    const char  *className()  const;					      \
protected:								      \
    void	 initMetaObject();					      \
private:								      \
    static QMetaObject *metaObj;

/* tmake ignore Q_OBJECT */
#define Q_OBJECT_FAKE Q_OBJECT
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

class QListM_QConnection;			// connection list
#define QConnectionList	  QListM_QConnection
#define QConnectionListIt QListIteratorM_QConnection

class QDictM_QListM_QConnection;		// signal dictionary
#define QSignalDict   QDictM_QListM_QConnection
#define QSignalDictIt QDictIteratorM_QListM_QConnection

class QListM_QObject;				// object list
#define QObjectList QListM_QObject

class QDictM_QMetaData;				// meta object dictionaries
#define QMemberDict QDictM_QMetaData


#endif // QOBJECTDEFS_H
