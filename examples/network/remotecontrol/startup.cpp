#include "startup.h"
#include "remotectrlimpl.h"
#include "maindialog.h"
#include "ipcserver.h"

#include <qsocket.h>
#include <qlabel.h>

#define IPC_PORT 54923

StartUp::StartUp()
{
    remoteCtrl = 0;
    mainDialog = 0;

    socket = new QSocket( this );
    connect( socket, SIGNAL(connected()), SLOT(startRemoteCtrl()) );
    connect( socket, SIGNAL(error(int)), SLOT(startMainDialog()) );
    socket->connectToHost( "localhost", IPC_PORT );
}

StartUp::~StartUp()
{
    delete remoteCtrl;
    delete mainDialog;
}

void StartUp::startRemoteCtrl()
{
    remoteCtrl = new RemoteCtrlImpl( socket );
    remoteCtrl->show();
}

void StartUp::startMainDialog()
{
    mainDialog = new MainDialog();
    mainDialog->show();

    IpcServer *server = new IpcServer( IPC_PORT, this );

    connect( server, SIGNAL(receivedText(const QString&)),
	    mainDialog->description, SLOT(setText(const QString&)) );
    connect( server, SIGNAL(receivedPixmap(const QPixmap&)),
	    mainDialog->image, SLOT(setPixmap(const QPixmap&)) );
}
