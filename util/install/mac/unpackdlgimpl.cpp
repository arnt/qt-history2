#include "unpackdlgimpl.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qarchive.h>
#include <qmessagebox.h>

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
#if 0
    QString src = sourcePath->text(), dest = destPath->text();
    int slsh = src.findRev('/');
    if(slsh != -1)
	src = src.right(src.length() - slsh - 1);
    if(!dest.isEmpty() && dest.right(1) != "/")
	dest += "/";
    if(!QFile::exists(sourcePath->text())) {
	QMessageBox::critical( NULL, "Failure", "Cannot open: " + sourcePath->text() );
    } else if(!dest.isEmpty() && !QFile::exists(dest) ) {
	QMessageBox::critical( NULL, "Failure", "Cannot open: " +  dest );
    } else {
	archive.setPath( dest + src );
	if( archive.open( IO_WriteOnly ) ) {
	    archive.writeDir( sourcePath->text(), true, src );
	    archive.close();
	}
    }
#endif
    unpackButton->setDisabled( false );
}

void UnpackDlgImpl::updateProgress( const QString& message )
{
    logOutput->append( message );
}
