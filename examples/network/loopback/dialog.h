#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui>
#include <QtNetwork>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

public slots:
    void start();
    void acceptConnection();
    void sendToServer();
    void receiveFromClient();
    void clientBytesWritten(Q_LLONG bytesWritten);
    void displayError(int socketError);

private:
    QProgressBar *clientProgressBar;
    QProgressBar *serverProgressBar;
    QLabel *clientStatusLabel;
    QLabel *serverStatusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;

    QTcpServer tcpServer;
    QTcpSocket tcpClient;
    QTcpSocket *tcpServerConnection;

    int totalBytes;
    int bytesWritten;
    int bytesReceived;

    QTime stopWatch;
};

#endif
