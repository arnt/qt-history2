#include "canvasview.h"
#include "chartform.h"

#include <qfile.h>
#include <qfiledialog.h>
#include <qpainter.h>
#include <qprinter.h>
#include <qstatusbar.h>


void ChartForm::load( const QString& filename )
{
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) ) {
	statusBar()->message( QString( "Failed to load \'%1\'" ).
				arg( filename ), 2000 );
	return;
    }

    init(); // Make sure we have colours
    fileName = filename;
    QTextStream ts( &file );
    Element element;
    int i = 0;
    while ( !ts.eof() ) {
	ts >> elements[i++];
	if ( i == MAX_ELEMENTS ) {
	    statusBar()->message(
		QString( "Read maximum number of elements (%1)"
		         " discarding others" ).arg( i ), 2000 );
	    break;
	}
    }

    file.close();

    setCaption( QString( "Chart -- %1" ).arg( filename ) );
    statusBar()->message( QString( "Read \'%1\'" ).arg( filename ), 2000 );
    updateRecentFiles( filename );

    drawElements();
    changed = false;
}


void ChartForm::fileSave()
{
    if ( fileName.isEmpty() ) {
	fileSaveAs();
	return;
    }

    QFile file( fileName );
    if ( !file.open( IO_WriteOnly ) ) {
	statusBar()->message( QString( "Failed to save \'%1\'" ).
				arg( fileName ), 2000 );
	return;
    }
    QTextStream ts( &file );
    for ( int i = 0; i < MAX_ELEMENTS; ++i )
	if ( elements[i].isValid() )
	    ts << elements[i];

    setCaption( QString( "Chart -- %1" ).arg( fileName ) );
    statusBar()->message( QString( "Saved \'%1\'" ).arg( fileName ), 2000 );
    changed = false;
}


void ChartForm::fileSaveAsPixmap()
{
    QString filename = QFileDialog::getSaveFileName(
			    QString::null, "Images (*.png *.xpm *.jpg)",
			    this, "file save as bitmap",
			    "Chart -- File Save As Bitmap" );
    if ( QPixmap::grabWidget( canvasView ).
	    save( filename,
		  filename.mid( filename.findRev( '.' ) + 1 ).upper() ) )
	statusBar()->message( QString( "Wrote \'%1\'" ).arg( filename ), 2000 );
    else
	statusBar()->message( QString( "Failed to write \'%1\'" ).
				arg( filename ), 2000 );
}

void ChartForm::filePrint()
{
    if ( !printer )
	printer = new QPrinter;
    if ( printer->setup() ) {
	QPainter painter( printer );
	canvas->drawArea( QRect( 0, 0, canvas->width(), canvas->height() ),
			  &painter, FALSE );
	if ( !printer->outputFileName().isEmpty() )
	    statusBar()->message( QString( "Printed \'%1\'" ).
				  arg( printer->outputFileName() ), 2000 );
    }
}

