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
const int COL_WEB = 2;

void MainForm::init()
{
    m_clip_as = CLIP_AS_HEX;
    m_show_web = TRUE;
    m_filename = "";
    m_changed = FALSE;
    m_table_dirty = TRUE;
    m_icons_dirty = TRUE;
    clearData( TRUE );
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
	for ( int row = 0; row < colorTable->numRows(); ++row )
	    for ( int col = 0; col < colorTable->numCols(); ++col )
		colorTable->clearCell( row, col );

	colorTable->setNumRows( m_colors.count() );
	QPixmap pixmap( 22, 22 );
	int row = 0;
	QMap<QString,QColor>::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    colorTable->setText( row, COL_NAME, it.key() );
	    colorTable->setPixmap( row, COL_NAME, pixmap );
	    colorTable->setText( row, COL_HEX, color.name().upper() );
	    if ( m_show_web ) {
		QCheckTableItem *item = new QCheckTableItem( colorTable, "" );
		item->setChecked(
		    isWebColor( color.red(), color.green(), color.blue() ) );
		colorTable->setItem( row, COL_WEB, item );
	    }
	    row++;
	}
	colorTable->adjustColumn( COL_NAME );
	colorTable->adjustColumn( COL_HEX );
	if ( m_show_web ) {
	    colorTable->showColumn( COL_WEB );
	    colorTable->adjustColumn( COL_WEB );
	}
	else
	    colorTable->hideColumn( COL_WEB );
	m_table_dirty = FALSE;
    }

    if ( m_icons_dirty ) {
	colorIconView->clear();

	QMap<QString,QColor>::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it )
	    (void) new QIconViewItem( colorIconView, it.key(),
				      colorSwatch( it.data() ) );
	m_icons_dirty = FALSE;
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
    if ( okToClear() ) {
	m_filename = "";
	m_changed = FALSE;
	m_table_dirty = TRUE;
	m_icons_dirty = TRUE;
	clearData( FALSE );
    }
}

void MainForm::fileOpen()
{
    if ( !okToClear() )
	return;

    QString filename = QFileDialog::getOpenFileName(
			    QString::null, "Colors (*.txt)", this,
			    "file open", "Color Tool -- File Open" );
    if ( !filename.isEmpty() )
	load( filename );
    else
	statusBar()->message( "File Open abandoned", 2000 );
}

void MainForm::fileSave()
{
    if ( m_filename.isEmpty() ) {
	fileSaveAs();
	return;
    }

    QFile file( m_filename );
    if ( file.open( IO_WriteOnly ) ) {
	QTextStream stream( &file );
	if ( !m_comments.isEmpty() )
	    stream << m_comments.join( "\n" ) << "\n";
	QMap<QString,QColor>::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    stream << QString( "%1 %2 %3\t\t%4" ).
			arg( color.red(), 3 ).
			arg( color.green(), 3 ).
			arg( color.blue(), 3 ).
			arg( it.key() ) << "\n";
	}
	file.close();
	setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	statusBar()->message( QString( "Saved %1 colors to '%2'" ).
				arg( m_colors.count() ).
				arg( m_filename ), 3000 );
	m_changed = FALSE;
    }
    else
	statusBar()->message( QString( "Failed to save '%1'" ).
				arg( m_filename ), 3000 );

}

void MainForm::fileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(
			    QString::null, "Colors (*.txt)", this,
			    "file save as", "Color Tool -- File Save As" );
    if ( !filename.isEmpty() ) {
	int ans = 0;
	if ( QFile::exists( filename ) )
	    ans = QMessageBox::warning(
			    this, "Color Tool -- Overwrite File",
			    QString( "Overwrite\n'%1'?" ).
				arg( filename ),
			    "&Yes", "&No", QString::null, 1, 1 );
	if ( ans == 0 ) {
	    m_filename = filename;
	    fileSave();
	    return;
	}
    }
    statusBar()->message( "Saving abandoned", 2000 );
}

void MainForm::load( const QString& filename )
{
    clearData( FALSE );
    QRegExp regex( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\S+.*)$" );
    QFile file( filename );
    if ( file.open( IO_ReadOnly ) ) {
	QTextStream stream( &file );
	QString line;
	while ( !stream.eof() ) {
	    line = stream.readLine();
	    if ( regex.search( line ) == -1 )
		m_comments += line;
	    else
		m_colors[regex.cap( 4 )] = QColor(
					    regex.cap( 1 ).toInt(),
					    regex.cap( 2 ).toInt(),
					    regex.cap( 3 ).toInt() );
	}
	file.close();
	setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	statusBar()->message( QString( "Loaded %1 colors from '%2'" ).
				arg( m_colors.count() ).
				arg( m_filename ), 3000 );
	m_filename = filename;
	m_table_dirty = TRUE;
	m_icons_dirty = TRUE;
	populate();
	m_changed = FALSE;
    }
    else
	statusBar()->message( QString( "Failed to load '%1'" ).
				arg( m_filename ), 3000 );
}


