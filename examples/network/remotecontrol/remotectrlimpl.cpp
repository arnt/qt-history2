#include "remotectrlimpl.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qsocket.h>

RemoteCtrlImpl::RemoteCtrlImpl( QSocket *s )
{
    socket = s;
    connect( sImage, SIGNAL(clicked()), SLOT(sendImage()) );
    connect( sText, SIGNAL(clicked()), SLOT(sendText()) );
    connect( sPalette, SIGNAL(clicked()), SLOT(sendPalette()) );
}

RemoteCtrlImpl::~RemoteCtrlImpl()
{
}

void RemoteCtrlImpl::sendPacket( const QByteArray &ba )
{
    QDataStream ds( socket );
    ds << (Q_UINT32) ba.size();
    socket->writeBlock( ba.data(), ba.size() );
}

void RemoteCtrlImpl::sendImage()
{
}

void RemoteCtrlImpl::sendText()
{
    QByteArray ba;
    QDataStream ds( ba, IO_WriteOnly );
    ds << textToSend->text();
    sendPacket( ba );
}

void RemoteCtrlImpl::sendPalette()
{
}
