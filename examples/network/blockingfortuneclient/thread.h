#ifndef THREAD_H
#define THREAD_H

#include <QThread>

class QString;

class FortuneThread : public QThread
{
    Q_OBJECT

public:
    void run();
    void requestNewFortune(const QString &hostName, Q_UINT16 port);

signals:
    void newFortune(const QString &);
    void error(int, const QString &);

private:
    QString hostName;
    Q_UINT16 port;
};

#endif

