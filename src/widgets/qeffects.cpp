/****************************************************************************
**
**
** Implementation of QEffects functions
**
** Created : 2000.06.21
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qapplication.h"
#ifndef QT_NO_EFFECTS
#include "qwidget.h"
#include "qeffects_p.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qtimer.h"
#include "qdatetime.h"
#include "qguardedptr.h"

/*
  Internal class to get access to protected QWidget-members
*/

class QAccessWidget : public QWidget
{
    friend class QAlphaWidget;
    friend class QRollEffect;
public:
    QAccessWidget( QWidget* parent=0, const char* name=0, WFlags f = 0 )
	: QWidget( parent, name, f ) {}
};

/*
  Internal class QAlphaWidget.

  The QAlphaWidget is shown while the animation lasts
  and displays the pixmap resulting from the alpha blending.
*/

class QAlphaWidget: public QWidget, private QEffects
{
    Q_OBJECT
public:
    QAlphaWidget( QWidget* w, WFlags f = 0 );

    void run( int time );

protected:
    void paintEvent( QPaintEvent* e );
    void closeEvent( QCloseEvent* );
    bool eventFilter( QObject* o, QEvent* e );
    void alphaBlend();

protected slots:
    void render();
    void goodBye();

private:
    QPixmap pm;
    double alpha;
    QImage back;
    QImage front;
    QImage mixed;
    QGuardedPtr<QAccessWidget> widget;
    int duration;
    int elapsed;
    bool showWidget;
    QTimer anim;
    QTime checkTime;
};

static QAlphaWidget* q_blend = 0;

/*
  Constructs a QAlphaWidget.
*/
QAlphaWidget::QAlphaWidget( QWidget* w, WFlags f )
    : QWidget( 0, "qt internal alpha effect widget", f )
{
    setEnabled( FALSE );

    pm.setOptimization( QPixmap::BestOptim );
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

    if ( !widget )
	return;

    elapsed = 0;
    checkTime.start();

    showWidget = TRUE;
    widget->installEventFilter( this );
    qApp->installEventFilter( this );

    widget->setWState( WState_Visible );
    widget->clearWState( WState_ForceHide );

    move( widget->geometry().x(),widget->geometry().y() );
    resize( widget->size().width(), widget->size().height() );

    front = QImage( widget->size(), 32 );
    front = QPixmap::grabWidget( widget );

    back = QImage( widget->size(), 32 );
    back = QPixmap::grabWindow( QApplication::desktop()->winId(),
				widget->geometry().x(), widget->geometry().y(),
				widget->geometry().width(), widget->geometry().height() );

    if ( !back.isNull() && checkTime.elapsed() < duration / 2 ) {
        mixed = back.copy();
	pm = mixed;
	show();

	connect( &anim, SIGNAL(timeout()), this, SLOT(render()));
	anim.start( 1 );
    } else {
	duration = 0;
	render();
    }
}

/*
  \reimp
*/
bool QAlphaWidget::eventFilter( QObject* o, QEvent* e )
{
    switch ( e->type() ) {
    case QEvent::Move:
	if ( o != widget )
	    break;
	move( widget->geometry().x(),widget->geometry().y() );
	update();
	break;
    case QEvent::Hide:
    case QEvent::Close:
	if ( o != widget )
	    break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
	showWidget = FALSE;
	render();
	break;
    case QEvent::KeyPress:
	{
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape )
		showWidget = FALSE;
	    else
		duration = 0;
	    render();
	    break;
	}
    default:
	break;
    }
    return QWidget::eventFilter( o, e );
}

/*
  \reimp
*/
void QAlphaWidget::closeEvent( QCloseEvent *e )
{
    if ( !q_blend )
	return;

    showWidget = FALSE;
    render();

    QWidget::closeEvent( e );
}

