#ifndef QT_QWS_NO_SHM

#include <qstring.h>

static const int max_id = 20;

class QWSSharedMemoryPrivate;
class QWSSharedMemory {

public:
	QWSSharedMemory(int size, QString file, int id);
	~QWSSharedMemory();

	/* active functions */
	bool create();
	void destroy();

	bool attach();
	void detach();

	/* query functions */
	inline void * base() { return shmBase; };

	bool exists() const;
	inline uint size() const { return shmSize; };

private:
	void *shmBase;
	int shmSize;
	QString shmFile;
	QWSSharedMemoryPrivate *d;
};

#endif
