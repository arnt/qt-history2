#include <qapplication.h>
#include <qtimer.h>
#include <qdns.h>
#include <qsocket.h>
#include <qtextstream.h>

class QSqlDatabase;

class LicProcApp : public QApplication
{
    Q_OBJECT;
public:
    LicProcApp( int argc, char** argv );

private:
    QTimer syncTimer;

    void ProcessFile( QString fileName );
    void updateDist( QString tag );
    QSqlDatabase* distDB;
    
    QString company;
    int interval;
    int port;
    QDns dns;
    QStringList licenseList;
    QString tag;
    QSocket sock;
    QTextStream stream;
    bool sawGreeting;
public slots:
    virtual void syncLicenses();
    virtual void sockConnected();
    virtual void sockClosed();
    virtual void sockError( int );
    virtual void readyRead();
};