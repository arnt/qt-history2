#include "unpackdlgimpl.h"
#include "licensedlgimpl.h"
#include <qdir.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qarchive.h>
#include <qmessagebox.h>
#include <qregexp.h>
#include <qapplication.h>
#include "keyinfo.h"
#include <unistd.h>

UnpackDlgImpl::UnpackDlgImpl( QString key, QWidget* pParent, const char* pName, WFlags f ) : 
    UnpackDlg( pParent, pName, f )
{
    destPath->setText( QDir::currentDirPath() );
    if(!key.isNull()) {
	srcKey->setText(key);
    } else if(QFile::exists( QDir::homeDirPath() + "/" + ".qt-license")) {
	QFile lic( QDir::homeDirPath() + "/" + ".qt-license");
	if( lic.open( IO_ReadOnly ) ) {
	    QString buffer;
	    while( lic.readLine( buffer, 1024 ) != -1 ) {
		if( buffer[ 0 ] != '#' ) {
		    QStringList components = QStringList::split( '=', buffer );
		    QStringList::Iterator it = components.begin();
		    QString key = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null );
		    if(key.upper() == "LICENSEKEY") {
			QString value = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null );
			srcKey->setText(value);
		    }
		}
	    }
	    lic.close();
	}
    }
    connect( srcKey, SIGNAL( textChanged( const QString& ) ), this, SLOT( licenseKeyChanged() ) );
    licenseKeyChanged();
    logOutput->setWordWrap( QTextView::WidgetWidth );
    logOutput->setWrapPolicy( QTextView::Anywhere );
}

void UnpackDlgImpl::clickedDestButton()
{
    QString dest =  QFileDialog::getExistingDirectory( destPath->text(), this, NULL, "Select destination directory" );
    if (!dest.isNull())
	destPath->setText( dest );
}

void UnpackDlgImpl::clickedUnpack()
{
    QArchive archive;
    connect( &archive, SIGNAL( operationFeedback( const QString& ) ), this, SLOT( updateProgress( const QString& ) ) );
    connect( &archive, SIGNAL( operationFeedback( int ) ), this, SLOT( updateProgress( int ) ) );

    archive.setVerbosity( QArchive::Destination | QArchive::Verbose | QArchive::Progress );

    QString dest = destPath->text(), src="qt-mac-commercial-3.0.0.app/Contents/Qt/qtmac.arq";
    if(!dest.isEmpty() && dest.right(1) != "/")
	dest += "/";
    archive.setPath( src );
    if( !archive.open( IO_ReadOnly ) ) {
	QMessageBox::critical( NULL, "Failure", "Failed to open input " + src);
	return;
    } else if(!QFile::exists(dest) ) {
	QDir d;
	if(!d.mkdir(dest)) {
	    QMessageBox::critical( NULL, "Failure", "Failed to open directory " + dest);
	    return;
	}
    }
    unpackButton->setDisabled( true );
    srcKey->setDisabled( true );
    destPath->setDisabled( true );
    destButton->setDisabled( true );
    LicenseDialogImpl licenseDialog( this );
    if((!licenseDialog.showLicense( featuresForKeyOnUnix( srcKey->text() ) & Feature_US )) ||
       (!licenseDialog.exec())) {
	QMessageBox::critical( NULL, "Failure", "The license agreement was rejected." );
	updateProgress( "License rejected" );
	unpackButton->setDisabled( false );
	srcKey->setDisabled( false );
	destPath->setDisabled( false );
	destButton->setDisabled( false );
	return;
    }

    QString srcName  = "qt-mac-commercial-3.0.0.app/Contents/Qt/LICENSE";
    QString destName = "/qt-mac-commercial-3.0.0/.LICENSE";
    QString srcName2 = srcName;
    if ( featuresForKeyOnUnix( srcKey->text() ) & Feature_US )
	srcName2 = srcName2 + "-US";
    if((!archive.readArchive( dest, srcKey->text() )) ||
       (!copyFile( srcName, dest + destName )) ||
       (!copyFile( srcName + "-US", dest + destName + "-US" )) ||
       (!copyFile( srcName2, dest + "/qt-mac-commercial-3.0.0/LICENSE" ))) 
    {
	QMessageBox::critical( NULL, "Failure", "Failed to unpack " + src);
	archive.close();
    }	
    else {
	QMessageBox::information( NULL, "Archive unpacked", "Qt has been "
	   "extracted to " + dest + "qt-mac-commerical-3.0.0\nPlease read "
	   "the INSTALL.macosx file for installation instructions."  );
	cancelButton->setText( "Quit" );
    }
}

bool UnpackDlgImpl::copyFile( const QString& src, const QString& dest )
{
    int len;
    const int buflen = 4096;
    char buf[buflen];
    QFileInfo info( src );
    QFile srcFile( src ), destFile( dest );
    if (!srcFile.open( IO_ReadOnly ))
	return false;
    destFile.remove();
    if (!destFile.open( IO_WriteOnly )) {
	srcFile.close();
	return false;
    }
    while (!srcFile.atEnd()) {
	len = srcFile.readBlock( buf, buflen );
	if (len <= 0)
	    break;
	if (destFile.writeBlock( buf, len ) != len)
	    return false;
    }
    destFile.flush();
    return true;
}

void UnpackDlgImpl::updateProgress( const QString& message )
{
    logOutput->append( message );
}

void UnpackDlgImpl::updateProgress( int )
{
    qApp->processEvents();
}

void UnpackDlgImpl::licenseKeyChanged()
{
    QRegExp keyExpr("^....-....-....");
    if ((keyExpr.search( srcKey->text() ) != -1) && 
	(featuresForKey( srcKey->text() ) & Feature_Mac))
	unpackButton->setEnabled( TRUE );
    else
	unpackButton->setEnabled( FALSE );
}

void UnpackDlgImpl::reject()
{
    exit( 0 );
}
