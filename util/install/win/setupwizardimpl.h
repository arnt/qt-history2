#include <qprocess.h>
#include <qtimer.h>
#include <qmap.h>

#include "setupwizard.h"
#include "shell.h"

class QCheckListItem;

class SetupWizardImpl : public SetupWizard
{
    Q_OBJECT
public:
    SetupWizardImpl( QWidget* pParent = NULL, const char* pName = NULL, bool modal = FALSE, WFlags f = 0, bool reconfig = false );

// Slots reimplementations
    virtual void clickedPath();
    virtual void clickedSystem( int );
    virtual void licenseAction( int );
    virtual void clickedFolderPath();
    virtual void clickedDevSysPath();
    virtual void clickedLicenseFile();
//    virtual void clickedEnvironmentButton();

    virtual void showPage( QWidget* );
    void stopProcesses();

    void setReconfigMode( bool );
protected:
//    virtual void pageChanged( const QString& );
private:
    int sysID;

    int totalFiles;
    QProcess configure;
    QProcess make;
    QProcess integrator;
    QProcess cleaner;

    QString programsFolder;
    QString devSysFolder;
    QString tmpPath;

    WinShell shell;

    void saveSettings();
    void saveSet( QListView* list );
protected slots:
    virtual void cleanDone();
    virtual void configDone();
    virtual void makeDone();
    virtual void integratorDone();
    virtual void readConfigureOutput();
    virtual void readConfigureError();
    virtual void readCleanerOutput();
    virtual void readCleanerError();
    virtual void readMakeOutput();
    virtual void readMakeError();
    virtual void readIntegratorOutput();
    virtual void readIntegratorError();
    virtual void timerFired();
    void optionSelected( QListViewItem * );
    void optionClicked( QListViewItem * );
    void configPageChanged();
    void archiveMsg(const QString &);
    void licenseChanged();
//    virtual void envDone();

private:
    void showPageLicense();
    void showPageFolders();
    void showPageConfig();
    void showPageProgress();
    void showPageBuild();
    void showPageFinish();

    void prepareEnvironment();

    bool findFileInPaths( QString fileName, QStringList paths );
    void setStaticEnabled( bool se );
    void setJpegDirect( bool jd );
    void readLicenseAgreement();

    bool copyFiles( const QString& sourcePath, const QString& destPath, bool topLevel );
    int totalRead;

    QString qt_version_str;
    QString buildQtShortcutText;

    bool filesCopied;
    bool persistentEnv;
    int filesToCompile;
    int filesCompiled;
    bool reconfigMode;
    bool usLicense;

    QString currentOLine;
    QString currentELine;

    void updateOutputDisplay( QProcess* proc );
    void updateErrorDisplay( QProcess* proc );
#if defined(Q_OS_WIN32)
    void installIcons( const QString& iconFolder, const QString& dirName, bool common );
#endif
    void doFinalIntegration();
    enum {
	MSVC = 0,
	Borland = 1,
	GCC = 2,
	MACX = 3
    };
    void logFiles( const QString& entry, bool close = false );
    void logOutput( const QString& entry, bool close = false );

    void setInstallStep( int step );
    void readLicense( QString filePath );
    void writeLicense( QString filePath );

    QFile fileLog;
    QFile outputLog;
    QMap<QString,QString> licenseInfo;
    QTimer autoContTimer;
    int timeCounter;
    bool triedToIntegrate;
    QStringList allModules;

    QCheckListItem *accOn, *accOff;
    QCheckListItem *bigCodecsOn, *bigCodecsOff;
    QCheckListItem *tabletOn, *tabletOff;
    QCheckListItem *advancedCppOn, *advancedCppOff;

    QCheckListItem /* *mngPresent, */ *mngDirect, *mngPlugin, *mngOff;
    QCheckListItem /* *jpegPresent, */ *jpegDirect, *jpegPlugin, *jpegOff;
    QCheckListItem /* *pngPresent, */ *pngDirect, *pngPlugin, *pngOff;
    QCheckListItem *gifDirect, *gifOff;

    QCheckListItem *sgiDirect, *sgiPlugin, *sgiOff;
    QCheckListItem *cdeDirect, *cdePlugin, *cdeOff;
    QCheckListItem *motifplusDirect, *motifplusPlugin, *motifplusOff;
    QCheckListItem *platinumDirect, *platinumPlugin, *platinumOff;
    QCheckListItem *motifDirect, *motifPlugin, *motifOff;

    QCheckListItem *mysqlDirect, *mysqlPlugin, *mysqlOff;
    QCheckListItem *ociDirect, *ociPlugin, *ociOff;
    QCheckListItem *odbcDirect, *odbcPlugin, *odbcOff;
    QCheckListItem *psqlDirect, *psqlPlugin, *psqlOff;
    QCheckListItem *tdsDirect, *tdsPlugin, *tdsOff;

    QCheckListItem *staticItem;

#if defined(EVAL)
    QLineEdit* evalName;
    QLineEdit* evalCompany;
    QLineEdit* evalSerialNumber;
#endif
};
