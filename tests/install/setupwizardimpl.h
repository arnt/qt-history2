#include <qprocess.h>

#include "setupwizard.h"
#include "shell.h"

class QCheckListItem;

class SetupWizardImpl : public SetupWizard
{
    Q_OBJECT;
public:
    SetupWizardImpl( QWidget* pParent = NULL, const char* pName = NULL, bool modal = FALSE, WFlags f = 0 );

    virtual void clickedPath();
    virtual void clickedSystem( int );
    virtual void licenseAction( int );
    virtual void clickedFolderPath();
    virtual void clickedDevSysPath();
//    virtual void clickedEnvironmentButton();

    virtual void showPage( QWidget* );
    QApplication* app;
protected:
//    virtual void pageChanged( const QString& );
private:
    int sysID;

    QProcess configure;
    QProcess make;
    QProcess integrator;
    QCheckListItem* debugMode;
    QCheckListItem* buildType;
    QCheckListItem* threadModel;
    QCheckListItem* modules;
    QCheckListItem* sqldrivers;

    QString programsFolder;
    QString devSysFolder;
    QString tmpPath;

    WinShell shell;

    void saveSettings();
    void saveSet( QListView* list );
protected slots:
    virtual void configDone();
    virtual void makeDone();
    virtual void integratorDone();
    virtual void readConfigureOutput();
    virtual void readConfigureError();
    virtual void readMakeOutput();
    virtual void readMakeError();
    virtual void readIntegratorOutput();
    virtual void readIntegratorError();
//    virtual void envDone();

private:
#if defined (USE_ARCHIVES)
    void readArchive( const QString& arcname, const QString& installPath );
#else
    bool copyFiles( const QString& sourcePath, const QString& destPath, bool topLevel = false );
#endif
    bool createDir( const QString& fullPath );
    int totalRead;

    bool filesCopied;
    bool persistentEnv;
    int filesToCompile;
    int filesCompiled;

    QString currentOLine;
    QString currentELine;

    void updateOutputDisplay( QProcess* proc );
    void updateErrorDisplay( QProcess* proc );
    void installIcons( const QString& iconFolder, const QString& dirName, bool common );
    void doFinalIntegration();
    enum {
	MSVC = 0,
	Borland = 1,
	GCC = 2
    };
    void logFiles( const QString& entry, bool close = false );
    void logOutput( const QString& entry, bool close = false );

    void setInstallStep( int step );
    QFile fileLog;
    QFile outputLog;
public:
    void stopProcesses();
};
