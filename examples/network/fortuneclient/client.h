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

private slots:
    void requestNewFortune();
    void readFortune();
    void displayError(int socketError);
    void enableGetFortuneButton();
    void booga() { qDebug("connected"); }

private:
    QLabel *hostLabel;
    QLabel *portLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLabel *statusLabel;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;

    QTcpSocket *tcpSocket;
    QString currentFortune;
    Q_UINT16 blockSize;
};

#endif
