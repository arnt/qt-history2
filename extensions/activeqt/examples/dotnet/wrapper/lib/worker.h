#ifndef WORKER_H
#define WORKER_H

#include <qobject.h>

// native Qt/C++ class
class Worker : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString statusString READ statusString WRITE setStatusString)
public:
    Worker();

    QString statusString() const;

public slots:
    void setStatusString(const QString &string);

signals:
    void statusStringChanged(const QString &string);

private:
    QString status;
};

#endif // WORKER_H
