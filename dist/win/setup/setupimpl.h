#ifndef SETUP_H
#define SETUP_H

#include "setup.h"

class QCloseEvent;

class Setup : public SetupBase
{
    Q_OBJECT

public:
    enum Error {
	NoLicenseAgreement,
	NoLicenseFile,
	InvalidLicense,
	CannotWriteConfig,
	CannotWritePlatform,
	CannotWriteQtConfig
    };

    Setup( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~Setup();

protected slots:
    void licenseAccepted( bool );
    void pageChanged( const QString &s );
    void useMSVC( bool b );
    void useBCC( bool b );
    void startCompilation();
    void moduleSelected( QListViewItem *i );
    void gifToggled( bool b );
    
protected:
    void closeEvent( QCloseEvent * ) { reject(); }
    void reject();
    void accept() { ::exit( 0 ); }

private:
    void initLicensePage();
    void initOptionsPage();
    void error( Error e );
    void writeConfigFile();
    void writePlatformFile();
    void writeQtConfigFile();

private:
    bool finished;
    bool isCompiling;

};

#endif // SETUP_H
