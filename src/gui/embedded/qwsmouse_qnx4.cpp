/****************************************************************************
**
** Implementation of Qt/Embedded Qnx mouse drivers.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifdef Q_OS_QNX4
#include "qwsmouse_qnx4.h"
#include <sys/mouse.h>
#include <sys/types.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <sys/dev.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

static int parentPID = -1;
static int chldPID = -1;

extern "C" void sigHandler(int sig) {
        if (parentPID == getpid())  {
                kill(chldPID, SIGKILL);
        } else {
                kill(parentPID, SIGKILL);
        }
        _exit(0);
}

QQnx4MouseHandlerPrivate::QQnx4MouseHandlerPrivate(MouseProtocol &protocol,QString dev) :
       QWSMouseHandler(), read_in(0), mpack(NULL) {
     int mouse_fds[2];
     pipe(mouse_fds);
        parentPID = getpid();
        signal(SIGTERM,sigHandler);
     if(!(chldPID=fork())) {
       close(mouse_fds[0]);
       struct _mouse_ctrl *mc = mouse_open((nid_t)0, dev.latin1(), 0);
       if(!mc)
         qFatal("mouse_open failed (%s)!", dev.latin1());
       mouse_flush(mc);
       const int buff_size = 10;
       mouse_event mbuff[buff_size];
       pid_t mprox = qnx_proxy_attach(0, 0, 0, -1);
       mouse_read(mc, mbuff, 0, mprox, NULL);
       while(1) {
         int msg = Receive(0, mbuff, buff_size);
         if(msg == mprox) {
            int armed = 0;
            while(!armed) {
              int n = mouse_read(mc, mbuff, buff_size, mprox, &armed);
              write(mouse_fds[1], mbuff, sizeof(mbuff[0]) * n);
            }
         }
       }
       mouse_close(mc);
       _exit(666);
    }
    if (chldPID == -1)
        qFatal("Failed to fork");
    close(mouse_fds[1]);
    mouseFD = mouse_fds[0];
    mouseNotifier = new QSocketNotifier(mouseFD,
                                    QSocketNotifier::Read, this);
    connect(mouseNotifier, SIGNAL(activated(int)),
          this, SLOT(readMouseData(int)));
}

QQnx4MouseHandlerPrivate::~QQnx4MouseHandlerPrivate()
{
  delete mouseNotifier;
  close(mouseFD);
  if(mpack)
        free(mpack);
  if (chldPID > 0) {
        kill(chldPID, SIGKILL);
        waitpid(chldPID,0,0);
  }
}

void QQnx4MouseHandlerPrivate::clearCalibration()
{
}

void QQnx4MouseHandlerPrivate::calibrate()
{
}

void QQnx4MouseHandlerPrivate::getCalibration(QWSPointerCalibrationData *) const
{
}

void QQnx4MouseHandlerPrivate::readMouseData(int fd) {
  if(!mpack)
    mpack = (mouse_event*)malloc(sizeof(mouse_event));
  int rin = read(fd, (void*)(((int)mpack)+read_in), sizeof(mouse_event) - read_in);
  if(rin > 0) {
    read_in += rin;
    if(read_in == sizeof(mouse_event)) {
      read_in = 0;
      long scaled_x = mpack->dx * qt_screen->width() /  _MOUSE_ABS_MAX,
           scaled_y = mpack->dy * qt_screen->height() / _MOUSE_ABS_MAX;
      QPoint mt(scaled_x, scaled_y);
      limitToScreen(mt);
      int button = Qt::NoButton;
      if(mpack->buttons & _MOUSE_LEFT)
        button |= Qt::LeftButton;
      else if(mpack->buttons & _MOUSE_RIGHT)
        button |= Qt::RightButton;
      mouseChanged(mt, button);
    }
  }
}

#endif

