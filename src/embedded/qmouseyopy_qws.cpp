/****************************************************************************
** $Id: $
**
** Implementation of Qt/Embedded mouse drivers
**
** Created : 20020220
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qmouseyopy_qws.h"

#include "qwindowsystem_qws.h"
#include "qsocketnotifier.h"
#include "qapplication.h"
#include "qgfx_qws.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>

class QWSYopyMouseHandlerPrivate : public QObject
{
    Q_OBJECT
public:
    QWSYopyMouseHandlerPrivate( QWSYopyMouseHandler *h );
    ~QWSYopyMouseHandlerPrivate();

private slots:
    void readMouseData();

private:
    int mouseFD;
    int prevstate;
    QWSYopyMouseHandler *handler;
};

QWSYopyMouseHandler::QWSYopyMouseHandler( const QString &, const QString & )
{
    d = new QWSYopyMouseHandlerPrivate( this );
}

QWSYopyMouseHandler::~QWSYopyMouseHandler()
{
    delete d;
}

QWSYopyMouseHandlerPrivate::QWSYopyMouseHandlerPrivate( QWSYopyMouseHandler *h )
    : handler( h )
{
    if ((mouseFD = open( "/dev/ts", O_RDONLY)) < 0) {
        qWarning( "Cannot open /dev/ts (%s)", strerror(errno));
	return;
    } else {
        sleep(1);
    }
    prevstate=0;
    QSocketNotifier *mouseNotifier;
    mouseNotifier = new QSocketNotifier( mouseFD, QSocketNotifier::Read,
					 this );
    connect(mouseNotifier, SIGNAL(activated(int)),this, SLOT(readMouseData()));
}

QWSYopyMouseHandlerPrivate::~QWSYopyMouseHandlerPrivate()
{
    if (mouseFD >= 0)
	close(mouseFD);
}

#define YOPY_XPOS(d) (d[1]&0x3FF)
#define YOPY_YPOS(d) (d[2]&0x3FF)
#define YOPY_PRES(d) (d[0]&0xFF)
#define YOPY_STAT(d) (d[3]&0x01 )

struct YopyTPdata {

  unsigned char status;
  unsigned short xpos;
  unsigned short ypos;

};

void QWSYopyMouseHandlerPrivate::readMouseData()
{
    if(!qt_screen)
	return;
    YopyTPdata data;

    unsigned int yopDat[4];

    int ret;

    ret=read(mouseFD,&yopDat,sizeof(yopDat));

    if(ret) {
        data.status= ( YOPY_PRES(yopDat) ) ? 1 : 0;
	data.xpos=YOPY_XPOS(yopDat);
	data.ypos=YOPY_YPOS(yopDat);
	QPoint q;
	q.setX(data.xpos);
	q.setY(data.ypos);
	if (data.status && !prevstate) {
          handler->mouseChanged(q,Qt::LeftButton);
        } else if( !data.status && prevstate ) {
	  handler->mouseChanged(q,0);
        }
        prevstate = data.status;
    }
    if(ret<0) {
	qDebug("Error %s",strerror(errno));
    }
}

#include "qmouseyopy_qws.moc"
