#include "dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent), tcpServer(this), tcpClient(this)
{
    setWindowTitle(tr("Loopback example"));

    clientProgressBar = new QProgressBar(100, this);
    clientStatusLabel = new QLabel(this);
    serverProgressBar = new QProgressBar(100, this);
    serverStatusLabel = new QLabel(this);

    startButton = new QPushButton(tr("&Start"), this);
    quitButton = new QPushButton(tr("&Quit"), this);
    connect(startButton, SIGNAL(clicked()), SLOT(start()));
    connect(quitButton, SIGNAL(clicked()), SLOT(close()));

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *buttonLayout = new QHBoxLayout();

    mainLayout->addWidget(clientProgressBar);
    mainLayout->addWidget(clientStatusLabel);
    mainLayout->addWidget(serverProgressBar);
    mainLayout->addWidget(serverStatusLabel);

    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(quitButton);
    mainLayout->addLayout(buttonLayout);

    connect(&tcpServer, SIGNAL(newConnection()), SLOT(acceptConnection()));
    connect(&tcpClient, SIGNAL(connected()), SLOT(sendToServer()));
    connect(&tcpClient, SIGNAL(bytesWritten(Q_LLONG)), SLOT(clientBytesWritten(Q_LLONG)));
    connect(&tcpClient, SIGNAL(error(int)), SLOT(displayError(int)));

    totalBytes = 50 * 1048576;
    clientStatusLabel->setText(tr("Client ready"));
    serverStatusLabel->setText(tr("Server ready"));
}

void Dialog::start()
{
    if (!startButton->isEnabled())
        return;
    startButton->setEnabled(false);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    bytesWritten = 0;
    bytesReceived = 0;

    while (!tcpServer.isListening() && !tcpServer.listen()) {
        if (QMessageBox::critical(this, tr("Unable to start the test"),
                                  tcpServer.errorString(),
                                  QMessageBox::Cancel,
                                  QMessageBox::Retry) == QMessageBox::Cancel) {
            return;
        }
    }

    serverStatusLabel->setText(tr("Listening"));
    clientStatusLabel->setText(tr("Connecting"));
    tcpClient.connectToHost(QHostAddress::LocalHostAddress, tcpServer.serverPort());
}

void Dialog::sendToServer()
{
    stopWatch.start();
    tcpClient.write(QByteArray(totalBytes, '@'));
    clientStatusLabel->setText(tr("Connected"));
}

void Dialog::displayError(int)
{
    QMessageBox::information(this, tr("Network error"),
                             tcpClient.errorString(),
                             QMessageBox::Ok);
    tcpClient.close();
    tcpServer.close();
    clientProgressBar->reset();
    serverProgressBar->reset();
    clientStatusLabel->setText("Client ready");
    serverStatusLabel->setText("Server ready");
}

void Dialog::clientBytesWritten(Q_LLONG bytes)
{
    bytesWritten += (int) bytes;
    clientProgressBar->setProgress(bytesWritten, totalBytes);
    clientStatusLabel->setText(QString(tr("Sent %1MB").arg(bytesWritten / 1048576)));
}

void Dialog::acceptConnection()
{
    tcpServerConnection = tcpServer.nextPendingConnection();
    connect(tcpServerConnection, SIGNAL(readyRead()), SLOT(receiveFromClient()));
    connect(tcpServerConnection, SIGNAL(error(int)), SLOT(displayError(int)));

    serverStatusLabel->setText(tr("Accepted connection"));
    tcpServer.close();
}

void Dialog::receiveFromClient()
{
    bytesReceived = (int) tcpServerConnection->bytesAvailable();
    serverProgressBar->setProgress(bytesReceived, totalBytes);
    serverStatusLabel->setText(QString(tr("Received %1MB").arg(bytesReceived / 1048576)));

    if (bytesReceived == totalBytes) {
        tcpServerConnection->close();
        QApplication::restoreOverrideCursor();
        startButton->setEnabled(true);
    }
}
