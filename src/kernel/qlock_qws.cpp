/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlock_qws.cpp $
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
#include <sys/sem.h>
#include <sys/ipc.h>
#include <errno.h>

#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
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

#define MAX_LOCKS   200	    // maximum simultaneous read locks

class QLockData
{
public:
    int id;
    int count;
};

#endif

QLock::QLock( const QString &filename, char id, bool create )
{
#ifndef QT_NO_QWS_MULTIPROCESS
    data = new QLockData;
    data->count = 0;
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
    if ( data->id == -1 ) {
	qWarning( "Cannot %s semaphore %s \'%c\'",
	    create ? "create" : "get", filename.latin1(), id );
	qDebug("Error %d %s\n",errno,strerror(errno));
    }
#endif
}

QLock::~QLock()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if ( locked() )
	unlock();
    delete data;
#endif
}

bool QLock::isValid() const
{
#ifndef QT_NO_QWS_MULTIPROCESS    
    return (data->id != -1);
#else
    return TRUE;
#endif
}

void QLock::lock( Type t )
{
#ifndef QT_NO_QWS_MULTIPROCESS    
    if ( !data->count ) {
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
    }
    data->count++;
#endif
}

void QLock::unlock()
{
#ifndef QT_NO_QWS_MULTIPROCESS    
    if ( data->count ) {
	data->count--;
	if ( !data->count ) {
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
	}
	return;
    } else {
	qDebug("Unlock without corresponding lock");
    }
#endif
}

bool QLock::locked() const
{
#ifndef QT_NO_QWS_MULTIPROCESS
    return (data->count > 0);
#else
    return FALSE;
#endif    
}

