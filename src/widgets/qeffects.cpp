#include "qeffects.h"
#include "qapplication.h"
#include "qwidget.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qtimer.h"
#include "qdatetime.h"

// Internal class to get access to protected QWidget-members

class QAccessWidget : public QWidget
{
    friend class QAlphaWidget;
    friend class QScrollEffect;
public:
    QAccessWidget( QWidget* parent = 0, const char* name = 0, WFlags f = 0 )
    {}
};

// Internal class QAlphaWidget

class QAlphaWidget: public QWidget
{
    Q_OBJECT
public:
    QAlphaWidget( QWidget* w, QWidget* parent = 0, const char* name = 0, WFlags f = 0)
	: QWidget( parent, name, 
	    f | WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WStyle_StaysOnTop | WResizeNoErase | WRepaintNoErase )
    {
	setBackgroundMode( NoBackground );
	widget = (QAccessWidget*)w;
	alpha = 0;
    }

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

void QAlphaWidget::paintEvent( QPaintEvent* e )
{
    bitBlt( this, QPoint(0,0), &pm );
}

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
	anim.start( 40 );
    } else {
        widget->clearWState( WState_Visible );
	widget->setWState( WState_ForceHide );
	widget->show();
    }
}


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

void QAlphaWidget::render()
{
    alpha = double(checkTime.elapsed()) / duration;
    if ( alpha >= 1 || !showWidget) {
	anim.stop();
	widget->removeEventFilter( this );
	widget->clearWState( WState_Visible );
	BackgroundMode bgm = widget->backgroundMode();
	widget->setBackgroundMode( NoBackground );
	if ( showWidget )
	    widget->show();
	hide();
	if ( showWidget ) {
	    widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
	    widget->setBackgroundMode( bgm );
	    widget->setWState( WState_Visible );
	}
	delete this;
    } else {
	alphaBlend();
	pm = mixed;
	repaint( FALSE );
    }
}

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

	    ((UINT32*)(md[sy]))[sx] =  qRgb(qRed(bp)*ia + qRed(fp)*alpha, 
		qGreen(bp)*ia + qGreen(fp)*alpha, 
		qBlue(bp)*ia + qBlue(fp)*alpha);
	}
    }
}

// Internal class QScrollEffect

class QScrollEffect : public QObject
{
    Q_OBJECT
public:
    QScrollEffect( QWidget* w, int time, Qt::Orientation orient );

    void run();

private slots:
    void scroll();

private:
    QAccessWidget* widget;

    int currentWidth;
    int currentHeight;
    int totalWidth;
    int totalHeight;
    QSize minSize;

    int duration;
    bool grow;
    Qt::Orientation orientation;

    QTimer anim;
    QTime checkTime;
};

QScrollEffect::QScrollEffect( QWidget* w, int time, Qt::Orientation orient )
: QObject( w ), duration(time), orientation(orient)
{
    widget = (QAccessWidget*) w; 
    ASSERT( widget );

    widget->setWFlags( WResizeNoErase | WRepaintNoErase );

    widget->installEventFilter( this );

    if ( widget->testWState( WState_Resized ) ) {
	totalWidth = widget->width();
	totalHeight = widget->height();
    } else {
	totalWidth = widget->sizeHint().width();
	totalHeight = widget->sizeHint().height();
    }

    minSize = widget->minimumSize();
    widget->setMinimumSize( 0, 0 );

    if ( widget->testWState( WState_ForceHide) ) {
	grow = TRUE;
	currentHeight = 0;
	currentWidth = 0;

	switch ( orientation )
	{
	case Horizontal:
	    widget->resize( currentWidth, totalHeight );
	    break;
	case Vertical:
	    widget->resize( totalWidth, currentHeight );
	    break;
	}

	widget->show();
    } else {
	grow = FALSE;
	currentHeight = totalHeight;
	currentWidth = totalWidth;    
    }
}

void QScrollEffect::run()
{
    if ( !widget )
	return;

    connect( &anim, SIGNAL(timeout()), this, SLOT(scroll()));
    anim.start( 40 );
    checkTime.start();
}

void QScrollEffect::scroll()
{
    bool done;

    switch ( orientation )
    {
    case Horizontal:
	if ( grow ) {
	    currentWidth = totalWidth * checkTime.elapsed() / duration;
	    done = (currentWidth >= totalWidth);
	    widget->resize( QMIN( currentWidth, totalWidth ), totalHeight );
	} else {
	    currentWidth = totalWidth - totalWidth * checkTime.elapsed() / duration;
	    done = (currentHeight <= 0 );
	    widget->resize( QMAX( currentWidth, 0 ), totalHeight );
	}
	break;
    case Vertical:
	if ( grow ) {
	    currentHeight = totalHeight * checkTime.elapsed() / duration;
	    done = (currentHeight >= totalHeight);
	    widget->resize( totalWidth, QMIN( currentHeight, totalHeight ) );
	} else {
	    currentHeight = totalHeight - totalHeight * checkTime.elapsed() / duration;
	    done = (currentHeight <= 0 );
	    widget->resize( totalWidth, QMAX( currentHeight, 0) );
	}	
	break;
    }
    if ( done ) {
	anim.stop();
	if ( minSize.isValid() )
	    widget->setMinimumSize( minSize );
	if ( !grow )
	    widget->hide();
	delete this;
    }
}

#include "qeffects.moc"

// external statical functions

static QScrollEffect* scroll = 0;

void scrollEffect( QWidget* w, Qt::Orientation orient, int time )
{
    scroll = new QScrollEffect( w, time, orient );
    scroll->run();
}

static QAlphaWidget* blend = 0;

void fadeEffect( QWidget* w, int time )
{
    blend = new QAlphaWidget( w );
    blend->run( time );
}