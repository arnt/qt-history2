#if !defined(QT_QWS_NO_SHM)

#include <qstring.h>
#include <sys/ipc.h>

class QWSSharedMemory {

public:
	QWSSharedMemory(){};
	QWSSharedMemory(int size, QString file);
	~QWSSharedMemory(){};

	bool create();
	void destroy();

	bool attach();
	void detach();

	void setPermissions(mode_t mode);
	void * base() { return shmBase; };

private:
	void *shmBase;
	int shmSize;
	QString shmFile;
#if defined(QT_QWS_POSIX_SHM)
	int shmFD;
#else
	bool idInitted;
	key_t key;
#endif
};

#endif
