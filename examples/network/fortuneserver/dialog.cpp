#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <cstdlib>

#include "dialog.h"

static const int NumFortunes = 8;

static const char * const fortunes[NumFortunes] = {
    "You've been leading a dog's life. Stay off the furniture.",
    "You've got to think about tomorrow.",
    "You will be surprised by a loud noise.",
    "You will feel hungry again in another hour.",
    "You might have mail.",
    "You cannot kill time without injuring eternity.",
    "You cannot achieve the impossible without attempting the absurd.",
    "Computers are not intelligent. They only think they are."
};

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    statusLabel = new QLabel(this);
    quitButton = new QPushButton(tr("&Quit"), this);

    fortuneServer = new QTcpServer(this);

    connect(quitButton, SIGNAL(clicked()), SLOT(close()));
    connect(fortuneServer, SIGNAL(newConnection()),
            this, SLOT(sendFortune()));

    if (!fortuneServer->listen()) {
        QMessageBox::critical(this, tr("Fortune Server"),
                              tr("Unable to start the server: %1.")
                              .arg(fortuneServer->errorString()));
        close();
        return;
    }

    statusLabel->setText(tr("The server is running on port %1")
                         .arg(fortuneServer->serverPort()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Fortune Server"));
    srand(QDateTime::currentDateTime().toTime_t());
}

void Dialog::sendFortune()
{
    QTcpSocket *clientConnection =
            fortuneServer->nextPendingConnection();

    QDataStream out(clientConnection);
    out.setVersion(7);
    out << fortunes[std::rand() % NumFortunes];

    connect(clientConnection, SIGNAL(closed()),
            clientConnection, SLOT(deleteLater()));
    clientConnection->close();
}
