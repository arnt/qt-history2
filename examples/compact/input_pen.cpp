

#include "input_pen.h"
#include <qwindowsystem_qws.h>
#include <qwsevent_qws.h>

QWSPenInput::QWSPenInput( QWidget *parent, const char *name,
			  int WFlags )
    : QIMPenInput( parent, name, WFlags )
{
    connect( this, SIGNAL(key(unsigned int)), SLOT(keyPress(unsigned int)) );
}


void QWSPenInput::keyPress( unsigned int unicode )
{
    QWSServer::sendKeyEvent( unicode&0xffff, unicode>>16, 0, TRUE, FALSE );
    QWSServer::sendKeyEvent( unicode&0xffff, unicode>>16, 0, FALSE, FALSE );
}

