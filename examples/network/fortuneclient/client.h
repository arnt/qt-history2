#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>

class QLabel;
class QLineEdit;
class QPushButton;
class QTcpSocket;

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);

public slots:
    void requestNewFortune();
    void readFortune();
    void displayError(int socketError);

private:
    int blockSize;
    QLabel *portLabel;
    QLabel *hostLabel;
    QLineEdit *portLineEdit;
    QLineEdit *hostLineEdit;
    QLabel *statusLabel;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    QTcpSocket *tcpSocket;
    QString currentFortune;
};

#endif
