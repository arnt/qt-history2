/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qlock_qws.cpp $
**
** Definition of QLock class. This manages interprocess locking
**
** Created : 20000406
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include "qlock_qws.h"
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

QLock::QLock( const QString &filename, char id, bool create )
{
    data = new QLockData;
    data->count = 0;
    int semkey = ftok(filename, id);
    data->id = semget(semkey,0,0);
    if ( create ) {
	semun arg; arg.val = 0;
	if ( data->id == -1 )
	    semctl(data->id,0,IPC_RMID,arg);
	data->id = semget(semkey,1,IPC_CREAT|0666);
	arg.val = MAX_LOCKS;
	semctl(data->id,0,SETVAL,arg);
    }
    if ( data->id == -1 ) {
	qWarning( "Cannot %s semaphore %s \'%c\'",
	    create ? "create" : "get", filename.latin1(), id );
	qDebug("Error %d %s\n",errno,strerror(errno));
    }
}

QLock::~QLock()
{
    if ( locked() )
	unlock();
    delete data;
}

bool QLock::isValid() const
{
    return (data->id != -1);
}

void QLock::lock( Type t )
{
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
		qDebug("Semop unlock failure %s",strerror(errno));
	} while ( rv == -1 && errno == EINTR );
    }
    data->count++;
}

void QLock::unlock()
{
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
}

bool QLock::locked() const
{
    return (data->count > 0);
}

