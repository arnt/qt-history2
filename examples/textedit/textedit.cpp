/****************************************************************************
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of an example program for Qt.
** EDITIONS: NOLIMITS
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "textedit.h"

#include <qaction.h>
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
#include <qprinter.h>
#include <qpaintdevicemetrics.h>
#include <qsimplerichtext.h>
#include <qcolordialog.h>
#include <qpainter.h>
#include <qlist.h>
#include <qtextcodec.h>
#include <qspinbox.h>
#include <qdebug.h>
#include <qtextdocumentfragment.h>
#include <qfiledialog.h>

#include <qtextedit.h>
#include <qtextcursor.h>
#include <qtextformat.h>
#include <qtextdocument.h>
#include <qtexttable.h>
#include "tableinsert.h"
#include <private/qtextdocumentlayout_p.h>

TextEdit::TextEdit( QWidget *parent, const char *name )
    : QMainWindow( parent, name )
{
    setupFileActions();
    setupEditActions();
    setupTableActions();
    setupTextActions();

    tabWidget = new QTabWidget( this );
    connect( tabWidget, SIGNAL( currentChanged( int ) ),
	     this, SLOT( editorChanged() ) );
    setCentralWidget( tabWidget );

    if ( qApp->argc() == 1 ) {
	load( "example.html" );
    } else {
	for ( int i = 1; i < qApp->argc(); ++i )
	    load( qApp->argv()[ i ] );
    }
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = new QToolBar( this );
    tb->setLabel( "File Actions" );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->addMenu( tr( "&File" ), menu );

    QAction *a;
    a = new QAction( QPixmap::fromMimeSource( "filenew.xpm" ), tr( "&New..." ), CTRL + Key_N, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileNew() ) );
    tb->addAction(a);
    menu->addAction(a);
    a = new QAction( QPixmap::fromMimeSource( "fileopen.xpm" ), tr( "&Open..." ), CTRL + Key_O, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileOpen() ) );
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = new QAction( QPixmap::fromMimeSource( "filesave.xpm" ), tr( "&Save..." ), CTRL + Key_S, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileSave() ) );
    tb->addAction(a);
    menu->addAction(a);
    a = new QAction( tr( "Save &As..." ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileSaveAs() ) );
    menu->addAction(a);
    menu->addSeparator();
    a = new QAction( QPixmap::fromMimeSource( "fileprint.xpm" ), tr( "&Print..." ), CTRL + Key_P, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( filePrint() ) );
    tb->addAction(a);
    menu->addAction(a);
    a = new QAction( tr( "&Close" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileClose() ) );
    menu->addAction(a);
    a = new QAction( tr( "E&xit" ), CTRL + Key_Q, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( fileExit() ) );
    menu->addAction(a);
}

void TextEdit::setupEditActions()
{
    editToolBar = new QToolBar( this );
    editToolBar->setLabel( "Edit Actions" );
    editMenu = new QPopupMenu( this );
    menuBar()->addMenu( tr( "&Edit" ), editMenu );

    QToolBar *tb = new QToolBar( this );
    tb->setLabel( "Edit Actions" );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->addMenu( tr( "&Edit" ), menu );

    QAction *a;
    a = new QAction( QPixmap::fromMimeSource( "editundo.xpm" ), tr( "&Undo" ), CTRL + Key_Z, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( editUndo() ) );
    tb->addAction(a);
    menu->addAction(a);
    a = new QAction( QPixmap::fromMimeSource( "editredo.xpm" ), tr( "&Redo" ), CTRL + Key_Y, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( editRedo() ) );
    tb->addAction(a);
    menu->addAction(a);
    menu->addSeparator();
    a = new QAction( QPixmap::fromMimeSource( "editcut.xpm" ), tr( "Cu&t" ), CTRL + Key_X, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( editCut() ) );
    tb->addAction(a);
    menu->addAction(a);
    a = new QAction( QPixmap::fromMimeSource( "editcopy.xpm" ), tr( "&Copy" ), CTRL + Key_C, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( editCopy() ) );
    tb->addAction(a);
    menu->addAction(a);
    a = new QAction( QPixmap::fromMimeSource( "editpaste.xpm" ), tr( "&Paste" ), CTRL + Key_V, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( editPaste() ) );
    tb->addAction(a);
    menu->addAction(a);
}

void TextEdit::setupTextActions()
{
    QToolBar *tb = new QToolBar( this );
    tb->setLabel( "Format Actions" );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->addMenu( tr( "F&ormat" ), menu );

    comboStyle = new QComboBox( FALSE, tb );
    comboStyle->insertItem( "Standard" );
    comboStyle->insertItem( "Bullet List (Disc)" );
    comboStyle->insertItem( "Bullet List (Circle)" );
    comboStyle->insertItem( "Bullet List (Square)" );
    comboStyle->insertItem( "Ordered List (Decimal)" );
    comboStyle->insertItem( "Ordered List (Alpha lower)" );
    comboStyle->insertItem( "Ordered List (Alpha upper)" );
    connect( comboStyle, SIGNAL( activated( int ) ),
	     this, SLOT( textStyle( int ) ) );

    comboFont = new QComboBox( TRUE, tb );
    QFontDatabase db;
    comboFont->insertStringList( db.families() );
    connect( comboFont, SIGNAL( activated( const QString & ) ),
	     this, SLOT( textFamily( const QString & ) ) );
    comboFont->lineEdit()->setText( QApplication::font().family() );

    comboSize = new QComboBox( TRUE, tb );
    QList<int> sizes = db.standardSizes();
    QList<int>::Iterator it = sizes.begin();
    for ( ; it != sizes.end(); ++it )
	comboSize->insertItem( QString::number( *it ) );
    connect( comboSize, SIGNAL( activated( const QString & ) ),
	     this, SLOT( textSize( const QString & ) ) );
    comboSize->lineEdit()->setText( QString::number( QApplication::font().pointSize() ) );

    actionTextBold = new QAction( QPixmap::fromMimeSource( "textbold.xpm" ), tr( "&Bold" ), CTRL + Key_B, this );
    connect( actionTextBold, SIGNAL( triggered() ), this, SLOT( textBold() ) );
    tb->addAction(actionTextBold);
    menu->addAction(actionTextBold);
    actionTextBold->setCheckable( TRUE );

    actionTextItalic = new QAction( QPixmap::fromMimeSource( "textitalic.xpm" ), tr( "&Italic" ), CTRL + Key_I, this );
    connect( actionTextItalic, SIGNAL( triggered() ), this, SLOT( textItalic() ) );
    tb->addAction(actionTextItalic);
    menu->addAction(actionTextItalic);
    actionTextItalic->setCheckable( TRUE );

    actionTextUnderline = new QAction( QPixmap::fromMimeSource( "textunder.xpm" ), tr( "&Underline" ), CTRL + Key_U, this );
    connect( actionTextUnderline, SIGNAL( triggered() ), this, SLOT( textUnderline() ) );
    tb->addAction(actionTextUnderline);
    menu->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable( TRUE );

    menu->addSeparator();

    QActionGroup *grp = new QActionGroup( this );
    connect( grp, SIGNAL( triggered( QAction* ) ), this, SLOT( textAlign( QAction* ) ) );

    actionAlignLeft = new QAction( QPixmap::fromMimeSource( "textleft.xpm" ), tr( "&Left" ), CTRL + Key_L, grp );
    actionAlignLeft->setCheckable( TRUE );
    actionAlignCenter = new QAction( QPixmap::fromMimeSource( "textcenter.xpm" ), tr( "C&enter" ), CTRL + Key_E, grp );
    actionAlignCenter->setCheckable( TRUE );
    actionAlignRight = new QAction( QPixmap::fromMimeSource( "textright.xpm" ), tr( "&Right" ), CTRL + Key_R, grp );
    actionAlignRight->setCheckable( TRUE );
    actionAlignJustify = new QAction( QPixmap::fromMimeSource( "textjustify.xpm" ), tr( "&Justify" ), CTRL + Key_J, grp );
    actionAlignJustify->setCheckable( TRUE );

    tb->addActions(grp->actions());
    menu->addActions(grp->actions());

    menu->addSeparator();

    QPixmap pix( 16, 16 );
    pix.fill( black );
    actionTextColor = new QAction( pix, tr( "&Color..." ), 0, this );
    connect( actionTextColor, SIGNAL( triggered() ), this, SLOT( textColor() ) );
    tb->addAction(actionTextColor);
    menu->addAction(actionTextColor);
}


void TextEdit::setupTableActions()
{
    QToolBar *tb = new QToolBar( this );
    tb->setLabel( "Table Actions" );
    QPopupMenu *menu = new QPopupMenu( this );
    menuBar()->addMenu( tr( "&Table" ), menu );

    QAction *a;

    a = new QAction( QPixmap::fromMimeSource( "table.png" ), tr( "&Insert Table" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( insertTable() ) );
    tb->addAction(a);
    menu->addAction(a);

    a = actionInsertTableRow = new QAction( QPixmap::fromMimeSource( "insert_table_row.png" ), tr( "&Insert Row" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( insertTableRow() ) );
    tb->addAction(a);
    menu->addAction(a);

    a = actionInsertTableColumn = new QAction( QPixmap::fromMimeSource( "insert_table_col.png" ), tr( "&Insert Column" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( insertTableCol() ) );
    tb->addAction(a);
    menu->addAction(a);

    a = actionDeleteTableRow = new QAction( QPixmap::fromMimeSource( "delete_table_row.png" ), tr( "&Delete Row" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( deleteTableRow() ) );
    tb->addAction(a);
    menu->addAction(a);

    a = actionDeleteTableColumn = new QAction( QPixmap::fromMimeSource( "delete_table_col.png" ), tr( "&Delete Column" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( deleteTableCol() ) );
    tb->addAction(a);
    menu->addAction(a);

    a = new QAction( QPixmap::fromMimeSource( "foo.png" ), tr( "&Insert Frame" ), 0, this );
    connect( a, SIGNAL( triggered() ), this, SLOT( insertFrame() ) );
    tb->addAction(a);
    menu->addAction(a);
}


void TextEdit::load( const QString &f )
{
    if ( !QFile::exists( f ) )
	return;
    QTextEdit *edit = createNewEditor(QFileInfo(f).fileName());
    QFile file( f );
    if ( !file.open( IO_ReadOnly ) )
	return;

    QByteArray data;
    data.resize(file.size());
    file.readBlock(data.data(), file.size());

    edit->setHtml(data);

    filenames.insert( edit, f );
}

QTextEdit *TextEdit::currentEditor() const
{
    return qt_cast<QTextEdit *>(tabWidget->currentWidget());
}

void TextEdit::fileNew()
{
    createNewEditor();
}

void TextEdit::fileOpen()
{
    QString fn = QFileDialog::getOpenFileName( QString::null, tr( "HTML-Files (*.htm *.html);;All Files (*)" ), this );
    if ( !fn.isEmpty() )
	load( fn );
}

void TextEdit::fileSave()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    QString fn;
    if ( filenames.find( edit ) == filenames.end() ) {
	fileSaveAs();
    } else {
	QFile file( *filenames.find( edit ) );
	if ( !file.open( IO_WriteOnly ) )
	    return;
	QTextStream ts( &file );
	ts << edit->document()->plainText();
    }
}

void TextEdit::fileSaveAs()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    QString fn = QFileDialog::getSaveFileName( QString::null, tr( "HTML-Files (*.htm *.html);;All Files (*)" ), this );
    if ( !fn.isEmpty() ) {
	filenames.insert( edit, fn );
	fileSave();
	tabWidget->setTabText( tabWidget->indexOf(edit), QFileInfo( fn ).fileName() );
    }
}

void TextEdit::filePrint()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
#ifndef QT_NO_PRINTER
    QPrinter printer( QPrinter::HighResolution );
    printer.setFullPage(TRUE);
    if ( printer.setup( this ) ) {
	QPainter p( &printer );
	// Check that there is a valid device to print to.
	if ( !p.device() ) return;
	QPaintDeviceMetrics metrics( p.device() );
	int dpiy = metrics.logicalDpiY();
	int margin = (int) ( (2/2.54)*dpiy ); // 2 cm margins
	QRect body( margin, margin, metrics.width() - 2*margin, metrics.height() - 2*margin );
	QFont font( edit->QWidget::font() );
 	font.setPointSize( 10 ); // we define 10pt to be a nice base size for printing

        QTextDocument doc;
        QTextCursor(&doc).insertFragment(QTextDocumentFragment(edit->document()));
        // ###
        QTextDocumentLayout *layout = qt_cast<QTextDocumentLayout *>(doc.documentLayout());
        layout->setPageSize(QSize(body.width(), INT_MAX));

  	QRect view( body );
	int page = 1;
        p.translate(body.left(), body.top());
	do {
            QAbstractTextDocumentLayout::PaintContext ctx;
            ctx.palette = palette();
            p.setClipRect(view);
	    layout->draw( &p, ctx);
	    view.moveBy( 0, body.height() );
	    p.translate( 0 , -body.height() );
	    p.setFont( font );
	    p.drawText( view.right() - p.fontMetrics().width( QString::number( page ) ),
			view.bottom() + p.fontMetrics().ascent() + 5, QString::number( page ) );
	    if ( view.top()  >= layout->totalHeight() )
		break;
	    printer.newPage();
	    page++;
	} while (TRUE);

        /*
	QSimpleRichText richText( edit->plainText(), font,
				  edit->context(),
				  edit->styleSheet(),
				  edit->mimeSourceFactory(),
				  body.height() );
	richText.setWidth( &p, body.width() );
  	QRect view( body );
	int page = 1;
	do {
	    richText.draw( &p, body.left(), body.top(), view, palette() );
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
        */
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
    QTextEdit *edit = currentEditor();
    if ( !edit )
       return;
    edit->undo();
}

