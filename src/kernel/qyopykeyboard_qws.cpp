/****************************************************************************
** $Id$
**
** Implementation of Qt/Embedded Yopy keyboard drivers
**
** Created : 991025
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
#if !defined(QT_NO_QWS_KEYBOARD) && defined(QT_QWS_YOPY)

#include <qyopykeyboard_qws.h>

/*
 * YOPY buttons driver
 * Contributed by Ron Victorelli (victorrj at icubed.com)
 */

extern "C" {
    int getpgid(int);
}

QWSyopyButtonsHandler::QWSyopyButtonsHandler() : QWSKeyboardHandler()
{
    terminalName = "/dev/tty1";
    buttonFD = -1;
    notifier = 0;

    if ((buttonFD = open(terminalName, O_RDWR | O_NDELAY, 0)) < 0) {
	qFatal("Cannot open %s\n", terminalName.latin1());
    } else {

       tcsetpgrp(buttonFD, getpgid(0));

       /* put tty into "straight through" mode.
       */
       if (tcgetattr(buttonFD, &oldT) < 0) {
           qFatal("Linux-kbd: tcgetattr failed");
       }

       newT = oldT;
       newT.c_lflag &= ~(ICANON | ECHO  | ISIG);
       newT.c_iflag &= ~(ISTRIP | IGNCR | ICRNL | INLCR | IXOFF | IXON);
       newT.c_iflag |= IGNBRK;
       newT.c_cc[VMIN]  = 0;
       newT.c_cc[VTIME] = 0;


       if (tcsetattr(buttonFD, TCSANOW, &newT) < 0) {
           qFatal("Linux-kbd: TCSANOW tcsetattr failed");
       }

       if (ioctl(buttonFD, KDSKBMODE, K_MEDIUMRAW) < 0) {
           qFatal("Linux-kbd: KDSKBMODE tcsetattr failed");
       }

	notifier = new QSocketNotifier( buttonFD, QSocketNotifier::Read, this );
	connect( notifier, SIGNAL(activated(int)),this,
		 SLOT(readKeyboardData()) );
    }
}

QWSyopyButtonsHandler::~QWSyopyButtonsHandler()
{
    if ( buttonFD > 0 ) {
	::close( buttonFD );
	buttonFD = -1;
    }
}

void QWSyopyButtonsHandler::readKeyboardData()
{
    uchar buf[1];
    char c='1';
    int fd;

    int n=read(buttonFD,buf,1);
    if (n<0) {
	qDebug("Keyboard read error %s",strerror(errno));
    } else {
	uint code = buf[0]&YPBUTTON_CODE_MASK;
        bool press = !(buf[0]&0x80);
        // printf("Key=%d/%d/%d\n",buf[1],code,press);
        int k=(-1);
        switch(code) {
          case 39:       k=Qt::Key_Up;     break;
          case 44:       k=Qt::Key_Down;   break;
          case 41:       k=Qt::Key_Left;   break;
          case 42:       k=Qt::Key_Right;  break;
          case 56:       k=Qt::Key_F1;     break; //windows
          case 29:       k=Qt::Key_F2;     break; //cycle
          case 24:       k=Qt::Key_F3;     break; //record
          case 23:       k=Qt::Key_F4;     break; //mp3
          case 4:        k=Qt::Key_F5;     break; // PIMS
          case 1:        k=Qt::Key_Escape; break; // Escape
          case 40:       k=Qt::Key_Up;     break; // prev
          case 45:       k=Qt::Key_Down;   break; // next
          case 35:       if( !press ) {
                           fd = open("/proc/sys/pm/sleep",O_RDWR,0);
                           if( fd >= 0 ) {
                               write(fd,&c,sizeof(c));
                               close(fd);
                               //
                               // Updates all widgets.
                               //
                               QWidgetList  *list = QApplication::allWidgets();
                               QWidgetListIt it( *list );          // iterate over the widgets
                               QWidget * w;
                               while ( (w=it.current()) != 0 ) {   // for each widget...
                                 ++it;
                                 w->update();
                               }
                               delete list;
                               // qApp->desktop()->repaint();
                           }
                         }
                         break;

          default: k=(-1); break;
        }

	if ( k >= 0 ) {
		qwsServer->processKeyEvent( 0, k, 0, press, FALSE );
	}
    }
}

#endif

