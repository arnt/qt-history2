#include "qeffects_p.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qtimer.h"
#include "qdatetime.h"

/*
  Internal class to get access to protected QWidget-members
*/

class QAccessWidget : public QWidget
{
    friend class QAlphaWidget;
    friend class QRollEffect;
public:
    QAccessWidget( QWidget* parent = 0, const char* name = 0, WFlags f = 0 )
	: QWidget( parent, name, f ) {}
};

/*
  Internal class QAlphaWidget.

  The QAlphaWidget is shown while the animation lasts
  and displays the pixmap resulting from the alpha blending.
*/

class QAlphaWidget: public QWidget, QEffects
{
    Q_OBJECT
public:
    QAlphaWidget( QWidget* w, QWidget* parent = 0, const char* name = 0, WFlags f = 0);

    void run( int time );

protected:
    void paintEvent( QPaintEvent* e );
    bool eventFilter( QObject* o, QEvent* e );
    void alphaBlend();

protected slots:
    void render();

private:
    QPixmap pm;
    double alpha;
    QImage back;
    QImage front;
    QImage mixed;
    QAccessWidget* widget;
    int duration;
    bool showWidget;
    QTimer anim;
    QTime checkTime;
};

static QAlphaWidget* blend = 0;

/*
  Constructs a QAlphaWidget.
*/
QAlphaWidget::QAlphaWidget( QWidget* w, QWidget* parent, const char* name, WFlags f )
    : QWidget( parent, name,
	       f | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WStyle_StaysOnTop | WResizeNoErase | WRepaintNoErase )
{
    setBackgroundMode( NoBackground );
    widget = (QAccessWidget*)w;
    alpha = 0;
}

/*
  \reimp
*/
void QAlphaWidget::paintEvent( QPaintEvent* )
{
    bitBlt( this, QPoint(0,0), &pm );
}

/*
  Starts the alphablending animation.
  The animation will take about \a time ms
*/
void QAlphaWidget::run( int time )
{
    duration = time;

    if ( duration < 0 )
	duration = 200;

    checkTime.start();

    if ( !widget )
	return;

    showWidget = TRUE;
    widget->installEventFilter( this );

    move( widget->geometry().x(),widget->geometry().y() );
    resize( widget->size().width(), widget->size().height() );

    front = QImage( widget->size(), 32 );
    front = QPixmap::grabWidget( widget );

    back = QImage( widget->size(), 32 );
    back = QPixmap::grabWindow( QApplication::desktop()->winId(),
				widget->geometry().x(), widget->geometry().y(),
				widget->geometry().width(), widget->geometry().height() );

    mixed = back.copy();

    if ( !mixed.isNull() ) {
	widget->setWState( WState_Visible );
	widget->clearWState( WState_ForceHide );
	pm = mixed;
	show();
	raise();

	connect( &anim, SIGNAL(timeout()), this, SLOT(render()));
	anim.start( 10 );
    } else {
        widget->clearWState( WState_Visible );
	widget->setWState( WState_ForceHide );
	widget->show();
    }
}

/*
  \reimp
*/
bool QAlphaWidget::eventFilter( QObject* o, QEvent* e )
{
    switch ( e->type() )
	{
	case QEvent::Move:
	    move( widget->geometry().x(),widget->geometry().y() );
	    update();
	    break;
	case QEvent::Hide:
	case QEvent::Close:
	    showWidget = FALSE;
	    render();
	    break;
	default:
	    break;
	}
    return QWidget::eventFilter( o, e );
}

/*
  Render alphablending for the time elapsed.

  Show the blended widget and free all allocated source
  if the blending is finished.
*/
void QAlphaWidget::render()
{
    alpha = double(checkTime.elapsed()) / duration;
    if ( alpha >= 1 || !showWidget) {
	anim.stop();
	widget->removeEventFilter( this );
	widget->clearWState( WState_Visible );
	widget->setWState( WState_ForceHide );
	
	BackgroundMode bgm = widget->backgroundMode();
	if ( showWidget ) {
	    widget->setBackgroundMode( NoBackground );
	    widget->show();
	}
	hide();
	if ( showWidget ) {
	    widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
	    widget->setBackgroundMode( bgm );
	    widget->setWState( WState_Visible );
	}
	delete this;
	blend = 0;
    } else {
	alphaBlend();
	pm = mixed;
	repaint( FALSE );
    }
}

