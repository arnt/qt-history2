#include <qprocess.h>

#include "setupwizard.h"

class QCheckListItem;

class SetupWizardImpl : public SetupWizard
{
    Q_OBJECT;
public:
    SetupWizardImpl( QWidget* pParent = NULL, const char* pName = NULL, bool modal = FALSE, WFlags f = 0 );

    virtual void clickedPath();
    virtual void clickedSystem( int );
    virtual void licenseAccepted();

    QApplication* app;
protected:
    virtual void pageChanged( const QString& );
private:
    int sysID;
    QByteArray tmpPath;

    QProcess configure;
    QProcess make;
    QCheckListItem* debugMode;
    QCheckListItem* buildType;
    QCheckListItem* threadModel;
    QCheckListItem* modules;
    QCheckListItem* sqldrivers;

    void saveSettings();
    void saveSet( QListView* list );
protected slots:
    void configDone();
    void readConfigureOutput();
    void makeDone();
    void readMakeOutput();

private:
    void readArchive( QString arcname, QString installPath );
    bool createDir( QString fullPath );
    int totalRead;

};