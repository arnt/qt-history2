#include "docuwindow.h"
#include <qtextbrowser.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qfiledialog.h>
#include <qfile.h>
#include <qstatusbar.h>
#include <qprinter.h>
#include <qpainter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>

static const char *filesave[] = {
"    14    14        4            1",
". c #040404",
"# c #808304",
"a c #bfc2bf",
"b c None",
"..............",
".#.aaaaaaaa.a.",
".#.aaaaaaaa...",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".#.aaaaaaaa.#.",
".##........##.",
".############.",
".##.........#.",
".##......aa.#.",
".##......aa.#.",
".##......aa.#.",
"b............."
};

static const char *fileprint[] = {
"    16    14        6            1",
". c #000000",
"# c #848284",
"a c #c6c3c6",
"b c #ffff00",
"c c #ffffff",
"d c None",
"ddddd.........dd",
"dddd.cccccccc.dd",
"dddd.c.....c.ddd",
"ddd.cccccccc.ddd",
"ddd.c.....c....d",
"dd.cccccccc.a.a.",
"d..........a.a..",
".aaaaaaaaaa.a.a.",
".............aa.",
".aaaaaa###aa.a.d",
".aaaaaabbbaa...d",
".............a.d",
"d.aaaaaaaaa.a.dd",
"dd...........ddd"
};


DocuWindow::DocuWindow( const QString& docu, QWidget *parent, QWidget *source )
    : QMainWindow( parent, 0, WDestructiveClose )
{
    setWindowTitle( source->windowTitle() + " - Documentation" );

    browser = new QTextBrowser( this );
    browser->setSource( docu );
    browser->setText( docu );
    browser->setLinkUnderline( TRUE );

    setCentralWidget( browser );

    QToolBar *fileTools = new QToolBar( this, "file operations" );
    fileTools->setLabel( "File" );

    QToolButton * fileSave
	= new QToolButton( QPixmap(filesave), "Save File", QString::null,
			   this, SLOT(save()), fileTools, "save file" );

    QToolButton * filePrint
	= new QToolButton( QPixmap(fileprint), "Print File", QString::null,
			   this, SLOT(print()), fileTools, "print file" );

    statusBar();
}

void DocuWindow::save()
{
    QString filename = QFileDialog::getSaveFileName( QString::null, QString::null,
					       this );

    if ( filename.isEmpty() )
	return;

    QString text = browser->text();
    QFile f( filename );
    if ( !f.open( IO_WriteOnly ) ) {
	statusBar()->message( QString("Could not write to %1").arg(filename), 2000 );
	return;
    }

    QTextStream t( &f );
    t << text;
    f.close();

    statusBar()->message( QString( "File %1 saved" ).arg(filename), 2000 );
}

void DocuWindow::print()
{
    QPrinter printer;
    if ( printer.printerName().isEmpty() ) {
	statusBar()->message( QString("No printer installed"), 2000 );
	return;
    }
    if ( !printer.setup( this ) ) {
	statusBar()->message( QString("Printing aborted"), 2000 );
	return;
    }
    
    QPainter painter;
    if ( !painter.begin( &printer ) ) {
	statusBar()->message( QString("Printing aborted"), 2000 );
	return;
    }

    QPaintDeviceMetrics metrics(painter.device());
    int dpix = metrics.logicalDpiX();
    int dpiy = metrics.logicalDpiY();
    const int margin = 72; // pt
    QRect body(margin*dpix/72, margin*dpiy/72,
	       metrics.width()-margin*dpix/72*2,
	       metrics.height()-margin*dpiy/72*2 );
    QSimpleRichText richText( browser->text(), QFont(), browser->context(), browser->styleSheet(),
			      browser->mimeSourceFactory(), body.height() );
    richText.setWidth( &painter, body.width() );
    QRect view( body );
    int page = 1;
    do {
	richText.draw( &painter, body.left(), body.top(), view, palette() );
	view.moveBy( 0, body.height() );
	painter.translate( 0 , -body.height() );
	painter.drawText( view.right() - painter.fontMetrics().width( QString::number(page) ),
		    view.bottom() + painter.fontMetrics().ascent() + 5, QString::number(page) );
	if ( view.top()  >= richText.height() )
	    break;
	printer.newPage();
	page++;
    } while (TRUE);
}
