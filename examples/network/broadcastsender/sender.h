#ifndef SENDER_H
#define SENDER_H

#include <QDialog>

class QLabel;
class QPushButton;
class QTimer;
class QUdpSocket;

class Sender : public QDialog
{
    Q_OBJECT

public:
    Sender(QWidget *parent = 0);

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
