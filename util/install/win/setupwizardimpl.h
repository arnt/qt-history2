#ifndef SETUPWIZARDIMPL_H
#define SETUPWIZARDIMPL_H

#include "pages/pages.h"
#include "shell.h"

#include <qprocess.h>
#include <qtimer.h>
#include <qmap.h>
#include <qptrdict.h>
#include <qwizard.h>
#include <qlistview.h>

class QCheckListItem;
class QListView;

class CheckListItem : public QCheckListItem
{
public:
    CheckListItem(QListView *listview, const QString &text, Type = RadioButtonController);
    CheckListItem(QCheckListItem *parent, const QString &text, Type = RadioButtonController);

    void setHelpText(const QString &help, QTextView *display);
    void setWarningText(const QString &warning, CheckListItem *conflict = 0);
    void addRequiredFiles(const QString &file);
    void setRequiredFileLocation(const QString &location);

    void setOpen(bool on);
    void setOn(bool on);
    void setCritical(bool on);

    int rtti() const;
    static int RTTI;

    void displayHelp();
    bool verify() const;

    void paintCell( QPainter *, const QColorGroup & cg, int column, int width, int alignment );

protected:
    bool testAndWarn();
    void activate();

private:
    QString help_text;
    QString warning_text;
    QTextView *help_display;
    QStringList required_files;    
    QString file_location;
    CheckListItem *conflict_with;
    bool critical;
};

class SetupWizardImpl : public QWizard
{
    Q_OBJECT
public:
    SetupWizardImpl( QWidget* parent = 0, const char* name = NULL, bool modal = false, WFlags f = 0 );

    void showPage( QWidget* );
    void stopProcesses();

    void optionClicked( CheckListItem * );

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
#if defined(QSA)
    QProcess assistant;
#endif

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
    void assistantDone();
    void restartBuild();
    void readConfigureOutput();
    void readConfigureError();
    void readCleanerOutput();
    void readCleanerError();
    void readMakeOutput();
    void readMakeError();
    void readAssistantOutput();
    void readAssistantError();
    void timerFired();
    void configPageChanged();
    void archiveMsg(const QString &);
    void licenseChanged();
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

    void fixEnvironment(const QString &var, const QString &file = QString::null);
    void prepareEnvironment();

    void makeDone( bool error );

    void setStaticEnabled( bool se );
    void setJpegDirect( bool jd );
    void readLicenseAgreement();

    bool copyFiles( const QString& sourcePath, const QString& destPath, bool topLevel );
    int totalRead;

    QString buildQtShortcutText;
    bool fixedPath;
    bool filesCopied;
    bool persistentEnv;
    int filesToCompile;
    int filesCompiled;
#if !defined(EVAL) && !defined(EDU) && !defined(NON_COMMERCIAL)
    bool usLicense;
#endif

    QString currentOLine;
    QString currentELine;

    void updateDisplay(const QString &input, QString &output);
#if defined(Q_OS_WIN32)
    void installIcons( const QString& iconFolder, const QString& dirName, bool common );
#endif
    void doIDEIntegration();
    void doStartMenuIntegration();
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
    QStringList allModules;

    CheckListItem *accOn, *accOff;
    CheckListItem *bigCodecsOn, *bigCodecsOff;
    CheckListItem *tabletOn, *tabletOff;
    CheckListItem *advancedSTL, *advancedExceptions, *advancedRTTI;

    CheckListItem /* *mngPresent, */ *mngDirect, *mngPlugin, *mngOff;
    CheckListItem /* *jpegPresent, */ *jpegDirect, *jpegPlugin, *jpegOff;
    CheckListItem /* *pngPresent, */ *pngDirect, *pngPlugin, *pngOff;
    CheckListItem *gifDirect, *gifOff;

    CheckListItem *sgiDirect, *sgiPlugin, *sgiOff;
    CheckListItem *cdeDirect, *cdePlugin, *cdeOff;
    CheckListItem *motifplusDirect, *motifplusPlugin, *motifplusOff;
    CheckListItem *platinumDirect, *platinumPlugin, *platinumOff;
    CheckListItem *motifDirect, *motifPlugin, *motifOff;
    CheckListItem *xpDirect, *xpPlugin, *xpOff;

    CheckListItem *mysqlDirect, *mysqlPlugin, *mysqlOff;
    CheckListItem *ociDirect, *ociPlugin, *ociOff;
    CheckListItem *odbcDirect, *odbcPlugin, *odbcOff;
    CheckListItem *psqlDirect, *psqlPlugin, *psqlOff;
    CheckListItem *tdsDirect, *tdsPlugin, *tdsOff;
    CheckListItem *db2Direct, *db2Plugin, *db2Off;
    CheckListItem *sqliteDirect, *sqlitePlugin, *sqliteOff;
    CheckListItem *ibaseDirect, *ibasePlugin, *ibaseOff;
    
    CheckListItem *zlibDirect, *zlibSystem, *zlibOff;

    CheckListItem *dspOff, *dspOn;
    CheckListItem *vcprojOff, *vcprojOn;

    CheckListItem *staticItem;

#if defined(EVAL) || defined(EDU)
    CheckListItem *mysqlPluginInstall;
    CheckListItem *ociPluginInstall;
    CheckListItem *odbcPluginInstall;
    CheckListItem *psqlPluginInstall;
    CheckListItem *tdsPluginInstall;
    CheckListItem *db2PluginInstall;
    CheckListItem *sqlitePluginInstall;
    CheckListItem *ibasePluginInstall;
#elif defined(NON_COMMERCIAL)
    CheckListItem *sqlitePluginInstall;
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
