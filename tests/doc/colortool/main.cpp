#include "optionsform.h"

#include <qaction.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qfiledialog.h>
#include <qiconview.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qradiobutton.h>
#include <qstatusbar.h>
#include <qtable.h>
#include <qtextedit.h>
#include <qwidget.h>
#include <qwidgetstack.h>

static const char *tick_xpm[] = {
"22 19 10 1",
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
"###################XoX",
"#################$+@@$",
"################$@@@O#",
"###############X.@@ ##",
"###############+@@.X##",
"##############O@@@X###",
"#############$@@@o####",
"#############.@@O#####",
"############ @@+######",
"###########o@@.X######",
"######X$##X.@@o#######",
"##### @@X# @@ ########",
"####o@@@%o@@.#########",
"####X@@@..@@$#########",
"#####+@@@@@O##########",
"#####%@@@@.###########",
"#####X@@@@$###########",
"######O@@ ############",
"#######XX#############"
};


typedef QMap<QString,QColor> ColorMap;


class MainWindow : public QMainWindow
{
    Q_OBJECT

    enum { VIEW_LIST = 0, VIEW_ICONS, VIEW_TEXT };
    enum { CLIP_AS_HEX, CLIP_AS_NAME, CLIP_AS_RGB };
    enum { COL_NAME = 0, COL_HEX, COL_R, COL_G, COL_B, COL_WEB };
public:
    MainWindow( const QString &filename,
		QWidget *parent = 0, const char *name = 0,
		WFlags f = WType_TopLevel )
	: QMainWindow( parent, name, f ), m_filename( filename )
    {
	setCaption( "Color Tool" );

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
	viewListAction = new QAction(
		"View List", "View &List", CTRL+Key_L, viewActionGroup );
	viewListAction->setToggleAction( true );
	viewIconsAction = new QAction(
		"View Icons", "View &Icons", CTRL+Key_I, viewActionGroup );
	viewIconsAction->setToggleAction( true );
	viewTextAction = new QAction(
		"View Text", "View &Text", CTRL+Key_T, viewActionGroup );
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
	viewListAction->addTo( viewTools );
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
	viewListAction->addTo( viewMenu );
	viewIconsAction->addTo( viewMenu );
	viewTextAction->addTo( viewMenu );


	m_view = new QWidgetStack( this );
	QTable *table = new QTable( 0, 6, this );
	table->setReadOnly( true );
	table->setLeftMargin( 0 );
	table->verticalHeader()->hide();
	//table->sortColumn( COL_NAME, true, true );
	//table->setSorting( true );
	QHeader *header = table->horizontalHeader();
	header->setLabel( COL_NAME, "Name" );
	header->setLabel( COL_HEX, "Hex" );
	header->setLabel( COL_R, "R" );
	header->setLabel( COL_G, "G" );
	header->setLabel( COL_B, "B" );
	header->setLabel( COL_WEB, "Web" );
	m_view->addWidget( table, VIEW_LIST );
	m_view->addWidget( new QIconView( this ), VIEW_ICONS );
	QTextEdit *te = new QTextEdit( this );
	te->setReadOnly( true );
	te->setTextFormat( RichText );
	m_view->addWidget( te, VIEW_TEXT );
	setCentralWidget( m_view );

	// Connect after the widget stack exists
	connect( viewActionGroup, SIGNAL( selected(QAction*) ),
		 this, SLOT( setView(QAction*) ) );
	viewListAction->setOn( true );

	m_testText = "The quick brown fox jumped over the lazy dogs.";
	m_clip_as = CLIP_AS_HEX;
	m_show_rgb = true;
	m_show_web = true;

	if ( !m_filename.isEmpty() )
	    load( m_filename );
	else
	    init( true );

	resize( 400, 500 );

	statusBar()->message( "Ready", 2000 );
    }

    ~MainWindow() {}

private slots:

    void fileNew()
    {
	// TODO okToClear()
	init();
    }

    void fileOpen()
    {
	// TODO okToClear()
	QString filename = QFileDialog::getOpenFileName(
				QString::null, "Colors (*.txt)", this,
				"file open", "Color Tool -- File Open" );
	if ( !filename.isEmpty() )
	    load( filename );
	else
	    statusBar()->message( "File Open abandoned", 2000 );
    }

    void fileSave() {} // TODO

    void fileSaveAs() {} // TODO

    void fileQuit()
    {
	// TODO okToClear()
	qApp->exit( 0 );
    }

