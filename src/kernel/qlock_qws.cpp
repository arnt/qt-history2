/****************************************************************************
** $Id: $
**
** Definition of QLock class. This manages interprocess locking
**
** Created : 20000406
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qlock_qws.h"
#ifndef QT_NO_QWS_MULTIPROCESS
#include <sys/types.h>
#include <sys/ipc.h>
#include <errno.h>

#ifdef Q_OS_MACX
#define Q_NO_SEMAPHORE
#include <sys/stat.h>
#else
#include <sys/sem.h>
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED) \
    || defined (Q_OS_FREEBSD)
/* union semun is defined by including <sys/sem.h> */
#else
/* according to X/OPEN we have to define it ourselves */
union semun {
       int val;                    /* value for SETVAL */
       struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
       unsigned short int *array;  /* array for GETALL, SETALL */
       struct seminfo *__buf;      /* buffer for IPC_INFO */
};
#endif
#endif

#define MAX_LOCKS   200	    // maximum simultaneous read locks

#ifdef Q_NO_SEMAPHORE
#include <unistd.h>
#include <sys/file.h>
#endif

class QLockData
{
public:
#ifdef Q_NO_SEMAPHORE
    QCString file;
    bool created;
#endif
    int id;
    int count;
};

#endif

/*!
  \class QLock qlock_qws.h
  \ingroup qws
  \ingroup io
  \brief The QLock class is a wrapper round a System V shared semaphore.

  It is used by Qt/Embedded for synchronizing access to the graphics
  card and shared memory region between processes.
*/
/*! \enum QLock::Type

    \value Read
    \value Write
*/

/*!
  \fn QLock::QLock( const QString &filename, char id, bool create )
  Creates a lock. \a filename is the file path of the Unix-domain
  socket the Qt/Embedded client is using. \a id is the name of the
  particular lock to be created on that socket. If \a create is TRUE
  the lock is to be created (as the Qt/Embedded server does); if \a
  create is FALSE the lock should exist already (as the Qt/Embedded
  client expects).
*/

QLock::QLock( const QString &filename, char id, bool create )
{
#ifndef QT_NO_QWS_MULTIPROCESS
    data = new QLockData;
    data->count = 0;
#ifdef Q_NO_SEMAPHORE
    data->file = QString(filename+id).local8Bit();
    for(int x = 0; x < 2; x++) {
	data->id = open(data->file, O_RDWR | (x ? O_CREAT : 0), S_IRWXU);
	if(data->id != -1 || !create) {
	    data->created = x;
	    break;
	}
    }
#else
    int semkey = ftok(filename, id);
    data->id = semget(semkey,0,0);
    if ( create ) {
	semun arg; arg.val = 0;
	if ( data->id == -1 )
	    semctl(data->id,0,IPC_RMID,arg);
	data->id = semget(semkey,1,IPC_CREAT|0600);
	arg.val = MAX_LOCKS;
	semctl(data->id,0,SETVAL,arg);
    }
#endif
    if ( data->id == -1 ) {
	qWarning( "Cannot %s semaphore %s \'%c\'",
	    create ? "create" : "get", filename.latin1(), id );
	qDebug("Error %d %s\n",errno,strerror(errno));
    }
#endif
}

/*!
\fn QLock::~QLock()
Destroys a lock
*/

QLock::~QLock()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( locked() )
	unlock();
#ifdef Q_NO_SEMAPHORE
    if(isValid()) {
	close(data->id);
	if( data->created )
	    unlink( data->file );
    }
#endif
    delete data;
#endif
}

/*!
\fn bool QLock::isValid() const
Returns TRUE if the lock constructor was succesful; returns FALSE if
the lock could not be created or was not available to connect to.
*/

bool QLock::isValid() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    return (data->id != -1);
#else
    return TRUE;
#endif
}

/*!
  Locks the semaphore with a lock of type \a t. Locks can either be
  Read or Write. If a lock is Read, attempts to lock by other
  processes as Read will succeed, Write attempts will block until the
  lock is unlocked. If locked as Write, all attempts to lock by other
  processes will block until the lock is unlocked. Locks are stacked:
  i.e. a given QLock can be locked multiple times by the same
  process without blocking, and will only be unlocked after a
  corresponding number of unlock() calls.
*/

void QLock::lock( Type t )
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( !data->count ) {
#ifdef Q_NO_SEMAPHORE
	int op = LOCK_SH;
	if(t == Write) 
	    op = LOCK_EX;
	for( int rv=1; rv; ) {
	    rv = flock(data->id, op);
	    if (rv == -1 && errno != EINTR) 
		qDebug("Semop lock failure %s",strerror(errno));
	} 
#else
	sembuf sops;
	sops.sem_num = 0;
	sops.sem_flg = SEM_UNDO;

	if ( t == Write ) {
	    sops.sem_op = -MAX_LOCKS;
	    type = Write;
	} else {
	    sops.sem_op = -1;
	    type = Read;
	}

	int rv;
	do {
	    rv = semop(data->id,&sops,1);
	    if (rv == -1 && errno != EINTR)
		qDebug("Semop lock failure %s",strerror(errno));
	} while ( rv == -1 && errno == EINTR );
#endif
    }
    data->count++;
#endif
}

/*!
\fn void QLock::unlock()
Unlocks the semaphore. If other processes were blocking waiting to lock()
the semaphore, one of them will wake up and succeed in lock()ing.
*/

void QLock::unlock()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if( data->count ) {
	data->count--;
	if( !data->count ) {
#ifdef Q_NO_SEMAPHORE
	    for( int rv=1; rv; ) {
		rv = flock(data->id, LOCK_UN);
		if (rv == -1 && errno != EINTR) 
		    qDebug("Semop lock failure %s",strerror(errno));
	    }
#else
	    sembuf sops;
	    sops.sem_num = 0;
	    sops.sem_op = 1;
	    sops.sem_flg = SEM_UNDO;
	    if ( type == Write )
		sops.sem_op = MAX_LOCKS;

	    int rv;
	    do {
		rv = semop(data->id,&sops,1);
		if (rv == -1 && errno != EINTR)
		    qDebug("Semop unlock failure %s",strerror(errno));
	    } while ( rv == -1 && errno == EINTR );
#endif
	}
    } else {
	qDebug("Unlock without corresponding lock");
    }
#endif
}

/*!
\fn bool QLock::locked() const
Returns TRUE if the lock is currently held by the current process;
otherwise returns FALSE.
*/

bool QLock::locked() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    return (data->count > 0);
#else
    return FALSE;
#endif
}

