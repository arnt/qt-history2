#include "generatordlgimpl.h"
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qtextview.h>
#include <qpushbutton.h>
#include "qarchive.h"

GeneratorDlgImpl::GeneratorDlgImpl( QWidget* pParent, const char* pName, WFlags f ) : GeneratorDlg( pParent, pName, f )
{
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
    archive.setPath( destPath->text() + "/docs" );
    if( archive.open( IO_WriteOnly ) ) {
	archive.writeDir( sourcePath->text() + "/doc", true, "doc" );
	archive.close();
    }
    archive.setPath( destPath->text() + "/tutorial" );
    if( archive.open( IO_WriteOnly ) ) {
	archive.writeDir( sourcePath->text() + "/tutorial", true, "tutorial" );
	archive.close();
    }
    archive.setPath( destPath->text() + "/examples" );
    if( archive.open( IO_WriteOnly ) ) {
	archive.writeDir( sourcePath->text() + "/examples", true, "examples" );
	archive.close();
    }
    archive.setPath( destPath->text() + "/qt" );
    if( archive.open( IO_WriteOnly ) ) {
	QStringList dirList;
	dirList << sourcePath->text() + "/src" << sourcePath->text() + "/include" << sourcePath->text() + "/mkspecs" << sourcePath->text() + "/qmake";
	dirList << sourcePath->text() + "/tools" << sourcePath->text() + "/bin" << sourcePath->text() + "/lib" << sourcePath->text() + "/extensions";
	dirList << sourcePath->text() + "/plugins";
	archive.writeDirList( dirList );
	QStringList fileList;
	fileList << sourcePath->text() + "/LICENSE";
	fileList << sourcePath->text() + "/changes-3.0.0-beta3" << sourcePath->text() + "/INSTALL" << sourcePath->text() + "/Makefile";
	fileList << sourcePath->text() + "/MANIFEST" << sourcePath->text() + "/PORTING" << sourcePath->text() + "/README";
	archive.writeFileList( fileList );
	archive.close();
    }
    archive.setPath( destPath->text() + "/sys" );
    if( archive.open( IO_WriteOnly ) ) {
	QStringList fileList;
	fileList << sourcePath->text() + "/LICENSE";
	archive.writeFileList( fileList );
	archive.close();
    }

    generateButton->setDisabled( false );
}

void GeneratorDlgImpl::updateProgress( const QString& message )
{
    logOutput->append( message );
}