/*
  Render alphablending for the time elapsed.

  Show the blended widget and free all allocated source
  if the blending is finished.
*/
void QAlphaWidget::render()
{
    int tempel = checkTime.elapsed();
    if ( elapsed >= tempel )
        elapsed++;
    else
        elapsed = tempel;

    if ( duration != 0 )
	alpha = tempel / double(duration);
    else
	alpha = 1;
    if ( alpha >= 1 || !showWidget) {
	anim.stop();
	qApp->removeEventFilter( this );
	BackgroundMode bgm;
	QColor erc;
	const QPixmap *erp = 0;
	if ( widget ) {
	    widget->removeEventFilter( this );
	    bgm = widget->backgroundMode();
	    erc = widget->eraseColor();
	    erp = widget->erasePixmap();

	    if ( showWidget ) {
		widget->clearWState( WState_Visible );
		widget->setWState( WState_ForceHide );
		widget->setBackgroundMode( NoBackground );
		widget->show();
	    } else {
		widget->hide();
	    }
	}
	hide();

	if ( showWidget && widget ) {
	    if ( bgm != FixedColor && bgm != FixedPixmap ) {
		widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
		widget->setBackgroundMode( bgm );
		widget->setWState( WState_Visible );
	    } 
	    if ( erc.isValid() ) {
		widget->setEraseColor( erc );
	    } else if ( erp ) {
		widget->setErasePixmap( *erp );
	    }
	}
	q_blend = 0;
	QTimer::singleShot( 0, this, SLOT(goodBye()) );
    } else {
	alphaBlend();
	pm = mixed;
	repaint( FALSE );
    }
}

/*
  Delete this after timout
*/
void QAlphaWidget::goodBye()
{
    delete this;
}

/*
  Calculate an alphablended image.
*/
void QAlphaWidget::alphaBlend()
{
    const double ia = 1-alpha;
    const int sw = front.width();
    const int sh = front.height();
    switch( front.depth() ) {
    case 32:
	{
	    Q_UINT32** md = (Q_UINT32**)mixed.jumpTable();
	    Q_UINT32** bd = (Q_UINT32**)back.jumpTable();
	    Q_UINT32** fd = (Q_UINT32**)front.jumpTable();

	    for (int sy = 0; sy < sh; sy++ ) {
		Q_UINT32* bl = ((Q_UINT32*)bd[sy]);
		Q_UINT32* fl = ((Q_UINT32*)fd[sy]);
		for (int sx = 0; sx < sw; sx++ ) {
		    Q_UINT32 bp = bl[sx];
		    Q_UINT32 fp = fl[sx];

		    ((Q_UINT32*)(md[sy]))[sx] =  qRgb(int (qRed(bp)*ia + qRed(fp)*alpha),
						    int (qGreen(bp)*ia + qGreen(fp)*alpha),
						    int (qBlue(bp)*ia + qBlue(fp)*alpha) );
		}
	    }
	}
    default:
	break;
    }
}

/*
  Internal class QRollEffect

  The QRollEffect widget is shown while the animation lasts
  and displays a scrolling pixmap.
*/

class QRollEffect : public QWidget, private QEffects
{
    Q_OBJECT
public:
    QRollEffect( QWidget* w, WFlags f, DirFlags orient );

    void run( int time );

protected:
    void paintEvent( QPaintEvent* );
    bool eventFilter( QObject*, QEvent* );
    void closeEvent( QCloseEvent* );

private slots:
    void scroll();
    void goodBye();

private:
    QGuardedPtr<QAccessWidget> widget;

    int currentHeight;
    int currentWidth;
    int totalHeight;
    int totalWidth;

    int duration;
    int elapsed;
    bool done;
    bool showWidget;
    int orientation;

    QTimer anim;
    QTime checkTime;

    QPixmap pm;
};

static QRollEffect* q_roll = 0;

/*
  Construct a QRollEffect widget.
*/
QRollEffect::QRollEffect( QWidget* w, WFlags f, DirFlags orient )
    : QWidget( 0, "qt internal roll effect widget", f ), orientation(orient)
{
    setEnabled( FALSE );
    widget = (QAccessWidget*) w;
    Q_ASSERT( widget );

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

    pm.setOptimization( QPixmap::BestOptim );
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
    switch ( e->type() ) {
    case QEvent::Move:
	if ( o != widget )
	    break;
	move( widget->geometry().x(),widget->geometry().y() );
	update();
	break;
    case QEvent::Hide:
    case QEvent::Close:
	if ( o != widget || done )
	    break;
	showWidget = FALSE;
	done = TRUE;
	scroll();
	break;
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonDblClick:
	if ( done )
	    break;
	showWidget = FALSE;
	done = TRUE;
	scroll();
	break;
    case QEvent::KeyPress:
	{
	    QKeyEvent *ke = (QKeyEvent*)e;
	    if ( ke->key() == Key_Escape )
		showWidget = FALSE;
	    done = TRUE;
	    scroll();
	    break;
	}
    default:
	break;
    }
    return QWidget::eventFilter( o, e );
}

