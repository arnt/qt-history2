#include "optionsform.h"
#include "colornameform.h"
#include "findform.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qclipboard.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpainter.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qregexp.h>
#include <qsettings.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <qwidgetstack.h>


static const char *tick_xpm[] = {
"24 19 10 1",
"  c #5555E1",
". c #1D1DD7",
"X c #E3E3FA",
"o c #AAAAF0",
"O c #7272E6",
"+ c #3939DC",
"@ c #0000D2",
"# c #FFFFFF",
"$ c #C7C7F5",
"% c #8E8EEB",
"###################XoX##",
"#################$+@@$##",
"################$@@@O###",
"###############X.@@ ####",
"###############+@@.X####",
"##############O@@@X#####",
"#############$@@@o######",
"#############.@@O#######",
"############ @@+########",
"###########o@@.X########",
"######X$##X.@@o#########",
"##### @@X# @@ ##########",
"####o@@@%o@@.###########",
"####X@@@..@@$###########",
"#####+@@@@@O############",
"#####%@@@@.#############",
"#####X@@@@$#############",
"######O@@ ##############",
"#######XX###############"
};


const QString WINDOWS_REGISTRY = "/QtExamples";
const QString APP_KEY = "/ColorTool/";


typedef QMap<QString,QColor> ColorMap;
typedef QMap<QString,QString> LookupMap;


class TableItem : public QTableItem
{
    enum { LIGHT = 0x01, DIM = 0x02, DARK = 0x04, DEEP = 0x08 };
    mutable LookupMap cache;
public:
    TableItem( QTable *table, EditType et, const QString& text )
	: QTableItem( table, et, text ) {}
    TableItem( QTable *table, EditType et, const QString& text, const QPixmap& p )
	: QTableItem( table, et, text, p ) {}
    ~TableItem() {}

    QString key() const
    {
	QString name = this->text();
	if ( cache.contains( name ) )
	    return cache[name];

	QString temp = name.lower();
	QString text;
	QString num;
	for ( int i = 0; i < (int)temp.length(); ++i )
	    if ( temp[i].isDigit() )
		num += temp[i];
	    else if ( !temp[i].isSpace() )
		text += temp[i];
	if ( !num.isNull() )
	    text += QString( "%1" ).arg( num.toInt(), 3 );
	int modifier = 0;
	if ( text.contains( "light" ) ) modifier |= LIGHT;
	if ( text.contains( "dim" ) )   modifier |= DIM;
	if ( text.contains( "dark" ) )  modifier |= DARK;
	if ( text.contains( "deep" ) )  modifier |= DEEP;
	text = text.replace( QRegExp( "light|dim|dark|deep" ), "" );
	QString midfix;
	if ( modifier ) {
	    if ( modifier & LIGHT ) midfix += "A";
	    if ( modifier & DIM )   midfix += "C";
	    if ( modifier & DARK )  midfix += "D";
	    if ( modifier & DEEP )  midfix += "E";
	}
	else
	    midfix = "B";
	return cache[name] = text + midfix + QString( "%1" ).arg( modifier, 3 );
    }
};



class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum { COL_NAME = 0, COL_HEX, COL_WEB };
    enum { VIEW_TABLE = 0, VIEW_ICONS, VIEW_TEXT };
    enum { CLIP_AS_HEX, CLIP_AS_NAME, CLIP_AS_RGB };
