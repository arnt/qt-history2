#include "arghintwidget.h"
#include <qbutton.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpixmap.h>

static const char * left_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"                ",
"                ",
"          +     ",
"         ++     ",
"        +++     ",
"       ++++     ",
"      +++++     ",
"     ++++++     ",
"     ++++++     ",
"      +++++     ",
"       ++++     ",
"        +++     ",
"         ++     ",
"          +     ",
"                ",
"                "};

static const char * right_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c #000000",
"                ",
"                ",
"     +          ",
"     ++         ",
"     +++        ",
"     ++++       ",
"     +++++      ",
"     ++++++     ",
"     ++++++     ",
"     +++++      ",
"     ++++       ",
"     +++        ",
"     ++         ",
"     +          ",
"                ",
"                "};

static const char * left_disabled_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c darkgray",
"                ",
"                ",
"          +     ",
"         ++     ",
"        +++     ",
"       ++++     ",
"      +++++     ",
"     ++++++     ",
"     ++++++     ",
"      +++++     ",
"       ++++     ",
"        +++     ",
"         ++     ",
"          +     ",
"                ",
"                "};

static const char * right_disabled_xpm[] = {
"16 16 3 1",
" 	c None",
".	c #FFFFFF",
"+	c darkgray",
"                ",
"                ",
"     +          ",
"     ++         ",
"     +++        ",
"     ++++       ",
"     +++++      ",
"     ++++++     ",
"     ++++++     ",
"     +++++      ",
"     ++++       ",
"     +++        ",
"     ++         ",
"     +          ",
"                ",
"                "};

class ArrowButton : public QButton
{
    Q_OBJECT

public:
    enum Dir { Left, Right };

    ArrowButton( QWidget *parent, Dir d );
    void drawButton( QPainter *p );

private:
    QPixmap pix, pix_disabled;

};

ArrowButton::ArrowButton( QWidget *parent, Dir d )
    : QButton( parent )
{
    setFixedSize( 16, 16 );
    if ( d == Left ) {
	pix = QPixmap( left_xpm );
	pix_disabled = QPixmap( left_disabled_xpm );
    } else {
	pix = QPixmap( right_xpm );
	pix_disabled = QPixmap( right_disabled_xpm );
    }
}

void ArrowButton::drawButton( QPainter *p )
{
    if ( isDown() )
	p->fillRect( 0, 0, width(), height(), darkGray );
    else
	p->fillRect( 0, 0, width(), height(), lightGray );
    if ( isEnabled() )
	p->drawPixmap( 0, 0, pix );
    else
	p->drawPixmap( 0, 0, pix_disabled );
}


ArgHintWidget::ArgHintWidget( QWidget *parent )
    : QFrame( parent, 0, WType_Popup ), curFunc( 0 ), numFuncs( 0 )
{
    setFrameStyle( QFrame::Box | QFrame::Plain );
    setLineWidth( 1 );
    setBackgroundColor( white );
    QHBoxLayout *hbox = new QHBoxLayout( this );
    hbox->setMargin( 1 );
    hbox->addWidget( ( prev = new ArrowButton( this, ArrowButton::Left ) ) );
    hbox->addWidget( ( funcLabel = new QLabel( this ) ) );
    hbox->addWidget( ( next = new ArrowButton( this, ArrowButton::Right ) ) );
    funcLabel->setBackgroundColor( white );
    funcLabel->setAlignment( AlignCenter );
    connect( prev, SIGNAL( clicked() ), this, SLOT( gotoPrev() ) );
    connect( next, SIGNAL( clicked() ), this, SLOT( gotoNext() ) );
    updateState();
}

void ArgHintWidget::setFunctionText( int func, const QString &text )
{
    funcs.replace( func, text );
    if ( func == curFunc )
	funcLabel->setText( text );
}

void ArgHintWidget::setNumFunctions( int num )
{
    funcs.clear();
    numFuncs = num;
    curFunc = 0;
    updateState();
}

void ArgHintWidget::gotoPrev()
{
    if ( curFunc > 0 ) {
	curFunc--;
	funcLabel->setText( funcs[ curFunc ] );
	updateState();
    }
}

void ArgHintWidget::gotoNext()
{
    if ( curFunc < numFuncs - 1 ) {
	curFunc++;
	funcLabel->setText( funcs[ curFunc ] );
	updateState();
    }
}

void ArgHintWidget::updateState()
{
    prev->setEnabled( curFunc > 0 );
    next->setEnabled( curFunc < numFuncs - 1 );
}

#include "arghintwidget.moc"