void TextEdit::editRedo()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
       return;
    edit->redo();
}

void TextEdit::editCut()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
       return;
    edit->cut();
}

void TextEdit::editCopy()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
       return;
    edit->copy();
}

void TextEdit::editPaste()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
       return;
    edit->paste();
}

void TextEdit::insertTable()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;

    TableInsert *ti = new TableInsert(this);
    if (ti->exec() == QDialog::Accepted) {
	QTextCursor c = edit->cursor();

	QTextTableFormat fmt;
	c.insertTable(ti->rowBox->value(), ti->colBox->value(), fmt);
	edit->setCursor(c);
    }
}

void TextEdit::insertFrame()
{
    QTextCursor c = currentEditor()->cursor();
    QTextFrameFormat fmt;
    fmt.setMargin(20);
    fmt.setBorder(5);
    fmt.setPadding(5);
    fmt.setPosition(QTextFrameFormat::FloatLeft);
    fmt.setWidth(200);
    fmt.setHeight(200);
    c.insertFrame(fmt);
}

void TextEdit::insertTableRow()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;

    QTextCursor c = edit->cursor();
    QTextTable *t = c.currentTable();
    if (!t)
	return;
    QTextTableCell cell = t->cellAt(c);
    Q_ASSERT(cell.isValid());
    t->insertRows(cell.row(), 1);
}

