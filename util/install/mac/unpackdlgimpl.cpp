#include "unpackdlgimpl.h"
#include <qdir.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qarchive.h>
#include <qmessagebox.h>
#include "keyinfo.h"

UnpackDlgImpl::UnpackDlgImpl( QWidget* pParent, const char* pName, WFlags f ) : 
    UnpackDlg( pParent, pName, f )
{
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
