#include "dialog.h"
#include <stdlib.h>
#include <time.h>

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    statusLabel = new QLabel(this);
    quitButton = new QPushButton(tr("&Quit"), this);
    connect(quitButton, SIGNAL(clicked()), SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(statusLabel);
    layout->addWidget(quitButton);

    fortuneServer = new QTcpServer(this);
    connect(fortuneServer, SIGNAL(newConnection()), SLOT(sendFortune()));

    srand(time(0));

    fortunes << "You've been leading a dog's life.  Stay off the furniture.\n"
             << "You've got to think about tomorrow.\n"
             << "You will be surprised by a loud noise.\n"
             << "You will feel hungry again in another hour.\n"
             << "You should emulate your heros, but don't carry it too far.  Especially if they are dead.\n"
             << "You never know how many friends you have until you rent a house on the beach.\n"
             << "You might have mail.\n"
             << "You have the capacity to learn from mistakes.  You'll learn a lot today.\n"
             << "You cannot kill time without injuring eternity.\n"
             << "You cannot achieve the impossible without attempting the absurd.\n"
             << "Computers are not intelligent. They only think they are.\n";

    if (!fortuneServer->listen()) {
        qDebug("Unable to start the fortune server: %s",
               fortuneServer->errorString().latin1());
        close();
        return;
    }

    statusLabel->setText(QString(tr("The fortune server is running on port %1")).arg(fortuneServer->serverPort()));
}

void Dialog::sendFortune()
{
    QTcpSocket *clientConnection = fortuneServer->nextPendingConnection();

    int fortuneNr = rand() % fortunes.size();

    QDataStream stream(clientConnection);
    stream << fortunes.at(fortuneNr);

    clientConnection->close();
    connect(clientConnection, SIGNAL(closed()), clientConnection, SLOT(deleteLater()));
}
