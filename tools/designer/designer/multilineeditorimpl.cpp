/**********************************************************************
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of Qt Designer.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "multilineeditorimpl.h"
#include "formwindow.h"
#include "command.h"
#include "mainwindow.h"
#include "richtextfontdialog.h"
#include "syntaxhighlighter_html.h"

#include <qmultilineedit.h>
#include <qtextedit.h>
#include <./private/qrichtext_p.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtoolbutton.h>
#include <qapplication.h>
#include <qaction.h>


static const char* const center_data[] = {
"22 22 4 1",
"a c None",
"# c None",
". c None",
"b c #000000",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.bbbbbbbbbbbbb.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.bbbbbbbbbbbbb.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.bbbbbbbbbbbbb.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.bbbbbbbbbbbbb.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a."};

static const char* const under_data[] = {
"22 22 4 1",
"a c None",
"# c None",
". c None",
"b c #000000",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.#.a.#.a.#.a.#.a.#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.bbbbb.a.bbbb#.a.#",
"a.a.a.bbb.a.a.aba.a.a.",
".a.#.abbba.#.a.b.a.#.a",
"a.a.a.bbb.a.a.aba.a.a.",
".#.a.#bbb#.a.#.b.#.a.#",
"a.a.a.bbb.a.a.aba.a.a.",
".a.#.abbba.#.a.b.a.#.a",
"a.a.a.bbb.a.a.aba.a.a.",
".#.a.#bbb#.a.#.b.#.a.#",
"a.a.a.bbb.a.a.aba.a.a.",
".a.#.abbba.#.abb.a.#.a",
"a.a.a.abbba.abb.a.a.a.",
".#.a.#.abbbbbb.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.#.bbbbbbbbbbbba.#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a."};

static const char* const right_data[] = {
"22 22 4 1",
"a c None",
"# c None",
". c None",
"b c #000000",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.bbbbbbbbbbbbb.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.bbbbbbbbbbbbb.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.bbbbbbbbbbbbb.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.bbbbbbbbbbbbb.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.bbbbbbbbbbbbbbbbb.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a."};

static const char* const bold_data[] = {
"22 22 4 1",
"a c None",
"# c None",
". c None",
"b c #000000",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.#.a.#.a.#.a.#.a.#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.bbbbbbbbb.a.#.a.#",
"a.a.a.bbb.a.bbb.a.a.a.",
".a.#.abbba.#.bbb.a.#.a",
"a.a.a.bbb.a.abbba.a.a.",
".#.a.#bbb#.a.bbb.#.a.#",
"a.a.a.bbb.a.bbb.a.a.a.",
".a.#.abbbbbbbb.#.a.#.a",
"a.a.a.bbb.a.bbbba.a.a.",
".#.a.#bbb#.a.bbbb#.a.#",
"a.a.a.bbb.a.a.bbb.a.a.",
".a.#.abbba.#.abbba.#.a",
"a.a.a.bbb.a.a.bbb.a.a.",
".#.a.#bbb#.a.bbb.#.a.#",
"a.a.abbbbbbbbbb.a.a.a.",
".a.#.a.#.a.#.a.#.a.#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a."};

static const char* const italic_data[] = {
"22 22 4 1",
"a c None",
"# c None",
". c None",
"b c #000000",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".a.#.a.#.a.#.a.#.a.#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#bbbbba.#.a.#",
"a.a.a.a.a.abbba.a.a.a.",
".a.#.a.#.a.bbb.#.a.#.a",
"a.a.a.a.a.abbba.a.a.a.",
".#.a.#.a.#.bbb.a.#.a.#",
"a.a.a.a.a.bbb.a.a.a.a.",
".a.#.a.#.abbba.#.a.#.a",
"a.a.a.a.a.bbb.a.a.a.a.",
".#.a.#.a.#bbb#.a.#.a.#",
"a.a.a.a.abbba.a.a.a.a.",
".a.#.a.#.bbb.a.#.a.#.a",
"a.a.a.a.abbba.a.a.a.a.",
".#.a.#.a.bbb.#.a.#.a.#",
"a.a.a.a.bbbbb.a.a.a.a.",
".a.#.a.#.a.#.a.#.a.#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a."};

static const char* const left_data[] = {
"22 22 4 1",
"a c None",
"# c None",
". c None",
"b c #000000",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".abbbbbbbbbbbbbbbbb#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#bbbbbbbbbbbbba.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".abbbbbbbbbbbbbbbbb#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#bbbbbbbbbbbbba.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".abbbbbbbbbbbbbbbbb#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#bbbbbbbbbbbbba.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".abbbbbbbbbbbbbbbbb#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#bbbbbbbbbbbbba.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a.",
".abbbbbbbbbbbbbbbbb#.a",
"a.a.a.a.a.a.a.a.a.a.a.",
".#.a.#.a.#.a.#.a.#.a.#",
"a.a.a.a.a.a.a.a.a.a.a."};

static const char* const block_data[] = {
"22 22 2 1",
". c None",
"# c #000000",
"......................",
"......................",
"......................",
"......................",
"....#############.....",
"......................",
"....#############.....",
"......................",
"....#############.....",
"......................",
"....#############.....",
"......................",
"....#############.....",
"......................",
"....#############.....",
"......................",
"....#############.....",
"......................",
"......................",
"......................",
"......................",
"......................"};

static const char* const fontdec_data[] = {
"22 22 4 1",
". c None",
"# c #000000",
"b c #0000c5",
"a c #838183",
"......................",
"......................",
"......................",
"......................",
"......................",
".............##.......",
"............###a......",
"...........####a......",
"............a##a......",
"...bbbbbb....##a......",
"...bbbbbba...##a......",
"....aaaaaa...##a......",
".............##a......",
"............####......",
"............####a.....",
".............aaaa.....",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................"};

static const char* const fontinc_data[] = {
"22 22 4 1",
". c None",
"# c #000000",
"a c #838183",
"b c #c50000",
"......................",
"......................",
"......................",
"......................",
"......................",
".............##.......",
"............###a......",
".....bb....####a......",
".....bb.....a##a......",
"...bbbbbb....##a......",
"...bbbbbba...##a......",
".....bbaaa...##a......",
".....bba.....##a......",
"......aa....####......",
"............####a.....",
".............aaaa.....",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................"};

static const char* const h1_data[] = {
"22 22 3 1",
". c None",
"# c #000000",
"a c #838183",
"......................",
"......................",
"......................",
"......................",
"......................",
"######..######....###.",
".a###aa..###aaa..####a",
"..###a...###a...#####a",
"..###a...###a....a###a",
"..###a...###a.....###a",
"..##########a.....###a",
"..###aaaa###a.....###a",
"..###a...###a.....###a",
"..###a...###a.....###a",
"..###a...###a.....###a",
"..###a...###a.....###a",
"######..######..######",
".aaaaaa..aaaaaa..aaaaa",
"......................",
"......................",
"......................",
"......................"};

static const char* const h2_data[] = {
"22 22 3 1",
". c None",
"# c #000000",
"a c #838183",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................",
"######.#####...####...",
".a##aaa.a##aa.######..",
"..##a....##a.##aa###a.",
"..##a....##a..aa..##a.",
"..##a....##a......##a.",
"..#########a......#aa.",
"..##aaaaa##a.....##a..",
"..##a....##a....##a...",
"..##a....##a...##a..#.",
"..##a....##a..######aa",
"######.#####.#######a.",
".aaaaaa.aaaaa.aaaaaaa.",
"......................",
"......................",
"......................",
"......................"};

static const char* const h3_data[] = {
"22 22 3 1",
". c None",
"# c #000000",
"a c #838183",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................",
"..####..####...###....",
"...##aa..##aa.#####...",
"...##a...##a.#.aa##a..",
"...##a...##a..a..#aa..",
"...########a...###a...",
"...##aaaa##a....a###..",
"...##a...##a......##a.",
"...##a...##a......##a.",
"...##a...##a.##...#aa.",
"..####..####.#####.a..",
"...aaaa..aaaa.aaaaa...",
"......................",
"......................",
"......................",
"......................"};

static const char* const break_data[] = {
"22 22 6 1",
". c None",
"d c #000000",
"b c #000083",
"# c #313031",
"c c #a4a1a4",
"a c #ffffff",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................",
"..............#####...",
"...............#a#....",
"...............#a#....",
"...............#a#....",
"...............#a#....",
".......b.......#a#....",
"......bc.......#a#....",
".....bc........#a#....",
"....bd##########a#....",
"...bdaaaaaaaaaaaa#....",
"....bd############....",
".....bc...............",
"......bc..............",
".......bc.............",
"......................",
"......................"};

static const char* const para_data[] = {
"22 22 4 1",
". c None",
"a c #313031",
"b c #a4a1a4",
"# c #c50000",
"......................",
"......................",
"......................",
"......................",
"......................",
"......................",
"......#.aaaaa..#......",
".....#.baabbaa..#.....",
"....#.b.aab.aab..#....",
"...#.b..aab.aab...#...",
"..#.b...aab.aab....#..",
".#.b....aaaaabb.....#.",
"..#.....aabbbb.....#.b",
"...#....aab.......#.b.",
"....#...aab......#.b..",
".....#..aab.....#.b...",
"......#.aab....#.b....",
".......b.bb.....b.....",
"......................",
"......................",
"......................",
"......................"};

static const char* const tt_data[] = {
"22 22 9 1",
". c None",
"d c #000000",
"f c #000008",
"g c #000400",
"e c #080008",
"c c #7b7d7b",
"# c #837d83",
"a c #838183",
"b c #83818b",
"......................",
"......................",
"......................",
"......................",
"......................",
".....##a#a#a#.........",
".....#b.b#.#a.........",
".....a..#a..c.........",
".....#..bdedddfde.....",
"........#dd.dd.dd.....",
"........ad..gd..f.....",
"........#d..dd..d.....",
"........a#..dd........",
"......#####add........",
"............dd........",
"............dd........",
"..........fdedfd......",
"......................",
"......................",
"......................",
"......................",
"......................"};

static const char* const font_data[] = {
"22 22 9 1",
". c None",
"d c #000000",
"f c #000008",
"g c #000400",
"e c #080008",
"c c #7b7d7b",
"# c #837d83",
"a c #838183",
"b c #83818b",
"......................",
"......................",
"......................",
"......................",
".....ddddddddddddda...",
".....ddddddddddddda...",
".....ddaaaaaaaaaaa....",
".....dda..............",
".....dda..............",
".....ddddddddddddda...",
".....ddddddddddddda...",
".....ddaaaaaaaaaaa....",
".....dda..............",
".....dda..............",
".....dda..............",
".....dda..............",
".....dda..............",
".....dda..............",
".....dda..............",
".....dda..............",
"......................",
"......................"};

ToolBarItem::ToolBarItem( QWidget *parent, QWidget *toolBar,
			  const QString &label, const QString &tagstr,
			  const QIconSet &icon, const QKeySequence &key )
    : QAction( parent )
{
    setIconSet( icon );
    setText( label );
    setAccel( key );
    addTo( toolBar );
    tag = tagstr;
    connect( this, SIGNAL( activated() ), this, SLOT( wasActivated() ) );
}

ToolBarItem::~ToolBarItem()
{

}

void ToolBarItem::wasActivated()
{
    emit clicked( tag );
}

TextEdit::TextEdit( QWidget *parent, const char *name )
    : QTextEdit( parent, name )
{
    setTextFormat( Qt::PlainText );
}

QTextParagraph* TextEdit::paragraph()
{
    QTextCursor *tc = new QTextCursor( QTextEdit::document() );
    return tc->paragraph();
}


MultiLineEditor::MultiLineEditor( QWidget *parent, QWidget *editWidget,
    FormWindow *fw, const QString &text )
    : MultiLineEditorBase( parent, 0,
	WType_Dialog | WShowModal | WDestructiveClose ), formwindow( fw )
{
    callStatic = FALSE;

    if ( !text.isNull() ) {
	applyButton->hide();
	callStatic = TRUE;
    }

    textEdit = new TextEdit( centralWidget(), "textedit" );
    Layout4->insertWidget( 0, textEdit );

    QPopupMenu *stylesMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "Styles" ), stylesMenu );

    basicToolBar = new QToolBar( tr( "Basics" ), this, DockTop );

    ToolBarItem *it = new ToolBarItem( this, basicToolBar, tr( "Italic" ),
		      "i", QPixmap( (const char **) italic_data ), CTRL+Key_I );
    it->addTo( stylesMenu );
    connect( it, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *b = new ToolBarItem( this, basicToolBar, tr( "Bold" ),
		      "b", QPixmap( (const char **) bold_data ), CTRL+Key_B );
    b->addTo( stylesMenu );
    connect( b, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *ul = new ToolBarItem( this, basicToolBar, tr( "Underline" ),
		      "u", QPixmap( (const char **) under_data ), CTRL+Key_U );
    ul->addTo( stylesMenu );
    connect( ul, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *tt = new ToolBarItem( this, basicToolBar, tr( "Typewriter" ),
		      "tt", QPixmap( (const char **) tt_data ) );
    tt->addTo( stylesMenu );
    connect( tt, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    basicToolBar->addSeparator();

    QPopupMenu *layoutMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "Layout" ), layoutMenu );

    QAction *brAction = new QAction( this );
    brAction->setIconSet( QPixmap( (const char **) break_data ) );
    brAction->setText( tr("Break" ) );
    brAction->addTo( basicToolBar );
    brAction->addTo( layoutMenu );
    connect( brAction, SIGNAL( activated() ) , this, SLOT( insertBR() ) );

    ToolBarItem *p = new ToolBarItem( this, basicToolBar, tr( "Paragraph" ),
		      "p", QPixmap( (const char **) para_data ) );
    p->addTo( layoutMenu );
    connect( p, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));
    layoutMenu->insertSeparator();
    basicToolBar->addSeparator();

    ToolBarItem *al = new ToolBarItem( this, basicToolBar, tr( "Align left" ),
		      "p align=\"left\"", QPixmap( (const char **) left_data ) );
    al->addTo( layoutMenu );
    connect( al, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *ac = new ToolBarItem( this, basicToolBar, tr( "Align center" ),
		      "p align=\"center\"", QPixmap( (const char **) center_data ) );
    ac->addTo( layoutMenu );
    connect( ac, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *ar = new ToolBarItem( this, basicToolBar, tr( "Align right" ),
		      "p align=\"right\"", QPixmap( (const char **) right_data ) );
    ar->addTo( layoutMenu );
    connect( ar, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *block = new ToolBarItem( this, basicToolBar, tr( "Blockquote" ),
		      "blockquote", QPixmap( (const char **) block_data ) );
    block->addTo( layoutMenu );
    connect( block, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));


    QPopupMenu *fontMenu = new QPopupMenu( this );
    menuBar->insertItem( tr( "Font" ), fontMenu );

    fontToolBar = new QToolBar( "Fonts", this, DockTop );

    QAction *fontAction = new QAction( this );
    fontAction->setIconSet( QPixmap( (const char **) font_data ) );
    fontAction->setText( tr("Font" ) );
    fontAction->addTo( fontToolBar );
    fontAction->addTo( fontMenu );
    connect( fontAction, SIGNAL( activated() ) , this, SLOT( showFontDialog() ) );


    ToolBarItem *fp1 = new ToolBarItem( this, fontToolBar, tr( "Fontsize +1" ),
		      "font size=\"+1\"", QPixmap( (const char **) fontinc_data ) );
    connect( fp1, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *fm1 = new ToolBarItem( this, fontToolBar, tr( "Fontsize -1" ),
		      "font size=\"-1\"", QPixmap( (const char **) fontdec_data ) );
    connect( fm1, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *h1 = new ToolBarItem( this, fontToolBar, tr( "Headline 1" ),
		      "h1", QPixmap( (const char **) h1_data ) );
    connect( h1, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *h2 = new ToolBarItem( this, fontToolBar, tr( "Headline 2" ),
		      "h2", QPixmap( (const char **) h2_data ) );
    connect( h2, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));

    ToolBarItem *h3 = new ToolBarItem( this, fontToolBar, tr( "Headline 3" ),
		      "h3", QPixmap( (const char **) h3_data ) );
    connect( h3, SIGNAL( clicked( const QString& ) ),
	     this, SLOT( insertTags( const QString& )));


    connect( helpButton, SIGNAL( clicked() ), MainWindow::self, SLOT( showDialogHelp() ) );
    textEdit->document()->setFormatter( new QTextFormatterBreakInWords );
    textEdit->document()->setUseFormatCollection( FALSE );
    textEdit->document()->setPreProcessor( new SyntaxHighlighter_HTML );

    if ( !callStatic ) {
	mlined = (QMultiLineEdit*)editWidget;
	mlined->setReadOnly( TRUE );
	textEdit->setAlignment( mlined->alignment() );
	textEdit->setWordWrap( mlined->wordWrap() );
	textEdit->setWrapColumnOrWidth( mlined->wrapColumnOrWidth() );
	textEdit->setWrapPolicy( mlined->wrapPolicy() );
	textEdit->setText( mlined->text() );
	if ( !mlined->text().isEmpty() )
	    textEdit->selectAll();
    }
    else {
	textEdit->setText( text );
	textEdit->selectAll();
    }
    textEdit->setFocus();
}

int MultiLineEditor::exec()
{
    res = 1;
    show();
    qApp->enter_loop();
    return res;
}

void MultiLineEditor::okClicked()
{
    applyClicked();
    qApp->exit_loop();
    hide();
}

void MultiLineEditor::applyClicked()
{
    if ( !callStatic ) {
	PopulateMultiLineEditCommand *cmd = new PopulateMultiLineEditCommand( tr( "Set the text of '%1'" ).arg( mlined->name() ),
						formwindow, mlined, textEdit->text() );
	cmd->execute();
	formwindow->commandHistory()->addCommand( cmd );
	textEdit->setFocus();
    }
    else {
	staticText = textEdit->text();
    }
}

void MultiLineEditor::cancelClicked()
{
    res = 0;
    hide();
}

void MultiLineEditor::insertTags( const QString &tag )
{
    int pfrom, pto, ifrom, ito;
    QString tagend(  tag.simplifyWhiteSpace() );
    tagend.remove( tagend.find( ' ', 0 ), tagend.length() );
    if ( textEdit->hasSelectedText() ) {
	textEdit->getSelection( &pfrom, &ifrom, &pto, &ito );
	QString buf = textEdit->selectedText();
	buf = QString( "<%1>%2</%3>" ).arg( tag ).arg( buf ).arg( tagend );
	textEdit->removeSelectedText();
	textEdit->insertAt( buf, pfrom, ifrom );
	textEdit->setCursorPosition( pto, ito + 2 + tag.length() );
    }
    else {
	int para, index;
	textEdit->getCursorPosition( &para, &index );
	textEdit->insert( QString( "<%1></%2>" ).arg( tag ).arg( tagend ) );
	index += 2 + tag.length();
	textEdit->setCursorPosition( para, index  );
    }
}

void MultiLineEditor::insertBR()
{
    textEdit->insert( "<br>" );
}

void MultiLineEditor::showFontDialog()
{
    bool selText = FALSE;
    int pfrom, pto, ifrom, ito;
    if ( textEdit->hasSelectedText() ) {
	textEdit->getSelection( &pfrom, &ifrom, &pto, &ito );
	selText = TRUE;
    }
    RichTextFontDialog *fd = new RichTextFontDialog( this );
    if ( fd->exec() == QDialog::Accepted ) {
	QString size, font;
	if ( fd->getSize() != "0" )
	    size = "size=\"" + fd->getSize() + "\"";
	QString color;
	if ( !fd->getColor().isEmpty() && fd->getColor() != "#000000" )
	    color = "color=\"" + fd->getColor() + "\"";
	if ( fd->getFont() != "default" )
	    font = "face=\"" + fd->getFont() + "\"";
	QString tag( QString( "font %1 %2 %3" )
	             .arg( color ).arg( size ).arg( font ) );

	if ( selText )
	    textEdit->setSelection( pfrom, ifrom, pto, ito );
	insertTags( tag.simplifyWhiteSpace() );
    }
    else if ( selText )
	textEdit->setSelection( pfrom, ifrom, pto, ito );
}

QString MultiLineEditor::getStaticText()
{
    return staticText;
}

QString MultiLineEditor::getText( QWidget *parent, const QString &text )
{
    MultiLineEditor medit( parent, 0, 0, text );
    if ( medit.exec() == QDialog::Accepted )
	return medit.getStaticText();

    return QString::null;
}
