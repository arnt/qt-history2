/****************************************************************************
** Form implementation generated from reading ui file '/home/mark/p4/qt/tools/designer/manual/sgml/eg/richedit/richedit.ui'
**
** Created: Thu Feb 8 13:07:58 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "/home/mark/p4/qt/tools/designer/manual/sgml/eg/richedit/richedit.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qcombobox.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qaction.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qimage.h>
#include <qpixmap.h>

static QPixmap uic_load_pixmap_EditorForm( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );
    if ( !m )
	return QPixmap();
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}
/* 
 *  Constructs a EditorForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 */
EditorForm::EditorForm( QWidget* parent,  const char* name, WFlags fl )
    : QMainWindow( parent, name, fl )
{
    (void)statusBar();
    if ( !name )
	setName( "EditorForm" );
    resize( 674, 478 ); 
    setCaption( tr( "Rich Edit" ) );
    setCentralWidget( new QWidget( this, "qt_central_widget" ) );
    EditorFormLayout = new QHBoxLayout( centralWidget() ); 
    EditorFormLayout->setSpacing( 6 );
    EditorFormLayout->setMargin( 11 );

    textEdit = new QTextEdit( centralWidget(), "textEdit" );
    textEdit->setTextFormat( QTextEdit::RichText );
    EditorFormLayout->addWidget( textEdit );

    fileNewAction = new QAction( this, "fileNewAction" );
    fileNewAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "filenew" ) ) );
    fileNewAction->setText( tr( "New" ) );
    fileNewAction->setMenuText( tr( "&New" ) );
    fileNewAction->setAccel( 4194382 );
    fileOpenAction = new QAction( this, "fileOpenAction" );
    fileOpenAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "fileopen" ) ) );
    fileOpenAction->setText( tr( "Open" ) );
    fileOpenAction->setMenuText( tr( "&Open..." ) );
    fileOpenAction->setAccel( 4194383 );
    fileSaveAction = new QAction( this, "fileSaveAction" );
    fileSaveAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "filesave" ) ) );
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
    editUndoAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "undo" ) ) );
    editUndoAction->setText( tr( "Undo" ) );
    editUndoAction->setMenuText( tr( "&Undo" ) );
    editUndoAction->setAccel( 4194394 );
    editRedoAction = new QAction( this, "editRedoAction" );
    editRedoAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "redo" ) ) );
    editRedoAction->setText( tr( "Redo" ) );
    editRedoAction->setMenuText( tr( "&Redo" ) );
    editRedoAction->setAccel( 4194393 );
    editCutAction = new QAction( this, "editCutAction" );
    editCutAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "editcut" ) ) );
    editCutAction->setText( tr( "Cut" ) );
    editCutAction->setMenuText( tr( "&Cut" ) );
    editCutAction->setAccel( 4194392 );
    editCopyAction = new QAction( this, "editCopyAction" );
    editCopyAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "editcopy" ) ) );
    editCopyAction->setText( tr( "Copy" ) );
    editCopyAction->setMenuText( tr( "C&opy" ) );
    editCopyAction->setAccel( 4194371 );
    editPasteAction = new QAction( this, "editPasteAction" );
    editPasteAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "editpaste" ) ) );
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
    boldAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "textbold" ) ) );
    boldAction->setText( tr( "bold" ) );
    boldAction->setMenuText( tr( "&Bold" ) );
    italicAction = new QAction( this, "italicAction" );
    italicAction->setToggleAction( TRUE );
    italicAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "textitalic" ) ) );
    italicAction->setText( tr( "italic" ) );
    italicAction->setMenuText( tr( "&Italic" ) );
    italicAction->setAccel( 272629833 );
    underlineAction = new QAction( this, "underlineAction" );
    underlineAction->setToggleAction( TRUE );
    underlineAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "textunder" ) ) );
    underlineAction->setText( tr( "underline" ) );
    underlineAction->setMenuText( tr( "&Underline" ) );
    underlineAction->setAccel( 272629845 );
    alignActionGroup = new QActionGroup( this, "alignActionGroup" );
    alignActionGroup->setText( tr( "align" ) );
    alignActionGroup->setUsesDropDown( FALSE );
    leftAlignAction = new QAction( alignActionGroup, "leftAlignAction" );
    leftAlignAction->setToggleAction( TRUE );
    leftAlignAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "textleft" ) ) );
    leftAlignAction->setText( tr( "left" ) );
    leftAlignAction->setMenuText( tr( "&Left" ) );
    leftAlignAction->setAccel( 272629836 );
    rightAlignAction = new QAction( alignActionGroup, "rightAlignAction" );
    rightAlignAction->setToggleAction( TRUE );
    rightAlignAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "textright" ) ) );
    rightAlignAction->setText( tr( "right" ) );
    rightAlignAction->setMenuText( tr( "&Right" ) );
    rightAlignAction->setAccel( 272629842 );
    centerAlignAction = new QAction( alignActionGroup, "centerAlignAction" );
    centerAlignAction->setToggleAction( TRUE );
    centerAlignAction->setIconSet( QIconSet( uic_load_pixmap_EditorForm( "textcenter" ) ) );
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

