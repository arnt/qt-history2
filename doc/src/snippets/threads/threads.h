#include <QThread>

class MyThread : public QThread
{
    Q_OBJECT

protected:
    void run();
};
