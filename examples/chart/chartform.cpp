#include "chartform.h"
#include "setdataform.h"

#include <qaction.h>
#include <qapplication.h>
#include <qbrush.h>
#include <qcanvas.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfont.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qrect.h>
#include <qstatusbar.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "images/file_new.xpm"
#include "images/file_open.xpm"
#include "images/file_save.xpm"
#include "images/file_print.xpm"
#include "images/options_setdata.xpm"
#include "images/options_setfont.xpm"
#include "images/options_setoptions.xpm"


ChartForm::ChartForm( const QString& filename )
    : QMainWindow( 0, 0, WDestructiveClose )
{
    QAction *fileNewAction;
    QAction *fileOpenAction;
    QAction *fileSaveAction;
    QAction *fileSaveAsAction;
    QAction *filePrintAction;
    QAction *fileQuitAction;
    QAction *optionsSetDataAction;
    QAction *optionsSetFontAction;
    QAction *optionsSetOptionsAction;

    fileNewAction = new QAction( "New Chart", QPixmap( file_new ),
				 "&New", CTRL+Key_N, this, "new" );
    connect( fileNewAction, SIGNAL( activated() ) , this, SLOT( fileNew() ) );

    fileOpenAction = new QAction( "Open Chart", QPixmap( file_open ),
				  "&Open", CTRL+Key_O, this, "open" );
    connect( fileOpenAction, SIGNAL( activated() ) , this, SLOT( fileOpen() ) );

    fileSaveAction = new QAction( "Save Chart", QPixmap( file_save ),
				  "&Save", CTRL+Key_S, this, "save" );
    connect( fileSaveAction, SIGNAL( activated() ) , this, SLOT( fileSave() ) );

    fileSaveAsAction = new QAction( "Save Chart As", QPixmap( file_save ),
				    "Save &As", 0,  this, "save as" );
    connect( fileSaveAsAction, SIGNAL( activated() ) ,
	     this, SLOT( fileSaveAs() ) );

    filePrintAction = new QAction( "Print Chart", QPixmap( file_print ),
				   "&Print Chart", CTRL+Key_P,
				   this, "print chart" );
    connect( filePrintAction, SIGNAL( activated() ) ,
	     this, SLOT( filePrint() ) );

    optionsSetDataAction = new QAction( "Set Data", QPixmap( options_setdata ),
					"Set &Data", CTRL+Key_D,
					this, "set data" );
    connect( optionsSetDataAction, SIGNAL( activated() ) ,
	     this, SLOT( optionsSetData() ) );

    optionsSetFontAction = new QAction( "Set Font", QPixmap( options_setfont ),
					"Set &Font", 0, this, "set font" );
    connect( optionsSetFontAction, SIGNAL( activated() ) ,
	     this, SLOT( optionsSetFont() ) );

    optionsSetOptionsAction = new QAction( "Set Options",
					   QPixmap( options_setoptions ),
					   "Set &Options", CTRL+Key_O,
					   this, "set options" );
    connect( optionsSetOptionsAction, SIGNAL( activated() ) ,
	     this, SLOT( optionsSetOptions() ) );

    fileQuitAction = new QAction( "Quit",
				  "&Quit", CTRL+Key_Q, this, "quit" );
    connect( fileQuitAction, SIGNAL( activated() ) , this, SLOT( fileQuit() ) );


    QToolBar* fileTools = new QToolBar( this, "file operations" );
    fileTools->setLabel( tr( "File Operations" ) );
    fileNewAction->addTo( fileTools );
    fileOpenAction->addTo( fileTools );
    fileSaveAction->addTo( fileTools );
    fileTools->addSeparator();
    filePrintAction->addTo( fileTools );

    QToolBar *optionsTools = new QToolBar( this, "options operations" );
    optionsTools->setLabel( tr( "Options Operations" ) );
    optionsSetDataAction->addTo( optionsTools );
    optionsSetFontAction->addTo( optionsTools );
    optionsTools->addSeparator();
    optionsSetOptionsAction->addTo( optionsTools );

    QPopupMenu *fileMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&File", fileMenu );
    fileNewAction->addTo( fileMenu );
    fileOpenAction->addTo( fileMenu );
    fileSaveAction->addTo( fileMenu );
    fileSaveAsAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    filePrintAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileQuitAction->addTo( fileMenu );

    QPopupMenu *optionsMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Options", optionsMenu );
    optionsSetDataAction->addTo( optionsMenu );
    optionsSetFontAction->addTo( optionsMenu );
    optionsSetOptionsAction->addTo( optionsMenu );

    QPopupMenu *helpMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", helpMenu );
    helpMenu->insertItem( "&Help", this, SLOT(helpHelp()), Key_F1 );
    helpMenu->insertItem( "&About", this, SLOT(helpAbout()) );
    helpMenu->insertItem( "About &Qt", this, SLOT(helpAboutQt()) );


    changed = false;
    printer = 0;
    elements.resize( MAX_ELEMENTS );

    // TODO loadSettings();

    canvas = new QCanvas( this );
    canvas->resize( 500, 500 ); // TODO Make this an Option
    canvasView = new QCanvasView( canvas, this );
    setCentralWidget( canvasView );
    canvasView->show();

    elements[0]  = Element(	 20, red ,	"Pied" );
    elements[1]  = Element(  	 50, green,	"The" );
    elements[2]  = Element(  	 35, blue,	"Piped" );
    elements[3]  = Element(  	 25, cyan,	"Piper" );
    elements[4]  = Element( INVALID, magenta );
    elements[5]  = Element( INVALID, yellow );
    elements[6]  = Element( INVALID, darkRed );
    elements[7]  = Element( INVALID, darkGreen );
    elements[8]  = Element( INVALID, darkBlue );
    elements[9]  = Element( INVALID, darkCyan );
    elements[10] = Element( INVALID, darkMagenta );
    elements[11] = Element( INVALID, darkYellow );
    elements[12] = Element( INVALID, darkGray );
    elements[13] = Element( INVALID, QColor( "turquoise" ) );
    elements[14] = Element( INVALID, gray );
    elements[15] = Element( INVALID, QColor( "khaki" ) );
    elements[16] = Element( INVALID, lightGray );
    elements[17] = Element( INVALID, QColor( "wheat" ) );
    elements[18] = Element( INVALID, QColor( "maroon" ) );
    elements[19] = Element( INVALID, QColor( "navy" ) );
    elements[20] = Element( INVALID, QColor( "gold" ) );
    elements[21] = Element( INVALID, QColor( "chocolate" ) );
    elements[22] = Element( INVALID, QColor( "tomato" ) );
    elements[23] = Element( INVALID, QColor( "bisque" ) );
    elements[24] = Element( INVALID, QColor( "honeydew" ) );
    elements[25] = Element( INVALID, QColor( "sea green" ) );
    for ( int i = 26; i < MAX_ELEMENTS; ++i ) {
	int x = ( ( i - 10 ) + 17 ) % 256;
	elements[i] = Element( INVALID, QColor( x, x, x ) );
    }

    if ( !filename.isNull() )
	load( filename );
    else
	paintElements();

    statusBar()->message( "Ready", 2000 );
    // TODO Resize to last saved size
    resize( canvas->width() + 10, canvas->height() + 80 );
}


