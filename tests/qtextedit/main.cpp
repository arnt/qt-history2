#include <qapplication.h>
#include <qfile.h>
#include <qmainwindow.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qfontdatabase.h>
#include <qtoolbar.h>
#include <qbuttongroup.h>
#include <qpixmap.h>
#include <qcolordialog.h>
#include <qlineedit.h>
#include <qtextstream.h>
#include <qprinter.h>
#include <qpaintdevicemetrics.h>

#include "qtextbrowser.h"
#include "qtextview.h"

//#define CPP_EDITOR
//#define SPELL_CHECKER
#define TEST_BIDI

#if defined(CPP_EDITOR) || defined(SPELL_CHECKER)
#define QTEXTEDIT_OPEN_API
#endif

#include "qtextedit.h"
#include "qsimplerichtext.h"

#if defined(QTEXTEDIT_OPEN_API)
#if defined(CPP_EDITOR)
#include "qcppsyntaxhighlighter.h"
#endif
#if defined(SPELL_CHECKER)
#include "qspellchecker.h"
#endif
#endif

class SimpleText : public QWidget
{
public:
    SimpleText() : QWidget() {
	QFile f( "/home/reggie/tmp/release/qt-2.2.0/doc/html/qcheckbox.html" );
	f.open( IO_ReadOnly );
	QTextStream ts( &f );
	s = new QSimpleRichText( ts.read(), QApplication::font(), "/home/reggie/tmp/release/qt-2.2.0/doc/html/qcheckbox.html" ); }
protected:
    void paintEvent( QPaintEvent *e ) {
	QPainter p( this );
	s->setWidth( &p, width() );
	s->draw( &p, 0, 0, e->region(), colorGroup() );
    }

    void mousePressEvent( QMouseEvent * ) {
	QPrinter printer;
	printer.setFullPage(TRUE);
	if ( printer.setup() ) {
	    QPainter p( &printer );
	    QPaintDeviceMetrics metrics(p.device());
	    int dpix = metrics.logicalDpiX();
	    int dpiy = metrics.logicalDpiY();
	    const int margin = 72; // pt
	    QRect body(margin*dpix/72, margin*dpiy/72,
		       metrics.width()-margin*dpix/72*2,
		       metrics.height()-margin*dpiy/72*2 );
	    QFont font("times", 10);
	    QFile f( "/home/reggie/tmp/release/qt-2.2.0/doc/html/qcheckbox.html" );
	    f.open( IO_ReadOnly );
	    QTextStream ts( &f );
	    QSimpleRichText richText( ts.read(), QFont( "times", 10 ),
				      "/home/reggie/tmp/release/qt-2.2.0/doc/html/qcheckbox.html", QStyleSheet::defaultSheet(),
				      QMimeSourceFactory::defaultFactory(), body.height() );
	    richText.setWidth( &p, body.width() );
	    QRect view( body );
	    int page = 1;
	    do {
		richText.draw( &p, body.left(), body.top() - ( page > 1 ? margin*dpiy/72 : 0 ), view, colorGroup() );
		view.moveBy( 0, body.height() );
		p.translate( 0 , -body.height() );
		p.setFont( font );
		p.drawText( view.right() - p.fontMetrics().width( QString::number(page) ),
			    view.bottom() + p.fontMetrics().ascent() + 5, QString::number(page) );
		if ( view.top()  >= richText.height() )
		    break;
		printer.newPage();
		page++;
		richText.setWidth( &p, body.width() );
	    } while (TRUE);
	}
    }

