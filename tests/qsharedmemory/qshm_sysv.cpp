#include <qshm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

class QWSSharedMemoryPrivate 
{
public:
    bool idInitted; 
    key_t key; 
    int shmId;
}

QWSSharedMemory::QWSSharedMemory(int size, QString filename) 
{
    shmSize = size;
    shmFile = filename;
    shmBase = 0;
    d = new QWSSharedMemoryPrivate;
    d->key = ftok( shmFile.latin1(), 'a' );
    d->idInitted = FALSE;
    d->shmId = -1;
}

bool QWSSharedMemory::create () 
{
    if(d->key == FALSE)
       return FALSE;

    d->shmId = shmget( d->key, shmSize, IPC_CREAT|IPC_EXCL|0000);
    if ( d->shmId == -1 ) 
	return FALSE;
    d->idInitted = TRUE;
    return TRUE;
}

void QWSSharedMemory::destroy() 
{
    if (!idInitted) 
	d->shmId = shmget( d->key, shmSize, 0 );
    if (d->shmId == -1)
	return;

    struct shmid_ds shm;
    shmctl( d->shmId, IPC_RMID, &shm );
}

bool QWSSharedMemory::attach() 
{
    if (!idInitted) 
	d->shmId = shmget( d->key, shmSize, 0 );
    if (d->shmId == -1)
        return FALSE;

    shmBase = shmat(d->shmId, 0, 0);
    if ( (int)shmBase == -1 || shmBase == 0 )
	return FALSE;

    return TRUE;
}

void QWSSharedMemory::detach() 
{
    if(shmBase)
	shmdt(shmBase);
}