/*
  \reimp
*/
void QRollEffect::closeEvent( QCloseEvent *e )
{
    if ( done )
	return;

    showWidget = FALSE;
    done = TRUE;
    scroll();

    QWidget::closeEvent( e );
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
    elapsed = 0;

    if ( duration < 0 )
	duration = QMIN( QMAX((totalWidth - currentWidth) +
			      (totalHeight - currentHeight), 100 ), 300 );
    connect( &anim, SIGNAL(timeout()), this, SLOT(scroll()));

    widget->setWState( WState_Visible );
    widget->clearWState( WState_ForceHide );

    move( widget->geometry().x(),widget->geometry().y() );
    resize( QMIN( currentWidth, totalWidth ), QMIN( currentHeight, totalHeight ) );

    show();

    widget->installEventFilter( this );
    qApp->installEventFilter( this );

    showWidget = TRUE;
    done = FALSE;
    anim.start( 0 );
    checkTime.start();
}

/*
  Roll according to the time elapsed.
*/
void QRollEffect::scroll()
{
    if ( !done ) {
        int tempel = checkTime.elapsed();
        if ( elapsed >= tempel )
            elapsed++;
        else
            elapsed = tempel;

	if ( currentWidth != totalWidth ) {
	    currentWidth = totalWidth * (elapsed/duration)
		+ ( 2 * totalWidth * (elapsed%duration) + duration )
		/ ( 2 * duration );
	    // equiv. to int( (totalWidth*elapsed) / duration + 0.5 )
	    done = (currentWidth >= totalWidth);
	}
	if ( currentHeight != totalHeight ) {
	    currentHeight = totalHeight * (elapsed/duration)
		+ ( 2 * totalHeight * (elapsed%duration) + duration )
		/ ( 2 * duration );
	    // equiv. to int( (totalHeight*elapsed) / duration + 0.5 )
	    done = (currentHeight >= totalHeight);
	}
	done = ( currentHeight >= totalHeight ) &&
	       ( currentWidth >= totalWidth );

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
	qApp->removeEventFilter( this );
	BackgroundMode bgm;
	QColor erc;
	const QPixmap *erp = 0;
	if ( widget ) {
	    widget->removeEventFilter( this );
	    bgm = widget->backgroundMode();
	    erc = widget->eraseColor();
	    erp = widget->erasePixmap();

	    if ( showWidget ) {
		widget->clearWState( WState_Visible );
		widget->setWState( WState_ForceHide );
		widget->setBackgroundMode( NoBackground );
		widget->show();
	    } else {
		widget->hide();
	    }
	}
	hide();

	if ( showWidget && widget ) {
	    if ( bgm != FixedColor && bgm != FixedPixmap ) {
		widget->clearWState( WState_Visible ); // prevent update in setBackgroundMode
		widget->setBackgroundMode( bgm );
		widget->setWState( WState_Visible );
	    } 
	    if ( erc.isValid() ) {
		widget->setEraseColor( erc );
	    } else if ( erp ) {
		widget->setErasePixmap( *erp );
	    }
	}
	q_roll = 0;
	QTimer::singleShot( 0, this, SLOT(goodBye()) );
    }
}

/*
  Delete this after timeout
*/
void QRollEffect::goodBye()
{
    delete this;
}

#include "qeffects.moc"

/*!
  Scroll widget \a w in \a time ms.
  \a orient may be 1 (vertical), 2 (horizontal)
  or 3 (diagonal).
*/
void qScrollEffect( QWidget* w, QEffects::DirFlags orient, int time )
{
    if ( q_roll ) {
	delete q_roll;
	q_roll = 0;
    }

    qApp->sendPostedEvents( w, QEvent::Move );
    qApp->sendPostedEvents( w, QEvent::Resize );

    q_roll = new QRollEffect( w, Qt::WStyle_Customize | Qt::WType_Popup | Qt::WX11BypassWM |
	Qt::WResizeNoErase | Qt::WRepaintNoErase | Qt::WStyle_StaysOnTop, orient );

    q_roll->run( time );
}

/*!
  Fade in widget \a w in \a time ms.
*/
void qFadeEffect( QWidget* w, int time )
{
    if ( q_blend ) {
	delete q_blend;
	q_blend = 0;
    }

    qApp->sendPostedEvents( w, QEvent::Move );
    qApp->sendPostedEvents( w, QEvent::Resize );

    q_blend = new QAlphaWidget( w, Qt::WStyle_Customize | Qt::WType_Popup | Qt::WX11BypassWM |
	Qt::WResizeNoErase | Qt::WRepaintNoErase | Qt::WStyle_StaysOnTop);

    q_blend->run( time );
}
#endif //QT_NO_EFFECTS
