#ifndef WORKER_H
#define WORKER_H

#include <qobject.h>

// native Qt/C++ class
class Worker : public QObject
{
public:
    Worker();

    void setStatusString(const QString &string);
    QString statusString() const;

private:
    QString status;
};

#endif // WORKER_H
