/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "textedit.h"

#include <q3textedit.h>
#include <q3action.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qtabwidget.h>
#include <qapplication.h>
#include <qfontdatabase.h>
#include <qcombobox.h>
#include <qlineedit.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qfiledialog.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qvaluelist.h>
using namespace Qt;

TextEdit::TextEdit( QWidget *parent, const char *name )
    : Q3MainWindow( parent, name, 0 )
{
    setupFileActions();
    setupEditActions();
    setupTextActions();

    tabWidget = new QTabWidget( this );
    connect( tabWidget, SIGNAL( currentChanged( QWidget * ) ),
	     this, SLOT( editorChanged( QWidget * ) ) );
    setCentralWidget( tabWidget );
}

void TextEdit::setupFileActions()
{
    Q3ToolBar *tb = new Q3ToolBar( this );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&File" ), menu );

    Q3Action *a;
    a = new Q3Action( tr( "New" ), QPixmap( "textdrawing/filenew.png" ), tr( "&New..." ), CTRL + Key_N, this, "fileNew" );
    connect( a, SIGNAL( activated() ), this, SLOT( fileNew() ) );
    a->addTo( tb );
    a->addTo( menu );
    a = new Q3Action( tr( "Open" ), QPixmap( "textdrawing/fileopen.png" ), tr( "&Open..." ), CTRL + Key_O, this, "fileOpen" );
    connect( a, SIGNAL( activated() ), this, SLOT( fileOpen() ) );
    a->addTo( tb );
    a->addTo( menu );
    menu->insertSeparator();
    a = new Q3Action( tr( "Save" ), QPixmap( "textdrawing/filesave.png" ), tr( "&Save..." ), CTRL + Key_S, this, "fileSave" );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSave() ) );
    a->addTo( tb );
    a->addTo( menu );
    a = new Q3Action( tr( "Save As" ), QPixmap(), tr( "Save &As..." ), 0, this, "fileSaveAs" );
    connect( a, SIGNAL( activated() ), this, SLOT( fileSaveAs() ) );
    a->addTo( menu );
    menu->insertSeparator();
    a = new Q3Action( tr( "Print" ), QPixmap( "textdrawing/print.png" ), tr( "&Print..." ), CTRL + Key_P, this, "filePrint" );
    connect( a, SIGNAL( activated() ), this, SLOT( filePrint() ) );
    a->addTo( tb );
    a->addTo( menu );
    a = new Q3Action( tr( "Close" ), QPixmap(), tr( "&Close" ), 0, this, "fileClose" );
    connect( a, SIGNAL( activated() ), this, SLOT( fileClose() ) );
    a->addTo( menu );
}

void TextEdit::setupEditActions()
{
    Q3ToolBar *tb = new Q3ToolBar( this );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->insertItem( tr( "&Edit" ), menu );

    Q3Action *a;
    a = new Q3Action( tr( "Undo" ), QPixmap( "textdrawing/undo.png" ), tr( "&Undo" ), CTRL + Key_Z, this, "editUndo" );
    connect( a, SIGNAL( activated() ), this, SLOT( editUndo() ) );
    a->addTo( tb );
    a->addTo( menu );
    a = new Q3Action( tr( "Redo" ), QPixmap( "textdrawing/redo.png" ), tr( "&Redo" ), CTRL + Key_Y, this, "editRedo" );
    connect( a, SIGNAL( activated() ), this, SLOT( editRedo() ) );
    a->addTo( tb );
    a->addTo( menu );
    menu->insertSeparator();
    a = new Q3Action( tr( "Cut" ), QPixmap( "textdrawing/editcut.png" ), tr( "&Cut" ), CTRL + Key_X, this, "editCut" );
    connect( a, SIGNAL( activated() ), this, SLOT( editCut() ) );
    a->addTo( tb );
    a->addTo( menu );
    a = new Q3Action( tr( "Copy" ), QPixmap( "textdrawing/editcopy.png" ), tr( "C&opy" ), CTRL + Key_C, this, "editCopy" );
    connect( a, SIGNAL( activated() ), this, SLOT( editCopy() ) );
    a->addTo( tb );
    a->addTo( menu );
    a = new Q3Action( tr( "Paste" ), QPixmap( "textdrawing/editpaste.png" ), tr( "&Paste" ), CTRL + Key_V, this, "editPaste" );
    connect( a, SIGNAL( activated() ), this, SLOT( editPaste() ) );
    a->addTo( tb );
    a->addTo( menu );
}

