#ifndef SERVER_H
#define SERVER_H

#include <QDialog>

class QLabel;
class QPushButton;
class QTimer;
class QUdpSocket;

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(QWidget *parent = 0);

private slots:
    void startBroadcasting();
    void broadcastDatagram();

private:
    QLabel *statusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;
    QUdpSocket *udpSocket;
    QTimer *timer;
    int messageNo;
};

#endif
