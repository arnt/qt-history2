#include "canvastext.h"
#include "canvasview.h"
#include "chartform.h"
#include "optionsform.h"
#include "setdataform.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcombobox.h>
#include <qbrush.h>
#include <qcanvas.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qfont.h>
#include <qfontdialog.h>
#include <qlabel.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qpopupmenu.h>
#include <qprinter.h>
#include <qradiobutton.h>
#include <qrect.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qstatusbar.h>
#include <qtextstream.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>

#include "images/file_new.xpm"
#include "images/file_open.xpm"
#include "images/file_save.xpm"
#include "images/file_print.xpm"
#include "images/options_setdata.xpm"
#include "images/options_setfont.xpm"
#include "images/options_setoptions.xpm"
#include "images/options_horizontalbarchart.xpm"
#include "images/options_piechart.xpm"
#include "images/options_verticalbarchart.xpm"


const QString WINDOWS_REGISTRY = "/QtExamples";
const QString APP_KEY = "/Chart/";


ChartForm::ChartForm( const QString& filename )
    : QMainWindow( 0, 0, WDestructiveClose )
{
    setIcon( QPixmap( options_piechart ) );

    QAction *fileNewAction;
    QAction *fileOpenAction;
    QAction *fileSaveAction;
    QAction *fileSaveAsAction;
    QAction *fileSaveAsPixmapAction;
    QAction *filePrintAction;
    QAction *fileQuitAction;
    QAction *optionsSetDataAction;
    QAction *optionsSetFontAction;
    QAction *optionsSetOptionsAction;

    fileNewAction = new QAction(
	    "New Chart", QPixmap( file_new ),
	    "&New", CTRL+Key_N, this, "new" );
    connect( fileNewAction, SIGNAL( activated() ) , this, SLOT( fileNew() ) );

    fileOpenAction = new QAction(
	    "Open Chart", QPixmap( file_open ),
	    "&Open...", CTRL+Key_O, this, "open" );
    connect( fileOpenAction, SIGNAL( activated() ) , this, SLOT( fileOpen() ) );

    fileSaveAction = new QAction(
	    "Save Chart", QPixmap( file_save ),
	    "&Save", CTRL+Key_S, this, "save" );
    connect( fileSaveAction, SIGNAL( activated() ) , this, SLOT( fileSave() ) );

    fileSaveAsAction = new QAction(
	    "Save Chart As", QPixmap( file_save ),
	    "Save &As...", 0, this, "save as" );
    connect( fileSaveAsAction, SIGNAL( activated() ) ,
	     this, SLOT( fileSaveAs() ) );

    fileSaveAsPixmapAction = new QAction(
	    "Save Chart As Bitmap", QPixmap( file_save ),
	    "Save As &Bitmap...", CTRL+Key_B, this, "save as bitmap" );
    connect( fileSaveAsPixmapAction, SIGNAL( activated() ) ,
	     this, SLOT( fileSaveAsPixmap() ) );

    filePrintAction = new QAction(
	    "Print Chart", QPixmap( file_print ),
	    "&Print Chart...", CTRL+Key_P, this, "print chart" );
    connect( filePrintAction, SIGNAL( activated() ) ,
	     this, SLOT( filePrint() ) );

    optionsSetDataAction = new QAction(
	    "Set Data", QPixmap( options_setdata ),
	    "Set &Data...", CTRL+Key_D, this, "set data" );
    connect( optionsSetDataAction, SIGNAL( activated() ) ,
	     this, SLOT( optionsSetData() ) );


    QActionGroup *chartGroup = new QActionGroup( this ); // Connected later
    chartGroup->setExclusive( true );

    optionsPieChartAction = new QAction(
	    "Pie Chart", QPixmap( options_piechart ),
	    "&Pie Chart", CTRL+Key_I, chartGroup, "pie chart" );
    optionsPieChartAction->setToggleAction( true );

    optionsHorizontalBarChartAction = new QAction(
	    "Horizontal Bar Chart", QPixmap( options_horizontalbarchart ),
	    "&Horizontal Bar Chart", CTRL+Key_H, chartGroup,
	    "horizontal bar chart" );
    optionsHorizontalBarChartAction->setToggleAction( true );

    optionsVerticalBarChartAction = new QAction(
	    "Vertical Bar Chart", QPixmap( options_verticalbarchart ),
	    "&Vertical Bar Chart", CTRL+Key_V, chartGroup, "Vertical bar chart" );
    optionsVerticalBarChartAction->setToggleAction( true );


    optionsSetFontAction = new QAction(
	    "Set Font", QPixmap( options_setfont ),
	    "Set &Font...", CTRL+Key_F, this, "set font" );
    connect( optionsSetFontAction, SIGNAL( activated() ) ,
	     this, SLOT( optionsSetFont() ) );

    optionsSetOptionsAction = new QAction(
	    "Set Options", QPixmap( options_setoptions ),
	    "Set &Options...", 0, this, "set options" );
    connect( optionsSetOptionsAction, SIGNAL( activated() ) ,
	     this, SLOT( optionsSetOptions() ) );

    fileQuitAction = new QAction( "Quit", "&Quit", CTRL+Key_Q, this, "quit" );
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
    optionsTools->addSeparator();
    optionsPieChartAction->addTo( optionsTools );
    optionsHorizontalBarChartAction->addTo( optionsTools );
    optionsVerticalBarChartAction->addTo( optionsTools );
    optionsTools->addSeparator();
    optionsSetFontAction->addTo( optionsTools );
    optionsTools->addSeparator();
    optionsSetOptionsAction->addTo( optionsTools );

    fileMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&File", fileMenu );
    fileNewAction->addTo( fileMenu );
    fileOpenAction->addTo( fileMenu );
    fileSaveAction->addTo( fileMenu );
    fileSaveAsAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileSaveAsPixmapAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    filePrintAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileQuitAction->addTo( fileMenu );
    fileMenu->insertSeparator();

    optionsMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Options", optionsMenu );
    optionsSetDataAction->addTo( optionsMenu );
    optionsMenu->insertSeparator();
    optionsPieChartAction->addTo( optionsMenu );
    optionsHorizontalBarChartAction->addTo( optionsMenu );
    optionsVerticalBarChartAction->addTo( optionsMenu );
    optionsMenu->insertSeparator();
    optionsSetFontAction->addTo( optionsMenu );
    optionsMenu->insertSeparator();
    optionsSetOptionsAction->addTo( optionsMenu );

    QPopupMenu *helpMenu = new QPopupMenu( this );
    menuBar()->insertItem( "&Help", helpMenu );
    helpMenu->insertItem( "&Help", this, SLOT(helpHelp()), Key_F1 );
    helpMenu->insertItem( "&About", this, SLOT(helpAbout()) );
    helpMenu->insertItem( "About &Qt", this, SLOT(helpAboutQt()) );


    printer = 0;
    elements.resize( MAX_ELEMENTS );

    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    int windowWidth = settings.readNumEntry( APP_KEY + "WindowWidth", 460 );
    int windowHeight = settings.readNumEntry( APP_KEY + "WindowHeight", 530 );
    int windowX = settings.readNumEntry( APP_KEY + "WindowX", 0 );
    int windowY = settings.readNumEntry( APP_KEY + "WindowY", 0 );
    chartType = ChartType(
		    settings.readNumEntry( APP_KEY + "ChartType", int(PIE) ));
    addValues = AddValuesType(
		    settings.readNumEntry( APP_KEY + "AddValues", int(NO) ));
    decimalPlaces = settings.readNumEntry( APP_KEY + "Decimals", 2 );
    font = QFont( "Helvetica", 18, QFont::Bold );
    font.fromString( settings.readEntry( APP_KEY + "Font", font.toString() ) );
    for ( int i = 0; i < MAX_RECENTFILES; ++i ) {
	QString filename = settings.readEntry( APP_KEY + "File" +
					       QString::number( i + 1 ) );
	if ( !filename.isEmpty() )
	    recentFiles.push_back( filename );
    }
    if ( recentFiles.count() )
	updateRecentFilesMenu();

    switch ( chartType ) {
	case PIE:
	    optionsPieChartAction->setOn( true );
	    break;
	case VERTICAL_BAR:
	    optionsVerticalBarChartAction->setOn( true );
	    break;
	case HORIZONTAL_BAR:
	    optionsHorizontalBarChartAction->setOn( true );
	    break;
    }
    // Connect *after* we've set the chart type on so we don't call
    // drawElements() prematurely.
    connect( chartGroup, SIGNAL( selected(QAction*) ),
	     this, SLOT( setChartType(QAction*) ) );

    resize( windowWidth, windowHeight );
    move( windowX, windowY );

    canvas = new QCanvas( this );
    canvas->resize( width(), height() );
    canvasView = new CanvasView( canvas, &elements, this );
    setCentralWidget( canvasView );
    canvasView->show();

    if ( !filename.isEmpty() )
	load( filename );
    else {
	init();
	elements[0].set( 20, red,    14, "Red" );
	elements[1].set( 70, cyan,    2, "Cyan",   darkGreen );
	elements[2].set( 35, blue,   11, "Blue" );
	elements[3].set( 55, yellow,  1, "Yellow", darkBlue );
	elements[4].set( 80, magenta, 1, "Magenta" );
	drawElements();
    }

    statusBar()->message( "Ready", 2000 );
}


