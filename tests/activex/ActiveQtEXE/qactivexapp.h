#include <qapplication.h>

class QActiveXThread;

class QActiveXApp : public QApplication
{
public:
    QActiveXApp( int argc, char** argv );
    ~QActiveXApp();

    QActiveXThread* pThread;
};
