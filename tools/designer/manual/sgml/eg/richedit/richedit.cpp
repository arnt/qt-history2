/****************************************************************************
** Form implementation generated from reading ui file 'richedit.ui'
**
** Created: Mon Feb 5 16:02:34 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "./richedit.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

static const char* const image0_data[] = { 
"22 22 7 1",
". c None",
"# c #000000",
"b c #2e2e2e",
"c c #5c5c5c",
"d c #878787",
"e c #c2c2c2",
"a c #ffffff",
"......................",
"....##########........",
"....#aaaaaaa#b#.......",
"....#aaaaaaa#cb#......",
"....#aaaaaaa#dcb#.....",
"....#aaaaaaa#edcb#....",
"....#aaaaaaa#aedcb#...",
"....#aaaaaaa#######...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....#aaaaaaaaaaaaa#...",
"....###############...",
"......................",
"......................"};

static const char* const image1_data[] = { 
"22 22 5 1",
". c None",
"# c #000000",
"c c #848200",
"a c #ffff00",
"b c #ffffff",
"......................",
"......................",
"......................",
"............####....#.",
"...........#....##.##.",
"..................###.",
".................####.",
".####...........#####.",
"#abab##########.......",
"#babababababab#.......",
"#ababababababa#.......",
"#babababababab#.......",
"#ababab###############",
"#babab##cccccccccccc##",
"#abab##cccccccccccc##.",
"#bab##cccccccccccc##..",
"#ab##cccccccccccc##...",
"#b##cccccccccccc##....",
"###cccccccccccc##.....",
"##cccccccccccc##......",
"###############.......",
"......................"};

static const char* const image2_data[] = { 
"22 22 5 1",
". c None",
"# c #000000",
"a c #848200",
"b c #c1c1c1",
"c c #cab5d1",
"......................",
".####################.",
".#aa#bbbbbbbbbbbb#bb#.",
".#aa#bbbbbbbbbbbb#bb#.",
".#aa#bbbbbbbbbcbb####.",
".#aa#bbbccbbbbbbb#aa#.",
".#aa#bbbccbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aa#bbbbbbbbbbbb#aa#.",
".#aaa############aaa#.",
".#aaaaaaaaaaaaaaaaaa#.",
".#aaaaaaaaaaaaaaaaaa#.",
".#aaa#############aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
".#aaa#########bbb#aa#.",
"..##################..",
"......................"};

static const char* const image3_data[] = { 
"22 22 3 1",
". c None",
"# c #000084",
"a c #848284",
"......................",
"......................",
"......................",
"......................",
"......................",
"...........######a....",
"..#......##########...",
"..##...####......##a..",
"..###.###.........##..",
"..######..........##..",
"..#####...........##..",
"..######..........##..",
"..#######.........##..",
"..########.......##a..",
"...............a###...",
"...............###....",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................"};

static const char* const image4_data[] = { 
"22 22 3 1",
". c None",
"a c #000084",
"# c #848284",
"......................",
"......................",
"......................",
"......................",
"......................",
"....#aaaaaa...........",
"...aaaaaaaaaa......a..",
"..#aa......aaaa...aa..",
"..aa.........aaa.aaa..",
"..aa..........aaaaaa..",
"..aa...........aaaaa..",
"..aa..........aaaaaa..",
"..aa.........aaaaaaa..",
"..#aa.......aaaaaaaa..",
"...aaa#...............",
"....aaa...............",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................"};

static const char* const image5_data[] = { 
"22 22 3 1",
". c None",
"# c #000000",
"a c #000082",
"......................",
".......#.....#........",
".......#.....#........",
".......#.....#........",
".......#....##........",
".......##...#.........",
"........#...#.........",
"........##.##.........",
".........###..........",
".........###..........",
"..........#...........",
".........a#a..........",
"........aa.aaa........",
".......a.a.a..a.......",
"......a..a.a...a......",
".....a...a.a....a.....",
"....a....a.a....a.....",
"....a....a..a...a.....",
"....a....a..a..a......",
"....a...a....aa.......",
".....aaa..............",
"......................"};

static const char* const image6_data[] = { 
"22 22 6 1",
". c None",
"# c #000000",
"b c #000082",
"c c #3c3cfd",
"d c #8b8bfd",
"a c #ffffff",
"......................",
"......................",
"########..............",
"#aaaaaa##.............",
"#a####a#a#............",
"#aaaaaa#aa#...........",
"#a####a#bbbbbbbb......",
"#aaaaaa#baaaaaabb.....",
"#a#####aba####abcb....",
"#aaaaaaabaaaaaabdcb...",
"#a#####aba####abadcb..",
"#aaaaaaabaaaaaabbbbbb.",
"#a#####aba####aaaaaab.",
"#aaaaaaabaaaaaaaaaaab.",
"#a#####aba#########ab.",
"#aaaaaaabaaaaaaaaaaab.",
"########ba#########ab.",
"........baaaaaaaaaaab.",
"........ba#########ab.",
"........baaaaaaaaaaab.",
"........bbbbbbbbbbbbb.",
"......................"};

static const char* const image7_data[] = { 
"22 22 8 1",
". c None",
"# c #000000",
"e c #000084",
"c c #848200",
"b c #848284",
"d c #c6c3c6",
"a c #ffff00",
"f c #ffffff",
"......................",
".......#####..........",
"..######aaa######.....",
".######aaaaa######....",
"##bcb##a###a##bcb##...",
"#bcb#ddddddddd#bcb#...",
"#cbc#ddddddddd#cbc#...",
"#bcb###########bcb#...",
"#cbcbcbcbcbcbcbcbc#...",
"#bcbcbcbcbcbcbcbcb#...",
"#cbcbcbceeeeeeeeee#...",
"#bcbcbcbefffffffefe...",
"#cbcbcbcefeeeeefeffe..",
"#bcbcbcbefffffffefffe.",
"#cbcbcbcefeeeeefeffffe",
"#bcbcbcbefffffffeeeeee",
"#cbcbcbcefeeeeeffffffe",
"#bcbcbcbeffffffffffffe",
"#cbcbcbcefeeeeeeeeeefe",
".#######effffffffffffe",
"........eeeeeeeeeeeeee",
"......................"};

static const char* const image8_data[] = { 
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"......................",
"......................",
".....#########........",
"......###...###.......",
"......###....###......",
"......###....###......",
"......###....###......",
"......###...###.......",
"......########........",
"......###...####......",
"......###....####.....",
"......###.....###.....",
"......###.....###.....",
"......###.....###.....",
"......###....###......",
".....##########.......",
"......................",
"......................",
"......................",
"......................"};

static const char* const image9_data[] = { 
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"......................",
"......................",
"..........#####.......",
"...........###........",
"...........###........",
"...........###........",
"...........###........",
"..........###.........",
"..........###.........",
"..........###.........",
"..........###.........",
".........###..........",
".........###..........",
".........###..........",
".........###..........",
"........#####.........",
"......................",
"......................",
"......................",
"......................"};

static const char* const image10_data[] = { 
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"......................",
"......................",
".....#####...####.....",
"......###......#......",
"......###......#......",
"......###......#......",
"......###......#......",
"......###......#......",
"......###......#......",
"......###......#......",
"......###......#......",
"......###......#......",
"......###.....##......",
".......###...##.......",
"........######........",
"......................",
".....############.....",
"......................",
"......................",
"......................"};

static const char* const image11_data[] = { 
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"..#################...",
"......................",
"..#############.......",
"......................",
"..#################...",
"......................",
"..#############.......",
"......................",
"..#################...",
"......................",
"..#############.......",
"......................",
"..#################...",
"......................",
"..#############.......",
"......................",
"..#################...",
"......................",
"......................",
"......................"};

static const char* const image12_data[] = { 
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"...#################..",
"......................",
".......#############..",
"......................",
"...#################..",
"......................",
".......#############..",
"......................",
"...#################..",
"......................",
".......#############..",
"......................",
"...#################..",
"......................",
".......#############..",
"......................",
"...#################..",
"......................",
"......................",
"......................"};

static const char* const image13_data[] = { 
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"...#################..",
"......................",
".....#############....",
"......................",
"...#################..",
"......................",
".....#############....",
"......................",
"...#################..",
"......................",
".....#############....",
"......................",
"...#################..",
"......................",
".....#############....",
"......................",
"...#################..",
"......................",
"......................",
"......................"};


/* 
 *  Constructs a EditorForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
EditorForm::EditorForm( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    QPixmap image0( ( const char** ) image0_data );
    QPixmap image1( ( const char** ) image1_data );
    QPixmap image2( ( const char** ) image2_data );
    QPixmap image3( ( const char** ) image3_data );
    QPixmap image4( ( const char** ) image4_data );
    QPixmap image5( ( const char** ) image5_data );
    QPixmap image6( ( const char** ) image6_data );
    QPixmap image7( ( const char** ) image7_data );
    QPixmap image8( ( const char** ) image8_data );
    QPixmap image9( ( const char** ) image9_data );
    QPixmap image10( ( const char** ) image10_data );
    QPixmap image11( ( const char** ) image11_data );
    QPixmap image12( ( const char** ) image12_data );
    QPixmap image13( ( const char** ) image13_data );
    if ( !name )
	setName( "EditorForm" );
    resize( 678, 480 ); 
    setCaption( tr( "Rich Edit" ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    EditorFormLayout = new QHBoxLayout( centralWidget() ); 
    EditorFormLayout->setSpacing( 6 );
    EditorFormLayout->setMargin( 11 );

    textEdit = new QTextEdit( centralWidget(), "textEdit" );
    textEdit->setTextFormat( QTextEdit::RichText );
    EditorFormLayout->addWidget( textEdit );

    fileNewAction = new QAction( this, "fileNewAction" );
    fileNewAction->setIconSet( QIconSet( image0 ) );
    fileNewAction->setText( tr( "New" ) );
    fileNewAction->setMenuText( tr( "&New" ) );
    fileNewAction->setAccel( 4194382 );
    fileOpenAction = new QAction( this, "fileOpenAction" );
    fileOpenAction->setIconSet( QIconSet( image1 ) );
    fileOpenAction->setText( tr( "Open" ) );
    fileOpenAction->setMenuText( tr( "&Open..." ) );
    fileOpenAction->setAccel( 4194383 );
    fileSaveAction = new QAction( this, "fileSaveAction" );
    fileSaveAction->setIconSet( QIconSet( image2 ) );
    fileSaveAction->setText( tr( "Save" ) );
    fileSaveAction->setMenuText( tr( "&Save" ) );
    fileSaveAction->setAccel( 4194387 );
    fileSaveAsAction = new QAction( this, "fileSaveAsAction" );
    fileSaveAsAction->setText( tr( "Save As" ) );
    fileSaveAsAction->setMenuText( tr( "Save &As..." ) );
    fileSaveAsAction->setAccel( 0 );
    fileExitAction = new QAction( this, "fileExitAction" );
    fileExitAction->setText( tr( "Exit" ) );
    fileExitAction->setMenuText( tr( "E&xit" ) );
    fileExitAction->setAccel( 0 );
    editUndoAction = new QAction( this, "editUndoAction" );
    editUndoAction->setIconSet( QIconSet( image3 ) );
    editUndoAction->setText( tr( "Undo" ) );
    editUndoAction->setMenuText( tr( "&Undo" ) );
    editUndoAction->setAccel( 4194394 );
    editRedoAction = new QAction( this, "editRedoAction" );
    editRedoAction->setIconSet( QIconSet( image4 ) );
    editRedoAction->setText( tr( "Redo" ) );
    editRedoAction->setMenuText( tr( "&Redo" ) );
    editRedoAction->setAccel( 4194393 );
    editCutAction = new QAction( this, "editCutAction" );
    editCutAction->setIconSet( QIconSet( image5 ) );
    editCutAction->setText( tr( "Cut" ) );
    editCutAction->setMenuText( tr( "&Cut" ) );
    editCutAction->setAccel( 4194392 );
    editCopyAction = new QAction( this, "editCopyAction" );
    editCopyAction->setIconSet( QIconSet( image6 ) );
    editCopyAction->setText( tr( "Copy" ) );
    editCopyAction->setMenuText( tr( "C&opy" ) );
    editCopyAction->setAccel( 4194371 );
    editPasteAction = new QAction( this, "editPasteAction" );
    editPasteAction->setIconSet( QIconSet( image7 ) );
    editPasteAction->setText( tr( "Paste" ) );
    editPasteAction->setMenuText( tr( "&Paste" ) );
    editPasteAction->setAccel( 4194390 );
    helpContentsAction = new QAction( this, "helpContentsAction" );
    helpContentsAction->setText( tr( "Contents" ) );
    helpContentsAction->setMenuText( tr( "&Contents..." ) );
    helpContentsAction->setAccel( 0 );
    helpIndexAction = new QAction( this, "helpIndexAction" );
    helpIndexAction->setText( tr( "Index" ) );
    helpIndexAction->setMenuText( tr( "&Index..." ) );
    helpIndexAction->setAccel( 0 );
    helpAboutAction = new QAction( this, "helpAboutAction" );
    helpAboutAction->setText( tr( "About" ) );
    helpAboutAction->setMenuText( tr( "&About..." ) );
    helpAboutAction->setAccel( 0 );
    boldAction = new QAction( this, "boldAction" );
    boldAction->setToggleAction( TRUE );
    boldAction->setIconSet( QIconSet( image8 ) );
    boldAction->setText( tr( "bold" ) );
    boldAction->setMenuText( tr( "&Bold" ) );
    italicAction = new QAction( this, "italicAction" );
    italicAction->setToggleAction( TRUE );
    italicAction->setIconSet( QIconSet( image9 ) );
    italicAction->setText( tr( "italic" ) );
    italicAction->setMenuText( tr( "&Italic" ) );
    italicAction->setAccel( 272629833 );
    underlineAction = new QAction( this, "underlineAction" );
    underlineAction->setToggleAction( TRUE );
    underlineAction->setIconSet( QIconSet( image10 ) );
    underlineAction->setText( tr( "underline" ) );
    underlineAction->setMenuText( tr( "&Underline" ) );
    underlineAction->setAccel( 272629845 );
    alignActionGroup = new QActionGroup( this, "alignActionGroup" );
    alignActionGroup->setText( tr( "align" ) );
    alignActionGroup->setUsesDropDown( FALSE );
    leftAlignAction = new QAction( alignActionGroup, "leftAlignAction" );
    leftAlignAction->setToggleAction( TRUE );
    leftAlignAction->setIconSet( QIconSet( image11 ) );
    leftAlignAction->setText( tr( "left" ) );
    leftAlignAction->setMenuText( tr( "&Left" ) );
    leftAlignAction->setAccel( 272629836 );
    rightAlignAction = new QAction( alignActionGroup, "rightAlignAction" );
    rightAlignAction->setToggleAction( TRUE );
    rightAlignAction->setIconSet( QIconSet( image12 ) );
    rightAlignAction->setText( tr( "right" ) );
    rightAlignAction->setMenuText( tr( "&Right" ) );
    rightAlignAction->setAccel( 272629842 );
    centerAlignAction = new QAction( alignActionGroup, "centerAlignAction" );
    centerAlignAction->setToggleAction( TRUE );
    centerAlignAction->setIconSet( QIconSet( image13 ) );
    centerAlignAction->setText( tr( "center" ) );
    centerAlignAction->setMenuText( tr( "&Center" ) );

    toolBar = new QToolBar( "Tools", this, Top ); 
    fileNewAction->addTo( toolBar );
    fileOpenAction->addTo( toolBar );
    fileSaveAction->addTo( toolBar );
    toolBar->addSeparator();
    editUndoAction->addTo( toolBar );
    editRedoAction->addTo( toolBar );
    editCutAction->addTo( toolBar );
    editCopyAction->addTo( toolBar );
    editPasteAction->addTo( toolBar );
    Toolbar = new QToolBar( "Toolbar", this, Top ); 
    leftAlignAction->addTo( Toolbar );
    rightAlignAction->addTo( Toolbar );
    centerAlignAction->addTo( Toolbar );
    Toolbar->addSeparator();
    boldAction->addTo( Toolbar );
    italicAction->addTo( Toolbar );
    underlineAction->addTo( Toolbar );
    Toolbar->addSeparator();

    fontComboBox = new QComboBox( FALSE, Toolbar, "fontComboBox" );

    SpinBox2 = new QSpinBox( Toolbar, "SpinBox2" );
    SpinBox2->setMinValue( 6 );
    SpinBox2->setValue( 10 );

    fileMenu = new QPopupMenu( this ); 
    fileNewAction->addTo( fileMenu );
    fileOpenAction->addTo( fileMenu );
    fileSaveAction->addTo( fileMenu );
    fileSaveAsAction->addTo( fileMenu );
    fileMenu->insertSeparator();
    fileExitAction->addTo( fileMenu );
    menuBar()->insertItem( tr("&File"), fileMenu );

    editMenu = new QPopupMenu( this ); 
    editUndoAction->addTo( editMenu );
    editRedoAction->addTo( editMenu );
    editMenu->insertSeparator();
    editCutAction->addTo( editMenu );
    editCopyAction->addTo( editMenu );
    editPasteAction->addTo( editMenu );
    menuBar()->insertItem( tr("&Edit"), editMenu );

    PopupMenu_2 = new QPopupMenu( this ); 
    leftAlignAction->addTo( PopupMenu_2 );
    rightAlignAction->addTo( PopupMenu_2 );
    centerAlignAction->addTo( PopupMenu_2 );
    PopupMenu_2->insertSeparator();
    boldAction->addTo( PopupMenu_2 );
    italicAction->addTo( PopupMenu_2 );
    underlineAction->addTo( PopupMenu_2 );
    menuBar()->insertItem( tr("F&ormat"), PopupMenu_2 );

    helpMenu = new QPopupMenu( this ); 
    helpContentsAction->addTo( helpMenu );
    helpIndexAction->addTo( helpMenu );
    helpMenu->insertSeparator();
    helpAboutAction->addTo( helpMenu );
    menuBar()->insertItem( tr("&Help"), helpMenu );



    // signals and slots connections
    connect( fileNewAction, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    connect( fileOpenAction, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    connect( fileSaveAction, SIGNAL( activated() ), this, SLOT( fileSave() ) );
    connect( fileSaveAsAction, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
    connect( fileExitAction, SIGNAL( activated() ), this, SLOT( fileExit() ) );
    connect( helpIndexAction, SIGNAL( activated() ), this, SLOT( helpIndex() ) );
    connect( helpContentsAction, SIGNAL( activated() ), this, SLOT( helpContents() ) );
    connect( helpAboutAction, SIGNAL( activated() ), this, SLOT( helpAbout() ) );
    connect( SpinBox2, SIGNAL( valueChanged(int) ), textEdit, SLOT( setPointSize(int) ) );
    connect( editCutAction, SIGNAL( activated() ), textEdit, SLOT( cut() ) );
    connect( editPasteAction, SIGNAL( activated() ), textEdit, SLOT( paste() ) );
    connect( editCopyAction, SIGNAL( activated() ), textEdit, SLOT( copy() ) );
    connect( editRedoAction, SIGNAL( activated() ), textEdit, SLOT( redo() ) );
    connect( editUndoAction, SIGNAL( activated() ), textEdit, SLOT( undo() ) );
    connect( alignActionGroup, SIGNAL( selected(QAction*) ), this, SLOT( changeAlignment(QAction*) ) );
    connect( underlineAction, SIGNAL( toggled(bool) ), textEdit, SLOT( setUnderline(bool) ) );
    connect( italicAction, SIGNAL( toggled(bool) ), textEdit, SLOT( setItalic(bool) ) );
    connect( boldAction, SIGNAL( toggled(bool) ), textEdit, SLOT( setBold(bool) ) );
    connect( fontComboBox, SIGNAL( activated(const QString&) ), textEdit, SLOT( setFamily(const QString&) ) );
    connect( fontComboBox, SIGNAL( activated(const QString&) ), textEdit, SLOT( setFocus() ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
EditorForm::~EditorForm()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

void EditorForm::fileNew()
{
    int continueAction;
    saveAndContinue( continueAction, "New" );
    if ( continueAction )
    	textEdit->clear();
}

void EditorForm::fileOpen()
{
    int continueAction;
    saveAndContinue( continueAction, "Open" );
    if ( continueAction ) {
    	QString fileName( QFileDialog::getOpenFileName( 
				QString::null, 
				"Rich Text Files (*.rtx)", this ) );   
    	if ( ! fileName.isEmpty() )
		textEdit->load( fileName );
    }
}

void EditorForm::fileSave()
{
    QString fileName = textEdit->fileName();
    if ( fileName.isEmpty() )
	fileSaveAs();
    else
	textEdit->save( fileName );
}

void EditorForm::fileSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName( 
    				"", "Rich Text Files (*.rtx)", this );
    if ( ! fileName.isEmpty() )
    	textEdit->save( fileName );
}

void EditorForm::helpAbout()
{
    qWarning( "EditorForm::helpAbout(): Not implemented yet!" );
}

void EditorForm::helpContents()
{
    qWarning( "EditorForm::helpContents(): Not implemented yet!" );
}

void EditorForm::helpIndex()
{
    qWarning( "EditorForm::helpIndex(): Not implemented yet!" );
}

void EditorForm::fileExit()
{
    int continueAction;
    saveAndContinue( continueAction, "Exit" );
    if ( continueAction )
	qApp->exit();
}

void EditorForm::changeAlignment( QAction *align )
{
    if ( align == leftAlignAction ) 
	textEdit->setAlignment( Qt::AlignLeft );
    else if ( align == rightAlignAction )
	textEdit->setAlignment( Qt::AlignRight );
    else if ( align == centerAlignAction )
	textEdit->setAlignment( Qt::AlignCenter );
}

void EditorForm::saveAndContinue( int &continueAction,const QString &action )
{
    continueAction = 1;
    
    if ( textEdit->isModified() ) {  
	switch( QMessageBox::information(     
		this, "Rich Edit",   
		"The document contains unsaved changes.\n"   
		"Do you want to save the changes?",   
		"&Save", "&Don't Save", "&Cancel " + action,   
		0, // Enter == button 0   
		2 ) ) { // Escape == button 2   
	case 0: // Save; continue  
     		fileSave();  
        	break;   
    	case 1: // Don't save; continue
        	break;   
    	case 2: // Cancel
		continueAction = 0;
        	break;  
    	}	  
    }  
}

void EditorForm::init()
{
    textEdit->setFocus();  
 
    QFontDatabase fonts;
    fontComboBox->insertStringList( fonts.families() );
    QString font = textEdit->family();
    font = font.lower();
    for ( int i = 0 ; i < fontComboBox->count(); i++ ) {
	if ( font == fontComboBox->text( i ) ) {
	    fontComboBox->setCurrentItem( i );
	    break;
	}
    }
}

void EditorForm::destroy()
{
}

