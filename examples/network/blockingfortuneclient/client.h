#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>

#include "thread.h"

class QLabel;
class QLineEdit;
class QPushButtonM;

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);

public slots:
    void requestNewFortune();
    void showFortune(const QString &fortune);
    void displayError(int, const QString &);

private:
    QLabel *portLabel;
    QLabel *hostLabel;
    QLineEdit *portLineEdit;
    QLineEdit *hostLineEdit;
    QLabel *statusLabel;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;
    FortuneThread fortuneThread;
    QString currentFortune;
};

#endif
