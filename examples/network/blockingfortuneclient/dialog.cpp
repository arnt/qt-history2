#include <QThread>

#include "dialog.h"

class FortuneThread : public QThread
{
    Q_OBJECT

public:
    void run();
    QString errorString();

public slots:
    void requestFortune(const QString &hostName, Q_UINT16 port);

signals:
    void newFortune(const QString &);
    void error(int, const QString &);

private:
    QString hostName;
    Q_UINT16 port;
    QString errorStr;
};

void FortuneThread::run()
{
    QTcpSocket socket;
    socket.setBlocking(true, 5000);
    if (!socket.connectToHost(hostName, port)) {
        emit error(socket.socketError(), socket.errorString());
        return;
    }

    while (socket.bytesAvailable() < sizeof(Q_UINT16)) {
        if (!socket.waitForReadyRead(5000)) {
            emit error(socket.socketError(),
                       socket.errorString());
            return;
        }
    }

    Q_UINT16 blockSize;
    QDataStream stream(&socket);
    stream.setVersion(7);
    stream >> blockSize;

    while (socket.bytesAvailable() < blockSize) {
        if (!socket.waitForReadyRead(5000)) {
            emit error(socket.socketError(),
                       socket.errorString());
            return;
        }
    }

    QString fortune;
    stream >> fortune;

    emit newFortune(fortune);
}

QString FortuneThread::errorString()
{
    return errorStr;
}

void FortuneThread::requestFortune(const QString &hostName, Q_UINT16 port)
{
    this->hostName = hostName;
    this->port = port;
    start();
}

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Fortune client"));

    serverHostEdit = new QLineEdit(this);
    serverHostEdit->setText("Localhost");
    serverHostEdit->setSelection(0, 9);
    serverPortEdit = new QLineEdit(this);
    serverPortEdit->setValidator(new QIntValidator(1, 65535, this));

    fortuneThread = new FortuneThread;

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

    connect(this, SIGNAL(fortuneRequestReady(const QString &, Q_UINT16)),
            fortuneThread, SLOT(requestFortune(const QString &, Q_UINT16)));

    connect(fortuneThread, SIGNAL(newFortune(const QString &)),
            this, SLOT(readFortune(const QString &)));
    connect(fortuneThread, SIGNAL(error(int, const QString &)),
            this, SLOT(displayError(int, const QString &)));
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

    emit fortuneRequestReady(serverHostEdit->text(),
                             serverPortEdit->text().toInt());
}

void Dialog::readFortune(const QString &fortune)
{
    if (fortune == lastFortune) {
        requestFortune();
        return;
    }
    lastFortune = fortune;

    statusLabel->setText(fortune.latin1());
}


void Dialog::displayError(int error, const QString &message)
{
    using namespace Qt;

    switch (error) {
    case HostNotFoundError:
        QMessageBox::information(this, tr("Blocking Fortune Client"),
                                 tr("The host was not found. Please check the "
                                    "host and port settings."));
        break;
    case ConnectionRefusedError:
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
        break;
    };
}

void Dialog::showMessage(const QString &title, const QString &text)
{
    QMessageBox::information(this, title, text, QMessageBox::Ok);
}

#include "dialog.moc"
