#include "qactivexapp.h"
#include "qactivexthread.h"

QActiveXApp::QActiveXApp( int argc, char** argv ) : QApplication( argc, argv )
{
    pThread = new QActiveXThread;

    if( pThread )
	pThread->start();
}

QActiveXApp::~QActiveXApp()
{
    if( pThread )
	delete pThread;
}
