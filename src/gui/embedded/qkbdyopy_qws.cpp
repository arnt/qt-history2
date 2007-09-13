/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
 * YOPY buttons driver
 * Contributed by Ron Victorelli (victorrj at icubed.com)
 */

#include "qkbdyopy_qws.h"

#ifndef QT_NO_QWS_KBD_YOPY

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <linux/kd.h>
#include <linux/fb.h>
#include <linux/yopy_button.h>

extern "C" {
    int getpgid(int);
}

#include <qwidget.h>
#include <qsocketnotifier.h>

QT_BEGIN_NAMESPACE

class QWSYopyKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSYopyKbPrivate(QWSYopyKeyboardHandler *h, const QString&);
    virtual ~QWSYopyKbPrivate();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    struct termios newT, oldT;
    QSocketNotifier *notifier;
    QWSYopyKeyboardHandler *handler;
};

QWSYopyKeyboardHandler::QWSYopyKeyboardHandler(const QString &device)
{
    d = new QWSYopyKbPrivate(this, device);
}

QWSYopyKeyboardHandler::~QWSYopyKeyboardHandler()
{
    delete d;
}

QWSYopyKbPrivate::QWSYopyKbPrivate(QWSYopyKeyboardHandler *h, const QString &device) : handler(h)
{
    terminalName = device.isEmpty()?"/dev/tty1":device.toLatin1().constData();
    buttonFD = -1;
    notifier = 0;

    buttonFD = ::open(terminalName.toLatin1().constData(), O_RDWR | O_NDELAY, 0);
    if (buttonFD < 0) {
        qWarning("Cannot open %s\n", qPrintable(terminalName));
        return;
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

        notifier = new QSocketNotifier(buttonFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)),this,
                 SLOT(readKeyboardData()));
    }
}

QWSYopyKbPrivate::~QWSYopyKbPrivate()
{
    if (buttonFD > 0) {
        ::close(buttonFD);
        buttonFD = -1;
    }
}

void QWSYopyKbPrivate::readKeyboardData()
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
          case 35:       if(!press) {
                           fd = open("/proc/sys/pm/sleep",O_RDWR,0);
                           if(fd >= 0) {
                               write(fd,&c,sizeof(c));
                               close(fd);
                               //
                               // Updates all widgets.
                               //
                               QWidgetList list = QApplication::allWidgets();
                               for (int i = 0; i < list.size(); ++i) {
                                   QWidget *w = list.at(i);
                                   w->update();
                               }
                           }
                         }
                         break;

          default: k=(-1); break;
        }

        if (k >= 0) {
            handler->processKeyEvent(0, k, 0, press, false);
        }
    }
}

QT_END_NAMESPACE

#include "qkbdyopy_qws.moc"

#endif // QT_NO_QWS_KBD_YOPY
