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

#if defined(Q_OS_WIN32)
#include <windows.h>
#endif

BuildPageImpl::BuildPageImpl( QWidget* parent, const char* name, WFlags fl )
    : BuildPage( parent, name, fl )
{
#if 0
    outputDisplay->setWordWrap( QTextView::WidgetWidth );
    outputDisplay->setWrapPolicy( QTextView::Anywhere );
#endif
}

ConfigPageImpl::ConfigPageImpl( QWidget* parent, const char* name, WFlags fl )
    : ConfigPage( parent, name, fl )
{
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
}

LicensePageImpl::LicensePageImpl( QWidget* parent, const char* name, WFlags fl )
    : LicensePage( parent, name, fl )
{
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
    evalSerialNumber = licenseeName;

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

OptionsPageImpl::OptionsPageImpl( QWidget* parent, const char* name, WFlags fl )
    : OptionsPage( parent, name, fl )
{
    installDocs->hide();
    installExamples->hide();
    installTutorials->hide();
#if defined(Q_OS_WIN32)
    installPath->setText(
	    QString( "C:\\Qt\\" ) +
	    QString( globalInformation.qtVersionStr() ).replace( QRegExp("\\s"), "" ).replace( QRegExp("-"), "" )
	    );
#elif defined(Q_OS_MACX)
    // ### the replace for Windows is done because qmake has problems with
    // spaces and Borland has problems with "-" in the filenames -- I don't
    // think that there is a need for this on Mac (rms)
    installPath->setText(
	    QString( QDir::homeDirPath() + "/" ) +
	    QString( globalInformation.qtVersionStr() ).replace( QRegExp("-"), "" )
	    );
    sysGroup->hide();
#endif
#if defined(EVAL)
    installDocs->setEnabled( FALSE );
    sysMsvc->setEnabled( FALSE );
    sysBorland->setEnabled( FALSE );
#endif
}

ProgressPageImpl::ProgressPageImpl( QWidget* parent, const char* name, WFlags fl )
    : ProgressPage( parent, name, fl )
{
#if 0
    setWordWrap( QTextView::WidgetWidth );
    setWrapPolicy( QTextView::Anywhere );
#endif
}

#if defined(Q_OS_WIN32)
WinIntroPageImpl::WinIntroPageImpl( QWidget* parent, const char* name, WFlags fl )
    : WinIntroPage( parent, name, fl )
{
}
#endif