ChartForm::~ChartForm()
{
    delete printer;
}



void ChartForm::fileNew()
{
    statusBar()->message( "fileNew() is not implemented yet", 2000 );
}


void ChartForm::fileOpen()
{
    statusBar()->message( "fileOpen() is not implemented yet", 2000 );
    // TODO filter for .cht files only
    QString filename = QFileDialog::getOpenFileName(
			    QString::null, QString::null, this);
    if ( !filename.isEmpty() )
	load( filename );
    else
	statusBar()->message( "File Open abandoned", 2000 );
}


void ChartForm::load( const QString& filename )
{
    statusBar()->message( QString( "load(%1) is not implemented yet" ).arg( filename ), 2000 );
    /*
    QFile file( filename );
    if ( !file.open( IO_ReadOnly ) )
	return;

    fileName = filename;

    // Load the data
    //
    file.close();

    setCaption( QString( "Chart -- %1" ).arg( filename ) );
    statusBar()->message( QString("Read \'%1\'").arg(filename), 2000 );
    */
    paintElements();
    changed = false;
}


void ChartForm::fileSave()
{
    if ( fileName.isEmpty() ) {
	fileSaveAs();
	return;
    }
    statusBar()->message( "fileSave() is not implemented yet" );
    /*
    QFile file( filename );
    if ( !file.open( IO_WriteOnly ) ) {
	statusBar()->message( QString("Failed to save \'%1\'").arg( filename ), 2000 );
	return;
    }

    setCaption( QString( "Chart -- %1" ).arg( filename ) );

    statusBar()->message( QString( "Saved \'%1\'" ).arg( filename ), 2000 );
    */
    changed = false;
}