/*
  Calculate an alphablended image.
*/
void QAlphaWidget::alphaBlend()
{
    const double ia = 1-alpha;
    const int sw = front.width();
    const int sh = front.height();
    UINT32** md = (UINT32**)mixed.jumpTable();
    UINT32** bd = (UINT32**)back.jumpTable();
    UINT32** fd = (UINT32**)front.jumpTable();

    for (int sy = 0; sy < sh; sy++ ) {
	UINT32* bl = ((UINT32*)bd[sy]);
	UINT32* fl = ((UINT32*)fd[sy]);
	for (int sx = 0; sx < sw; sx++ ) {
	    UINT32 bp = bl[sx];
	    UINT32 fp = fl[sx];

	    ((UINT32*)(md[sy]))[sx] =  qRgb(int (qRed(bp)*ia + qRed(fp)*alpha),
					    int (qGreen(bp)*ia + qGreen(fp)*alpha),
					    int (qBlue(bp)*ia + qBlue(fp)*alpha) );
	}
    }
}

/*
  Internal class QRollEffect

  The QRollEffect widget is shown while the animation lasts
  and displays a scrolling pixmap.
*/

class QRollEffect : public QWidget, QEffects
{
    Q_OBJECT
public:
    QRollEffect( QWidget* w, DirFlags orient );

    void run( int time );

protected:
    void paintEvent( QPaintEvent* );
    bool eventFilter( QObject*, QEvent* );

private slots:
    void scroll();

private:
    QAccessWidget* widget;

    int currentHeight;
    int currentWidth;
    int totalHeight;
    int totalWidth;

    int duration;
    bool done;
    bool showWidget;
    int orientation;

    QTimer anim;
    QTime checkTime;

    QPixmap pm;
};

static QRollEffect* roll = 0;

/*
  Construct a QRollEffect widget.
*/
QRollEffect::QRollEffect( QWidget* w, DirFlags orient )
    : QWidget(0, 0,
	      WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WStyle_StaysOnTop | WResizeNoErase | WRepaintNoErase )
, orientation(orient)
{
    widget = (QAccessWidget*) w;
    ASSERT( widget );

    setBackgroundMode( NoBackground );

    if ( widget->testWState( WState_Resized ) ) {
	totalWidth = widget->width();
	totalHeight = widget->height();
    } else {
	totalWidth = widget->sizeHint().width();
	totalHeight = widget->sizeHint().height();
    }

    currentHeight = totalHeight;
    currentWidth = totalWidth;

    if ( orientation & RightScroll || orientation & LeftScroll )
	currentWidth = 0;
    if ( orientation & DownScroll || orientation & UpScroll )
	currentHeight = 0;

    pm = QPixmap::grabWidget( widget );
}

/*
  \reimp
*/
void QRollEffect::paintEvent( QPaintEvent* )
{
    int x = orientation & RightScroll ? QMIN(0, currentWidth - totalWidth) : 0;
    int y = orientation & DownScroll ? QMIN(0, currentHeight - totalHeight) : 0;

    bitBlt( this, x, y, &pm, 
		  0, 0, pm.width(), pm.height(), CopyROP, TRUE );
}

/*
  \reimp
*/
bool QRollEffect::eventFilter( QObject* o, QEvent* e )
{
    switch ( e->type() )
	{
	case QEvent::Move:
	    move( widget->geometry().x(),widget->geometry().y() );
	    update();
	    break;
	case QEvent::Hide:
	case QEvent::Close:
	    showWidget = FALSE;
	    done = TRUE;
	    scroll();
	    break;
	default:
	    break;
	}
    return QWidget::eventFilter( o, e );
}

