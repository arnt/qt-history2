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
    m_filename = filename;
    QTextStream ts( &file );
    Element element;
    int i = 0;
    while ( !ts.eof() ) {
	ts >> m_elements[i++];
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
    m_changed = false;
}


void ChartForm::fileSave()
{
    if ( m_filename.isEmpty() ) {
	fileSaveAs();
	return;
    }

    QFile file( m_filename );
    if ( !file.open( IO_WriteOnly ) ) {
	statusBar()->message( QString( "Failed to save \'%1\'" ).
				arg( m_filename ), 2000 );
	return;
    }
    QTextStream ts( &file );
    for ( int i = 0; i < MAX_ELEMENTS; ++i )
	if ( m_elements[i].isValid() )
	    ts << m_elements[i];

    setCaption( QString( "Chart -- %1" ).arg( m_filename ) );
    statusBar()->message( QString( "Saved \'%1\'" ).arg( m_filename ), 2000 );
    m_changed = false;
}


void ChartForm::fileSaveAsPixmap()
{
    QString filename = QFileDialog::getSaveFileName(
			    QString::null, "Images (*.png *.xpm *.jpg)",
			    this, "file save as bitmap",
			    "Chart -- File Save As Bitmap" );
    if ( QPixmap::grabWidget( m_canvasView ).
	    save( filename,
		  filename.mid( filename.findRev( '.' ) + 1 ).upper() ) )
	statusBar()->message( QString( "Wrote \'%1\'" ).arg( filename ), 2000 );
    else
	statusBar()->message( QString( "Failed to write \'%1\'" ).
				arg( filename ), 2000 );
}

void ChartForm::filePrint()
{
    if ( !m_printer )
	m_printer = new QPrinter;
    if ( m_printer->setup() ) {
	QPainter painter( m_printer );
	m_canvas->drawArea( QRect( 0, 0, m_canvas->width(), m_canvas->height() ),
			    &painter, FALSE );
	if ( !m_printer->outputFileName().isEmpty() )
	    statusBar()->message( QString( "Printed \'%1\'" ).
				  arg( m_printer->outputFileName() ), 2000 );
    }
}

