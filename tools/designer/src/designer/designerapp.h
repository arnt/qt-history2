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

#ifndef DESIGNERAPP_H
#define DESIGNERAPP_H

#include <qapplication.h>

class QTcpServer;
class QTcpSocket;
class DesignerServer : public QObject
{
    Q_OBJECT
public:
    DesignerServer(QObject* parent);
    static void sendOpenRequest(int port, const QStringList &files);
    quint16 serverPort() const;
private slots:
    void handleNewConnection();
    void readFromClient();
    void socketClosed();
private:
    QTcpServer *m_server;
    QTcpSocket *m_socket;
};

class MainWindow;
class DesignerApplication : public QApplication
{
public:
    DesignerApplication(int &argc, char *argv[]);
    virtual ~DesignerApplication();
    void setMainWindow(MainWindow *mw);

protected:
    bool event(QEvent *ev);

private:
    MainWindow *mMainWindow;
};

#endif // DESIGNERAPP_H
