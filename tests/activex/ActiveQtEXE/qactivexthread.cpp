#include "qactivexthread.h"
#include <qapplication.h>

extern QApplication* pApp;

QActiveXThread::QActiveXThread()
{
}

QActiveXThread::~QActiveXThread()
{
}

void QActiveXThread::run()
{
    if( qApp )
	qApp->exec();
}