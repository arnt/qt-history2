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

#include "qtextedit.h"
#include "qtexteditintern_h.cpp"
#include "qcppsyntaxhighlighter.h"

//#define CPP_EDITOR

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
	styleCombo->insertItem( "Bullet List" );
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
	connect( edit, SIGNAL( currentParagTypeChanged( int ) ),
		 this, SLOT( paragTypeChanged( int ) ) );
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
	    edit->setParagType( QTextEditParag::Normal );
	else if ( i == 1 )
	    edit->setParagType( QTextEditParag::BulletList );
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
    
    void paragTypeChanged( int t ) {
	if ( lock )
	    return;
	lock = TRUE;
	if ( t == QTextEditParag::Normal )
	    styleCombo->setCurrentItem( 0 );
	else if ( t == QTextEditParag::BulletList )
	    styleCombo->setCurrentItem( 1 );
	lock = FALSE;
    }
    
private:
    QToolBar *tb;
    QComboBox *fontCombo, *sizeCombo, *styleCombo;
    QPushButton *bold, *italic, *underline, *color, *left, *center, *right;
    QTextEdit *edit;
    bool lock;
    
};

int main( int argc, char ** argv )
{
    QApplication a( argc, argv );

    QString fn = "qtextedit.cpp";
    if ( argc > 1 )
	fn = argv[ 1 ];
    if ( !QFile::exists( fn ) )
	fn = "qtextedit.cpp";
    
    MainWindow mw;
    
#ifndef CPP_EDITOR
    QTextEditDocument *d = new QTextEditDocument( fn, FALSE );
    QTextEdit ed( &mw, d );
    mw.setCentralWidget( &ed );
    mw.setEdit( &ed );
#else
    QTextEditDocument *d = new QTextEditDocument( fn, TRUE );
    d->setSyntaxHighlighter( new QCppSyntaxHighlighter( d ) );
    d->setIndent( new QCppIndent( d ) );
    d->setParenCheckingEnabled( TRUE );
    d->setCompletionEnabled( TRUE );
    QTextEdit ed( &mw, d );
    mw.setCentralWidget( &ed );
    d->setFormatter( new QTextEditFormatterBreakInWords( d ) );
    mw.setEdit( &ed );
#endif
    ed.viewport()->setFocus();
    
    a.setMainWidget( &mw );
    mw.resize( 600, 800 );
    mw.show();

    return a.exec();
}

#include "main.moc"

