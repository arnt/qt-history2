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
    void requestFortune();
    void readFortune();
    void displayError(int);

private:
    void showMessage(const QString &title, const QString &text);

    QLineEdit *serverPortEdit;
    QLineEdit *serverHostEdit;
    QLabel *statusLabel;
    QPushButton *fortuneButton;
    QPushButton *quitButton;

    QTcpSocket *fortuneClient;

    QByteArray lastFortune;
};

#endif
