#include <qshm.h>

#if !defined(QT_QWS_NO_SHM)

#include <sys/ipc.h>
#include <sys/types.h>

#if defined(QT_QWS_POSIX_SHM)
#include <fcntl.h>
#include <sys/mman.h>

QWSSharedMemory::QWSSharedMemory(int size, QString filename) {
	shmSize = size;
	shmFile = filename;
}

bool QWSSharedMemory::create () {
	shmFD = shm_open( shmFile.latin1(), O_RDWR|O_EXCL|O_CREAT , 0666 );
	if (shmFD == -1)
		return FALSE;
    else if (ftruncate( shmFD, shmSize ) == -1) {
		close(shmFD);
		return FALSE;
	}

	return TRUE;
}

void QWSSharedMemory::destroy() {
	shm_unlink(shmFile.latin1());
}

bool QWSSharedMemory::attach () {
    shmBase = mmap( 0, shmSize , PROT_READ|PROT_WRITE,
            MAP_SHARED, shmFD, 0 );

    if (shmBase == MAP_FAILED)
        return FALSE;

	close(shmFD);
	return TRUE;
}

void QWSSharedMemory::detach () {
	munmap( shmBase, shmSize );
}

void QWSSharedMemory::setPermissions(mode_t mode) {
	mprotect(shmBase, shmSize, mode); // Provide defines to make prot work properly
}

#else // Assume SysV for backwards compat
#include <sys/shm.h>

QWSSharedMemory::QWSSharedMemory(int size, QString filename) {
	shmSize = size;
	shmFile = filename;
    key = ftok( shmFile.latin1(), 'a' );
	idInitted = FALSE;
}

bool QWSSharedMemory::create () {
	shmId = shmget( key, shmSize, IPC_CREAT|0000);
	if ( shmId == -1 ) 
		return FALSE;
	idInitted = TRUE;
	return TRUE;
}

void QWSSharedMemory::destroy() {
	struct shmid_ds shm;
    shmctl( shmId, IPC_RMID, &shm );
}

bool QWSSharedMemory::attach() {
	if ( !idInitted ) 
		shmId = shmget( key, shmSize, 0 );

	shmBase = shmat(shmId, 0, 0);
	if ( (int)shmBase == -1 || shmBase == 0 )
		return FALSE;

	return TRUE;
}

void QWSSharedMemory::detach() {
	shmdt( shmBase );
}

void QWSSharedMemory::setPermissions(mode_t mode) {
	struct shmid_ds shm;
	shmctl (shmId, IPC_STAT, &shm);
	shm.shm_perm.mode = mode;
	shmctl (shmId, IPC_SET, &shm);
}

#endif

#endif
