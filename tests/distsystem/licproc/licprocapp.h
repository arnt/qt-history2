#include <qapplication.h>
#include <qtimer.h>
#include <qdns.h>

class QSqlDatabase;

class LicProcApp : public QApplication
{
    Q_OBJECT;
public:
    LicProcApp( int argc, char** argv );

private:
    QTimer syncTimer;

    void ProcessFile( QString fileName );
    QSqlDatabase* distDB;
    
    QString company;
    int interval;
    int port;
    QDns dns;
    QStringList licenseList;
public slots:
    virtual void syncLicenses();
    virtual void dnsReady();
};