    void editAdd() {} // TODO

    void editCopy() {} // TODO

    void editDelete() {} // TODO

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
	options->textLineEdit->setText( m_testText );
	options->rgbCheckBox->setChecked( m_show_rgb );
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
		m_testText = text;
	    m_show_rgb = options->rgbCheckBox->isChecked();
	    m_show_web = options->webCheckBox->isChecked();

	    clear();
	    repopulate();
	}

	delete options;
    }

    void setView( QAction *action )
    {
	if ( action == viewListAction )
	    m_view->raiseWidget( VIEW_LIST );
	else if ( action == viewIconsAction )
	    m_view->raiseWidget( VIEW_ICONS );
	else if ( action == viewTextAction )
	    m_view->raiseWidget( VIEW_TEXT );
    }


private:
    void clear()
    {
	QTable *table = (QTable*)m_view->widget( VIEW_LIST );
	for ( int row = 0; row < table->numRows(); ++row )
	    for ( int col = 0; col < table->numCols(); ++col )
		table->clearCell( row, col );
	((QIconView*)m_view->widget( VIEW_ICONS ))->clear();
	((QTextEdit*)m_view->widget( VIEW_TEXT ))->clear();
    }

    void repopulate()
    {
	for ( int i = VIEW_LIST; i <= VIEW_TEXT; ++i )
	    populateView( i );
    }

    void init( bool fillWithDefaults = FALSE )
    {
	clear();
	m_colors.clear();

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
	    repopulate();
	}
    }

    void populateView( int view )
    {
	switch ( view ) {
	    case VIEW_LIST: populateListView(); break;
	    case VIEW_ICONS: populateIconsView(); break;
	    case VIEW_TEXT: populateTextView(); break;
	}
    }

    void populateListView()
    {
	QTable *table = (QTable*)m_view->widget( VIEW_LIST );
	table->setNumRows( m_colors.count() );
	QPixmap pixmap( 22, 22 );
	int r, g, b;
	int row = 0;
	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    color.rgb( &r, &g, &b );
	    table->setText( row, COL_NAME, it.key() );
	    table->setPixmap( row, COL_NAME, pixmap );
	    table->setText( row, COL_HEX, color.name().upper() );
	    if ( m_show_rgb ) {
		table->setText( row, COL_R, QString::number( r ) );
		table->setText( row, COL_G, QString::number( g ) );
		table->setText( row, COL_B, QString::number( b ) );
	    }
	    if ( m_show_web && isWebColor( r, g, b ) )
		table->setPixmap( row, COL_WEB, QPixmap( tick_xpm ) );
	    row++;
	}
	table->adjustColumn( COL_NAME );
	table->adjustColumn( COL_HEX );
	if ( m_show_rgb ) {
	    table->showColumn( COL_R );
	    table->adjustColumn( COL_R );
	    table->showColumn( COL_G );
	    table->adjustColumn( COL_G );
	    table->showColumn( COL_B );
	    table->adjustColumn( COL_B );
	}
	else {
	    table->hideColumn( COL_R );
	    table->hideColumn( COL_G );
	    table->hideColumn( COL_B );
	}
	if ( m_show_web ) {
	    table->showColumn( COL_WEB );
	    table->adjustColumn( COL_WEB );
	}
	else
	    table->hideColumn( COL_WEB );
    }

    void populateIconsView()
    {
	QIconView *iv = (QIconView*)m_view->widget( VIEW_ICONS );
	QPixmap pixmap( 80, 80 );
	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    pixmap.fill( color );
	    (void) new QIconViewItem( iv, it.key(), pixmap );
	}
    }

    void populateTextView()
    {
	QTextEdit *te = (QTextEdit*)m_view->widget( VIEW_TEXT );
	ColorMap::Iterator it;
	for ( it = m_colors.begin(); it != m_colors.end(); ++it ) {
	    QColor color = it.data();
	    te->append( QString( "<font color=%1>%2</font> %3" ).
			arg( color.name() ).arg( m_testText ).arg( it.key() ) );
	}
    }

    void load( const QString& filename )
    {
	init();
	// TODO
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

    QAction *viewListAction;
    QAction *viewIconsAction;
    QAction *viewTextAction;

    QWidgetStack *m_view;
    QString m_filename;
    ColorMap m_colors;
    QString m_testText;
    int m_clip_as;
    bool m_show_rgb;
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
