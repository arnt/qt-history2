#include "qtitlebar_p.h"

#include "qapplication.h"
#include "../kernel/qapplication_p.h"
#include "qtooltip.h"
#include "qimage.h"
#include "qdatetime.h"
#include "qpainter.h"
#include "qcleanuphandler.h"

#ifndef QT_NO_WORKSPACE
#include <qworkspace.h>
#endif

#if defined(Q_WS_WIN)
#include "qt_windows.h"

extern QRgb qt_colorref2qrgb(COLORREF);

#define TITLEBAR_SEPARATION 1
#define BUTTON_WIDTH 16
#define BUTTON_HEIGHT 14

const char * const qt_close_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"..##....##..",
"...##..##...",
"....####....",
".....##.....",
"....####....",
"...##..##...",
"..##....##..",
"............",
"............",
"............"};

const char * const qt_maximize_xpm[]={
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".##########.",
".##########.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".#........#.",
".##########.",
"............"};


const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"...######...",
"...######...",
"............",
"............",
"............"};

const char * const qt_normalize_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"...#######..",
"...#######..",
"...#.....#..",
".#######.#..",
".#######.#..",
".#.....#.#..",
".#.....###..",
".#.....#....",
".#.....#....",
".#######....",
"............"};

const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"...#######..",
"...#######..",
"...#.....#..",
".#######.#..",
".#######.#..",
".#.....#.#..",
".#.....###..",
".#.....#....",
".#.....#....",
".#######....",
"............"};


const char * const qt_shade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"............",
".....#......",
"....###.....",
"...#####....",
"..#######...",
"............",
"............",
"............"};

const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
"............",
"............",
"..#######...",
"...#####....",
"....###.....",
".....#......",
"............",
"............",
"............",
"............"};



#else // !Q_WS_WIN

#define TITLEBAR_SEPARATION 1
#define BUTTON_WIDTH 16
#define BUTTON_HEIGHT 14
#define RANGE 16

const char * const qt_close_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"   .    .   ",
"  ...  ...  ",
"   ......   ",
"    ....    ",
"    ....    ",
"   ......   ",
"  ...  ...  ",
"   .    .   ",
"            ",
"            "};

const char * const qt_maximize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"     .      ",
"    ...     ",
"   .....    ",
"  .......   ",
" .........  ",
"            ",
"            ",
"            ",
"            "};

const char * const qt_minimize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"            ",
" .........  ",
"  .......   ",
"   .....    ",
"    ...     ",
"     .      ",
"            ",
"            ",
"            "};

const char * const qt_normalize_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"  .         ",
"  ..        ",
"  ...       ",
"  ....      ",
"  .....     ",
"  ......    ",
"  .......   ",
"            ",
"            ",
"            "};

const char * const qt_normalizeup_xpm[] = {
"12 12 2 1",
"       s None  c None",
".      c black",
"            ",
"            ",
"            ",
"  .......   ",
"   ......   ",
"    .....   ",
"     ....   ",
"      ...   ",
"       ..   ",
"        .   ",
"            ",
"            "};

const char * const qt_shade_xpm[] = {
"12 12 2 1", "# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
"............",
"............",
"............",
"............",
"............",
"............",
"............",
"............"};


const char * const qt_unshade_xpm[] = {
"12 12 2 1",
"# c #000000",
". c None",
"............",
"............",
".#########..",
".#########..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#.......#..",
".#########..",
"............",
"............"};


#endif // !Q_WS_WIN

static QPixmap *buffer = 0;
static QCleanupHandler<QPixmap> qtb_cleanup;

#ifndef QT_NO_WORKSPACE

