#ifndef THREAD_H
#define THREAD_H

#include <QThread>
#include <QMutex>

class Thread : public QThread
{
    Q_OBJECT

public:
    void requestNewFortune(const QString &hostName, Q_UINT16 port);
    void run();

signals:
    void newFortune(const QString &fortune);
    void error(int socketError, const QString &message);

private:
    QString hostName;
    Q_UINT16 port;
    QMutex mutex;
};

#endif

