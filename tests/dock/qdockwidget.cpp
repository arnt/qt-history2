#include "qdockwidget.h"
#include "qdockarea.h"

#include <qpainter.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>

static const char * close_xpm[] = {
"8 8 2 1",
"# c #000000",
". c None",
"##....##",
".##..##.",
"..####..",
"...##...",
"..####..",
".##..##.",
"##....##",
"........"};

class QDockWidgetResizeHandle : public QWidget
{
public:
    QDockWidgetResizeHandle( Qt::Orientation o, QWidget *parent, QDockWidget *w, const char* name=0 );
    void setOrientation( Qt::Orientation o );
    Qt::Orientation orientation() const { return orient; }

    QSize sizeHint() const;

protected:
    void paintEvent( QPaintEvent * );
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseReleaseEvent( QMouseEvent * );

private:
    void startLineDraw();
    void endLineDraw();
    void drawLine( const QPoint &globalPos );

private:
    Qt::Orientation orient;
    bool mousePressed;
    QPainter *unclippedPainter;
    QPoint lastPos, firstPos;
    QDockWidget *dockWidget;

};

QDockWidgetResizeHandle::QDockWidgetResizeHandle( Qt::Orientation o, QWidget *parent, 
						  QDockWidget *w, const char * name )
    : QWidget( parent, name ), mousePressed( FALSE ), unclippedPainter( 0 ), dockWidget( w )
{
    setOrientation( o );
}

QSize QDockWidgetResizeHandle::sizeHint() const
{
    int sw = style().splitterWidth();
    return QSize(sw,sw).expandedTo( QApplication::globalStrut() );
}

void QDockWidgetResizeHandle::setOrientation( Qt::Orientation o )
{
    orient = o;
    if ( o == QDockArea::Horizontal ) {
	setCursor( splitVCursor );
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
	setCursor( splitHCursor );
	setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding ) );
    }
}

void QDockWidgetResizeHandle::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    startLineDraw();
    lastPos = firstPos = e->globalPos();
    drawLine( e->globalPos() );
}

void QDockWidgetResizeHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    drawLine( lastPos );
    lastPos = e->globalPos();
    drawLine( e->globalPos() );
}

void QDockWidgetResizeHandle::mouseReleaseEvent( QMouseEvent *e )
{
#if 0
    if ( mousePressed ) {
	drawLine( lastPos );
	endLineDraw();
	if ( orientation() == Horizontal ) {
	    int dy = e->globalPos().y() - firstPos.y();
	    if ( s->orientation() == orientation() ) {
		for ( QWidget *w = widgetList.first(); w; w = widgetList.next() ) {
		    ( (QDockWidget*)w )->unsetSizeHint();
		    ( (QDockWidget*)w )->setSizeHint( QSize( w->width(), w->height() + dy ) );
		}
	    } else {
		int dy = e->globalPos().y() - firstPos.y();
		QDockWidget *dw = (QDockWidget*)widgetList.first();
		dw->unsetSizeHint();
		dw->setSizeHint( QSize( dw->width(), dw->height() + dy ) );
		dw = (QDockWidget*)widgetList.next();
		if ( dw )
		    dw->setSizeHint( QSize( dw->width(), dw->height() - dy ) );
	    }
	} else {
	    int dx = e->globalPos().x() - firstPos.x();
	    if ( s->orientation() == orientation() ) {
		for ( QWidget *w = widgetList.first(); w; w = widgetList.next() ) {
		    ( (QDockWidget*)w )->unsetSizeHint();
		    ( (QDockWidget*)w )->setSizeHint( QSize( w->width() + dx, w->height() ) );
		}
	    } else {
		int dx = e->globalPos().x() - firstPos.x();
		QDockWidget *dw = (QDockWidget*)widgetList.first();
		dw->unsetSizeHint();
		dw->setSizeHint( QSize( dw->width() + dx, dw->height() ) );
		dw = (QDockWidget*)widgetList.next();
		if ( dw )
		    dw->setSizeHint( QSize( dw->width() - dx, dw->height() ) );
	    }
	}
    }

    s->QWidget::layout()->invalidate();
    s->QWidget::layout()->activate();
#endif
    mousePressed = FALSE;
}

void QDockWidgetResizeHandle::paintEvent( QPaintEvent * )
{
    QPainter p( this );
    style().drawSplitter( &p, 0, 0, width(), height(), colorGroup(), orientation() == Horizontal ? Vertical : Horizontal );
}

