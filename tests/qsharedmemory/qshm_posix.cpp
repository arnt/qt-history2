#include <qshm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

class QWSSharedMemoryPrivate 
{
public:
    int shmFD;
    int connected;
}

QWSSharedMemory::QWSSharedMemory(int size, QString filename) 
{
    shmSize = size;
    shmFile = filename;
    shmBase = 0;
    d = new QWSSharedMemoryPrivate;
    d->shmFD = -1;
    d->connected = FALSE;
}

QWSSharedMemory::~QWSharedMemory() {}

bool QWSSharedMemory::create () 
{
    d->shmFD = shm_open( shmFile.latin1(), O_RDWR|O_EXCL|O_CREAT , 0666 );
    if (d->shmFD == -1)
	return FALSE;
    else if (ftruncate( d->shmFD, shmSize ) == -1) {
	close(d->shmFD);
	return FALSE;
    }
    return TRUE;
}

void QWSSharedMemory::destroy() 
{
    shm_unlink(shmFile.latin1());
}

bool QWSSharedMemory::attach () 
{
    shmBase = mmap( 0, shmSize , PROT_READ|PROT_WRITE, 
		   MAP_SHARED, d->shmFD, 0 );

    if (shmBase == MAP_FAILED)
	return FALSE;

    close(d->shmFD);
    return TRUE;
}

void QWSSharedMemory::detach () 
{
    munmap( shmBase, shmSize );
}

/*! 
    Returns TRUE if the memory object described already been created by 
    some process, otherwise returns FALSE 
*/
bool QWSSharedMemory::exists() 
{
    /* if we already have it open, of course it exists */
    if(d->connected)
	return TRUE;
 
    int dummy_desc = shm_open( shmFile.latin1(), O_RDWR, 0666 );
    if ((dummy_desc == -1) && (errno == ENOENT)) {
	/* file does not exist */
	return FALSE;
    }
    /* file exists */
    close(dummy_desc);
    return TRUE;
}
