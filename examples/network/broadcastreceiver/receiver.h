#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>

class QLabel;
class QPushButton;
class QUdpSocket;

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);

private slots:
    void processPendingDatagrams();

private:
    QLabel *statusLabel;
    QPushButton *quitButton;
    QUdpSocket *udpSocket;
};

#endif
