#ifndef BLOCKINGPROCESS_H
#define BLOCKINGPROCESS_H

#include <qprocess.h>
#include <qbytearray.h>

class BlockingProcess : public QProcess
{
    Q_OBJECT

public:
    BlockingProcess();

    virtual bool start(QStringList *env=0);

public slots:
    void readOut();
    void readErr();
    void exited();

public:
    QByteArray out;
    QByteArray err;
    int outUsed;
    int errUsed;
};

#endif // BLOCKINGPROCESS_H