ChartForm::~ChartForm()
{
    delete printer;
}


void ChartForm::init()
{
    setCaption( "Chart" );
    fileName = QString::null;
    changed = false;

    elements[0]  = Element( Element::INVALID, red );
    elements[1]  = Element( Element::INVALID, cyan );
    elements[2]  = Element( Element::INVALID, blue );
    elements[3]  = Element( Element::INVALID, yellow );
    elements[4]  = Element( Element::INVALID, green );
    elements[5]  = Element( Element::INVALID, magenta );
    elements[6]  = Element( Element::INVALID, darkYellow );
    elements[7]  = Element( Element::INVALID, darkRed );
    elements[8]  = Element( Element::INVALID, darkCyan );
    elements[9]  = Element( Element::INVALID, darkGreen );
    elements[10] = Element( Element::INVALID, darkMagenta );
    elements[11] = Element( Element::INVALID, darkBlue );
    for ( int i = 12; i < MAX_ELEMENTS; ++i ) {
	double x = (double(i) / MAX_ELEMENTS) * 360;
	int y = (int(x * 256) % 105) + 151;
	int z = ((i * 17) % 105) + 151;
	elements[i] = Element( Element::INVALID, QColor( x, y, z, QColor::Hsv ) );
    }
}


void ChartForm::fileNew()
{
    if ( okToClear() ) {
	init();
	drawElements();
    }
}