QTitleBar::QTitleBar (QWorkspace* w, QWidget* win, QWidget* parent,
						  const char* name, bool iconMode )
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder )
{
    workspace = w;
    window = win;
    buttonDown = FALSE;
    imode = iconMode;

    titleL = new QTitleBarLabel( this );
    closeB = new QTitleBarButton( this, QTitleBarButton::Close, "close" );
    QToolTip::add( closeB, tr( "Close" ) );
    closeB->setFocusPolicy( NoFocus );
    closeB->setPixmap( QPixmap( (const char **)qt_close_xpm ) );
    connect( closeB, SIGNAL( clicked() ),
	     this, SIGNAL( doClose() ) ) ;
    maxB = new QTitleBarButton( this, QTitleBarButton::Max, "maximize" );
    QToolTip::add( maxB, tr( "Maximize" ) );
    maxB->setFocusPolicy( NoFocus );
    maxB->setPixmap( QPixmap( (const char **)qt_maximize_xpm ));
    connect( maxB, SIGNAL( clicked() ),
	     this, SIGNAL( doMaximize() ) );
    iconB = new QTitleBarButton( this, QTitleBarButton::Min, "iconify" );
    iconB->setFocusPolicy( NoFocus );

    shadeB = new QTitleBarButton( this, QTitleBarButton::ShadeUp, "shade" );
    QToolTip::add( shadeB, tr( "Roll up" ) );
    shadeB->setPixmap( QPixmap( (const char **)qt_shade_xpm ) );
    shadeB->setFocusPolicy( NoFocus );
    connect( shadeB, SIGNAL( clicked() ),
	     this, SIGNAL( doShade() ) );

    iconL = new QLabel( this, "icon" );
    iconL->setAlignment( AlignCenter );
    iconL->setAutoMask( TRUE );
    iconL->setFocusPolicy( NoFocus );
    iconL->installEventFilter( this );

    if ( window ) {
	if ( !window->testWFlags( WStyle_Tool ) ) {
#ifdef Q_WS_WIN
	    titleHeight = GetSystemMetrics( SM_CYCAPTION );
	    if ( !titleHeight )
		titleHeight = 18;
#else
	    titleHeight = 18;
#endif
	    closeB->resize(titleHeight-4, titleHeight-4);
	    maxB->resize(titleHeight-4, titleHeight-4);
	    iconB->resize(titleHeight-4, titleHeight-4);
	    shadeB->resize(titleHeight-4, titleHeight-4);
	    iconL->resize(titleHeight-4, titleHeight-4);
	    shadeB->hide();
	    if ( !window->testWFlags( WStyle_MinMax ) ) {
		maxB->hide();
		iconB->hide();
	    }
	    if ( !window->testWFlags( WStyle_SysMenu ) ) {
		iconL->hide();
		closeB->hide();
		maxB->hide();
		iconB->hide();
	    }
	} else {
#ifdef Q_WS_WIN
	    titleHeight = GetSystemMetrics( SM_CYSMCAPTION );
	    if ( !titleHeight )
		titleHeight = 16;
#else
	    titleHeight = 16;
#endif
   	    closeB->resize( titleHeight-2, titleHeight-2 );
	    maxB->resize( titleHeight-2, titleHeight-2 );
	    iconB->resize( titleHeight-2, titleHeight-2 );
	    shadeB->resize( titleHeight-2, titleHeight-2 );
	    iconL->resize(titleHeight-2, titleHeight-2);
	    maxB->hide();
	    iconB->hide();
	    iconL->hide();
	    if ( !window->testWFlags( WStyle_MinMax ) )
		shadeB->hide();
        }
    } else if ( iconMode ) {
#ifdef Q_WS_WIN
	titleHeight = GetSystemMetrics( SM_CYCAPTION );
	if ( !titleHeight )
	    titleHeight = 18;
#else
	titleHeight = 18;
#endif
	closeB->resize(titleHeight-4, titleHeight-4);
	maxB->resize(titleHeight-4, titleHeight-4);
	iconB->resize(titleHeight-4, titleHeight-4);
	shadeB->resize(titleHeight-4, titleHeight-4);
	iconL->resize(titleHeight-4, titleHeight-4);
	shadeB->hide();
    }

    if ( imode ) {
	iconB->setPixmap( QPixmap( (const char **)qt_normalizeup_xpm ) );
	iconB->setType( QTitleBarButton::RestoreUp );
	QToolTip::add( iconB, tr( "Restore Up" ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doNormal() ) );
    }
    else {
	iconB->setPixmap( QPixmap( (const char **)qt_minimize_xpm ) );
	iconB->setType( QTitleBarButton::Min );
	QToolTip::add( iconB, tr( "Minimize" ) );
	connect( iconB, SIGNAL( clicked() ),
		 this, SIGNAL( doMinimize() ) );
    }

    titleL->installEventFilter( this );
    QFont f = font();
    f.setBold( TRUE );
#ifdef Q_WS_WIN // Don't scale fonts on X
    if ( window && window->testWFlags( WStyle_Tool ) )
	f.setPointSize( f.pointSize() - 1 );
#endif
    titleL->setFont( f );
    setActive( FALSE );
}

