#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>

class QLabel;
class QProgressBar;
class QPushButton;
class QTcpServer;
class QTcpSocket;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

public slots:
    void start();
    void acceptConnection();
    void startTransfer();
    void updateServerProgress();
    void updateClientProgress(Q_LONGLONG numBytes);
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
    int bytesWritten;
    int bytesReceived;
};

#endif