void ChartForm::fileOpen()
{
    if ( !okToClear() )
	return;

    QString filename = QFileDialog::getOpenFileName(
			    QString::null, "Charts (*.cht)", this,
			    "file open", "Chart -- File Open" );
    if ( !filename.isEmpty() )
	load( filename );
    else
	statusBar()->message( "File Open abandoned", 2000 );
}


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


void ChartForm::fileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(
			    QString::null, "Charts (*.cht)", this,
			    "file save as", "Chart -- File Save As" );
    if ( !filename.isEmpty() ) {
	int answer = 0;
	if ( QFile::exists( filename ) )
	    answer = QMessageBox::warning(
			    this, "Chart -- Overwrite File",
			    QString( "Overwrite\n\'%1\'?" ).
				arg( filename ),
			    "&Yes", "&No", QString::null, 1, 1 );
	if ( answer == 0 ) {
	    fileName = filename;
	    updateRecentFiles( filename );
	    fileSave();
	    return;
	}
    }
    statusBar()->message( "Saving abandoned", 2000 );
}


void ChartForm::fileOpenRecent( int index )
{
    if ( !okToClear() )
	return;

    load( recentFiles[index] );
}


void ChartForm::updateRecentFiles( const QString& filename )
{
    if ( recentFiles.find( filename ) != recentFiles.end() )
	return;

    recentFiles.push_back( filename );
    if ( recentFiles.count() > MAX_RECENTFILES )
	recentFiles.pop_front();

    updateRecentFilesMenu();
}


