#include <qthread.h>

class QActiveXThread : public QThread
{
public:
    QActiveXThread();
    ~QActiveXThread();

    void run();
};