void TextEdit::setupTextActions()
{
    Q3ToolBar *tb = new Q3ToolBar( this );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->insertItem( tr( "For&mat" ), menu );

    comboStyle = new QComboBox( FALSE, tb );
    comboStyle->insertItem( tr("Standard") );
    comboStyle->insertItem( tr("Bullet List (Disc)") );
    comboStyle->insertItem( tr("Bullet List (Circle)") );
    comboStyle->insertItem( tr("Bullet List (Square)") );
    comboStyle->insertItem( tr("Ordered List (Decimal)") );
    comboStyle->insertItem( tr("Ordered List (Alpha lower)") );
    comboStyle->insertItem( tr("Ordered List (Alpha upper)") );
    connect( comboStyle, SIGNAL( activated( int ) ),
	     this, SLOT( textStyle( int ) ) );

    comboFont = new QComboBox( TRUE, tb );
    QFontDatabase db;
    comboFont->insertStringList( db.families() );
    connect( comboFont, SIGNAL( activated( const QString & ) ),
	     this, SLOT( textFamily( const QString & ) ) );
    comboFont->lineEdit()->setText( QApplication::font().family() );

    comboSize = new QComboBox( TRUE, tb );
    QValueList<int> sizes = db.standardSizes();
    QValueList<int>::Iterator it = sizes.begin();
    for ( ; it != sizes.end(); ++it )
	comboSize->insertItem( QString::number( *it ) );
    connect( comboSize, SIGNAL( activated( const QString & ) ),
	     this, SLOT( textSize( const QString & ) ) );
    comboSize->lineEdit()->setText( QString::number( QApplication::font().pointSize() ) );

    actionTextBold = new Q3Action( tr( "Bold" ), QPixmap( "textdrawing/textbold.png" ), tr( "&Bold" ), CTRL + Key_B, this, "textBold" );
    connect( actionTextBold, SIGNAL( activated() ), this, SLOT( textBold() ) );
    actionTextBold->addTo( tb );
    actionTextBold->addTo( menu );
    actionTextBold->setToggleAction( TRUE );
    actionTextItalic = new Q3Action( tr( "Italic" ), QPixmap( "textdrawing/textitalic.png" ), tr( "&Italic" ), CTRL + Key_I, this, "textItalic" );
    connect( actionTextItalic, SIGNAL( activated() ), this, SLOT( textItalic() ) );
    actionTextItalic->addTo( tb );
    actionTextItalic->addTo( menu );
    actionTextItalic->setToggleAction( TRUE );
    actionTextUnderline = new Q3Action( tr( "Underline" ), QPixmap( "textdrawing/textunderline.png" ), tr( "&Underline" ), CTRL + Key_U, this, "textUnderline" );
    connect( actionTextUnderline, SIGNAL( activated() ), this, SLOT( textUnderline() ) );
    actionTextUnderline->addTo( tb );
    actionTextUnderline->addTo( menu );
    actionTextUnderline->setToggleAction( TRUE );
    menu->insertSeparator();

    Q3ActionGroup *grp = new Q3ActionGroup( this );
    grp->setExclusive( TRUE );
    connect( grp, SIGNAL( selected( Q3Action* ) ), this, SLOT( textAlign( Q3Action* ) ) );

    actionAlignLeft = new Q3Action( tr( "Left" ), QPixmap( "textdrawing/textleft.png" ), tr( "&Left" ), CTRL + Key_L, grp, "textLeft" );
    actionAlignLeft->addTo( tb );
    actionAlignLeft->addTo( menu );
    actionAlignLeft->setToggleAction( TRUE );
    actionAlignCenter = new Q3Action( tr( "Center" ), QPixmap( "textdrawing/textcenter.png" ), tr( "C&enter" ), CTRL + Key_M, grp, "textCenter" );
    actionAlignCenter->addTo( tb );
    actionAlignCenter->addTo( menu );
    actionAlignCenter->setToggleAction( TRUE );
    actionAlignRight = new Q3Action( tr( "Right" ), QPixmap( "textdrawing/textright.png" ), tr( "&Right" ), CTRL + Key_R, grp, "textRight" );
    actionAlignRight->addTo( tb );
    actionAlignRight->addTo( menu );
    actionAlignRight->setToggleAction( TRUE );
    actionAlignJustify = new Q3Action( tr( "Justify" ), QPixmap( "textdrawing/textjustify.png" ), tr( "&Justify" ), CTRL + Key_J, grp, "textjustify" );
    actionAlignJustify->addTo( tb );
    actionAlignJustify->addTo( menu );
    actionAlignJustify->setToggleAction( TRUE );

    menu->insertSeparator();

    QPixmap pix( 16, 16 );
    pix.fill( black );
    actionTextColor = new Q3Action( tr( "Color" ), pix, tr( "&Color..." ), 0, this, "textColor" );
    connect( actionTextColor, SIGNAL( activated() ), this, SLOT( textColor() ) );
    actionTextColor->addTo( tb );
    actionTextColor->addTo( menu );
}

