#include "generatordlgimpl.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include <qarchive.h>
#include <qmessagebox.h>

GeneratorDlgImpl::GeneratorDlgImpl( const QString &dest, QWidget* pParent, const char* pName, WFlags f ) : 
    GeneratorDlg( pParent, pName, f )
{
    destPath->setText(dest);
}

void GeneratorDlgImpl::clickedDestButton()
{
    destPath->setText( QFileDialog::getExistingDirectory( destPath->text(), this, NULL, "Select destination directory" ) );
}

void GeneratorDlgImpl::clickedSourceButton()
{
    sourcePath->setText( QFileDialog::getExistingDirectory( sourcePath->text(), this, NULL, "Select source directory" ) );
}

void GeneratorDlgImpl::clickedGenerate()
{
    QArchive archive;

    connect( &archive, SIGNAL( operationFeedback( const QString& ) ), this, SLOT( updateProgress( const QString& ) ) );

    generateButton->setDisabled( true );

    archive.setVerbosity( QArchive::Destination | QArchive::Verbose );
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
    generateButton->setDisabled( false );
}

void GeneratorDlgImpl::updateProgress( const QString& message )
{
    logOutput->append( message );
}
