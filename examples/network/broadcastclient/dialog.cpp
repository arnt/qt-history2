#include "dialog.h"

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    statusLabel = new QLabel(this);
    statusLabel->setText(tr("Listening for broadcasted messages"));

    quitButton = new QPushButton(tr("&Quit"), this);
    connect(quitButton, SIGNAL(clicked()), SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->addWidget(statusLabel);
    layout->addWidget(quitButton);

    client = new QUdpSocket(this);
    connect(client, SIGNAL(readyRead()), SLOT(processMessage()));
    client->bind(45454);
}

void Dialog::processMessage()
{
    while (client->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(client->pendingDatagramSize());
        client->receiveDatagram(datagram.data(), datagram.size());
        statusLabel->setText(QString(tr("Received datagram: \"%1\"")).arg(datagram.data()));
    }
}