void TextEdit::load( const QString &f )
{
    if ( !QFile::exists( f ) )
	return;
    Q3TextEdit *edit = new Q3TextEdit( tabWidget );
    doConnections( edit );
    tabWidget->addTab( edit, QFileInfo( f ).fileName() );

    QFile fl( f );
    fl.open( IO_ReadOnly );
    QByteArray array = fl.readAll();
    array.resize( array.size() +1 );
    array[ (int)array.size() - 1 ] = '\0';
    QString text = ( f.find( "bidi.txt" ) != -1 ? QString::fromUtf8( array.data() ) : QString::fromLatin1( array.data() ) );
    edit->setText( text );

    edit->viewport()->setFocus();
    edit->setTextFormat( Qt::RichText );
}

Q3TextEdit *TextEdit::currentEditor() const
{
    if ( tabWidget->currentPage() &&
	 tabWidget->currentPage()->inherits( "Q3TextEdit" ) )
	return (Q3TextEdit*)tabWidget->currentPage();
    return 0;
}

void TextEdit::doConnections( Q3TextEdit *e )
{
    connect( e, SIGNAL( currentFontChanged( const QFont & ) ),
	     this, SLOT( fontChanged( const QFont & ) ) );
    connect( e, SIGNAL( currentColorChanged( const QColor & ) ),
	     this, SLOT( colorChanged( const QColor & ) ) );
    connect( e, SIGNAL( currentAlignmentChanged( int ) ),
	     this, SLOT( alignmentChanged( int ) ) );
}

void TextEdit::fileNew()
{
    Q3TextEdit *edit = new Q3TextEdit( tabWidget );
    doConnections( edit );
    tabWidget->addTab( edit, tr( "noname" ) );
    tabWidget->showPage( edit );
    edit->viewport()->setFocus();
}

void TextEdit::fileOpen()
{
    QString fn;// = QFileDialog::getOpenFileName( QString::null, tr( "HTML-Files (*.htm *.html);;All Files (*)" ), this );
    if ( !fn.isEmpty() )
	load( fn );
}

void TextEdit::fileSave()
{
    if ( !currentEditor() )
	return;
    QString fn;
    if ( filenames.find( currentEditor() ) == filenames.end() ) {
	fileSaveAs();
    } else {
	QFile file( *filenames.find( currentEditor() ) );
	if ( !file.open( IO_WriteOnly ) )
	    return;
	QTextStream ts( &file );
	ts << currentEditor()->text();
    }
}

void TextEdit::fileSaveAs()
{
    if ( !currentEditor() )
	return;
    QString fn;// = QFileDialog::getSaveFileName( QString::null, tr( "HTML-Files (*.htm *.html);;All Files (*)" ), this );
    if ( !fn.isEmpty() ) {
	filenames.replace( currentEditor(), fn );
	fileSave();
	tabWidget->setTabLabel( currentEditor(), QFileInfo( fn ).fileName() );
    }
}

void TextEdit::filePrint()
{
    if ( !currentEditor() )
	return;
#ifndef QT_NO_PRINTER
    QPrinter printer;
    printer.setFullPage(TRUE);
    QPaintDeviceMetrics screen( this );
    printer.setResolution( screen.logicalDpiY() );
    if ( printer.setup( this ) ) {
	QPainter p( &printer );
	QPaintDeviceMetrics metrics( p.device() );
	int dpix = metrics.logicalDpiX();
	int dpiy = metrics.logicalDpiY();
	const int margin = 72; // pt
	QRect body( margin * dpix / 72, margin * dpiy / 72,
		    metrics.width() - margin * dpix / 72 * 2,
		    metrics.height() - margin * dpiy / 72 * 2 );
	QFont font( "times", 10 );
	QSimpleRichText richText( currentEditor()->text(), font, currentEditor()->context(), currentEditor()->styleSheet(),
				  currentEditor()->mimeSourceFactory(), body.height() );
	richText.setWidth( &p, body.width() );
	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, colorGroup() );
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.setFont( font );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number( page ) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number( page ) );
	    if ( view.top()  >= richText.height() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);
    }