    QSimpleRichText *s;

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow() {
#ifndef CPP_EDITOR
	lock = TRUE;
	QFontDatabase db;
	tb = new QToolBar( this );

	styleCombo = new QComboBox( FALSE, tb );
	styleCombo->insertItem( "Standard" );
	styleCombo->insertItem( "Bullet List (Disc)" );
	styleCombo->insertItem( "Bullet List (Circle)" );
	styleCombo->insertItem( "Bullet List (Square)" );
	styleCombo->insertItem( "Ordered List (Decimal)" );
	styleCombo->insertItem( "Ordered List (Alpha lower)" );
	styleCombo->insertItem( "Ordered List (Alpha upper)" );
	connect( styleCombo, SIGNAL( activated( int ) ),
		 this, SLOT( styleChanged( int ) ) );
	
	fontCombo = new QComboBox( TRUE, tb );
	tb->setStretchableWidget( fontCombo );
	fontCombo->insertStringList( db.families() );
	connect( fontCombo, SIGNAL( activated( const QString & ) ),
		 this, SLOT( familyChanged( const QString & ) ) );
	fontCombo->lineEdit()->setText( QApplication::font().family() );
	
	sizeCombo = new QComboBox( TRUE, tb );
	QValueList<int> sizes = db.standardSizes();
	QValueList<int>::Iterator it = sizes.begin();
	for ( ; it != sizes.end(); ++it )
	    sizeCombo->insertItem( QString::number( *it ) );
	connect( sizeCombo, SIGNAL( activated( const QString & ) ),
		 this, SLOT( sizeChanged( const QString & ) ) );
	sizeCombo->lineEdit()->setText( QString::number( QApplication::font().pointSize() ) );
	
	tb->addSeparator();
	
	int s = fontCombo->height();
	bold = new QPushButton( "B", tb );
	bold->setToggleButton( TRUE );
	bold->setFixedSize( s, s );
	connect( bold, SIGNAL( stateChanged( int ) ),
		 this, SLOT( boldChanged( int ) ) );
	italic = new QPushButton( "I", tb );
	italic->setToggleButton( TRUE );
	italic->setFixedSize( s, s );
	connect( italic, SIGNAL( stateChanged( int ) ),
		 this, SLOT( italicChanged( int ) ) );
	underline = new QPushButton( "U", tb );
	underline->setToggleButton( TRUE );
	underline->setFixedSize( s, s );
	connect( underline, SIGNAL( stateChanged( int ) ),
		 this, SLOT( underlineChanged( int ) ) );

	tb->addSeparator();

	left = new QPushButton( "L", tb );
	left->setToggleButton( TRUE );
	left->setOn( TRUE );
	left->setFixedSize( s, s );
	connect( left, SIGNAL( stateChanged( int ) ),
		 this, SLOT( leftChanged( int ) ) );
	center = new QPushButton( "C", tb );
	center->setToggleButton( TRUE );
	center->setFixedSize( s, s );
	connect( center, SIGNAL( stateChanged( int ) ),
		 this, SLOT( centerChanged( int ) ) );
	right = new QPushButton( "R", tb );
	right->setToggleButton( TRUE );
	right->setFixedSize( s, s );
	connect( right, SIGNAL( stateChanged( int ) ),
		 this, SLOT( rightChanged( int ) ) );
	QButtonGroup *g = new QButtonGroup( this );
	g->hide();
	g->setExclusive( TRUE );
	g->insert( left );
	g->insert( center );
	g->insert( right );
	
	tb->addSeparator();

	color = new QPushButton( tb );
	color->setFixedSize( s, s );
	connect( color, SIGNAL( clicked() ),
		 this, SLOT( changeColor() ) );
	QPixmap pix( 16, 16 );
	pix.fill( black );
	color->setPixmap( pix );
	lock = FALSE;
	
	save = new QPushButton( "Save HTML", tb );
	connect( save, SIGNAL( clicked() ),
		 this, SLOT( saveClicked() ) );
#else
	tb = 0;
#endif
	
    }

