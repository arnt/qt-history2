/****************************************************************************
** $Id$
**
** Implementation of a custom QMacControl
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include <qapplication.h>
#include <Carbon/Carbon.h>
#include "qmaccontrol_mac.h"
#include <qpushbutton.h>
#include "qmacscrollbar.h"

void pb_action(ControlRef, ControlPartCode)
{
    qDebug("my own action callback");
}

int
main(int argc, char **argv)
{
    Rect r; 
    //1) standard things
    QApplication a(argc, argv);
    QWidget *window = new QWidget(NULL, "TopLevel");
    a.setMainWidget(window);
    window->show();

    //2) interesting part, create a macpushbutton...
    SetRect(&r, 0, 0, 50, 35);
    ControlRef button;
    CreatePushButtonControl((WindowPtr)window->handle(), &r, NULL, &button);
    SetControlAction(button, NewControlActionUPP(pb_action));
    const Boolean t = true;
    SetControlData(button, 0, kControlPushButtonDefaultTag, sizeof(t), &t);
    //...and bind it to a qwidget, we can even make some calls to it..
    QMacControl qmc_bt(window, button);
    qmc_bt.setGeometry(50, 50, 200, 25);
    qmc_bt.setCaption("Hello World");

    //3) create a custom MacControl binding
    QMacScrollBar sc(window);

    //4) Create a text input control..
    ControlRef tedit;
    SetRect(&r, 50, 0, 200, 50);
    CreateEditTextControl((WindowPtr)window->handle(), &r, CFSTR( "" ), false, true, nil, &tedit);
    //..and another binding
    QMacControl qmc_te(window, tedit);

    //5) To show how this can interact with other QWidgets, we create one
    QPushButton pb(window, "ButtonControl2");
    pb.setGeometry(200, 40, 200, 25);
    pb.setCaption("Hello World2");
    pb.show();

    //6) finally event loop
    return a.exec();
}
