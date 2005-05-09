/****************************************************************************
**
** Copyright (C) 2004-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>

class QLabel;
class QProgressBar;
class QPushButton;
class QTcpServer;
class QTcpSocket;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

public slots:
    void start();
    void acceptConnection();
    void startTransfer();
    void updateServerProgress();
    void updateClientProgress(qint64 numBytes);
    void displayError(QTcpSocket::SocketError socketError);

private:
    QProgressBar *clientProgressBar;
    QProgressBar *serverProgressBar;
    QLabel *clientStatusLabel;
    QLabel *serverStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;

    QTcpServer tcpServer;
    QTcpSocket tcpClient;
    QTcpSocket *tcpServerConnection;
    int bytesToWrite;
    int bytesWritten;
    int bytesReceived;
};

#endif
