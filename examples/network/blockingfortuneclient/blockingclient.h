#ifndef BLOCKINGCLIENT_H
#define BLOCKINGCLIENT_H

#include <QDialog>

#include "fortunethread.h"

class QLabel;
class QLineEdit;
class QPushButton;

class BlockingClient : public QDialog
{
    Q_OBJECT

public:
    BlockingClient(QWidget *parent = 0);

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

    FortuneThread thread;
    QString currentFortune;
};

#endif