public:
    MainWindow( const QString &filename,
		QWidget *parent = 0, const char *name = 0,
		WFlags f = WType_TopLevel )
	: QMainWindow( parent, name, f ), m_filename( filename )
    {
	QAction *fileNewAction;
	QAction *fileOpenAction;
	QAction *fileSaveAction;
	QAction *fileSaveAsAction;
	QAction *fileQuitAction;
	QAction *editAddAction;
	QAction *editOptionsAction;
	QActionGroup *viewActionGroup;

	fileNewAction = new QAction(
		"New Colors", "&New", CTRL+Key_N, this, "new" );
	connect( fileNewAction, SIGNAL( activated() ),
		 this, SLOT( fileNew() ) );

	fileOpenAction = new QAction(
		"Open Colors", "&Open...", CTRL+Key_O, this, "open" );
	connect( fileOpenAction, SIGNAL( activated() ),
		 this, SLOT( fileOpen() ) );

	fileSaveAction = new QAction(
		"Save Colors", "&Save", CTRL+Key_S, this, "save" );
	connect( fileSaveAction, SIGNAL( activated() ),
		 this, SLOT( fileSave() ) );

	fileSaveAsAction = new QAction(
		"Save Colors As", "Save &As...", 0, this, "save as" );
	connect( fileSaveAsAction, SIGNAL( activated() ),
		 this, SLOT( fileSaveAs() ) );

	fileQuitAction = new QAction( "Quit", "&Quit", CTRL+Key_Q, this, "quit" );
	connect( fileQuitAction, SIGNAL( activated() ),
		 this, SLOT( fileQuit() ) );

	editAddAction = new QAction(
		"Add Color", "&Add...", CTRL+Key_A, this, "add" );
	connect( editAddAction, SIGNAL( activated() ),
		 this, SLOT( editAdd() ) );

	editCopyAction = new QAction(
		"Copy Color to the clipboard", "&Copy", CTRL+Key_C,
		this, "copy" );
	connect( editCopyAction, SIGNAL( activated() ),
		 this, SLOT( editCopy() ) );

	editFindAction = new QAction(
		"Find Color", "&Find...", CTRL+Key_F, this, "find" );
	connect( editFindAction, SIGNAL( activated() ),
		 this, SLOT( editFind() ) );

	editDeleteAction = new QAction(
		"Delete Color", "&Delete", CTRL+Key_D, this, "delete" );
	connect( editDeleteAction, SIGNAL( activated() ),
		 this, SLOT( editDelete() ) );

	editOptionsAction = new QAction(
		"Set Options", "&Options...", 0, this, "options" );
	connect( editOptionsAction, SIGNAL( activated() ),
		 this, SLOT( editOptions() ) );

	viewActionGroup = new QActionGroup( this );
	viewTableAction = new QAction(
		"View Table", "View &Table", CTRL+Key_T, viewActionGroup );
	viewTableAction->setToggleAction( true );
	viewIconsAction = new QAction(
		"View Icons", "View &Icons", CTRL+Key_I, viewActionGroup );
	viewIconsAction->setToggleAction( true );
	viewTextAction = new QAction(
		"View Text", "View Te&xt", CTRL+Key_X, viewActionGroup );
	viewTextAction->setToggleAction( true );

	QToolBar *fileTools = new QToolBar( this, "file operations" );
	fileTools->setLabel( "File Operations" );
	fileNewAction->addTo( fileTools );
	fileOpenAction->addTo( fileTools );
	fileSaveAction->addTo( fileTools );
	QToolBar *editTools = new QToolBar( this, "editing" );
	editTools->setLabel( "Editing" );
	editAddAction->addTo( editTools );
	editCopyAction->addTo( editTools );
	editFindAction->addTo( editTools );
	editDeleteAction->addTo( editTools );
	editTools->addSeparator();
	editOptionsAction->addTo( editTools );
	QToolBar *viewTools = new QToolBar( this, "viewing" );
	viewTools->setLabel( "Viewing" );
	viewTableAction->addTo( viewTools );
	viewIconsAction->addTo( viewTools );
	viewTextAction->addTo( viewTools );

	QPopupMenu *fileMenu = new QPopupMenu( this );
	menuBar()->insertItem( "&File", fileMenu );
	fileNewAction->addTo( fileMenu );
	fileOpenAction->addTo( fileMenu );
	fileSaveAction->addTo( fileMenu );
	fileSaveAsAction->addTo( fileMenu );
	fileMenu->insertSeparator();
	fileQuitAction->addTo( fileMenu );
	QPopupMenu *editMenu = new QPopupMenu( this );
	menuBar()->insertItem( "&Edit", editMenu );
	editAddAction->addTo( editMenu );
	editCopyAction->addTo( editMenu );
	editFindAction->addTo( editMenu );
	editDeleteAction->addTo( editMenu );
	editMenu->insertSeparator();
	editOptionsAction->addTo( editMenu );
	QPopupMenu *viewMenu = new QPopupMenu( this );
	menuBar()->insertItem( "&View", viewMenu );
	viewTableAction->addTo( viewMenu );
	viewIconsAction->addTo( viewMenu );
	viewTextAction->addTo( viewMenu );

	QSettings settings;
	settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
	int windowWidth = settings.readNumEntry( APP_KEY + "WindowWidth", 450 );
	int windowHeight = settings.readNumEntry( APP_KEY + "WindowHeight", 500 );
	int windowX = settings.readNumEntry( APP_KEY + "WindowX", 0 );
	int windowY = settings.readNumEntry( APP_KEY + "WindowY", 0 );
	m_clip_as = settings.readNumEntry( APP_KEY + "ClipAs", CLIP_AS_HEX );
	m_show_web = settings.readBoolEntry( APP_KEY + "ShowWeb", true );
	m_test_text = settings.readEntry( APP_KEY + "TestText",
			"The quick brown fox jumped over the lazy dogs." );
	int view = settings.readNumEntry( APP_KEY + "View", VIEW_TABLE );
	resize( windowWidth, windowHeight );
	move( windowX, windowY );

	findForm = 0;
	m_currentcolorname = "white";
	m_currentcolor = white;
	m_changed = false;
	m_dirty[VIEW_TABLE] = true;
	m_dirty[VIEW_ICONS] = true;
	m_dirty[VIEW_TEXT] = true;

	clipboard = QApplication::clipboard();
	if ( clipboard->supportsSelection() )
	    clipboard->setSelectionMode( TRUE );

	m_view = new QWidgetStack( this );
	m_table = new QTable( 0, 3, this );
	m_table->setReadOnly( true );
	m_table->setLeftMargin( 0 );
	m_table->verticalHeader()->hide();
	QHeader *header = m_table->horizontalHeader();
	header->setLabel( COL_NAME, "Name" );
	header->setLabel( COL_HEX, "Hex" );
	header->setLabel( COL_WEB, "Web" );
	connect( m_table, SIGNAL( currentChanged(int,int) ),
		 this, SLOT( changedTableColor(int) ) );
	m_view->addWidget( m_table, VIEW_TABLE );
	m_iconview = new QIconView( this );
	connect( m_iconview, SIGNAL( currentChanged(QIconViewItem*) ),
		 this, SLOT( changedIconColor(QIconViewItem*) ) );
	m_view->addWidget( m_iconview, VIEW_ICONS );
	m_textview = new QTextEdit( this );
	m_textview->setReadOnly( true );
	m_textview->setTextFormat( RichText );
	connect( m_textview, SIGNAL( clicked(int, int) ),
		 this, SLOT( changedTextColor(int) ) );
	m_view->addWidget( m_textview, VIEW_TEXT );
	setCentralWidget( m_view );
	connect( m_view, SIGNAL( aboutToShow(int) ),
		 this, SLOT( aboutToShow(int) ) );

	// Connect after the widget stack exists
	connect( viewActionGroup, SIGNAL( selected(QAction*) ),
		 this, SLOT( setView(QAction*) ) );

	switch ( view ) {
	    case VIEW_TABLE : viewTableAction->setOn( true ); break;
	    case VIEW_ICONS : viewIconsAction->setOn( true ); break;
	    case VIEW_TEXT :  viewTextAction->setOn( true ); break;
	}

	if ( !m_filename.isEmpty() )
	    load( m_filename );
	else
	    init( true );
    }

    ~MainWindow() {}

