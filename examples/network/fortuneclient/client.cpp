#include <QtGui>
#include <QtNetwork>

#include "client.h"

Client::Client(QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("Host name:"), this);
    portLabel = new QLabel(tr("Port:"), this);

    hostLineEdit = new QLineEdit("Localhost", this);
    portLineEdit = new QLineEdit(this);
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    statusLabel = new QLabel(this);

    getFortuneButton = new QPushButton(tr("&Get Fortune"), this);
    quitButton = new QPushButton(tr("&Quit"), this);

    tcpSocket = new QTcpSocket(this);

    connect(getFortuneButton, SIGNAL(clicked()),
            this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readFortune()));
    connect(tcpSocket, SIGNAL(error(int)), this, SLOT(displayError(int)));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(getFortuneButton);
    buttonLayout->addWidget(quitButton);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0);
    mainLayout->addLayout(buttonLayout, 3, 0, 1, 2);

    setWindowTitle(tr("Fortune client"));
}

void Client::requestNewFortune()
{
    if (hostLineEdit->text().isEmpty() || portLineEdit->text().isEmpty()) {
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("Enter the host name and the port of the "
                                    "fortune server."));
        return;
    }

    tcpSocket->connectToHost(hostLineEdit->text(),
                             portLineEdit->text().toInt());
}

void Client::readFortune()
{
    QString nextFortune;

    QDataStream in(tcpSocket);
    in >> nextFortune;

    if (nextFortune == currentFortune) {
        requestNewFortune();
        return;
    }
    currentFortune = nextFortune;
    statusLabel->setText(currentFortune);
}

void Client::displayError(int error)
{
    switch (error) {
    case Qt::HostNotFoundError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host name and port settings."));
        break;
    case Qt::ConnectionRefusedError:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(tcpSocket->errorString()));
    }
}
