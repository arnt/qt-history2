#include "remotectrlimpl.h"

#include <qpushbutton.h>
#include <qlineedit.h>
#include <qsocket.h>
#include <qfiledialog.h>
#include <qcolordialog.h>
#include <qimage.h>

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

void RemoteCtrlImpl::sendPacket( const QByteArray &ba, Q_UINT8 type )
{
    QDataStream ds( socket );
    ds << (Q_UINT32) ba.size();
    ds << type;
    socket->writeBlock( ba.data(), ba.size() );
}

void RemoteCtrlImpl::sendImage()
{
    QString imageName = QFileDialog::getOpenFileName( QString::null,
	    "Images (*.png *.xpm *.jpg)", this );
    QImage image( imageName );
    if ( !image.isNull() ) {
	QByteArray ba;
	QDataStream ds( ba, IO_WriteOnly );
	ds << image;
	sendPacket( ba, QVariant::Image );
    }
}

void RemoteCtrlImpl::sendText()
{
    QByteArray ba;
    QDataStream ds( ba, IO_WriteOnly );
    ds << textToSend->text();
    sendPacket( ba, QVariant::String );
}

void RemoteCtrlImpl::sendPalette()
{
    QColor col = QColorDialog::getColor( white, this );
    if ( col.isValid() ) {
	QByteArray ba;
	QDataStream ds( ba, IO_WriteOnly );
	ds << QPalette( col, col );
	sendPacket( ba, QVariant::Palette );
    }
}
