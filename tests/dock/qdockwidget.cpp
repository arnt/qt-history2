#include "qdockwidget.h"
#include "qdockarea.h"

#include <qpainter.h>
#include <qapplication.h>
#include <qtoolbutton.h>

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
				   dockWidget->dockArea->orientation(), FALSE, colorGroup() );
    } else {
	if ( dockWidget->area()->orientation() == Horizontal )
	    style().drawToolBarHandle( &p, QRect( 0, 14, width(), height() - 14 ),
				       dockWidget->dockArea->orientation(), FALSE, colorGroup() );
	else
	    style().drawToolBarHandle( &p, QRect( 0, 1, width() - 14, height() - 1 ),
				       dockWidget->dockArea->orientation(), FALSE, colorGroup() );
    }

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
      wid( 0 ), unclippedPainter( 0 ), dockArea( 0 ), closeEnabled( FALSE ), resizeEnabled( FALSE )
{
    titleBar = new QDockWidgetTitleBar( this );
    handle = new QDockWidgetHandle( this );
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( 2 );
    updateGui();
    setCloseEnabled( TRUE );
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
    QPoint offset( mapFromGlobal( pos ) );
    currRect.moveBy( offset.x(), offset.y() );
    if ( !w || !w->inherits( "QDockArea" ) ) {
	unclippedPainter->setPen( QPen( gray, 3 ) );
	unclippedPainter->drawRect( currRect );
	state = OutsideDock;
	return;
    }

    state = InDock;
    QDockArea *area = (QDockArea*)w;
    unclippedPainter->setPen( QPen( gray, 1 ) );
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
	titleBar->updateGui();
	setLineWidth( 2 );
    } else {
	titleBar->hide();
	if ( dockArea && dockArea->orientation() == Horizontal ) {
	    handle->setMinimumWidth( 14 );
	    handle->setGeometry( 1, 1, handle->sizeHint().width() - 2, height() - 2 );
	    if ( wid )
		wid->setGeometry( handle->width() + 1, 1, width() - handle->width() - 2, height() - 2);
	} else {
	    handle->setMinimumHeight( 14 );
	    handle->setGeometry( 1, 1, width() - 2, handle->sizeHint().height() - 2 );
	    if ( wid )
		wid->setGeometry( 1, handle->height() + 1, width() - 2, height() - handle->height() - 2 );
	}
	handle->show();
	handle->updateGui();
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
    unclippedPainter->setPen( QPen( gray, 3 ) );
    unclippedPainter->setRasterOp( XorROP );

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

#include "qdockwidget.moc"
