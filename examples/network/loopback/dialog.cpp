#include <QtGui>
#include <QtNetwork>

#include "dialog.h"

static const int TotalBytes = 50 * 1024 * 1024;

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    clientProgressBar = new QProgressBar(this);
    clientStatusLabel = new QLabel(tr("Client ready"), this);
    serverProgressBar = new QProgressBar(this);
    serverStatusLabel = new QLabel(tr("Server ready"), this);

    startButton = new QPushButton(tr("&Start"), this);
    quitButton = new QPushButton(tr("&Quit"), this);

    connect(startButton, SIGNAL(clicked()), this, SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(&tcpServer, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));
    connect(&tcpClient, SIGNAL(connected()), this, SLOT(startTransfer()));
    connect(&tcpClient, SIGNAL(bytesWritten(Q_LONGLONG)),
            this, SLOT(updateClientProgress(Q_LONGLONG)));
    connect(&tcpClient, SIGNAL(error(int)), this, SLOT(displayError(int)));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);
    mainLayout->addLayout(buttonLayout);

    setWindowTitle(tr("Loopback"));
}

void Dialog::start()
{
    startButton->setEnabled(false);

    QApplication::setOverrideCursor(Qt::WaitCursor);

    bytesWritten = 0;
    bytesReceived = 0;

    while (!tcpServer.isListening() && !tcpServer.listen()) {
        int ret = QMessageBox::critical(this, tr("Loopback"),
                                        tr("Unable to start the test: %1.")
					.arg(tcpServer.errorString()),
                                        QMessageBox::Retry,
					QMessageBox::Cancel);
        if (ret == QMessageBox::Cancel)
            return;
    }

    serverStatusLabel->setText(tr("Listening"));
    clientStatusLabel->setText(tr("Connecting"));
    tcpClient.connectToHost(QHostAddress::LocalHost, tcpServer.serverPort());
}

void Dialog::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()),
            this, SLOT(updateServerProgress()));
    connect(tcpServerConnection, SIGNAL(error(int)),
            this, SLOT(displayError(int)));

    serverStatusLabel->setText(tr("Accepted connection"));
    tcpServer.close();
}

void Dialog::startTransfer()
{
    tcpClient.write(QByteArray(TotalBytes, '@'));
    clientStatusLabel->setText(tr("Connected"));
}

void Dialog::updateServerProgress()
{
    bytesReceived = (int)tcpServerConnection->bytesAvailable();
    serverProgressBar->setMaximum(TotalBytes);
    serverProgressBar->setValue(bytesReceived);
    serverStatusLabel->setText(tr("Received %1MB")
                               .arg(bytesReceived / (1024 * 1024)));

    if (bytesReceived == TotalBytes) {
        tcpServerConnection->close();
        startButton->setEnabled(true);
        QApplication::restoreOverrideCursor();
    }
}

void Dialog::updateClientProgress(Q_LONGLONG numBytes)
{
    bytesWritten += (int)numBytes;
    clientProgressBar->setMaximum(TotalBytes);
    clientProgressBar->setValue(bytesWritten);
    clientStatusLabel->setText(tr("Sent %1MB")
                               .arg(bytesWritten / (1024 * 1024)));
}

void Dialog::displayError(int /* socketError */)
{
    QMessageBox::information(this, tr("Network error"),
                             tr("The following error occurred: %1.")
                             .arg(tcpClient.errorString()));

    tcpClient.close();
    tcpServer.close();
    clientProgressBar->reset();
    serverProgressBar->reset();
    clientStatusLabel->setText(tr("Client ready"));
    serverStatusLabel->setText(tr("Server ready"));
    startButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}
