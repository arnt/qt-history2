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
	: QWidget( parent, name, f )
    {}
};

/*
  Internal class QAlphaWidget.

  The QAlphaWidget is shown while the animation lasts
  and displays the pixmap resulting from the alpha blending.
*/

class QAlphaWidget: public QWidget
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
    void init();

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
    if ( blend )
	delete blend;

    setBackgroundMode( NoBackground );
    widget = (QAccessWidget*)w;
    alpha = 0;
}

/*!
  \reimp
*/
void QAlphaWidget::paintEvent( QPaintEvent* )
{
    bitBlt( this, QPoint(0,0), &pm );
}

/*
  Grabs the images that are to be blended and moves
  the alphawidget into position.
*/
void QAlphaWidget::init()
{
    move( widget->geometry().x(),widget->geometry().y() );
    resize( widget->size().width(), widget->size().height() );

    front = QImage( widget->size(), 32 );
    front = QPixmap::grabWidget( widget );

    back = QImage( widget->size(), 32 );
    back = QPixmap::grabWindow( QApplication::desktop()->winId(),
	widget->geometry().x(), widget->geometry().y(),
	widget->geometry().width(), widget->geometry().height() );
}

/*
  Starts the alphablending animation.
  The animation will take about \a time ms
*/
void QAlphaWidget::run( int time )
{
    checkTime.start();

    if ( !widget )
	return;

    showWidget = TRUE;
    widget->installEventFilter( this );
    init();

    mixed = back.copy();

    if ( !mixed.isNull() ) {
	widget->setWState( WState_Visible );
	widget->clearWState( WState_ForceHide );
	pm = mixed;
	show();
	raise();

	duration = time;

	connect( &anim, SIGNAL(timeout()), this, SLOT(render()));
	anim.start( 10 );
    } else {
        widget->clearWState( WState_Visible );
	widget->setWState( WState_ForceHide );
	widget->show();
    }
}

/*!
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
  Caluclate an alphablended image
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

// Internal class QRollEffect

class QRollEffect : public QWidget
{
    Q_OBJECT
public:
    QRollEffect( QWidget* w, Qt::Orientation orient );

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
    bool grow;
    bool done;
    bool showWidget;
    Qt::Orientation orientation;

    QTimer anim;
    QTime checkTime;

    QPixmap pm;
};

static QRollEffect* roll = 0;

/*
  Construct a QRollEffect widget

  The QRollEffect widget is shown while the animation lasts
  and displays the pixmap shifted.
*/
QRollEffect::QRollEffect( QWidget* w, Qt::Orientation orient )
: QWidget(0, 0,
	  WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WStyle_StaysOnTop | WResizeNoErase | WRepaintNoErase )
  , orientation(orient)
{
    if ( roll )
	delete roll;

    widget = (QAccessWidget*) w;
    ASSERT( widget );

    setBackgroundMode( NoBackground );

    widget->installEventFilter( this );

    if ( widget->testWState( WState_Resized ) ) {
	totalWidth = widget->width();
	totalHeight = widget->height();
    } else {
	totalWidth = widget->sizeHint().width();
	totalHeight = widget->sizeHint().height();
    }

    if ( widget->testWState( WState_ForceHide) ) {
	grow = TRUE;
	currentHeight = orientation == Horizontal ? totalHeight : 0;
	currentWidth = orientation == Vertical ? totalWidth : 0;
    } else {
	grow = FALSE;
	currentHeight = totalHeight;
	currentWidth = totalWidth;
    }

    move( widget->geometry().x(),widget->geometry().y() );
    resize( widget->size().width(), widget->size().height() );

    pm = QPixmap::grabWidget( widget );
}

/*!
  \reimp
*/
void QRollEffect::paintEvent( QPaintEvent* )
{
    bitBlt( this, currentWidth - totalWidth,currentHeight - totalHeight,
	&pm, 0,0, pm.width(), pm.height(), Qt::CopyROP, TRUE );
}

/*!
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

  The animation will take about \a time ms
*/
void QRollEffect::run( int time )
{
    duration  = time;
    if ( !widget )
	return;

    connect( &anim, SIGNAL(timeout()), this, SLOT(scroll()));

    widget->setWState( WState_Visible );
    widget->clearWState( WState_ForceHide );

    show();

    if ( !grow )
	widget->hide();

    showWidget = grow;
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
	switch ( orientation )
	{
	case Horizontal:
	    if ( grow ) {
		currentWidth = totalWidth * checkTime.elapsed() / duration;
		done = (currentWidth >= totalWidth);
		resize( QMIN( currentWidth, totalWidth ), totalHeight );
	    } else {
		currentWidth = totalWidth - totalWidth * checkTime.elapsed() / duration;
		done = (currentHeight <= 0 );
		resize( QMAX( currentWidth, 0 ), currentHeight );
	    }
	    break;
	case Vertical:
	    if ( grow ) {
		currentHeight = totalHeight * checkTime.elapsed() / duration;
		done = (currentHeight >= totalHeight);
		resize( totalWidth, QMIN( currentHeight, totalHeight ) );
	    } else {
		currentHeight = totalHeight - totalHeight * checkTime.elapsed() / duration;
		done = (currentHeight <= 0 );
		resize( currentWidth, QMAX( currentHeight, 0) );
	    }
	    break;
	}
    }
    if ( done ) {
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
	if ( showWidget) {
	    widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
	    widget->setBackgroundMode( bgm );
	    widget->setWState( WState_Visible );
	}
	delete this;
	roll = 0;
    }
}


#include "qeffects.moc"

// global functions

void scrollEffect( QWidget* w, Qt::Orientation orient, int time )
{
    qApp->sendPostedEvents( w, QEvent::Move );
    qApp->sendPostedEvents( w, QEvent::Resize );

    roll = new QRollEffect( w, orient );
    roll->run( time );
}

void fadeEffect( QWidget* w, int time )
{
    qApp->sendPostedEvents( w, QEvent::Move );
    qApp->sendPostedEvents( w, QEvent::Resize );

    blend = new QAlphaWidget( w );
    blend->run( time );
}