/*
  Start the animation.

  The animation will take about \a time ms, or is
  calculated if \a time is negative
*/
void QRollEffect::run( int time )
{
    if ( !widget )
	return;

    duration  = time;

    if ( duration < 0 )
	duration = QMIN( QMAX((totalWidth - currentWidth) +
			      (totalHeight - currentHeight), 200 ), 400 );

    connect( &anim, SIGNAL(timeout()), this, SLOT(scroll()));

    widget->setWState( WState_Visible );
    widget->clearWState( WState_ForceHide );

    move( widget->geometry().x(),widget->geometry().y() );
    resize( QMIN( currentWidth, totalWidth ), QMIN( currentHeight, totalHeight ) );

    show();

    widget->installEventFilter( this );

    showWidget = TRUE;
    done = FALSE;
    anim.start( 10 );
    checkTime.start();
}

/*
  Roll according to the time elapsed.
*/
void QRollEffect::scroll()
{
    if ( !done ) {
	if ( currentWidth != totalWidth ) {
	    currentWidth = totalWidth * checkTime.elapsed() / duration;
	    done = (currentWidth >= totalWidth);
	}
	if ( currentHeight != totalHeight ) {
	    currentHeight = totalHeight * checkTime.elapsed() / duration;
	    done = (currentHeight >= totalHeight);
	}
	done = ( ( currentHeight >= totalHeight ) && 
		 ( currentWidth >= totalWidth ) );

	int w = totalWidth;
	int h = totalHeight;
	int x = widget->geometry().x();
	int y = widget->geometry().y();

	if ( orientation & RightScroll || orientation & LeftScroll )
	    w = QMIN( currentWidth, totalWidth );
	if ( orientation & DownScroll || orientation & UpScroll )
	    h = QMIN( currentHeight, totalHeight );

	setUpdatesEnabled( FALSE );
	if ( orientation & UpScroll )
	    y = widget->geometry().y() + QMAX( 0, totalHeight - currentHeight );
	if ( orientation & LeftScroll )
	    x = widget->geometry().x() + QMAX( 0, totalWidth - currentWidth );
	if ( orientation & UpScroll || orientation & LeftScroll )
	    move( x, y );

	resize( w, h );	
	setUpdatesEnabled( TRUE );
	repaint( FALSE );
    }
    if ( done ) {
	anim.stop();
	widget->removeEventFilter( this );
	BackgroundMode bgm = widget->backgroundMode();
	if ( showWidget ) {
	    widget->clearWState( WState_Visible );
	    widget->setWState( WState_ForceHide );
	    widget->setBackgroundMode( NoBackground );
	    widget->show();
	}
	hide();
	if ( showWidget) {
	    widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
	    widget->setBackgroundMode( bgm );
	    widget->setWState( WState_Visible );
	    if ( widget->inherits( "QLabel" ) && widget->testWFlags( WStyle_Tool ) )
		widget->update();
	}
	delete this;
	roll = 0;
    }
}

#include "qeffects.moc"

/*!
  Scroll widget \a w in \a time ms.
  \a orient may be 1 (vertical), 2 (horizontal)
  or 3 (diagonal).
*/
void qScrollEffect( QWidget* w, QEffects::DirFlags orient, int time )
{
    if ( roll ) {
	delete roll;
	roll = 0;
    }

    qApp->sendPostedEvents( w, QEvent::Move );
    qApp->sendPostedEvents( w, QEvent::Resize );

    roll = new QRollEffect( w, orient );
    roll->run( time );
}

/*!
  Fade in widget \a w in \a time ms.
*/
void qFadeEffect( QWidget* w, int time )
{
    if ( blend ) {
	delete blend;
	blend = 0;
    }
	
    qApp->sendPostedEvents( w, QEvent::Move );
    qApp->sendPostedEvents( w, QEvent::Resize );

    blend = new QAlphaWidget( w );
    blend->run( time );
}
