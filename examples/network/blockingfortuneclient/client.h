#ifndef CLIENT_H
#define CLIENT_H

#include <QDialog>

#include "thread.h"

class QLabel;
class QLineEdit;
class QPushButton;

class Client : public QDialog
{
    Q_OBJECT

public:
    Client(QWidget *parent = 0);

private slots:
    void requestNewFortune();
    void showFortune(const QString &fortune);
    void displayError(int socketError, const QString &message);
    void enableGetFortuneButton();

private:
    QLabel *hostLabel;
    QLabel *portLabel;
    QLineEdit *hostLineEdit;
    QLineEdit *portLineEdit;
    QLabel *statusLabel;
    QPushButton *getFortuneButton;
    QPushButton *quitButton;

    Thread thread;
    QString currentFortune;
};

#endif