void TextEdit::insertTableCol()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;

    QTextCursor c = edit->cursor();
    QTextTable *t = c.currentTable();
    if (!t)
	return;
    QTextTableCell cell = t->cellAt(c);
    Q_ASSERT(cell.isValid());
    t->insertColumns(cell.column(), 1);
}

void TextEdit::deleteTableRow()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;

    QTextCursor c = edit->cursor();
    QTextTable *t = c.currentTable();
    if (!t)
	return;
    QTextTableCell cell = t->cellAt(c);
    Q_ASSERT(cell.isValid());
    t->removeRows(cell.row(), 1);
}

void TextEdit::deleteTableCol()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;

    QTextCursor c = edit->cursor();
    QTextTable *t = c.currentTable();
    if (!t)
	return;
    QTextTableCell cell = t->cellAt(c);
    Q_ASSERT(cell.isValid());
    t->removeColumns(cell.column(), 1);
}


void TextEdit::textBold()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    edit->setFontWeight( actionTextBold->isChecked() ? QFont::Bold : QFont::Normal );
}

void TextEdit::textUnderline()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    edit->setFontUnderline( actionTextUnderline->isChecked() );
}

void TextEdit::textItalic()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    edit->setFontItalic( actionTextItalic->isChecked() );
}

void TextEdit::textFamily( const QString &f )
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    edit->setFontFamily( f );
    edit->viewport()->setFocus();
}