QTitleBar::~QTitleBar()
{
}

void QTitleBar::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = TRUE;
	moveOffset = mapToParent( e->pos() );
	emit doActivate();
    }
}

void QTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	buttonDown = FALSE;
	releaseMouse();
    }
}

void QTitleBar::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown )
	return;

    if ( (moveOffset - mapToParent( e->pos() ) ).manhattanLength() < 4 )
	return;

    QPoint p = workspace->mapFromGlobal( e->globalPos() );
    if ( !workspace->rect().contains(p) ) {
	if ( p.x() < 0 )
	    p.rx() = 0;
	if ( p.y() < 0 )
	    p.ry() = 0;
	if ( p.x() > workspace->width() )
	    p.rx() = workspace->width();
	if ( p.y() > workspace->height() )
	    p.ry() = workspace->height();
    }

    QPoint pp = p - moveOffset;

    parentWidget()->move( pp );
}

void QTitleBar::setText( const QString& title )
{
    text = title;
    titleL->setText( title );
}


void QTitleBar::setIcon( const QPixmap& icon )
{
    if ( icon.height() > titleHeight || icon.width() > titleHeight ) {
	QPixmap p;
	p.convertFromImage( icon.convertToImage().smoothScale( titleHeight, titleHeight ) );
	iconL->setPixmap( p );
    } else {
	iconL->setPixmap( icon );
    }
}

bool QTitleBar::eventFilter( QObject * o, QEvent * e)
{
    static QTime* t = 0;
    static QTitleBar* tc = 0;
    if ( o == titleL ) {
	if ( e->type() == QEvent::MouseButtonPress
	     || e->type() == QEvent::MouseButtonRelease
	     || e->type() == QEvent::MouseMove) {
	    QMouseEvent* me = (QMouseEvent*) e;
	    QMouseEvent ne( me->type(), titleL->mapToParent(me->pos()), me->globalPos(),
			    me->button(), me->state() );

	    if (e->type() == QEvent::MouseButtonPress )
		mousePressEvent( &ne );
	    else if (e->type() == QEvent::MouseButtonRelease )
		mouseReleaseEvent( &ne );
	    else
		mouseMoveEvent( &ne );
	} else if ( (e->type() == QEvent::MouseButtonDblClick) &&
		  ((QMouseEvent*)e)->button() == LeftButton ) {
	    if ( imode )
		emit doNormal();
	    else if ( !maxB->testWState(WState_ForceHide) )
		emit doMaximize();
	    else
	        emit doShade();
	    return TRUE;
	}
    } else if ( o == iconL ) {
	switch ( e->type() ) {
	case QEvent::MouseButtonPress:
	    emit doActivate();
	    if ( !t )
		t = new QTime;
	    if ( tc != this || t->elapsed() > QApplication::doubleClickInterval() ) {
		emit showOperationMenu();
		t->start();
		tc = this;
	    } else {
		tc = 0;
		emit doClose();
	    }
	    return TRUE;
	default:
	    break;
	}
    }
    return QWidget::eventFilter(o, e);
}

void QTitleBar::enterEvent( QEvent * )
{
    QEvent e( QEvent::Leave );
    QApplication::sendEvent( parentWidget(), &e );
}

