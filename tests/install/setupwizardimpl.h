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
    virtual void licenseAccepted();
    virtual void clickedFolderPath();
    virtual void clickedDevSysPath();
    virtual void clickedSave();

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
    void configDone();
    void makeDone();
    void integratorDone();
    void readConfigureOutput();
    void readMakeOutput();
    void readIntegratorOutput();

private:
    void readArchive( QString arcname, QString installPath );
    bool createDir( QString fullPath );
    int totalRead;

    bool filesCopied;
    int filesToCompile;
    int filesCompiled;

    QString currentLine;

    void updateOutputDisplay( QProcess* );
    void installIcons( QString iconFolder, QString dirName, bool common );

    enum {
	MSVC = 0,
	Borland = 1,
	GCC = 2
    };
};