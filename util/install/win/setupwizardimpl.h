#ifndef SETUPWIZARDIMPL_H
#define SETUPWIZARDIMPL_H

#include "pages/pages.h"
#include "shell.h"

#include <qprocess.h>
#include <qtimer.h>
#include <qmap.h>
#include <qptrdict.h>
#include <qwizard.h>

class QCheckListItem;
class QListView;

class SetupWizardImpl : public QWizard
{
    Q_OBJECT
public:
    SetupWizardImpl( QWidget* pParent = NULL, const char* pName = NULL, bool modal = FALSE, WFlags f = 0 );

    void showPage( QWidget* );
    void stopProcesses();

signals:
    void wizardPages( const QPtrList<Page>& );
    void wizardPageShowed( int );
    void wizardPageFailed( int );
    void editionString( const QString& );

private:
    int totalFiles;
    QProcess configure;
    QProcess make;
    QProcess cleaner;

    QString programsFolder;
    QString devSysFolder;
    QString tmpPath;

    WinShell shell;

    void saveSettings();
    void saveSet( QListView* list );

protected slots:
    void accept(); // reimplemented from QDialog

private slots:
    void clickedSystem( int );
    void sysOtherComboChanged( int ); 
    void clickedFolderPath();
    void clickedDevSysPath();
    void clickedLicenseFile();
    void cleanDone();
    void configDone();
    void makeDone();
    void restartBuild();
    void readConfigureOutput();
    void readConfigureError();
    void readCleanerOutput();
    void readCleanerError();
    void readMakeOutput();
    void readMakeError();
    void timerFired();
    void optionSelected( QListViewItem * );
    void optionClicked( QListViewItem * );
    void configPageChanged();
    void archiveMsg(const QString &);
    void licenseChanged();
    void clickedSkipBuild();
    bool verifyConfig();

private:
    void showPageLicense();
    void showPageOptions();
    void showPageFolders();
    void showPageConfig();
    void showPageProgress();
    void showPageBuild();
    void showPageFinish();

    void initPages();
    void initConnections();

    void prepareEnvironment();

    bool findFile( const QString &fileName );
    bool findFileInPaths( const QString &fileName, const QStringList &paths );

    void makeDone( bool error );

    void setStaticEnabled( bool se );
    void setJpegDirect( bool jd );
    void readLicenseAgreement();
    bool findXPSupport();

    bool copyFiles( const QString& sourcePath, const QString& destPath, bool topLevel );
    int totalRead;

    QString buildQtShortcutText;

    bool filesCopied;
    bool persistentEnv;
    int filesToCompile;
    int filesCompiled;
#if !defined(EVAL) && !defined(EDU)
    bool usLicense;
#endif

    QString currentOLine;
    QString currentELine;

    void updateOutputDisplay( QProcess* proc );
    void updateErrorDisplay( QProcess* proc );
#if defined(Q_OS_WIN32)
    void installIcons( const QString& iconFolder, const QString& dirName, bool common );
#endif
    void doFinalIntegration();
    void logFiles( const QString& entry, bool close = FALSE );
    void logOutput( const QString& entry, bool close = FALSE );

    void setInstallStep( int step );
    void readLicense( QString filePath );
    void writeLicense( QString filePath );

    QFile fileLog;
    QFile outputLog;
    QMap<QString,QString> licenseInfo;
    QTimer autoContTimer;
    int timeCounter;
    QStringList allModules;

    QCheckListItem *accOn, *accOff;
    QCheckListItem *bigCodecsOn, *bigCodecsOff;
    QCheckListItem *tabletOn, *tabletOff;
    QCheckListItem *advancedSTL, *advancedExceptions, *advancedRTTI;

    QCheckListItem /* *mngPresent, */ *mngDirect, *mngPlugin, *mngOff;
    QCheckListItem /* *jpegPresent, */ *jpegDirect, *jpegPlugin, *jpegOff;
    QCheckListItem /* *pngPresent, */ *pngDirect, *pngPlugin, *pngOff;
    QCheckListItem *gifDirect, *gifOff;

    QCheckListItem *sgiDirect, *sgiPlugin, *sgiOff;
    QCheckListItem *cdeDirect, *cdePlugin, *cdeOff;
    QCheckListItem *motifplusDirect, *motifplusPlugin, *motifplusOff;
    QCheckListItem *platinumDirect, *platinumPlugin, *platinumOff;
    QCheckListItem *motifDirect, *motifPlugin, *motifOff;
    QCheckListItem *xpDirect, *xpPlugin, *xpOff;

    QCheckListItem *mysqlDirect, *mysqlPlugin, *mysqlOff;
    QCheckListItem *ociDirect, *ociPlugin, *ociOff;
    QCheckListItem *odbcDirect, *odbcPlugin, *odbcOff;
    QCheckListItem *psqlDirect, *psqlPlugin, *psqlOff;
    QCheckListItem *tdsDirect, *tdsPlugin, *tdsOff;
    QCheckListItem *db2Direct, *db2Plugin, *db2Off;
    
    QCheckListItem *zlibDirect, *zlibSystem, *zlibOff;

    QCheckListItem *dspOff, *dspOn;
    QCheckListItem *vcprojOff, *vcprojOn;

    QCheckListItem *staticItem;

#if defined(EVAL) || defined(EDU)
    QCheckListItem *mysqlPluginInstall;
    QCheckListItem *ociPluginInstall;
    QCheckListItem *odbcPluginInstall;
    QCheckListItem *psqlPluginInstall;
    QCheckListItem *tdsPluginInstall;
    QCheckListItem *db2PluginInstall;
#endif

    // wizard pages
    LicenseAgreementPageImpl	*licenseAgreementPage;
    LicenseAgreementPageImpl	*licenseAgreementPageQsa;
    LicensePageImpl		*licensePage;
    OptionsPageImpl		*optionsPage;
    OptionsPageImpl		*optionsPageQsa;
    FoldersPageImpl		*foldersPage;
    ConfigPageImpl		*configPage;
    ProgressPageImpl		*progressPage;
    BuildPageImpl		*buildPage;
    FinishPageImpl		*finishPage;
#if defined(Q_OS_WIN32)
    WinIntroPageImpl		*winIntroPage;
#endif
};

#endif
