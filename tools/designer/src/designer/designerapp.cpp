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

#include "designerapp.h"
#include "mainwindow.h"
#include <qevent.h>
#include <QHostAddress>
#include <QFileInfo>
#include <QTcpServer>
#include <QTcpSocket>

DesignerServer::DesignerServer(QObject *parent)
    : QObject(parent)
{
    m_socket = 0;
    m_server = new QTcpServer(this);
    m_server->listen(QHostAddress::LocalHost, 0);
    if (m_server->isListening())
    {
        connect(m_server, SIGNAL(newConnection()),
            this, SLOT(handleNewConnection()));
    }
}

quint16 DesignerServer::serverPort() const
{
    return m_server ? m_server->serverPort() : 0;
}

void DesignerServer::sendOpenRequest(int port, const QStringList &files)
{
    QTcpSocket *sSocket = new QTcpSocket();
    sSocket->connectToHost(QHostAddress::LocalHost, port);
    if(sSocket->waitForConnected(3000))
    {
        foreach(QString file, files)
        {
            QFileInfo fi(file);
            sSocket->write(fi.absoluteFilePath().toUtf8() + '\n');
        }
        sSocket->waitForBytesWritten(3000);
        sSocket->close();
    }
    delete sSocket;
}

void DesignerServer::readFromClient()
{
    QString file = QString::null;
    while (m_socket->canReadLine())
        file = QString::fromUtf8(m_socket->readLine());
    if (!file.isNull())
    {
        file = file.replace(QLatin1String("\n"), QLatin1String(""));
        file = file.replace(QLatin1String("\r"), QLatin1String(""));
        QApplication::postEvent(parent(), new QFileOpenEvent(file));
    }
}

void DesignerServer::socketClosed()
{
    m_socket = 0;
}

void DesignerServer::handleNewConnection()
{
    // no need for more than one connection
    if (m_socket == 0)
    {
        m_socket = m_server->nextPendingConnection();
        connect(m_socket, SIGNAL(readyRead()),
            this, SLOT(readFromClient()));
        connect(m_socket, SIGNAL(closed()),
            this, SLOT(socketClosed()));
    }
}

DesignerApplication::DesignerApplication(int &argc, char *argv[])
    : QApplication(argc, argv), mMainWindow(0)
{
    setOrganizationDomain("Trolltech");
    setApplicationName("Designer");
}

DesignerApplication::~DesignerApplication()
{
}

bool DesignerApplication::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen) {
        mMainWindow->readInForm(static_cast<QFileOpenEvent *>(ev)->file());
        return true;
    } else if (ev->type() == QEvent::Close) {
        QCloseEvent *closeEv = static_cast<QCloseEvent *>(ev);
        sendEvent(mMainWindow, closeEv);
        if (closeEv->isAccepted())
            return QApplication::event(ev);
        return true;
    }
    return QApplication::event(ev);
}

void DesignerApplication::setMainWindow(MainWindow *mw)
{
    mMainWindow = mw;
}