void TextEdit::textSize( const QString &p )
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    edit->setFontPointSize( p.toFloat() );
    edit->viewport()->setFocus();
}

void TextEdit::textStyle( int i )
{
    Q_UNUSED(i);

    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;

    QTextCursor cursor = edit->cursor();

    if ( i != 0 ) {
	QTextListFormat::Style style = QTextListFormat::ListDisc;

	if ( i == 1 )
	    style = QTextListFormat::ListDisc;
	else if ( i == 2 )
	    style = QTextListFormat::ListCircle;
	else if ( i == 3 )
	    style = QTextListFormat::ListSquare;
	else if ( i == 4 )
	    style = QTextListFormat::ListDecimal;
	else if ( i == 5 )
	    style = QTextListFormat::ListLowerAlpha;
	else if ( i == 6 )
	    style = QTextListFormat::ListUpperAlpha;

        cursor.beginEditBlock();

        QTextBlockFormat blockFmt = cursor.blockFormat();

	QTextListFormat listFmt;
	listFmt.setStyle(style);
	listFmt.setIndent(blockFmt.indent() + 1);

        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);

	cursor.createList(listFmt);

        cursor.endEditBlock();
    } else {
	QTextBlockFormat bfmt;
	bfmt.setObjectIndex(-1);
	cursor.mergeBlockFormat(bfmt);
    }

    edit->viewport()->setFocus();
}