void QTitleBar::resizeEvent( QResizeEvent * )
{ // NOTE: called with 0 pointer
    int bo = ( height()- closeB->height() ) / 2;
    closeB->move( width() - closeB->width() - bo - 1, bo  );
    maxB->move( closeB->x() - maxB->width() - bo, closeB->y() );
    iconB->move( maxB->x() - iconB->width(), maxB->y() );
    shadeB->move( closeB->x() - shadeB->width(), closeB->y() );
    iconL->move( 2, 1 );

    int right = closeB->isVisibleTo( this ) ? closeB->x() : width();
    if ( iconB->isVisibleTo( this ) )
	right = iconB->x();
    if ( shadeB->isVisibleTo( this ) )
	right = shadeB->x();

    int bottom = rect().bottom() - imode - 1;

    if ( right != width() )
	right-=2;

    titleL->setRightMargin( width() - right );
    titleL->setLeftMargin( iconL->isVisibleTo( this ) ? iconL->width()+4 : 2 );


    titleL->setGeometry( QRect( QPoint( 1, 0 ),
 				QPoint( rect().right()-1, bottom ) ) );
}

void QTitleBar::setActive( bool active )
{
    if ( act == active )
	return ;

    act = active;

    titleL->setActive( active );
#ifndef Q_WS_WIN
    if ( active ) {
	if ( imode ){
	    iconB->show();
	    maxB->show();
	    closeB->show();
	}
    } else {
	if ( imode ){
	    iconB->hide();
	    closeB->hide();
	    maxB->hide();
	}
    }
#endif
    if ( imode )
	resizeEvent( 0 );
}

bool QTitleBar::isActive() const
{
    return act;
}


QSize QTitleBar::sizeHint() const
{
    constPolish();

    return QSize( 128, QMAX( titleHeight, fontMetrics().lineSpacing() ) );
}

#endif

/*
  QTitleBarLabel
*/

QTitleBarLabel::QTitleBarLabel( QWidget *parent, const char* name )
    : QFrame( parent, name, WRepaintNoErase | WResizeNoErase ), 
    leftm( 2 ), rightm( 2 ), act( TRUE )
{
    getColors();
}

void QTitleBarLabel::getColors()
{
#ifdef _WS_WIN
    aleftc = arightc = palette().active().highlight();
    ileftc = irightc = palette().inactive().dark();
    atextc = palette().active().highlightedText();
    itextc = palette().inactive().background();
#else
    aleftc = arightc = palette().active().highlight();
    ileftc = irightc = palette().inactive().background();
    atextc = palette().active().highlightedText();
    itextc = palette().inactive().foreground();
#endif

#ifdef Q_WS_WIN // ask system properties on windows
#ifndef SPI_GETGRADIENTCAPTIONS
#define SPI_GETGRADIENTCAPTIONS 0x1008
#endif
#ifndef COLOR_GRADIENTACTIVECAPTION
#define COLOR_GRADIENTACTIVECAPTION 27
#endif
#ifndef COLOR_GRADIENTINACTIVECAPTION
#define COLOR_GRADIENTINACTIVECAPTION 28
#endif
    if ( qt_winver == Qt::WV_98 || qt_winver == WV_2000 || qt_winver == WV_XP ) {
	if ( QApplication::desktopSettingsAware() ) {
	    aleftc = qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION));
	    ileftc = qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION));
	    atextc = qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT));
	    itextc = qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT));

	    arightc = aleftc;
	    irightc = ileftc;

	    BOOL gradient;
	    SystemParametersInfo( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
	    if ( gradient ) {
		arightc = qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTACTIVECAPTION));
		irightc = qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTINACTIVECAPTION));
	    }
	}
    }
#endif // Q_WS_WIN

    setActive( act );
}

void QTitleBarLabel::setText( const QString& text )
{
    titletext = text;
    cutText();
}

