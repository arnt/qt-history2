#include <qobject.h>
#include <qvbox.h>
#include <qlabel.h>

#include "../qprocess.h"


class Some : public QObject
{
    Q_OBJECT

public:
    Some( QObject *p );
    ~Some() { };

public slots:
    void kill();
    void hup();
    void updateInfo();
    void showInfo();
    void procExited();

signals:

private:
    QVBox main;
    QVBox info;
    QLabel *isRunningInfo;
    QLabel *normalExitInfo;
    QLabel *exitStatusInfo;
    QProcess *proc;
};


class SomeFactory : public QObject
{
    Q_OBJECT

public:
    SomeFactory()
    { parent = new QObject; };
    ~SomeFactory()
    { delete parent; };

public slots:
    void newProcess();
    void quit();

signals:
    void quitted();

private:
    QObject *parent;
};