void TextEdit::textColor()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    QColor col = QColorDialog::getColor( edit->color(), this );
    if ( !col.isValid() )
	return;
    edit->setColor( col );
    QPixmap pix( 16, 16 );
    pix.fill( black );
    actionTextColor->setIcon( pix );
}

void TextEdit::textAlign( QAction *a )
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
	return;
    if ( a == actionAlignLeft )
	edit->setAlignment( AlignLeft );
    else if ( a == actionAlignCenter )
	edit->setAlignment( AlignHCenter );
    else if ( a == actionAlignRight )
	edit->setAlignment( AlignRight );
    else if ( a == actionAlignJustify )
	edit->setAlignment( AlignJustify );
}

void TextEdit::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.color());

    QTextCursor cursor = currentEditor()->cursor();
    alignmentChanged(cursor.blockFormat().alignment());

    const bool isInTable = (cursor.currentTable());
    actionInsertTableRow->setEnabled(isInTable);
    actionInsertTableColumn->setEnabled(isInTable);
    actionDeleteTableRow->setEnabled(isInTable);
    actionDeleteTableColumn->setEnabled(isInTable);
}

void TextEdit::fontChanged( const QFont &f )
{
    comboFont->lineEdit()->setText( f.family() );
    comboSize->lineEdit()->setText( QString::number( f.pointSize() ) );
    actionTextBold->setChecked( f.bold() );
    actionTextItalic->setChecked( f.italic() );
    actionTextUnderline->setChecked( f.underline() );
}

void TextEdit::colorChanged( const QColor &c )
{
    QPixmap pix( 16, 16 );
    pix.fill( c );
    actionTextColor->setIcon( pix );
}

void TextEdit::alignmentChanged( Qt::Alignment a )
{
    if ( ( a == AlignAuto ) || ( a & AlignLeft ))
	actionAlignLeft->setChecked( TRUE );
    else if ( ( a & AlignHCenter ) )
	actionAlignCenter->setChecked( TRUE );
    else if ( ( a & AlignRight ) )
	actionAlignRight->setChecked( TRUE );
    else if ( ( a & AlignJustify ) )
	actionAlignJustify->setChecked( TRUE );
}

void TextEdit::editorChanged()
{
    QTextEdit *edit = currentEditor();
    if ( !edit )
        return;
    fontChanged( edit->font() );
    colorChanged( edit->color() );
    alignmentChanged( edit->alignment() );
}

QTextEdit *TextEdit::createNewEditor(const QString &title)
{
    QTextEdit *edit = new QTextEdit( tabWidget );
    //edit->setTextFormat( RichText );
    connect( edit, SIGNAL( currentCharFormatChanged( const QTextCharFormat & ) ),
	     this, SLOT( currentCharFormatChanged( const QTextCharFormat & ) ) );

    if (!title.isEmpty())
        tabWidget->addTab(edit, title);
    else
        tabWidget->addTab(edit, tr("noname"));

    tabWidget->showPage( edit );
    edit->viewport()->setFocus();

    return edit;
}