private slots:

    void aboutToShow( int view )
    {
	bool disableForText = view == VIEW_TEXT ? false : true;
	editCopyAction->setEnabled( disableForText );
	editFindAction->setEnabled( disableForText );
	editDeleteAction->setEnabled( disableForText );
	populate( view );
    }

    void changedTableColor( int row )
    {
	changedColor( m_table->text( row, 0 ) );
    }

    void changedIconColor( QIconViewItem *item )
    {
	changedColor( item->text() );
    }

    void changedTextColor( int para )
    {
	QString name = m_textview->text( para );
	changedColor( name.left( name.find( ":" ) ) );
    }

    void changedColor( const QString& name )
    {
	QColor color = m_colors[name];
	m_currentcolorname = name;
	m_currentcolor = color;
	int r, g, b;
	color.rgb( &r, &g, &b );
	statusBar()->message( QString( "%1 \"%2\" (%3,%4,%5)%6" ).
				arg( name ).
				arg( color.name() ).
				arg( r ).arg( g ).arg( b ).
				arg( isWebColor( r, g, b ) ? " web" : "" ) );
    }

    void fileNew()
    {
	if ( okToClear() )
	    init();
    }

    void fileOpen()
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

    void fileSave()
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
	    int r, g, b;
	    ColorMap::Iterator it;
	    for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
		QColor color = it.data();
		color.rgb( &r, &g, &b );
		stream << QString( "%1 %2 %3\t\t%4" ).
			    arg( r, 3 ).arg( g, 3 ).arg( b, 3 ).
			    arg( it.key() ) << "\n";
	    }
	    file.close();
	    m_changed = false;
	    setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	    statusBar()->message( QString( "Saved %1 colors to '%2'" ).
				    arg( m_colors.count() ).
				    arg( m_filename ), 3000 );
	}
	else
	    statusBar()->message( QString( "Failed to save '%1'" ).
				    arg( m_filename ), 3000 );
    }

    void fileSaveAs()
    {
	QString filename = QFileDialog::getSaveFileName(
				QString::null, "Colors (*.txt)", this,
				"file save as", "Color Tool -- File Save As" );
	if ( !filename.isEmpty() ) {
	    int answer = 0;
	    if ( QFile::exists( filename ) )
		answer = QMessageBox::warning(
				this, "Color Tool -- Overwrite File",
				QString( "Overwrite\n'%1'?" ).
				    arg( filename ),
				"&Yes", "&No", QString::null, 1, 1 );
	    if ( answer == 0 ) {
		m_filename = filename;
		fileSave();
		return;
	    }
	}
	statusBar()->message( "Saving abandoned", 2000 );
    }

    void fileQuit()
    {
	if ( okToClear() ) {
	    saveOptions();
	    qApp->exit( 0 );
	}
    }

    void editAdd()
    {
	QColor color = white;
	if ( !m_colors.isEmpty() ) {
	    int id = m_view->id( m_view->visibleWidget() );
	    if ( id == VIEW_TABLE )
		color = m_table->text( m_table->currentRow(),
				       m_table->currentColumn() );
	    else if ( id == VIEW_ICONS )
		color = m_iconview->currentItem()->text();
	}
	color = QColorDialog::getColor( color, this );
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
		m_table->insertRows( row, 1 );
		m_table->setItem( row, COL_NAME,
				  new TableItem( m_table, QTableItem::Never,
						 name, pixmap ) ) ;
		m_table->setText( row, COL_HEX, color.name().upper() );
		m_table->setCurrentCell( row, m_table->currentColumn() );
		if ( m_show_web &&
		     isWebColor( color.red(), color.green(), color.blue() ) )
		    m_table->setPixmap( row, COL_WEB, QPixmap( tick_xpm ) );
		(void) new QIconViewItem( m_iconview, name,
					  colorSwatch( color ) );
		m_textview->append( QString( "%1: <font color=%2>%3</font>" ).
					arg( name ).arg( color.name() ).
					arg( m_test_text ) );
		m_currentcolorname = name;
		m_currentcolor = color;
		m_changed = true;
		m_dirty[VIEW_TABLE] = true;
		m_dirty[VIEW_ICONS] = true;
		m_dirty[VIEW_TEXT] = true;
	    }
	}
    }

    void editCopy()
    {
	QString text;
	switch ( m_clip_as ) {
	    case CLIP_AS_HEX:
		text = m_currentcolor.name();
		break;
	    case CLIP_AS_NAME:
		text = m_currentcolorname;
		break;
	    case CLIP_AS_RGB:
		text = QString( "%1,%2,%3" ).
			    arg( m_currentcolor.red() ).
			    arg( m_currentcolor.green() ).
			    arg( m_currentcolor.blue() );
		break;
	}
	clipboard->setText( text );
    }

    void editFind()
    {
	if ( !findForm ) {
	    findForm = new FindForm( this );
	    connect( findForm, SIGNAL( lookfor(const QString&) ),
		     this, SLOT( lookfor(const QString&) ) );
	}
	findForm->show();
    }

    void lookfor( const QString& text )
    {
	QString ltext = text.lower();
	bool found = false;
	int id = m_view->id( m_view->visibleWidget() );
	if ( id == VIEW_TABLE ) {
	    int row = m_table->currentRow();
	    for ( int i = row + 1; i < m_table->numRows(); ++i )
		if ( m_table->text( i, 0 ).lower().contains( ltext ) ) {
		    m_table->setCurrentCell( i, 0 );
		    found = true;
		    break;
		}
	    if ( !found )
		m_table->setCurrentCell( row, 0 );

	}
	else if ( id == VIEW_ICONS ) {
	    QIconViewItem *start = m_iconview->currentItem();
	    for ( QIconViewItem *item = start->nextItem();
		  item; item = item->nextItem() )
		if ( item->text().lower().contains( ltext ) ) {
		    m_iconview->setCurrentItem( item );
		    m_iconview->ensureItemVisible( item );
		    found = true;
		    break;
		}
	    if ( !found && start )
		m_iconview->setCurrentItem( start );
	}
	if ( !found )
	    findForm->notfound();
    }

    void editDelete()
    {
	QString name;
	m_currentcolorname = QString::null;
	int id = m_view->id( m_view->visibleWidget() );
	if ( id == VIEW_TABLE && m_table->numRows() ) {
	    int row = m_table->currentRow();
	    name = m_table->text( row, 0 );
	    m_table->removeRow( m_table->currentRow() );
	    if ( row < m_table->numRows() )
		m_table->setCurrentCell( row, 0 );
	    else if ( m_table->numRows() )
		m_table->setCurrentCell( m_table->numRows() - 1, 0 );
	    if ( m_table->numRows() )
		m_currentcolorname = m_table->text( m_table->currentRow(),
						    m_table->currentColumn() );
	    m_dirty[VIEW_ICONS] = true;
	}
	else if ( id == VIEW_ICONS && m_iconview->currentItem() ) {
	    QIconViewItem *item = m_iconview->currentItem();
	    name = item->text();
	    QIconViewItem *current = item->nextItem();
	    if ( !current ) current = item->prevItem();
	    delete item;
	    if ( current ) {
		m_iconview->setCurrentItem( current );
		m_currentcolorname = current->text();
	    }
	    m_iconview->arrangeItemsInGrid();
	    m_dirty[VIEW_TABLE] = true;
	}

	m_dirty[VIEW_TEXT] = true;

	if ( !name.isNull() )
	    m_colors.remove( name );
	if ( !m_currentcolorname.isNull() ) {
	    changedColor( m_currentcolorname );
	}
	else {
	    m_currentcolorname = "white"; // These always exist
	    m_currentcolor = white;
	}
	m_changed = true;
    }

    void editOptions()
    {
	OptionsForm *options = new OptionsForm( this, "options", TRUE );

	switch ( m_clip_as ) {
	    case CLIP_AS_HEX:
		options->hexRadioButton->setChecked( true );
		break;
	    case CLIP_AS_NAME:
		options->nameRadioButton->setChecked( true );
		break;
	    case CLIP_AS_RGB:
		options->rgbRadioButton->setChecked( true );
		break;
	}
	options->textLineEdit->setText( m_test_text );
	options->webCheckBox->setChecked( m_show_web );

	if ( options->exec() ) {
	    if ( options->hexRadioButton->isChecked() )
		m_clip_as = CLIP_AS_HEX;
	    else if ( options->nameRadioButton->isChecked() )
		m_clip_as = CLIP_AS_NAME;
	    else if ( options->rgbRadioButton->isChecked() )
		m_clip_as = CLIP_AS_RGB;
	    QString text = options->textLineEdit->text();
	    m_dirty[VIEW_TEXT] = m_test_text != text;
	    if ( !text.isEmpty() )
		m_test_text = text;
	    m_dirty[VIEW_TABLE] = m_show_web !=
				  options->webCheckBox->isChecked();
	    m_show_web = options->webCheckBox->isChecked();

	    populate();
	}

	delete options;
    }

    void setView( QAction *action )
    {
	if ( action == viewTableAction ) {
	    m_view->raiseWidget( VIEW_TABLE );
	    if ( !m_colors.isEmpty() )
		changedTableColor( m_table->currentRow() );
	}
	else if ( action == viewIconsAction ) {
	    m_view->raiseWidget( VIEW_ICONS );
	    if ( !m_colors.isEmpty() ) {
		QIconViewItem *item = m_iconview->currentItem();
		if ( !item ) {
		    m_iconview->setCurrentItem( m_iconview->firstItem() );
		    item = m_iconview->currentItem();
		}
		if ( item )
		    changedIconColor( item );
	    }
	}
	else if ( action == viewTextAction ) {
	    m_view->raiseWidget( VIEW_TEXT );
	    statusBar()->message( "Click the text for color details", 5000 );
	}
    }


