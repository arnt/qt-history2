#include "pages.h"
#include "../environment.h"
#include <qcombobox.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qdir.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qmultilineedit.h>
#include <qtabwidget.h>
#include <qvalidator.h>
#include <qmessagebox.h>
#include <setupwizardimpl.h>

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif

extern SetupWizardImpl *wizard;

BuildPageImpl::BuildPageImpl( QWidget* parent, const char* name, WFlags fl )
    : BuildPage( parent, name, fl )
{
}

ConfigPageImpl::ConfigPageImpl( QWidget* parent, const char* name, WFlags fl )
    : ConfigPage( parent, name, fl )
{
    if( globalInformation.reconfig() ) {
	currentInstLabel->show();
	currentInstallation->show();
#if defined(Q_OS_WIN32)
	// Makes no sense to have the rebuild installation option on DOS based
	// Windows
	if ( qWinVersion() & WV_NT_based )
#endif
	    rebuildInstallation->show();
#if defined(Q_OS_WIN32)
	else {
	    rebuildInstallation->setChecked( FALSE );
	    rebuildInstallation->hide();
	}
#endif
    } else {
	currentInstLabel->hide();
	currentInstallation->hide();
	rebuildInstallation->hide();
    }
#if defined(EVAL) || defined(EDU)
    // ### these pages should probably be included but all options should be
    // disabled so that the evaluation customer can see how he can configure Qt
    configTabs->removePage( generalTab );
    configTabs->removePage( advancedTab );
#else
    configTabs->removePage( installTab );
#endif
}

FinishPageImpl::FinishPageImpl( QWidget* parent, const char* name, WFlags fl )
    : FinishPage( parent, name, fl )
{
#if !defined(Q_OS_WIN32)
    showReadmeCheck->hide();
#endif
}

FoldersPageImpl::FoldersPageImpl( QWidget* parent, const char* name, WFlags fl )
    : FoldersPage( parent, name, fl )
{
#if defined(Q_OS_WIN32)
    QByteArray buffer( 256 );
    unsigned long buffSize( buffer.size() );
    GetUserNameA( buffer.data(), &buffSize );
    folderGroups->insertItem( "Anyone who uses this computer (all users)" );
    folderGroups->insertItem( QString( "Only for me (" ) + QString( buffer.data() ) + ")" );
    folderPath->setText( QString( "Qt " ) + globalInformation.qtVersionStr() );
    if( qWinVersion() & Qt::WV_NT_based )   // On NT we also have a common folder
	folderGroups->setEnabled( true );
    else
	folderGroups->setDisabled( true );
    qtDirCheck->setChecked( ( QEnvironment::getEnv( "QTDIR" ).length() == 0 ) );
#elif defined(Q_OS_UNIX)
    folderGroups->setDisabled( true );
#endif
}

LicenseAgreementPageImpl::LicenseAgreementPageImpl( QWidget* parent, const char* name, WFlags fl )
    : LicenseAgreementPage( parent, name, fl )
{
    connect( licenceButtons, SIGNAL(clicked(int)), SLOT(licenseAction(int)));
}

void LicenseAgreementPageImpl::licenseAction(int act)
{
    if( act )
	wizard->setNextEnabled( this, false );
    else
	wizard->setNextEnabled( this, true );
}

LicensePageImpl::LicensePageImpl( QWidget* parent, const char* name, WFlags fl )
    : LicensePage( parent, name, fl )
{
#if defined(Q_OS_MACX)
    // StyledPanel style looks very windowsish
    customerID->setFrameShape( QFrame::LineEditPanel );
#endif
    customerID->setFocus();
#if defined(EVAL)
    // ### improve text
    licenseInfoHeader->setText( tr("Thank you for your interest in Qt.\n"
		"Please enter the license information you got for this evaluation version of Qt.") );

    customerIDLabel->setText( tr("Name") );
    licenseIDLabel->setText( tr("Company name") );
    licenseeLabel->setText( tr("Serial number") );
    evalName = customerID;
    evalCompany = licenseID;
    serialNumber = licenseeName;

    expiryLabel->hide();
    expiryDate->hide();
    productsLabel->hide();
    productsString->hide();
    keyLabel->hide();
    key->hide();
    readLicenseButton->hide();
#elif defined(EDU)
    licenseInfoHeader->setText( tr("Please enter the license information for the educational edition of Qt.") );

    customerIDLabel->setText( tr("Educational institution") );
    licenseeLabel->setText( tr("Serial number") );
    university = customerID;
    serialNumber = licenseeName;

    licenseIDLabel->hide();
    licenseID->hide();
    expiryLabel->hide();
    expiryDate->hide();
    productsLabel->hide();
    productsString->hide();
    keyLabel->hide();
    key->hide();
    readLicenseButton->hide();
#else
    licenseID->setValidator( new QIntValidator( 100000, -1, licenseID ) );

    // expiryDate and productsString comes from the license key
    expiryDate->setEnabled( FALSE );
    productsString->setEnabled( FALSE );
    keyLabel->setText( tr("License key") );
    licenseInfoHeader->setText( tr("Please enter your license information.\n"
		"The License key is required to be able to proceed with the installation process.") );
#endif
}

QValidator::State InstallPathValidator::validate( QString& input, int& ) const
{
    if ( ( globalInformation.sysId() == GlobalInformation::MSVC ||
	   globalInformation.sysId() == GlobalInformation::MSVCNET )
	   && input.contains( QRegExp("\\s") ) ) {
	QMessageBox::warning( 0, "Invalid directory", "No whitespace is allowed in the directory name due to a limitation with MSVC" );
	return Intermediate;
    } else if ( globalInformation.sysId() == GlobalInformation::Borland && input.contains( "-" ) ) {
	QMessageBox::warning( 0, "Invalid directory", "No '-' characters are allowed in the directory name due to a limitation in the "
			      "Borland linker" );
	return Intermediate;
    }
    return Acceptable;
}

OptionsPageImpl::OptionsPageImpl( QWidget* parent, const char* name, WFlags fl )
    : OptionsPage( parent, name, fl )
{
#if defined(Q_OS_WIN32)
    installPath->setText(
	    QString( "C:\\Qt\\" ) +
	    QString( globalInformation.qtVersionStr() ).replace( QRegExp("\\s"), "" ).replace( QRegExp("-"), "" )
	    );
    installPath->setValidator( new InstallPathValidator( this ) );
    connect( sysMsvcNet, SIGNAL(toggled(bool)), installNETIntegration, SLOT(setEnabled(bool)) );
#elif defined(Q_OS_MACX)
    // ### the replace for Windows is done because qmake has problems with
    // spaces and Borland has problems with "-" in the filenames -- I don't
    // think that there is a need for this on Mac (rms)
    QString base("QtMac-");
#if defined(EVAL)
    base += "Eval-";
#elif defined(EDU)
    base += "Edu-";
#endif
    installPath->setText(base + QString( globalInformation.qtVersionStr() ).replace( QRegExp("\\s"), "" ));
    sysGroup->hide();
#endif
}

ProgressPageImpl::ProgressPageImpl( QWidget* parent, const char* name, WFlags fl )
    : ProgressPage( parent, name, fl )
{
    // ######### At the moment, we show only one line when unpacking. So the
    // horizontal scrollbar is never shown for now to avoid flickering.
    filesDisplay->setHScrollBarMode( QScrollView::AlwaysOff );
}

#if defined(Q_OS_WIN32)
WinIntroPageImpl::WinIntroPageImpl( QWidget* parent, const char* name, WFlags fl )
    : WinIntroPage( parent, name, fl )
{
}
#endif