void ChartForm::updateRecentFilesMenu()
{
    for ( int i = 0; i < MAX_RECENTFILES; ++i ) {
	if ( fileMenu->findItem( i ) )
	    fileMenu->removeItem( i );
	if ( i < int(recentFiles.count()) )
	    fileMenu->insertItem( QString( "&%1 %2" ).
				    arg( i + 1 ).arg( recentFiles[i] ),
				  this, SLOT( fileOpenRecent(int) ),
				  0, i );
    }
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


void ChartForm::fileQuit()
{
    if ( okToClear() ) {
	saveOptions();
	qApp->exit( 0 );
    }
}


void ChartForm::closeEvent( QCloseEvent *ce )
{
    if ( okToClear() ) {
	saveOptions();
	ce->accept();
    }
    else
	ce->ignore();
}


bool ChartForm::okToClear()
{
    if ( changed ) {
	QString msg;
	if ( fileName.isEmpty() )
	    msg = "Unnamed chart ";
	else
	    msg = QString( "Chart '%1'\n" ).arg( fileName );
	msg += "has been changed.";
	switch( QMessageBox::information( this, "Chart -- Unsaved Changes",
					  msg, "&Save", "Cancel", "&Abandon",
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


void ChartForm::saveOptions()
{
    QSettings settings;
    settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
    settings.writeEntry( APP_KEY + "WindowWidth", width() );
    settings.writeEntry( APP_KEY + "WindowHeight", height() );
    settings.writeEntry( APP_KEY + "WindowX", x() );
    settings.writeEntry( APP_KEY + "WindowY", y() );
    settings.writeEntry( APP_KEY + "ChartType", int(chartType) );
    settings.writeEntry( APP_KEY + "AddValues", int(addValues) );
    settings.writeEntry( APP_KEY + "Decimals", decimalPlaces );
    settings.writeEntry( APP_KEY + "Font", font.toString() );
    for ( int i = 0; i < int(recentFiles.count()); ++i )
	settings.writeEntry( APP_KEY + "File" + QString::number( i + 1 ),
			     recentFiles[i] );
}


void ChartForm::optionsSetData()
{
    SetDataForm *setDataForm = new SetDataForm( &elements, decimalPlaces, this );
    if ( setDataForm->exec() ) {
	changed = true;
	drawElements();
    }
    delete setDataForm;
}


void ChartForm::setChartType( QAction *action )
{
    if ( action == optionsPieChartAction ) {
	chartType = PIE;
    }
    else if ( action == optionsHorizontalBarChartAction ) {
	chartType = HORIZONTAL_BAR;
    }
    else if ( action == optionsVerticalBarChartAction ) {
	chartType = VERTICAL_BAR;
    }

    drawElements();
}


void ChartForm::optionsSetFont()
{
    bool ok;
    QFont newFont = QFontDialog::getFont( &ok, font, this );
    if ( ok ) {
	font = newFont;
	drawElements();
    }
}


void ChartForm::optionsSetOptions()
{
    OptionsForm *optionsForm = new OptionsForm( this );
    optionsForm->chartTypeComboBox->setCurrentItem( chartType );
    optionsForm->setFont( font );
    switch ( addValues ) {
	case NO:
	    optionsForm->noRadioButton->setChecked( true );
	    break;
	case YES:
	    optionsForm->yesRadioButton->setChecked( true );
	    break;
	case AS_PERCENTAGE:
	    optionsForm->asPercentageRadioButton->setChecked( true );
	    break;
    }
    optionsForm->decimalPlacesSpinBox->setValue( decimalPlaces );
    if ( optionsForm->exec() ) {
	chartType = ChartType(optionsForm->chartTypeComboBox->currentItem());
	font = optionsForm->getFont();
	if ( optionsForm->noRadioButton->isChecked() )
	    addValues = NO;
	else if ( optionsForm->yesRadioButton->isChecked() )
	    addValues = YES;
	else if ( optionsForm->asPercentageRadioButton->isChecked() )
	    addValues = AS_PERCENTAGE;
	decimalPlaces = optionsForm->decimalPlacesSpinBox->value();
	drawElements();
    }
    delete optionsForm;
}


void ChartForm::helpHelp()
{
    statusBar()->message( "Help is not implemented yet", 2000 );
}


void ChartForm::helpAbout()
{
    QMessageBox::about( this, "Chart -- About",
			"<center><h1><font color=blue>Chart<font></h1></center>"
			"<p>Chart your data with <i>chart</i>.</p>"
			);
}


void ChartForm::helpAboutQt()
{
    QMessageBox::aboutQt( this, "Chart -- About Qt" );
}


void ChartForm::drawElements()
{
    QCanvasItemList list = canvas->allItems();
    for ( QCanvasItemList::iterator it = list.begin(); it != list.end(); ++it )
	delete *it;

	// 360 * 16 for pies; Qt works with 16ths of degrees
    int scaleFactor = chartType == PIE ? 5760 :
			chartType == VERTICAL_BAR ? canvas->height() :
			    canvas->width();
    double biggest = 0.0;
    int count = 0;
    double total = 0.0;
    static double scales[MAX_ELEMENTS];

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    double value = elements[i].getValue();
	    count++;
	    total += value;
	    if ( value > biggest )
		biggest = value;
	    scales[i] = elements[i].getValue() * scaleFactor;
	}
    }

    if ( count ) {
	    // 2nd loop because of total and biggest
	for ( int i = 0; i < MAX_ELEMENTS; ++i )
	    if ( elements[i].isValid() )
		if ( chartType == PIE )
		    scales[i] = (elements[i].getValue() * scaleFactor) / total;
		else
		    scales[i] = (elements[i].getValue() * scaleFactor) / biggest;

	switch ( chartType ) {
	    case PIE:
		drawPieChart( scales, total, count );
		break;
	    case VERTICAL_BAR:
		drawVerticalBarChart( scales, total, count );
		break;
	    case HORIZONTAL_BAR:
		drawHorizontalBarChart( scales, total, count );
		break;
	}
    }

    canvas->update();
}


void ChartForm::drawPieChart( const double scales[], double total, int )
{
    int width = canvas->width();
    int height = canvas->height();
    int size = width > height ? height : width;
    int x = width / 2;
    int y = height / 2;
    int angle = 0;

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasEllipse *arc = new QCanvasEllipse(
					    size, size, angle, extent, canvas );
	    arc->setX( x );
	    arc->setY( y );
	    arc->setZ( 0 );
	    arc->setBrush( QBrush( elements[i].getValueColour(),
				   BrushStyle(elements[i].getValuePattern()) ) );
	    arc->show();
	    angle += extent;
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() || addValues != NO ) {
		int labelX = elements[i].getX( PIE );
		int labelY = elements[i].getY( PIE );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    QRect rect = arc->boundingRect();
		    labelX = ( abs( rect.right() - rect.left() ) / 2 ) + rect.x();
		    labelY = ( abs( rect.top() - rect.bottom() ) / 2 ) + rect.y();
		}
		label = valueLabel( label, elements[i].getValue(), total );
		CanvasText *text = new CanvasText( i, label, font, canvas );
		text->setColor( elements[i].getLabelColour() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		elements[i].setX( PIE, labelX );
		elements[i].setY( PIE, labelY );
	    }
	}
    }
}