    void setEdit( QTextEdit *e ) {
	edit = e;
	connect( edit, SIGNAL( currentFontChanged( const QFont & ) ),
		 this, SLOT( fontChanged( const QFont & ) ) );
	connect( edit, SIGNAL( currentColorChanged( const QColor & ) ),
		 this, SLOT( colorChanged( const QColor & ) ) );
	connect( edit, SIGNAL( currentAlignmentChanged( int ) ),
		 this, SLOT( alignChanged( int ) ) );
    }

private slots:
    void familyChanged( const QString &f ) {
	if ( lock )
	    return;
	lock = TRUE;
	edit->setFamily( f );
	lock = FALSE;
	lock = FALSE;
	edit->viewport()->setFocus();
    }
    void sizeChanged( const QString &s ) {
	if ( lock )
	    return;
	lock = TRUE;
	edit->setPointSize( s.toInt() );
	lock = FALSE;
	lock = FALSE;
	edit->viewport()->setFocus();
    }
    void boldChanged( int state ) {
	if ( lock )
	    return;
	lock = TRUE;
	edit->setBold( state == 2 );
	lock = FALSE;
	lock = FALSE;
    }
    void italicChanged( int state ) {
	if ( lock )
	    return;
	lock = TRUE;
	edit->setItalic( state == 2 );
	lock = FALSE;
    }
    void underlineChanged( int state ) {
	if ( lock )
	    return;
	lock = TRUE;
	edit->setUnderline( state == 2 );
	lock = FALSE;
    }
    void changeColor() {
	if ( lock )
	    return;
	lock = TRUE;
	QColor col = QColorDialog::getColor( QColor(), this );
	if ( col.isValid() ) {
	    edit->setColor( col );
	    colorChanged( col );
	}
	lock = FALSE;
    }
    void leftChanged( int state ) {
	if ( lock || state != 2 )
	    return;
	lock = TRUE;
	edit->setAlignment( Qt::AlignLeft );
	lock = FALSE;
    }
    void centerChanged( int state ) {
	if ( lock || state != 2 )
	    return;
	lock = TRUE;
	edit->setAlignment( Qt::AlignHCenter );
	lock = FALSE;
    }
    void rightChanged( int state ) {
	if ( lock || state != 2 )
	    return;
	lock = TRUE;
	edit->setAlignment( Qt::AlignRight );
	lock = FALSE;
    }
    void styleChanged( int i ) {
	if ( lock )
	    return;
	lock = TRUE;
 	if ( i == 0 )
 	    edit->setParagType( QStyleSheetItem::DisplayBlock, -1 );
 	else if ( i == 1 )
 	    edit->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDisc );
 	else if ( i == 2 )
 	    edit->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListCircle );
 	else if ( i == 3 )
 	    edit->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListSquare );
 	else if ( i == 4 )
 	    edit->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListDecimal );
 	else if ( i == 5 )
 	    edit->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListLowerAlpha );
 	else if ( i == 6 )
 	    edit->setParagType( QStyleSheetItem::DisplayListItem, QStyleSheetItem::ListUpperAlpha );
	lock = FALSE;
	edit->viewport()->setFocus();
    }


    void fontChanged( const QFont &f ) {
	if ( lock )
	    return;
	lock = TRUE;
	bold->setOn( f.bold() );
	italic->setOn( f.italic() );
	underline->setOn( f.underline() );
	fontCombo->lineEdit()->setText( f.family() );
	sizeCombo->lineEdit()->setText( QString::number( f.pointSize() ) );
	lock = FALSE;
    }

    void colorChanged( const QColor &c ) {
	QPixmap pix( 16, 16 );
	pix.fill( c );
	color->setPixmap( pix );
    }

    void alignChanged( int a ) {
	if ( lock )
	    return;
	lock = TRUE;
	left->setOn( a == Qt::AlignLeft );
	center->setOn( a == Qt::AlignHCenter );
	right->setOn( a == Qt::AlignRight );
	lock = FALSE;
    }

    void saveClicked() {
	edit->setTextFormat( Qt::RichText );
	edit->save( QString( "test.html" ) );
    }

private:
    QToolBar *tb;
    QComboBox *fontCombo, *sizeCombo, *styleCombo;
    QPushButton *bold, *italic, *underline, *color, *left, *center, *right, *save;
    QTextEdit *edit;
    bool lock;

};

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );
    QFont fnt;
    fnt.setFamily("tahoma");
    fnt.setCharSet( QFont::Unicode );
    a.setFont( fnt );
    MainWindow mw;

#ifdef TEST_BIDI
    QString fn = "bidi.txt";
    if ( argc > 1 )
	fn = argv[ 1 ];
    if ( !QFile::exists( fn ) )
	fn = "bidi.txt";
    QFile f( fn );
    f.open( IO_ReadOnly );
    QByteArray array = f.readAll();
    QString text = QString::fromUtf8( array.data() );
    QTextEdit ed ( &mw );
    ed.setText( text );
    mw.setCentralWidget( &ed );
    mw.setEdit( &ed );
#else    
    QString fn = "qtextedit.cpp";
    if ( argc > 1 )
	fn = argv[ 1 ];
    if ( !QFile::exists( fn ) )
	fn = "qtextedit.cpp";


#ifndef CPP_EDITOR
    QTextEdit ed( &mw );
    ed.load( fn );
    mw.setCentralWidget( &ed );
    mw.setEdit( &ed );
#if defined(SPELL_CHECKER)
    ed.document()->setSyntaxHighlighter( new QSpellChecker( ed.document() ) );
#endif
#else
    QTextEdit ed( &mw );
    ed.load( fn, TRUE );
    ed.document()->setSyntaxHighlighter( new QCppSyntaxHighlighter( ed.document() ) );
    ed.document()->setIndent( new QCppIndent( ed.document() ) );
    ed.document()->setParenCheckingEnabled( TRUE );
    ed.document()->setCompletionEnabled( TRUE );
    mw.setCentralWidget( &ed );
    ed.document()->setFormatter( new QTextFormatterBreakInWords( ed.document() ) );
    mw.setEdit( &ed );
#endif
#endif

    ed.viewport()->setFocus();

    a.setMainWidget( &mw );
    mw.resize( 650, 700 );
    mw.show();

    SimpleText t;
    t.resize ( 100, 100 );
    t.show();

    QTextBrowser b;
    ( (QMimeSourceFactory*)b.mimeSourceFactory() )->setFilePath( "/home/reggie/tmp/release/qt-2.2.0/doc/html" );
    b.setSource( "qcheckbox.html" );
    b.show();

    return a.exec();
}

#include "main.moc"

