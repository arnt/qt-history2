#ifndef DIALOG_H_INCLUDED
#define DIALOG_H_INCLUDED

#include <QtGui>
#include <QtNetwork>

class FortuneThread;

class Dialog : public QDialog
{
    Q_OBJECT

public:
    Dialog(QWidget *parent = 0);

public slots:
    void requestFortune();
    void readFortune(const QString &fortune);
    void displayError(int, const QString &);

signals:
    void newFortune(const QString &fortune);
    void fortuneRequestReady(const QString &hostName, Q_UINT16 port);

private:
    void showMessage(const QString &title, const QString &text);

    QLineEdit *serverPortEdit;
    QLineEdit *serverHostEdit;
    QLabel *statusLabel;
    QPushButton *fortuneButton;
    QPushButton *quitButton;

    FortuneThread *fortuneThread;

    QString lastFortune;
};

#endif
