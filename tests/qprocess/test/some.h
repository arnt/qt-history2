#include <qobject.h>
#include <qvbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qprocess.h>
#include <qtextview.h>
#include <qfile.h>


class Some : public QObject
{
    Q_OBJECT

public:
    Some( QObject *, bool start, bool cStdout, bool cStderr, bool cExitp, int com );
    ~Some() { };

public slots:
    void kill();
    void hup();
    void updateInfo();
    void showInfo();
    void procExited();
    void startCat();

    void connectStdout( bool enable );
    void connectStderr( bool enable );
    void connectExit( bool enable );
    void writeMuch();
    void writeToStdin();

    void readyReadStdout();
    void readyReadStderr();

signals:

private:
    void procInit( bool start, int com );

    QVBox main;
    QVBox info;
    int protocolReadStdout;
    int protocolReadStderr;
    QTextView protocol;
    QLabel *out;
    QLabel *err;
    QLabel *isRunningInfo;
    QLabel *normalExitInfo;
    QLabel *exitStatusInfo;
    QProcess *proc;
    bool hideAfterExit;
    bool stdoutConnected;
    bool stderrConnected;
    bool exitConnected;

    static QFile logFile;
    void logMessage( const QString& );
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
    void startProcess0();
    void startProcess1();
    void startProcess2();
    void startProcess3();
    void launchProcess0();
    void launchProcess1();
    void launchProcess2();
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
