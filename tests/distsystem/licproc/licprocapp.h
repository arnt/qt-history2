#include <qapplication.h>
#include <qtimer.h>

class QSqlDatabase;

class LicProcApp : public QApplication
{
    Q_OBJECT;
public:
    LicProcApp( int argc, char** argv );

private:
    QTimer syncTimer;

    void ProcessFile( QString fileName );
    QString CreatePassword();
    QSqlDatabase* distDB;

public slots:
    virtual void syncLicenses();
};