void ChartForm::drawVerticalBarChart(
	const double scales[], double total, int count )
{
    int width = canvas->width() / count;
    int x = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    int y = canvas->height() - extent;
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    x, y, width, extent, canvas );
	    rect->setBrush( QBrush( elements[i].getValueColour(),
				    BrushStyle(elements[i].getValuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() || addValues != NO ) {
		int labelX = elements[i].getX( VERTICAL_BAR );
		int labelY = elements[i].getY( VERTICAL_BAR );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    labelX = x;
		    labelY = y;
		}
		label = valueLabel( label, elements[i].getValue(), total );
		CanvasText *text = new CanvasText( i, label, font, canvas );
		text->setColor( elements[i].getLabelColour() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		elements[i].setX( VERTICAL_BAR, labelX );
		elements[i].setY( VERTICAL_BAR, labelY );
	    }
	    x += width;
	}
    }
}


void ChartForm::drawHorizontalBarChart(
	const double scales[], double total, int count )
{
    int height = canvas->height() / count;
    int y = 0;
    QPen pen;
    pen.setStyle( NoPen );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	if ( elements[i].isValid() ) {
	    int extent = int(scales[i]);
	    QCanvasRectangle *rect = new QCanvasRectangle(
					    0, y, extent, height, canvas );
	    rect->setBrush( QBrush( elements[i].getValueColour(),
				    BrushStyle(elements[i].getValuePattern()) ) );
	    rect->setPen( pen );
	    rect->setZ( 0 );
	    rect->show();
	    QString label = elements[i].getLabel();
	    if ( !label.isEmpty() || addValues != NO ) {
		int labelX = elements[i].getX( HORIZONTAL_BAR );
		int labelY = elements[i].getY( HORIZONTAL_BAR );
		if ( labelX == Element::NO_POSITION ||
		     labelY == Element::NO_POSITION ) {
		    labelX = 0;
		    labelY = y;
		}
		label = valueLabel( label, elements[i].getValue(), total );
		CanvasText *text = new CanvasText( i, label, font, canvas );
		text->setColor( elements[i].getLabelColour() );
		text->setX( labelX );
		text->setY( labelY );
		text->setZ( 1 );
		text->show();
		elements[i].setX( HORIZONTAL_BAR, labelX );
		elements[i].setY( HORIZONTAL_BAR, labelY );
	    }
	    y += height;
	}
    }
}


QString ChartForm::valueLabel(
	    const QString& label, double value, double total )
{
    if ( addValues == NO )
	return label;

    QString newLabel = label;
    if ( !label.isEmpty() )
	if ( chartType == VERTICAL_BAR )
	    newLabel += '\n';
	else
	    newLabel += ' ';
    if ( addValues == YES )
	newLabel += QString::number( value, 'f', decimalPlaces );
    else if ( addValues == AS_PERCENTAGE )
	newLabel += QString::number( (value / total) * 100, 'f', decimalPlaces )
		    + '%';
    return newLabel;
}

