#include "dialog.h"

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    statusLabel = new QLabel(this);
    statusLabel->setText(tr("Ready to broadcast"));

    startButton = new QPushButton(tr("&Start"), this);
    quitButton = new QPushButton(tr("&Quit"), this);
    connect(startButton, SIGNAL(clicked()), SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), SLOT(close()));

    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    layout->addWidget(statusLabel);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);
    layout->addLayout(buttonLayout);

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(broadcast()));

    broadcastServer = new QUdpSocket(this);

    messageCount = 0;
}

void Dialog::start()
{
    if (!startButton->isEnabled())
        return;
    startButton->setEnabled(false);

    timer->start(1000);
}

void Dialog::broadcast()
{
    QByteArray datagram = "Broadcast message #";
    datagram += QString::number(messageCount).toLatin1();

    statusLabel->setText(QString(tr("Now broadcasting message #%1").arg(messageCount)));
    ++messageCount;

    broadcastServer->sendDatagram(datagram.data(), datagram.size(),
                                  QHostAddress::BroadcastAddress, 45454);
}