void QDockWidgetResizeHandle::startLineDraw()
{
    if ( unclippedPainter )
	endLineDraw();
    bool unclipped = QApplication::desktop()->testWFlags( WPaintUnclipped );
    ( (QDockWidgetResizeHandle*)QApplication::desktop() )->setWFlags( WPaintUnclipped );
    unclippedPainter = new QPainter;
    unclippedPainter->begin( QApplication::desktop() );
    if ( !unclipped )
	( (QDockWidgetResizeHandle*)QApplication::desktop() )->clearWFlags( WPaintUnclipped );
    unclippedPainter->setPen( QPen( gray, orientation() == Horizontal ? height() : width() ) );
    unclippedPainter->setRasterOp( XorROP );
}

void QDockWidgetResizeHandle::endLineDraw()
{
    if ( !unclippedPainter )
	return;
    delete unclippedPainter;
    unclippedPainter = 0;
}

void QDockWidgetResizeHandle::drawLine( const QPoint &globalPos )
{
    QPoint start = mapToGlobal( QPoint( 0, 0 ) );
    if ( orientation() == Horizontal )
	unclippedPainter->drawLine( start.x(), globalPos.y(), start.x() + width(), globalPos.y() );
    else
	unclippedPainter->drawLine( globalPos.x(), start.y(), globalPos.x(), start.y() + height() );
}




static QPoint realWidgetPos( QWidget *w )
{
    if ( !w->parentWidget() )
	return w->pos();
    return w->parentWidget()->mapToGlobal( w->geometry().topLeft() );
}

class QDockWidgetHandle : public QWidget
{
    Q_OBJECT

public:
    QDockWidgetHandle( QDockWidget *dw );
    void updateGui();

    QSize minimumSizeHint() const;
    QSize minimumSize() const { return minimumSizeHint(); }
    QSize sizeHint() const { return minimumSize(); }

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    QDockWidget *dockWidget;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;

};

QDockWidgetHandle::QDockWidgetHandle( QDockWidget *dw )
    : QWidget( dw ), dockWidget( dw ), closeButton( 0 )
{
}

void QDockWidgetHandle::paintEvent( QPaintEvent *e )
{
    if ( !dockWidget->dockArea )
	return;
    QPainter p( this );
    if ( !dockWidget->area() || !dockWidget->isCloseEnabled() ) {
	style().drawToolBarHandle( &p, QRect( 0, 0, width(), height() ),
				   dockWidget->orientation(), FALSE, colorGroup() );
    } else {
	if ( dockWidget->area()->orientation() == Horizontal )
	    style().drawToolBarHandle( &p, QRect( 0, 14, width(), height() - 14 ),
				       dockWidget->orientation(), FALSE, colorGroup() );
	else
	    style().drawToolBarHandle( &p, QRect( 0, 1, width() - 14, height() - 1 ),
				       dockWidget->orientation(), FALSE, colorGroup() );
    }

    QWidget::paintEvent( e );
}

void QDockWidgetHandle::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    offset = e->pos();
    dockWidget->startRectDraw( e->pos() );
}

void QDockWidgetHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    dockWidget->handleMoveOutsideDock( e->globalPos() - offset, e->globalPos() );
}

void QDockWidgetHandle::mouseReleaseEvent( QMouseEvent *e )
{
    dockWidget->endRectDraw();
    mousePressed = FALSE;
    dockWidget->updatePosition( e->globalPos() );
}

void QDockWidgetHandle::resizeEvent( QResizeEvent * )
{
    updateGui();
}

void QDockWidgetHandle::updateGui()
{
    if ( !closeButton ) {
	closeButton = new QToolButton( this );
	closeButton->setPixmap( close_xpm );
	closeButton->setFixedSize( 12, 12 );
	connect( closeButton, SIGNAL( clicked() ),
		 dockWidget, SLOT( close() ) );
    }

    if ( dockWidget->isCloseEnabled() && dockWidget->area() )
	closeButton->show();
    else
	closeButton->hide();

    if ( !dockWidget->area() )
	return;

    if ( dockWidget->area()->orientation() == Horizontal )
	closeButton->move( 2, 2 );
    else
	closeButton->move( width() - closeButton->width() - 2, 2 );
}

QSize QDockWidgetHandle::minimumSizeHint() const
{
    if ( dockWidget->orientation() == Horizontal )
	return QSize( dockWidget->isCloseEnabled() ? 18 : 14, 0 );
    return QSize( 0, dockWidget->isCloseEnabled() ? 18 : 14 );
}


class QDockWidgetTitleBar : public QWidget
{
    Q_OBJECT

public:
    QDockWidgetTitleBar( QDockWidget *dw );
    void updateGui();

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    QDockWidget *dockWidget;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;

};

QDockWidgetTitleBar::QDockWidgetTitleBar( QDockWidget *dw )
    : QWidget( dw ), dockWidget( dw ), mousePressed( FALSE ),
      closeButton( 0 )
{
    setMouseTracking( TRUE );
    setMinimumHeight( 13 );
}

void QDockWidgetTitleBar::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    offset = e->pos();
    dockWidget->startRectDraw( e->pos() );
}

void QDockWidgetTitleBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    dockWidget->handleMoveOutsideDock( e->globalPos() - offset, e->globalPos() );
}

void QDockWidgetTitleBar::mouseReleaseEvent( QMouseEvent *e )
{
    dockWidget->endRectDraw();
    mousePressed = FALSE;
    dockWidget->updatePosition( e->globalPos() );
}

void QDockWidgetTitleBar::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );
    QPainter p( this );
    p.fillRect( rect(), colorGroup().highlight() );
}

void QDockWidgetTitleBar::resizeEvent( QResizeEvent * )
{
    updateGui();
}

void QDockWidgetTitleBar::updateGui()
{
    if ( !closeButton ) {
	closeButton = new QToolButton( this );
	closeButton->setPixmap( close_xpm );
	closeButton->setFixedSize( 12, 12 );
	connect( closeButton, SIGNAL( clicked() ),
		 dockWidget, SLOT( close() ) );
    }

    if ( dockWidget->isCloseEnabled() )
	closeButton->show();
    else
	closeButton->hide();

    closeButton->move( width() - closeButton->width(), 1 );
}



QDockWidget::QDockWidget( QWidget *parent, const char *name )
    : QFrame( parent, name, WStyle_Customize | WStyle_NoBorderEx ), curPlace( OutsideDock ),
      wid( 0 ), unclippedPainter( 0 ), dockArea( 0 ), closeEnabled( FALSE ), resizeEnabled( FALSE ),
      addY( 0 ), addX( 0 ), offs( 0 ), fExtend( -1, -1 )
{
    titleBar = new QDockWidgetTitleBar( this );
    handle = new QDockWidgetHandle( this );
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( 2 );
    updateGui();
    setCloseEnabled( TRUE );
    setResizeEnabled( TRUE );
    stretchable[ Horizontal ] = FALSE;
    stretchable[ Vertical ] = FALSE;
}

void QDockWidget::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    updateGui();
}

void QDockWidget::handleMoveInDock( const QPoint & )
{
}

static void swapRect( QRect &r, Qt::Orientation o, const QPoint &offset )
{
    int w = r.width();
    if ( o == Qt::Horizontal )
	r.moveBy( -r.height() / 2, 0 );
    else
	r.moveBy( 0, -r.width() / 2 );
    r.moveBy( offset.x(), offset.y() );
    r.setWidth( r.height() );
    r.setHeight( w );
}

static QWidget *widgetAt( const QPoint &gp )
{
    QWidget *w = qApp->widgetAt( gp, TRUE );
    while ( w ) {
	if ( w->inherits( "QDockArea" ) )
	    return w;
	w = w->parentWidget();
    }
    return 0;
}

void QDockWidget::handleMoveOutsideDock( const QPoint &pos, const QPoint &gp )
{
    if ( !unclippedPainter )
	return;

    unclippedPainter->drawRect( currRect );
    currRect = QRect( realWidgetPos( this ), size() );
    QWidget *w = widgetAt( gp );
    QPoint offset( mapFromGlobal( pos ) );
    currRect.moveBy( offset.x(), offset.y() );
    if ( !w || !w->inherits( "QDockArea" ) ) {
	if ( startOrientation != Horizontal )
	    swapRect( currRect, Horizontal, startOffset );
	unclippedPainter->setPen( QPen( gray, 3 ) );
	unclippedPainter->drawRect( currRect );
	state = OutsideDock;
	return;
    }

    state = InDock;
    QDockArea *area = (QDockArea*)w;
    if ( startOrientation != orientation() )
	    swapRect( currRect, orientation(), startOffset );
    unclippedPainter->setPen( QPen( gray, 1 ) );
    unclippedPainter->drawRect( currRect );
    tmpDockArea = area;
}

void QDockWidget::updateGui()
{
    addX = addY = 2;
    if ( curPlace == OutsideDock ) {
	addX++;
	addY++;
	handle->hide();
	titleBar->setGeometry( 2, 2, width() - 4, titleBar->sizeHint().height() - 4 );
	if ( wid )
	    wid->setGeometry( 2, titleBar->height() + 2, width() - 4, height() - titleBar->height() - 4 );
	titleBar->show();
	titleBar->updateGui();
	setLineWidth( 2 );
	addY += titleBar->height();
    } else {
	titleBar->hide();
	if ( dockArea && orientation() == Horizontal ) {
	    handle->setGeometry( 1, 1, handle->sizeHint().width() - 2, height() - 2 );
	    if ( wid )
		wid->setGeometry( handle->width() + 1, 1, width() - handle->width() - 2, height() - 2);
	    addX += handle->width();
	} else {
	    handle->setGeometry( 1, 1, width() - 2, handle->sizeHint().height() - 2 );
	    if ( wid )
		wid->setGeometry( 1, handle->height() + 1, width() - 2, height() - handle->height() - 2 );
	    addY += handle->height();
	}
	handle->show();
	handle->updateGui();
	setLineWidth( 1 );
    }
}