bool MainForm::okToClear()
{
    if ( m_changed ) {
	QString msg;
	if ( m_filename.isEmpty() )
	    msg = "Unnamed colors ";
	else
	    msg = QString( "Colors '%1'\n" ).arg( m_filename );
	msg += "has been changed.";
	int ans = QMessageBox::information(
			this,
			"Color Tool -- Unsaved Changes",
			msg, "&Save", "Cancel", "&Abandon",
			0, 1 );
	if ( ans == 0 )
	    fileSave();
	else if ( ans == 1 )
	    return FALSE;
    }

    return TRUE;
}



void MainForm::fileExit()
{
    if ( okToClear() )
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
	QString name;
	QWidget *visible = colorWidgetStack->visibleWidget();

	if ( visible == tablePage && colorTable->numRows() ) {
	    int row = colorTable->currentRow();
	    name = colorTable->text( row, 0 );
	    colorTable->removeRow( colorTable->currentRow() );
	    if ( row < colorTable->numRows() )
		colorTable->setCurrentCell( row, 0 );
	    else if ( colorTable->numRows() )
		colorTable->setCurrentCell( colorTable->numRows() - 1, 0 );
	    m_icons_dirty = TRUE;
	}
	else if ( visible == iconsPage && colorIconView->currentItem() ) {
	    QIconViewItem *item = colorIconView->currentItem();
	    name = item->text();
	    QIconViewItem *current = item->nextItem();
	    if ( !current ) current = item->prevItem();
	    delete item;
	    if ( current )
		colorIconView->setCurrentItem( current );
	    colorIconView->arrangeItemsInGrid();
	    m_table_dirty = TRUE;
	}

	if ( name ) {
	    m_colors.remove( name );
	    m_changed = TRUE;
	    statusBar()->message( QString( "Deleted %1" ).arg( name ), 5000 );
	}
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


void MainForm::changedTableColor( int row, int )
{
    	changedColor( colorTable->text( row, 0 ) );
}

void MainForm::changedIconColor( QIconViewItem *item )
{
	changedColor( item->text() );
}

void MainForm::changedColor( const QString& name )
{
    QColor color = m_colors[name];
    statusBar()->message( QString( "%1 \"%2\" (%3,%4,%5)%6" ).
			  arg( name ).
			  arg( color.name().upper() ).
			  arg( color.red() ).
			  arg( color.green() ).
			  arg( color.blue() ).
			  arg( isWebColor( color.red(),
					   color.green(),
					   color.blue() ) ? " web" : "" ) );
}

void MainForm::aboutToShow( int )
{
    populate();
}


void MainForm::changeView(QAction* action)
{
    if ( action == viewTableAction )
	colorWidgetStack->raiseWidget( tablePage );
    else
	colorWidgetStack->raiseWidget( iconsPage );
}

bool MainForm::isWebColor( int r, int g, int b )
{
    return ( ( r ==   0 || r ==  51 || r == 102 ||
		r == 153 || r == 204 || r == 255 ) &&
		( g ==   0 || g ==  51 || g == 102 ||
		g == 153 || g == 204 || g == 255 ) &&
		( b ==   0 || b ==  51 || b == 102 ||
		b == 153 || b == 204 || b == 255 ) );
}




void MainForm::editAdd()
{
    QColor color = white;
    if ( !m_colors.isEmpty() ) {
	QWidget *visible = colorWidgetStack->visibleWidget();
	if ( visible == tablePage )
	    color = colorTable->text( colorTable->currentRow(),
					colorTable->currentColumn() );
	else
	    color = colorIconView->currentItem()->text();
    }
    color = QColorDialog::getColor( color, this );
    /*
    if ( color.isValid() ) {
	QPixmap pixmap( 80, 10 );
	pixmap.fill( color );
	ColorNameForm *colorForm = new ColorNameForm( this, "color", TRUE );
	colorForm->setColors( m_colors );
	colorForm->colorTextLabel->setPixmap( pixmap );
	if ( colorForm->exec() ) {
	    QString name = colorForm->nameLineEdit->text();
	    m_colors[name] = color;
	    QPixmap pixmap( 22, 22 );
	    pixmap.fill( color );
	    int row = m_table->currentRow();
	    colorTable->insertRows( row, 1 );
	    colorTable->setItem( row, COL_NAME,
				 new TableItem( colorTable, QTableItem::Never,
						name, pixmap ) ) ;
	    colorTable->setText( row, COL_HEX, color.name().upper() );
	    colorTable->setCurrentCell( row, colorTable->currentColumn() );
	    if ( m_show_web &&
		isWebColor( color.red(), color.green(), color.blue() ) )
	    (void) new QIconViewItem( colorIconView, name,
				      colorSwatch( color ) );
	    m_changed = TRUE;
	    m_table_dirty = TRUE;
	    m_icons_dirty = TRUE;
	}
    }
    */
}
