#ifndef DIALOG_H_INCLUDED
#define DIALOG_H_INCLUDED

#include <QtGui>
#include <QtNetwork>

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

public slots:
    void sendFortune();

private:
    QLabel *statusLabel;
    QPushButton *quitButton;

    QTcpServer *fortuneServer;

    QList<QByteArray> fortunes;
};

#endif
