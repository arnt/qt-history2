/*
    TODO copy/delete slots
    TODO QSettings
*/

#include "optionsform.h"
#include "colornameform.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qfiledialog.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qregexp.h>
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


typedef QMap<QString,QColor> ColorMap;


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
	QAction *editCopyAction;
	QAction *editDeleteAction;
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
	editDeleteAction->addTo( editMenu );
	editMenu->insertSeparator();
	editOptionsAction->addTo( editMenu );
	QPopupMenu *viewMenu = new QPopupMenu( this );
	menuBar()->insertItem( "&View", viewMenu );
	viewTableAction->addTo( viewMenu );
	viewIconsAction->addTo( viewMenu );
	viewTextAction->addTo( viewMenu );

	// TODO read as settings, along with winpos & winsize
	m_clip_as = CLIP_AS_HEX;
	m_show_web = true;
	m_test_text = "The quick brown fox jumped over the lazy dogs.";


	m_view = new QWidgetStack( this );
	m_table = new QTable( 0, 3, this );
	m_table->setReadOnly( true );
	m_table->setLeftMargin( 0 );
	m_table->verticalHeader()->hide();
//m_table->sortColumn( COL_NAME, true, true );
//m_table->setSorting( true );
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

	// Connect after the widget stack exists
	connect( viewActionGroup, SIGNAL( selected(QAction*) ),
		 this, SLOT( setView(QAction*) ) );
	viewTableAction->setOn( true );


	if ( !m_filename.isEmpty() )
	    load( m_filename );
	else
	    init( true );

	resize( 450, 500 );
    }

    ~MainWindow() {}

private slots:

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
	    statusBar()->message( QString( "Saved %1 colors to \'%2\'" ).
				    arg( m_colors.count() ).
				    arg( m_filename ), 3000 );
	}
	else
	    statusBar()->message( QString( "Failed to save \'%1\'" ).
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
				QString( "Overwrite\n\'%1\'?" ).
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
	if ( okToClear() )
	    qApp->exit( 0 );
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
		m_changed = true;
		addColor( name, color );
	    }
	}
    }

    void editCopy()
    {
	qDebug( "copy not done yet" ); // TODO
    }

    void editDelete()
    {
	qDebug( "delete not done yet" ); // TODO
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
	    if ( !text.isEmpty() )
		m_test_text = text;
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
	    if ( !m_colors.isEmpty() )
		changedIconColor( m_iconview->currentItem() );
	}
	else if ( action == viewTextAction ) {
	    m_view->raiseWidget( VIEW_TEXT );
	    statusBar()->message( "Click the text for color details" );
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

    void populate()
    {
	populateTableView();
	populateIconsView();
	populateTextView();
    }

    void populateTableView()
    {
	for ( int row = 0; row < m_table->numRows(); ++row )
	    for ( int col = 0; col < m_table->numCols(); ++col )
		m_table->clearCell( row, col );

	m_table->setNumRows( m_colors.count() );
	QPixmap pixmap( 22, 22 );
	int r, g, b;
	int row = 0;
	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    color.rgb( &r, &g, &b );
	    m_table->setText( row, COL_NAME, it.key() );
	    m_table->setPixmap( row, COL_NAME, pixmap );
	    m_table->setText( row, COL_HEX, color.name().upper() );
	    if ( m_show_web && isWebColor( r, g, b ) )
		m_table->setPixmap( row, COL_WEB, QPixmap( tick_xpm ) );
	    row++;
	}
	m_table->adjustColumn( COL_NAME );
	m_table->adjustColumn( COL_HEX );
	if ( m_show_web ) {
	    m_table->showColumn( COL_WEB );
	    m_table->adjustColumn( COL_WEB );
	}
	else
	    m_table->hideColumn( COL_WEB );
    }

    void populateIconsView()
    {
	m_iconview->clear();

	QPixmap pixmap( 80, 80 );
	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    (void) new QIconViewItem( m_iconview, it.key(), pixmap );
	}
    }

    void populateTextView()
    {
	m_textview->clear();

	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    m_textview->append( QString( "%1: <font color=%2>%3</font>" ).
				    arg( it.key() ).arg( color.name() ).
				    arg( m_test_text ) );
	}
    }

    void addColor( const QString& name, const QColor& color )
    {
	m_colors[name] = color;
	QPixmap pixmap( 22, 22 );
	pixmap.fill( color );
	int r, g, b;
	color.rgb( &r, &g, &b );
	int row = m_table->currentRow();
	m_table->insertRows( row, 1 );
	m_table->setText( row, COL_NAME, name );
	m_table->setPixmap( row, COL_NAME, pixmap );
	m_table->setText( row, COL_HEX, color.name().upper() );
	m_table->setCurrentCell( row, m_table->currentColumn() );
	if ( m_show_web && isWebColor( r, g, b ) )
	    m_table->setPixmap( row, COL_WEB, QPixmap( tick_xpm ) );
	pixmap.resize( 80, 80 );
	pixmap.fill( color );
	(void) new QIconViewItem( m_iconview, name, pixmap );
	m_textview->append( QString( "%1: <font color=%2>%3</font>" ).
				arg( name ).arg( color.name() ).
				arg( m_test_text ) );
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
	    populate();
	    m_changed = false;
	    setCaption( QString( "Color Tool -- %1" ).arg( m_filename ) );
	    statusBar()->message( QString( "Loaded %1 colors from \'%2\'" ).
				    arg( m_colors.count() ).
				    arg( m_filename ), 3000 );
	}
	else
	    statusBar()->message( QString( "Failed to load \'%1\'" ).
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
	    switch( QMessageBox::information( this,
			"Color Tool -- Unsaved Changes",
			msg, "&Save", "Cancel", "&Abandon",
			0, 1 ) ) {
		case 0:
		    fileSave();
		    break;
		case 1: // fallthrough
		default:
		    return false;
		    break;
		case 2:
		    break;
	    }
	}

	return true;
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
    QPopupMenu *contextMenu;

    QWidgetStack *m_view;
    QTable *m_table;
    QIconView *m_iconview;
    QTextEdit *m_textview;
    QString m_filename;
    QStringList m_comments;
    ColorMap m_colors;
    bool m_changed;
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
    app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );

    return app.exec();
}

#include "main.moc"
