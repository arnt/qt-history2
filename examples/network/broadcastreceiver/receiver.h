#ifndef RECEIVER_H
#define RECEIVER_H

#include <QDialog>

class QLabel;
class QPushButton;
class QUdpSocket;

class Receiver : public QDialog
{
    Q_OBJECT

public:
    Receiver(QWidget *parent = 0);

private slots:
    void processPendingDatagrams();

private:
    QLabel *statusLabel;
    QPushButton *quitButton;
    QUdpSocket *udpSocket;
};

#endif
