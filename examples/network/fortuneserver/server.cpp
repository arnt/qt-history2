#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <cstdlib>

#include "server.h"

Server::Server(QWidget *parent)
    : QDialog(parent)
{
    statusLabel = new QLabel(this);
    quitButton = new QPushButton(tr("&Quit"), this);

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen()) {
        QMessageBox::critical(this, tr("Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(tcpServer->errorString()));
        close();
        return;
    }

    statusLabel->setText(tr("The server is running on port %1.")
                         .arg(tcpServer->serverPort()));

    fortunes << tr("You've been leading a dog's life. Stay off the furniture.")
             << tr("You've got to think about tomorrow.")
             << tr("You will be surprised by a loud noise.")
             << tr("You will feel hungry again in another hour.")
             << tr("You might have mail.")
             << tr("You cannot kill time without injuring eternity.")
             << tr("Computers are not intelligent. They only think they are.");

    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(sendFortune()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Fortune Server"));
    std::srand(QDateTime::currentDateTime().toTime_t());
}

void Server::sendFortune()
{
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);
    out << (Q_UINT16)0;
    out << fortunes.at(std::rand() % fortunes.size());
    out.device()->seek(0);
    out << (Q_UINT16)(block.size() - sizeof(Q_UINT16));

    QTcpSocket *clientConnection = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(closed()),
            clientConnection, SLOT(deleteLater()));

    clientConnection->write(block);
    clientConnection->close();
}