void ChartForm::fileSaveAs()
{
    statusBar()->message( "fileSaveAs() is not implemented yet", 2000 );
    // TODO filter for .cht files only
    QString filename = QFileDialog::getSaveFileName(
			    QString::null, QString::null, this );
    if ( !filename.isEmpty() ) {
	fileName = filename;
	fileSave();
    }
    else
	statusBar()->message( "Saving aborted", 2000 );
}


void ChartForm::filePrint()
{
    if ( !printer )
	printer = new QPrinter;
    if ( printer->setup() ) {
	QPainter painter( printer );
	canvas->drawArea( QRect( 0, 0, canvas->width(), canvas->height() ),
			  &painter, FALSE );
	if ( !printer->outputFileName().isNull() )
	    statusBar()->message( QString( "Wrote \'%1\'" ).
				  arg( printer->outputFileName() ), 2000 );
    }
}


void ChartForm::fileQuit()
{
    if ( okToQuit() )
	qApp->exit( 0 );
}


void ChartForm::closeEvent( QCloseEvent *ce )
{
    if ( okToQuit() )
	ce->accept();
    else
	ce->ignore();
}


bool ChartForm::okToQuit()
{
    /* TODO
    saveSettings(); // Always save settings
    */

    if ( changed ) {
	switch( QMessageBox::information( this,
		    "Chart",
		    "The chart has been changed.",
		    "&Save", "Cancel", "&Abandon",
		    0, 1 ) ) {
	    case 0:
		fileSave();
		break;
	    case 1:
	    default:
		return false;
		break;
	    case 2:
		break;
	}
    }

    return true;
}


void ChartForm::optionsSetData()
{
    SetDataForm *setDataForm = new SetDataForm( &elements, this );
    if ( setDataForm->exec() )
	paintElements();
    delete setDataForm;
}


void ChartForm::optionsSetFont()
{
    statusBar()->message( "optionsSetFont() is not implemented yet", 2000 );
}


void ChartForm::optionsSetOptions()
{
    statusBar()->message( "optionsSetOptions() is not implemented yet", 2000 );
    /* TODO if OK
    saveSettings();
    */
}


void ChartForm::helpHelp()
{
    statusBar()->message( "helpHelp() is not implemented yet", 2000 );
}

void ChartForm::helpAbout()
{
    // TODO
    QMessageBox::about( this, "Chart",
			"Chart your data" );
}


void ChartForm::helpAboutQt()
{
    QMessageBox::aboutQt( this, "Chart" );
}


void ChartForm::paintElements()
{
    statusBar()->message( "Painting..." );

    QCanvasItemList list = canvas->allItems();
    for ( QCanvasItemList::iterator it = list.begin(); it != list.end(); ++it )
	delete *it;

    double total = 0.0;
    int count = 0;
    double scales[MAX_ELEMENTS];

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    count++;
	    total += elements[i].getValue();
	    scales[i] = elements[i].getValue() * CIRCLE_FACTOR;
	}
    }

    int pos = canvas->width() / 2;
    int size = canvas->width();
    int angle = 0;

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i] / total);
	    QCanvasEllipse *arc = new QCanvasEllipse( size, size,
						      angle, extent, canvas );
	    arc->setX( pos );
	    arc->setY( pos );
	    arc->setZ( 0 );
	    arc->setBrush( QBrush( elements[i].getValueColour() ) );
	    arc->show();
	    angle += extent;
	    // Crude initial label positioning
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() ) {
		QRect rect = arc->boundingRect();
		int x = ( abs( rect.right() - rect.left() ) / 2 ) + rect.x();
		int y = ( abs( rect.top() - rect.bottom() ) / 2 ) + rect.y();
		// TODO Use Options font
		QCanvasText *text = new QCanvasText( label,
						     QFont( "Helvetica",
							    18,
							    QFont::Bold ),
						     canvas );
		text->setX( x );
		text->setY( y );
		text->setZ( 1 );
		text->show();
	    }
	}
    }
    canvas->update();

    statusBar()->message( "Ready" );
}
