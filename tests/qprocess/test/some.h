#include <qobject.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qprocess.h>


class Some : public QObject
{
    Q_OBJECT

public:
    Some( QObject *, bool cStdout, bool cStderr, bool cExitp, int com );
    ~Some() { };

public slots:
    void kill();
    void hup();
    void updateInfo();
    void showInfo();
    void procExited();

    void connectStdout( bool enable );
    void connectStderr( bool enable );
    void connectExit( bool enable );
    void writeMuch();
    void wroteStdin();

signals:

private:
    QVBox main;
    QVBox info;
    QLabel *isRunningInfo;
    QLabel *normalExitInfo;
    QLabel *exitStatusInfo;
    QProcess *proc;
    bool hideAfterExit;
};


class SomeFactory : public QObject
{
    Q_OBJECT

public:
    SomeFactory() : cStdout(FALSE), cStderr(FALSE), cExit(FALSE)
    { parent = new QObject; };
    ~SomeFactory()
    { delete parent; };

public slots:
    void newProcess0();
    void newProcess1();
    void newProcess2();
    void quit();

    void connectStdout( bool enable );
    void connectStderr( bool enable );
    void connectExit( bool enable );

signals:
    void quitted();

private:
    QObject *parent;
    bool cStdout;
    bool cStderr;
    bool cExit;
};