#endif
}

void TextEdit::fileClose()
{
    delete currentEditor();
    if ( currentEditor() )
	currentEditor()->viewport()->setFocus();
}

void TextEdit::fileExit()
{
    qApp->quit();
}

void TextEdit::editUndo()
{
    if ( !currentEditor() )
	return;
    currentEditor()->undo();
}

void TextEdit::editRedo()
{
    if ( !currentEditor() )
	return;
    currentEditor()->redo();
}

void TextEdit::editCut()
{
    if ( !currentEditor() )
	return;
    currentEditor()->cut();
}

void TextEdit::editCopy()
{
    if ( !currentEditor() )
	return;
    currentEditor()->copy();
}

void TextEdit::editPaste()
{
    if ( !currentEditor() )
	return;
    currentEditor()->paste();
}

void TextEdit::textBold()
{
    if ( !currentEditor() )
	return;
    currentEditor()->setBold( actionTextBold->isOn() );
}

void TextEdit::textUnderline()
{
    if ( !currentEditor() )
	return;
    currentEditor()->setUnderline( actionTextUnderline->isOn() );
}

void TextEdit::textItalic()
{
    if ( !currentEditor() )
	return;
    currentEditor()->setItalic( actionTextItalic->isOn() );
}

void TextEdit::textFamily( const QString &f )
{
    if ( !currentEditor() )
	return;
    currentEditor()->setFamily( f );
    currentEditor()->viewport()->setFocus();
}

void TextEdit::textSize( const QString &p )
{
    if ( !currentEditor() )
	return;
    currentEditor()->setPointSize( p.toInt() );
    currentEditor()->viewport()->setFocus();
}

void TextEdit::textStyle( int i )
{
    if ( !currentEditor() )
	return;
    if ( i == 0 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayBlock, QStyleSheetItem::ListDisc );
    else if ( i == 1 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDisc );
    else if ( i == 2 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListCircle );
    else if ( i == 3 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListSquare );
    else if ( i == 4 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDecimal );
    else if ( i == 5 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListLowerAlpha );
    else if ( i == 6 )
	currentEditor()->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListUpperAlpha );
    currentEditor()->viewport()->setFocus();
}

void TextEdit::textColor()
{
    if ( !currentEditor() )
	return;
    QColor col = QColorDialog::getColor( currentEditor()->color(), this );
    if ( !col.isValid() )
	return;
    currentEditor()->setColor( col );
    QPixmap pix( 16, 16 );
    pix.fill( col );
    actionTextColor->setIconSet( pix );
}

void TextEdit::textAlign( Q3Action *a )
{
    if ( !currentEditor() )
	return;
    if ( a == actionAlignLeft )
	currentEditor()->setAlignment( AlignLeft );
    else if ( a == actionAlignCenter )
	currentEditor()->setAlignment( AlignHCenter );
    else if ( a == actionAlignRight )
	currentEditor()->setAlignment( AlignRight );
    else if ( a == actionAlignJustify )
	currentEditor()->setAlignment( AlignJustify );
}

void TextEdit::fontChanged( const QFont &f )
{
    comboFont->lineEdit()->setText( f.family() );
    comboSize->lineEdit()->setText( QString::number( f.pointSize() ) );
    actionTextBold->setOn( f.bold() );
    actionTextItalic->setOn( f.italic() );
    actionTextUnderline->setOn( f.underline() );
}

void TextEdit::colorChanged( const QColor &c )
{
    QPixmap pix( 16, 16 );
    pix.fill( c );
    actionTextColor->setIconSet( pix );
}

void TextEdit::alignmentChanged( int a )
{
    if ( ( a == AlignAuto ) || ( a & AlignLeft ))
	actionAlignLeft->setOn( TRUE );
    else if ( ( a & AlignHCenter ) )
	actionAlignCenter->setOn( TRUE );
    else if ( ( a & AlignRight ) )
	actionAlignRight->setOn( TRUE );
    else if ( ( a & AlignJustify ) )
	actionAlignJustify->setOn( TRUE );
}

void TextEdit::editorChanged( QWidget * )
{
    if ( !currentEditor() )
	return;
    fontChanged( currentEditor()->font() );
    colorChanged( currentEditor()->color() );
    alignmentChanged( currentEditor()->alignment() );
}
