#include "unpackdlgimpl.h"
#include <qdir.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qarchive.h>
#include <qmessagebox.h>
#include "keyinfo.h"

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
		    if(key.upper() == "LICENSEID") {
			QString value = (*it++).stripWhiteSpace().replace( QRegExp( QString( "\"" ) ), QString::null );
			srcKey->setText(value);
		    }
		}
	    }
	    lic.close();
	}
    }
}

void UnpackDlgImpl::clickedDestButton()
{
    destPath->setText( QFileDialog::getExistingDirectory( destPath->text(), this, NULL, "Select destination directory" ) );
}

void UnpackDlgImpl::clickedUnpack()
{
    QArchive archive;
    connect( &archive, SIGNAL( operationFeedback( const QString& ) ), this, SLOT( updateProgress( const QString& ) ) );
    unpackButton->setDisabled( true );
    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );

    QString dest = destPath->text(), src="qtmac.arq";
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
    if(!archive.readArchive( dest, srcKey->text() )) {
	QMessageBox::critical( NULL, "Failure", "Failed to unpack " + src);
	archive.close();
    }
    unpackButton->setDisabled( false );
}

void UnpackDlgImpl::updateProgress( const QString& message )
{
    logOutput->append( message );
}