private:

    void init( bool fillWithDefaults = FALSE )
    {
	setCaption( "Color Tool" );

	m_colors.clear();
	m_comments.clear();

	if ( fillWithDefaults ) {
	    m_colors["black"] = black;
	    m_colors["blue"] = blue;
	    m_colors["cyan"] = cyan;
	    m_colors["darkblue"] = darkBlue;
	    m_colors["darkcyan"] = darkCyan;
	    m_colors["darkgray"] = darkGray;
	    m_colors["darkgreen"] = darkGreen;
	    m_colors["darkmagenta"] = darkMagenta;
	    m_colors["darkred"] = darkRed;
	    m_colors["darkyellow"] = darkYellow;
	    m_colors["gray"] = gray;
	    m_colors["green"] = green;
	    m_colors["lightgray"] = lightGray;
	    m_colors["magenta"] = magenta;
	    m_colors["red"] = red;
	    m_colors["white"] = white;
	    m_colors["yellow"] = yellow;
	}

	populate();
    }

    void populate( int view = -1 )
    {
	if ( view == -1 )
	    view = m_view->id( m_view->visibleWidget() );

	switch ( view ) {
	    case VIEW_TABLE: populateTableView(); break;
	    case VIEW_ICONS: populateIconsView(); break;
	    case VIEW_TEXT: populateTextView(); break;
	}
    }

    void populateTableView()
    {
	if ( !m_dirty[VIEW_TABLE] )
	    return;

	for ( int row = 0; row < m_table->numRows(); ++row )
	    for ( int col = 0; col < m_table->numCols(); ++col )
		m_table->clearCell( row, col );

	m_table->setNumRows( m_colors.count() );
	QPixmap pixmap( 22, 22 );
	int row = 0;
	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    m_table->setItem( row, COL_NAME,
			      new TableItem( m_table, QTableItem::Never,
					     it.key(), pixmap ) ) ;
	    m_table->setText( row, COL_HEX, color.name().upper() );
	    if ( m_show_web &&
		 isWebColor( color.red(), color.green(), color.blue() ) )
		m_table->setPixmap( row, COL_WEB, QPixmap( tick_xpm ) );
	    row++;
	}
	m_table->sortColumn( COL_NAME, true, true );
	m_table->adjustColumn( COL_NAME );
	m_table->adjustColumn( COL_HEX );
	if ( m_show_web ) {
	    m_table->showColumn( COL_WEB );
	    m_table->adjustColumn( COL_WEB );
	}
	else
	    m_table->hideColumn( COL_WEB );
	m_dirty[VIEW_TABLE] = false;
    }

    void populateIconsView()
    {
	if( !m_dirty[VIEW_ICONS] )
	    return;

	m_iconview->clear();

	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it )
	    (void) new QIconViewItem( m_iconview, it.key(),
				      colorSwatch( it.data() ) );
	m_dirty[VIEW_ICONS] = false;
    }

    void populateTextView()
    {
	if ( !m_dirty[VIEW_TEXT] )
	    return;

	m_textview->clear();

	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    m_textview->append( QString( "%1: <font color=%2>%3</font>" ).
				    arg( it.key() ).arg( color.name() ).
				    arg( m_test_text ) );
	}
	m_dirty[VIEW_TEXT] = false;
    }

    QPixmap colorSwatch( QColor color )
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

    void load( const QString& filename )
    {
	init();
	QRegExp regex( "^\\s*(\\d+)\\s+(\\d+)\\s+(\\d+)\\s+(\\S+.*)$" );
	QFile file( filename );
	if ( file.open( IO_ReadOnly ) ) {
	    QTextStream stream( &file );
	    QString line;
	    while ( !stream.eof() ) {
		line = stream.readLine();
		if ( regex.search( line ) == -1 )
		    m_comments += line;
		else {
		    m_colors[regex.cap( 4 )] = QColor(
						regex.cap( 1 ).toInt(),
						regex.cap( 2 ).toInt(),
						regex.cap( 3 ).toInt() );
		}
	    }
	    file.close();
	    m_filename = filename;
	    m_dirty[VIEW_TABLE] = true;
	    m_dirty[VIEW_ICONS] = true;
	    m_dirty[VIEW_TEXT] = true;
	    populate();
	    m_changed = false;
	    if ( !m_colors.isEmpty() ) {
		ColorMap::Iterator it = m_colors.begin();
		m_currentcolorname = it.key();
		m_currentcolor = m_colors[m_currentcolorname];
	    }
	    setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	    statusBar()->message( QString( "Loaded %1 colors from '%2'" ).
				    arg( m_colors.count() ).
				    arg( m_filename ), 3000 );
	}
	else
	    statusBar()->message( QString( "Failed to load '%1'" ).
				    arg( m_filename ), 3000 );
    }

    bool okToClear()
    {
	if ( m_changed ) {
	    QString msg;
	    if ( m_filename.isEmpty() )
		msg = "Unnamed colors ";
	    else
		msg = QString( "Colors '%1'\n" ).arg( m_filename );
	    msg += "has been changed.";
	    int ans = QMessageBox::information( this,
			"Color Tool -- Unsaved Changes",
			msg, "&Save", "Cancel", "&Abandon",
			0, 1 );
	    if ( ans == 0 )
		fileSave();
	    else if ( ans == 1 )
		return false;
	}

	return true;
    }

    void saveOptions()
    {
	QSettings settings;
	settings.insertSearchPath( QSettings::Windows, WINDOWS_REGISTRY );
	settings.writeEntry( APP_KEY + "WindowWidth", width() );
	settings.writeEntry( APP_KEY + "WindowHeight", height() );
	settings.writeEntry( APP_KEY + "WindowX", x() );
	settings.writeEntry( APP_KEY + "WindowY", y() );
	settings.writeEntry( APP_KEY + "ClipAs", m_clip_as );
	settings.writeEntry( APP_KEY + "ShowWeb", m_show_web );
	settings.writeEntry( APP_KEY + "TestText", m_test_text );
	settings.writeEntry( APP_KEY + "View",
			     m_view->id( m_view->visibleWidget() ) );
    }

    bool isWebColor( int r, int g, int b )
    {
	return ( ( r ==   0 || r ==  51 || r == 102 ||
		   r == 153 || r == 204 || r == 255 ) &&
		 ( g ==   0 || g ==  51 || g == 102 ||
		   g == 153 || g == 204 || g == 255 ) &&
		 ( b ==   0 || b ==  51 || b == 102 ||
		   b == 153 || b == 204 || b == 255 ) );
    }

    QAction *viewTableAction;
    QAction *viewIconsAction;
    QAction *viewTextAction;
    QAction *editCopyAction;
    QAction *editFindAction;
    QAction *editDeleteAction;
    QPopupMenu *contextMenu;
    QClipboard *clipboard;
    FindForm *findForm;

    QWidgetStack *m_view;
    QTable *m_table;
    QIconView *m_iconview;
    QTextEdit *m_textview;
    QString m_filename;
    QStringList m_comments;
    ColorMap m_colors;
    bool m_changed;
    bool m_dirty[3]; // One per VIEW
    QString m_currentcolorname;
    QColor m_currentcolor;
    QString m_test_text;
    int m_clip_as;
    bool m_show_web;
};



int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );
    QString filename;
    if ( app.argc() > 1 )
	filename = app.argv()[1];

    MainWindow *mw = new MainWindow( filename );
    app.setMainWidget( mw );
    mw->show();
    app.connect( &app, SIGNAL( lastWindowClosed() ), mw, SLOT( fileQuit() ) );

    return app.exec();
}

#include "main.moc"