void QDockWidget::updatePosition( const QPoint &globalPos )
{
    if ( state == InDock ) {
	if ( dockArea && dockArea != tmpDockArea )
	    dockArea->removeDockWidget( this, FALSE, FALSE );
	dockArea = tmpDockArea;
	dockArea->moveDockWidget( this, globalPos, currRect, startOrientation != orientation() );
    } else {
	if ( dockArea )
	    dockArea->removeDockWidget( this, TRUE, startOrientation != Horizontal );
	dockArea = 0;
	move( currRect.topLeft() );
	show();
    }
    curPlace = state;
    updateGui();
    if ( state == OutsideDock )
	adjustSize();
    emit orientationChanged( orientation() );
}

void QDockWidget::setWidget( QWidget *w )
{
    wid = w;
    updateGui();

    if ( w->inherits( "QToolBar" ) )
	QObject::connect( this, SIGNAL( orientationChanged( Orientation ) ),
			  (QToolBar*)w, SLOT( setOrientation( Orientation ) ) );
}

QWidget *QDockWidget::widget() const
{
    return wid;
}

void QDockWidget::startRectDraw( const QPoint &so )
{
    state = place();
    if ( unclippedPainter )
	endRectDraw();
    bool unclipped = QApplication::desktop()->testWFlags( WPaintUnclipped );
    ( (QDockWidget*)QApplication::desktop() )->setWFlags( WPaintUnclipped );
    unclippedPainter = new QPainter;
    unclippedPainter->begin( QApplication::desktop() );
    if ( !unclipped )
	( (QDockWidget*)QApplication::desktop() )->clearWFlags( WPaintUnclipped );
    unclippedPainter->setPen( QPen( gray, 3 ) );
    unclippedPainter->setRasterOp( XorROP );

    currRect = QRect( realWidgetPos( this ), size() );
    unclippedPainter->drawRect( currRect );
    startRect = currRect;
    startOrientation = orientation();
    startOffset = so;
}

void QDockWidget::endRectDraw()
{
    if ( !unclippedPainter )
	return;
    unclippedPainter->drawRect( currRect );
    delete unclippedPainter;
    unclippedPainter = 0;
}

void QDockWidget::setResizeEnabled( bool b )
{
    resizeEnabled = b;
}

bool QDockWidget::isResizeEnabled() const
{
    return resizeEnabled;
}

void QDockWidget::setCloseEnabled( bool b )
{
    closeEnabled = b;
}

bool QDockWidget::isCloseEnabled() const
{
    return closeEnabled;
}

QSize QDockWidget::sizeHint() const
{
    if ( !wid )
	return QFrame::sizeHint();
    QSize s = wid->sizeHint();
    s += QSize( addX, addY );

    return s;
}

QSize QDockWidget::minimumSize() const
{
    if ( !wid )
	return QFrame::minimumSize();
    if ( orientation() == Horizontal )
	return QSize( 20, QFrame::minimumSize().height() );
    return QSize( QFrame::minimumSize().width(), 20 );
}

QSize QDockWidget::minimumSizeHint() const
{
    return minimumSize();
}

void QDockWidget::setHorizontalStretchable( bool b )
{
    stretchable[ Horizontal ] = b;
}

void QDockWidget::setVerticalStretchable( bool b )
{
    stretchable[ Vertical ] = b;
}

bool QDockWidget::isHorizontalStretchable() const
{
    return isResizeEnabled() || stretchable[ Horizontal ];
}

bool QDockWidget::isVerticalStretchable() const
{
    return isResizeEnabled() || stretchable[ Vertical ];
}

bool QDockWidget::isStretchable() const
{
    if ( orientation() == Horizontal )
	return isHorizontalStretchable();
    return isVerticalStretchable();
}

Qt::Orientation QDockWidget::orientation() const
{
    if ( !dockArea || dockArea->orientation() == Horizontal )
	return Horizontal;
    return Vertical;
}

int QDockWidget::offset() const
{
    return offs;
}

void QDockWidget::setOffset( int o ) 
{
    offs = o;
}

QSize QDockWidget::fixedExtend() const
{
    return fExtend;
}

void QDockWidget::setFixedExtendWidth( int w )
{
    fExtend.setWidth( w );
}

void QDockWidget::setFixedExtendHeight( int h )
{
    fExtend.setHeight( h );
}

bool QDockWidget::hasFixedExtend() const
{
    if ( orientation() == Horizontal )
	return fExtend.width() != -1;
    return fExtend.height() != -1;
}

#include "qdockwidget.moc"
