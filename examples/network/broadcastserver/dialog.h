#ifndef DIALOG_H_INCLUDED
#define DIALOG_H_INCLUDED

#include <QTimer>
#include <QtGui>
#include <QtNetwork>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

public slots:
    void start();
    void broadcast();

private:
    QLabel *statusLabel;
    QPushButton *startButton;
    QPushButton *quitButton;

    QUdpSocket *broadcastServer;

    QTimer *timer;

    int messageCount;
};

#endif
