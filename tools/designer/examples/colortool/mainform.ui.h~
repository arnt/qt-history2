/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions use Qt Designer which will
** update this file, preserving your code. Create an init() function in place
** of a constructor, and a destroy() function in place of a destructor.
*****************************************************************************/


const int CLIP_AS_HEX = 0;
const int CLIP_AS_NAME = 1;
const int CLIP_AS_RGB = 2;
const int COL_NAME = 0;
const int COL_HEX = 1;

void MainForm::init()
{
    m_filename = "";
    m_changed = false;
    m_table_dirty = true;
    m_icons_dirty = true;
    m_clip_as = CLIP_AS_HEX;
    clearData( true );
}

void MainForm::clearData( bool fillWithDefaults )
{
	setCaption( "Color Tool" );

	m_colors.clear();
	m_comments.clear();

	if ( fillWithDefaults ) {
	    m_colors["black"] = Qt::black;
	    m_colors["blue"] = Qt::blue;
	    m_colors["cyan"] = Qt::cyan;
	    m_colors["darkblue"] = Qt::darkBlue;
	    m_colors["darkcyan"] = Qt::darkCyan;
	    m_colors["darkgray"] = Qt::darkGray;
	    m_colors["darkgreen"] = Qt::darkGreen;
	    m_colors["darkmagenta"] = Qt::darkMagenta;
	    m_colors["darkred"] = Qt::darkRed;
	    m_colors["darkyellow"] = Qt::darkYellow;
	    m_colors["gray"] = Qt::gray;
	    m_colors["green"] = Qt::green;
	    m_colors["lightgray"] = Qt::lightGray;
	    m_colors["magenta"] = Qt::magenta;
	    m_colors["red"] = Qt::red;
	    m_colors["white"] = Qt::white;
	    m_colors["yellow"] = Qt::yellow;
	}

	populate();
}

void MainForm::populate()
{
    if ( m_table_dirty ) {
	for ( int row = 0; row < Table->numRows(); ++row )
	    for ( int col = 0; col < Table->numCols(); ++col )
		Table->clearCell( row, col );

	Table->setNumRows( m_colors.count() );
	QPixmap pixmap( 22, 22 );
	int row = 0;
	QMap<QString,QColor>::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    Table->setText( row, COL_NAME, it.key() );
	    Table->setPixmap( row, COL_NAME, pixmap );
	    Table->setText( row, COL_HEX, color.name().upper() );
	    row++;
	}
	Table->adjustColumn( COL_NAME );
	Table->adjustColumn( COL_HEX );
	m_table_dirty = false;
    }
    
    if ( m_icons_dirty ) {
	IconView->clear();

	QMap<QString,QColor>::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it )
	    (void) new QIconViewItem( IconView, it.key(),
				      colorSwatch( it.data() ) );
	m_icons_dirty = false;
    }
}

QPixmap MainForm::colorSwatch( QColor color )
{
	QPixmap pixmap( 80, 80 );
	pixmap.fill( white );
	QPainter painter;
	painter.begin( &pixmap );
	painter.setPen( NoPen );
	painter.setBrush( color );
	painter.drawEllipse( 0, 0, 80, 80 );
	painter.end();
	return pixmap;
}

void MainForm::fileNew()
{

}

void MainForm::fileOpen()
{

}

void MainForm::fileSave()
{

}

void MainForm::fileSaveAs()
{

}

void MainForm::filePrint()
{

}

void MainForm::fileExit()
{
    QApplication::exit( 0 );
}

void MainForm::editUndo()
{

}

void MainForm::editRedo()
{

}

void MainForm::editCut()
{

}

void MainForm::editCopy()
{

}

void MainForm::editPaste()
{

}

void MainForm::editFind()
{

}

void MainForm::helpIndex()
{

}

void MainForm::helpContents()
{

}

void MainForm::helpAbout()
{

}


void MainForm::changedTableColor( int, int )
{

}

void MainForm::changedIconColor( QIconViewItem * )
{

}

void MainForm::aboutToShow( QWidget * )
{

}


