/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef STARTUP_H
#define STARTUP_H

#include <qobject.h>

class QSocket;
class RemoteCtrlImpl;
class MainDialog;

class StartUp : public QObject
{
    Q_OBJECT

public:
    StartUp();
    ~StartUp();

private slots:
    void startRemoteCtrl();
    void startMainDialog();

private:
    QSocket *socket;
    RemoteCtrlImpl *remoteCtrl;
    MainDialog *mainDialog;
};

#endif // STARTUP_H