void QTitleBarLabel::cutText()
{
    QFontMetrics fm( font() );

    int maxw = contentsRect().width() - leftm - rightm;

    if ( fm.width( titletext+"m" ) > maxw ) {
	QToolTip::remove( this );
	QToolTip::add( this, titletext );
	int i = titletext.length();
	while ( (fm.width(titletext.left( i ) + "...")  > maxw) && i>0 )
	    i--;
	cuttext = titletext.left( i ) + "...";
	drawLabel();
    } else {
	QToolTip::remove( this );
	cuttext = titletext;
    }
    drawLabel();
}

void QTitleBarLabel::setLeftMargin( int x )
{
    leftm = x;
    cutText();
}

void QTitleBarLabel::setRightMargin( int x )
{
    rightm = x;
    cutText();
}

void QTitleBarLabel::drawLabel( bool redraw )
{
    if ( !buffer ) {
	buffer = new QPixmap;
	qtb_cleanup.add( buffer );
    }

    if ( buffer->isNull() )
	return;

    buffer->fill( yellow );
    QPainter p( buffer );
    p.setFont( font() );

    style().drawTitleBar( &p, contentsRect(), leftc, rightc, act );
    drawFrame( &p );
    QRect cr( 1, 1, width()-2, height()-2);
    style().drawTitleBarLabel( &p, QRect( cr.x() + leftm, cr.y(), cr.width() - rightm, cr.height() ), cuttext, textc, act );

    p.end();

    if ( redraw )
	QApplication::postEvent( this, new QPaintEvent( rect(), FALSE ) );
}

void QTitleBarLabel::frameChanged()
{
    cutText();
}

void QTitleBarLabel::paintEvent( QPaintEvent* )
{
    if ( !buffer ) 
	return;
    buffer->resize( QSize( QMAX( size().width(), buffer->width() ), QMAX( size().height(), buffer->height() ) ) );
    drawLabel( FALSE );
    bitBlt( this, 0, 0, buffer, 0, 0, width(), height() );
}

void QTitleBarLabel::resizeEvent( QResizeEvent* e )
{
    QFrame::resizeEvent( e );

    if ( !buffer ) {
	buffer = new QPixmap;
	qtb_cleanup.add( buffer );
    }
    buffer->resize( QSize( QMAX( size().width(), buffer->width() ), QMAX( size().height(), buffer->height() ) ) );
    cutText();
}

bool QTitleBarLabel::event( QEvent* e )
{
    if ( e->type() == QEvent::ApplicationPaletteChange ) {
	getColors();
	return TRUE;
    } else if ( e->type() == QEvent::WindowActivate ) {
	setActive( act );
    } else if ( e->type() == QEvent::WindowDeactivate ) {
	bool wasActive = act;
	setActive( FALSE );
	act = wasActive;
    }

    return QFrame::event( e );
}

void QTitleBarLabel::setActive( bool a )
{
    act = a;
    if ( a ) {
	textc = atextc;
	leftc = aleftc;
	rightc = arightc;
    } else {
	textc = itextc;
	leftc = ileftc;
	rightc = irightc;
    }

    drawLabel();
}

QTitleBarButton::QTitleBarButton( QWidget* parent, ButtonType t, const char *name )
    : QButton( parent, name ), type( t )
{
    setFocusPolicy( NoFocus );
}

void QTitleBarButton::setPixmap( const QPixmap& pm )
{
    if ( pm.height() > 14 || pm.width() > 14 ) {
	QPixmap p;
	p.convertFromImage( pm.convertToImage().smoothScale( 14, 14 ) );
	QButton::setPixmap( p );
    } else {
	QButton::setPixmap( pm );
    }
}

void QTitleBarButton::setType( ButtonType t )
{
    type = t;
    update();
}

QSize QTitleBarButton::sizeHint() const
{
    return QSize( 14,14 );
}

void QTitleBarButton::drawButton( QPainter *p )
{
    style().drawTitleBarButton( p, QRect( 0, 0, width(), height() ), colorGroup(), isDown() );
    drawButtonLabel( p );
}

void QTitleBarButton::drawButtonLabel( QPainter *p )
{
    style().drawTitleBarButtonLabel( p, QRect( 0, 0, width(), height() ), pixmap(), type, isDown() );
}
