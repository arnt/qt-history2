#include "dialog.h"
#include <stdlib.h>

Dialog::Dialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Fortune client"));

    serverHostEdit = new QLineEdit(this);
    serverHostEdit->setText("Localhost");
    serverHostEdit->setSelection(0, 9);
    serverPortEdit = new QLineEdit(this);
    serverPortEdit->setValidator(new QIntValidator(1, 65535, this));

    QGridLayout *layout = new QGridLayout(this);

    statusLabel = new QLabel(this);
    fortuneButton = new QPushButton(tr("Get &Fortune"), this);
    quitButton = new QPushButton(tr("&Quit"), this);
    connect(fortuneButton, SIGNAL(clicked()), SLOT(requestFortune()));
    connect(quitButton, SIGNAL(clicked()), SLOT(close()));

    layout->addWidget(new QLabel(tr("Hostname:"), this), 0, 0);
    layout->addWidget(serverHostEdit, 0, 1);
    layout->addWidget(new QLabel(tr("Port:"), this), 1, 0);
    layout->addWidget(serverPortEdit, 1, 1);
    layout->addWidget(statusLabel, 2, 0);
    layout->addWidget(fortuneButton, 3, 0);
    layout->addWidget(quitButton, 3, 1);

    fortuneClient = new QTcpSocket(this);
    connect(fortuneClient, SIGNAL(readyRead()), SLOT(readFortune()));
    connect(fortuneClient, SIGNAL(error(int)), SLOT(displayError(int)));
}

void Dialog::requestFortune()
{
    if (serverHostEdit->text().isEmpty()) {
        showMessage(tr("Empty host field"),
                    tr("Please fill in the name of the host running"
                       " the fortune server."));
        return;
    }

    if (serverPortEdit->text().isEmpty()) {
        showMessage(tr("Empty port field"),
                    tr("Please fill in the port of the host running"
                       " the fortune server."));
        return;
    }

    fortuneClient->connectToHost(serverHostEdit->text(),
                                 serverPortEdit->text().toInt());
}

void Dialog::readFortune()
{
    QDataStream stream(fortuneClient);
    QByteArray fortune;
    stream >> fortune;

    if (fortune == lastFortune) {
        requestFortune();
        return;
    }
    lastFortune = fortune;

    statusLabel->setText(QString::fromLatin1(fortune.data(), fortune.size()));
}


void Dialog::displayError(int error)
{
    using namespace Qt;

    switch (error) {
    case HostNotFoundError:
        showMessage(fortuneClient->errorString(),
                    tr("The host was not found. Please check the "
                       "host and port settings."));
        break;
    case ConnectionRefusedError:
        showMessage(fortuneClient->errorString(),
                    tr("Make sure the fortune server is running, "
                    "and check that your host and port settings "
                    "are correct."));
        break;
    default:
        break;
    };
}

void Dialog::showMessage(const QString &title, const QString &text)
{
    QMessageBox::information(this, title, text, QMessageBox::Ok);
}
