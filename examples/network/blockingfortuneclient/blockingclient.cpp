#include <QtGui>
#include <QtNetwork>

#include "blockingclient.h"

BlockingClient::BlockingClient(QWidget *parent)
    : QDialog(parent)
{
    hostLabel = new QLabel(tr("&Host name:"), this);
    portLabel = new QLabel(tr("&Port:"), this);

    hostLineEdit = new QLineEdit("Localhost", this);
    portLineEdit = new QLineEdit(this);
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));

    hostLabel->setBuddy(hostLineEdit);
    portLabel->setBuddy(portLineEdit);

    statusLabel = new QLabel(tr("You are about to receive a fortune. "
                                "Please click Get Fortune."), this);

    getFortuneButton = new QPushButton(tr("Get Fortune"), this);
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(false);

    quitButton = new QPushButton(tr("Quit"), this);

    connect(hostLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableGetFortuneButton()));
    connect(portLineEdit, SIGNAL(textChanged(const QString &)),
            this, SLOT(enableGetFortuneButton()));
    connect(getFortuneButton, SIGNAL(clicked()),
            this, SLOT(requestNewFortune()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(&thread, SIGNAL(newFortune(const QString &)),
            this, SLOT(showFortune(const QString &)));
    connect(&thread, SIGNAL(error(int, const QString &)),
            this, SLOT(displayError(int, const QString &)));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(getFortuneButton);
    buttonLayout->addWidget(quitButton);

    QGridLayout *mainLayout = new QGridLayout(this);
    mainLayout->addWidget(hostLabel, 0, 0);
    mainLayout->addWidget(hostLineEdit, 0, 1);
    mainLayout->addWidget(portLabel, 1, 0);
    mainLayout->addWidget(portLineEdit, 1, 1);
    mainLayout->addWidget(statusLabel, 2, 0, 1, 2);
    mainLayout->addLayout(buttonLayout, 3, 0, 1, 2);

    setWindowTitle(tr("Blocking Fortune Client"));
    portLineEdit->setFocus();
}

void BlockingClient::requestNewFortune()
{
    thread.requestNewFortune(hostLineEdit->text(),
                             portLineEdit->text().toInt());
}

void BlockingClient::showFortune(const QString &nextFortune)
{
    if (nextFortune == currentFortune) {
        requestNewFortune();
        return;
    }

    currentFortune = nextFortune;
    statusLabel->setText(currentFortune);
}

void BlockingClient::displayError(int socketError, const QString &message)
{
    switch (socketError) {
    case Qt::HostNotFoundError:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host and port settings."));
        break;
    case Qt::ConnectionRefusedError:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The connection was refused by the peer. "
                                    "Make sure the fortune server is running, "
                                    "and check that the host name and port "
                                    "settings are correct."));
        break;
    default:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The following error occurred: %1.")
                                 .arg(message));
    }
}

void BlockingClient::enableGetFortuneButton()
{
    getFortuneButton->setEnabled(!hostLineEdit->text().isEmpty()
                                 && !portLineEdit->text().isEmpty());
}
