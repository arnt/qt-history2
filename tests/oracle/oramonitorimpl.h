#include "oramonitor.h"
#include "qtimer.h"

class QSqlDatabase;

class OraMonitorImpl : public OraMonitor
{
    Q_OBJECT
public:
    OraMonitorImpl( QWidget* parent = NULL, const char* name = NULL, WFlags w = 0 );
    ~OraMonitorImpl();
    
    virtual void clickedConfig();
    virtual void initDB();
    virtual void destroy();
public slots:
    virtual void timerFired();
private:
    QString net8Name;
    QString userName;
    QString password;
    QSqlDatabase* database;
    QTimer monitorTimer;

};
