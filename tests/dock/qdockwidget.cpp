#include "qdockwidget.h"
#include "qdockarea.h"

#include <qpainter.h>
#include <qapplication.h>

static QPoint realWidgetPos( QWidget *w )
{
    if ( !w->parentWidget() )
	return w->pos();
    return w->parentWidget()->mapToGlobal( w->pos() );
}

class QDockWidgetHandle : public QWidget
{
    Q_OBJECT

public:
    QDockWidgetHandle( QDockWidget *dw );

protected:
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    QDockWidget *dockWidget;
    QPoint offset;
    bool mousePressed;

};

QDockWidgetHandle::QDockWidgetHandle( QDockWidget *dw )
    : QWidget( dw ), dockWidget( dw )
{
}

void QDockWidgetHandle::paintEvent( QPaintEvent *e )
{
    if ( !dockWidget->dockArea )
	return;
    QPainter p( this );
    style().drawToolBarHandle( &p, QRect( 0, 0, width(), height() ),
 			       dockWidget->dockArea->orientation(), FALSE, colorGroup() );
    QWidget::paintEvent( e );
}

void QDockWidgetHandle::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    offset = e->pos();
    dockWidget->startRectDraw();
}

void QDockWidgetHandle::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    dockWidget->handleMoveOutsideDock( e->globalPos() - offset, e->globalPos() );
}

void QDockWidgetHandle::mouseReleaseEvent( QMouseEvent * )
{
    dockWidget->endRectDraw();
    mousePressed = FALSE;
    dockWidget->updatePosition();
}



class QDockWidgetTitleBar : public QWidget
{
    Q_OBJECT

public:
    QDockWidgetTitleBar( QDockWidget *dw );

protected:
    void paintEvent( QPaintEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );

private:
    QDockWidget *dockWidget;
    QPoint offset;
    bool mousePressed;

};

QDockWidgetTitleBar::QDockWidgetTitleBar( QDockWidget *dw )
    : QWidget( dw ), dockWidget( dw ), mousePressed( FALSE )
{
    setMouseTracking( TRUE );
    setMinimumHeight( 12 );
}

void QDockWidgetTitleBar::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    offset = e->pos();
    dockWidget->startRectDraw();
}

void QDockWidgetTitleBar::mouseMoveEvent( QMouseEvent *e )
{
    if ( !mousePressed )
	return;
    dockWidget->handleMoveOutsideDock( e->globalPos() - offset, e->globalPos() );
}

void QDockWidgetTitleBar::mouseReleaseEvent( QMouseEvent * )
{
    dockWidget->endRectDraw();
    mousePressed = FALSE;
    dockWidget->updatePosition();
}

void QDockWidgetTitleBar::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );
    QPainter p( this );
    p.fillRect( rect(), colorGroup().highlight() );
}


QDockWidget::QDockWidget( QWidget *parent, const char *name )
    : QFrame( parent, name, WStyle_Customize | WStyle_NoBorderEx ), curPlace( OutsideDock ),
      wid( 0 ), unclippedPainter( 0 ), dockArea( 0 )
{
    titleBar = new QDockWidgetTitleBar( this );
    handle = new QDockWidgetHandle( this );
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( 2 );
    updateGui();
}

void QDockWidget::resizeEvent( QResizeEvent *e )
{
    QFrame::resizeEvent( e );
    updateGui();
}

void QDockWidget::handleMoveInDock( const QPoint &pos )
{
}

void QDockWidget::handleMoveOutsideDock( const QPoint &pos, const QPoint &gp )
{
    if ( !unclippedPainter )
	return;

    unclippedPainter->drawRect( currRect );
    currRect = QRect( realWidgetPos( this ), size() );
    QWidget *w = qApp->widgetAt( gp );
    if ( !w || !w->inherits( "QDockArea" ) ) {
	QPoint offset( mapFromGlobal( pos ) );
	currRect.moveBy( offset.x(), offset.y() );
	unclippedPainter->drawRect( currRect );
	state = OutsideDock;
	return;
    }

    state = InDock;
    QDockArea *area = (QDockArea*)w;
    currRect = QRect( realWidgetPos( area ), size() );
    unclippedPainter->drawRect( currRect );
    tmpDockArea = area;
}

void QDockWidget::updateGui()
{
    if ( curPlace == OutsideDock ) {
	handle->hide();
	titleBar->setGeometry( 2, 2, width() - 4, titleBar->sizeHint().height() - 4 );
	if ( wid )
	    wid->setGeometry( 2, titleBar->height() + 2, width() - 4, height() - titleBar->height() - 4 );
	titleBar->show();
	setLineWidth( 2 );
    } else {
	titleBar->hide();
	if ( dockArea && dockArea->orientation() == Horizontal ) {
	    handle->setMinimumWidth( style().toolBarHandleExtend() );
	    handle->setGeometry( 1, 1, handle->sizeHint().width() - 2, height() - 2 );
	    if ( wid )
		wid->setGeometry( handle->width() + 1, 1, width() - handle->width() - 2, height() - 2);
	} else {
	    handle->setMinimumHeight( style().toolBarHandleExtend() );
	    handle->setGeometry( 1, 1, width() - 2, handle->sizeHint().height() - 2 );
	    if ( wid )
		wid->setGeometry( 1, handle->height() + 1, width() - 2, height() - handle->height() - 2 );
	}
	handle->show();
	setLineWidth( 1 );
    }
}

void QDockWidget::updatePosition()
{
    if ( state == InDock ) {
	if ( dockArea && dockArea != tmpDockArea )
	    dockArea->removeDockWidget( this, FALSE );
	dockArea = tmpDockArea;
	dockArea->moveDockWidget( this );
    } else {
	if ( dockArea )
	    dockArea->removeDockWidget( this, TRUE );
	dockArea = 0;
	show();
	move( currRect.topLeft() );
    }
    curPlace = state;
    updateGui();
}

void QDockWidget::setWidget( QWidget *w )
{
    wid = w;
    updateGui();
}

QWidget *QDockWidget::widget() const
{
    return wid;
}

void QDockWidget::startRectDraw()
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
    unclippedPainter->setPen( QPen( color0, 3 ) );
    unclippedPainter->setRasterOp( NotROP );

    currRect = QRect( realWidgetPos( this ), size() );
    unclippedPainter->drawRect( currRect );
}

void QDockWidget::endRectDraw()
{
    if ( !unclippedPainter )
	return;
    unclippedPainter->drawRect( currRect );
    delete unclippedPainter;
    unclippedPainter = 0;
}

#include "qdockwidget.moc"
