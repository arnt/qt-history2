#include "qdockwidget.h"
#include "qdockarea.h"

#include <qpainter.h>
#include <qapplication.h>
#include <qtoolbutton.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qmainwindow.h>

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
						  QDockWidget *w, const char * )
    : QWidget( parent, "qt_dockwidget_internal" ), mousePressed( FALSE ), unclippedPainter( 0 ), dockWidget( w )
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
#ifndef QT_NO_CURSOR
	setCursor( splitVCursor );
#endif
	setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
    } else {
#ifndef QT_NO_CURSOR
	setCursor( splitHCursor );
#endif
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
    if ( mousePressed ) {
	drawLine( lastPos );
	endLineDraw();
	if ( orientation() == Horizontal ) {
	    int dy;
	    if ( dockWidget->area()->gravity() == QDockArea::Normal )
		dy = e->globalPos().y() - firstPos.y();
	    else
		dy =  firstPos.y() - e->globalPos().y();
	    dockWidget->setFixedExtendHeight( dockWidget->height() + dy );
	} else {
	    int dx;
	    if ( dockWidget->area()->gravity() == QDockArea::Normal )
		dx = e->globalPos().x() - firstPos.x();
	    else
		dx = firstPos.x() - e->globalPos().x();
	    dockWidget->setFixedExtendWidth( dockWidget->width() + dx );
	}
	if ( orientation() != dockWidget->area()->orientation() )
	    dockWidget->area()->invalidNextOffset( dockWidget );
    }

    QApplication::postEvent( dockWidget->area(), new QEvent( QEvent::LayoutHint ) );
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
    QPoint starta = dockWidget->area()->mapToGlobal( QPoint( 0, 0 ) );
    if ( orientation() == Horizontal ) {
	if ( orientation() == dockWidget->orientation() )
	    unclippedPainter->drawLine( starta.x() , globalPos.y(), starta.x() + dockWidget->area()->width(), globalPos.y() );
	else
	    unclippedPainter->drawLine( start.x(), globalPos.y(), start.x() + width(), globalPos.y() );
    } else {
	if ( orientation() == dockWidget->orientation() )
	    unclippedPainter->drawLine( globalPos.x(), starta.y(), globalPos.x(), starta.y() + dockWidget->area()->height() );
	else
	    unclippedPainter->drawLine( globalPos.x(), start.y(), globalPos.x(), start.y() + height() );
    }
}




static QPoint realWidgetPos( QWidget *w )
{
    if ( !w->parentWidget() || !w->parentWidget()->inherits( "QDockArea" ) )
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
    QSizePolicy sizePolicy() const;

signals:
    void doubleClicked();

protected:
    void paintEvent( QPaintEvent *e );
    void resizeEvent( QResizeEvent *e );
    void mousePressEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseDoubleClickEvent( QMouseEvent *e );

private:
    QDockWidget *dockWidget;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;
    bool hadDblClick;

};

QDockWidgetHandle::QDockWidgetHandle( QDockWidget *dw )
    : QWidget( dw, "qt_dockwidget_internal" ), dockWidget( dw ), closeButton( 0 )
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
	    style().drawToolBarHandle( &p, QRect( 0, 12, width(), height() - 12 ),
				       dockWidget->orientation(), FALSE, colorGroup() );
	else
	    style().drawToolBarHandle( &p, QRect( 0, 1, width() - 12, height() - 1 ),
				       dockWidget->orientation(), FALSE, colorGroup() );
    }

    QWidget::paintEvent( e );
}

void QDockWidgetHandle::mousePressEvent( QMouseEvent *e )
{
    hadDblClick = FALSE;
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
    if ( !hadDblClick )
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
		 dockWidget, SLOT( hide() ) );
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
	return QSize( dockWidget->isCloseEnabled() ? 18 : 12, 0 );
    return QSize( 0, dockWidget->isCloseEnabled() ? 18 : 12 );
}

QSizePolicy QDockWidgetHandle::sizePolicy() const
{
    if ( dockWidget->orientation() != Horizontal )
	return QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Fixed );
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );
}

void QDockWidgetHandle::mouseDoubleClickEvent( QMouseEvent * )
{
    emit doubleClicked();
    hadDblClick = TRUE;
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
    void mouseDoubleClickEvent( QMouseEvent *e );

signals:
    void doubleClicked();

private:
    QDockWidget *dockWidget;
    QPoint offset;
    bool mousePressed;
    QToolButton *closeButton;
    bool hadDblClick;

};

QDockWidgetTitleBar::QDockWidgetTitleBar( QDockWidget *dw )
    : QWidget( dw, "qt_dockwidget_internal" ), dockWidget( dw ), mousePressed( FALSE ),
      closeButton( 0 )
{
    setMouseTracking( TRUE );
    setMinimumHeight( 13 );
}

void QDockWidgetTitleBar::mousePressEvent( QMouseEvent *e )
{
    mousePressed = TRUE;
    hadDblClick = FALSE;
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
    if ( !hadDblClick )
	dockWidget->updatePosition( e->globalPos() );
}

void QDockWidgetTitleBar::paintEvent( QPaintEvent *e )
{
    QWidget::paintEvent( e );
    QPainter p( this );
    p.fillRect( rect(), colorGroup().highlight() );
    QFont f = p.font();
    f.setPointSize( 10 );
    p.setFont( f );
    p.setPen( colorGroup().highlightedText() );
    p.drawText( 5, height() - p.fontMetrics().descent(), dockWidget->caption() );
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
		 dockWidget, SLOT( hide() ) );
    }

    if ( dockWidget->isCloseEnabled() )
	closeButton->show();
    else
	closeButton->hide();

    closeButton->move( width() - closeButton->width(), 1 );
}

void QDockWidgetTitleBar::mouseDoubleClickEvent( QMouseEvent * )
{
    emit doubleClicked();
    hadDblClick = TRUE;
}


QDockWidget::QDockWidget( Place p, QWidget *parent, const char *name, WFlags f )
    : QFrame( parent, name, f | ( p == OutsideDock ? WStyle_Customize | WStyle_NoBorderEx | WType_TopLevel | WStyle_Dialog : 0 ) ),
      curPlace( p ), wid( 0 ), unclippedPainter( 0 ), dockArea( 0 ), tmpDockArea( 0 ), resizeEnabled( FALSE ), cMode( Never ),
      offs( 0 ), fExtend( -1, -1 ), nl( FALSE ), dockWidgetData( 0 ), lastPos( -1, -1 )
{
    hbox = new QVBoxLayout( this );
    hbox->setMargin( 2 );
    hbox->setSpacing( 1 );
    titleBar = new QDockWidgetTitleBar( this );
    horHandle = new QDockWidgetHandle( this );
    hHandleTop = new QDockWidgetResizeHandle( Qt::Horizontal, this, this, "horz. handle" );
    hbox->addWidget( titleBar );
    hbox->addWidget( horHandle );
    hbox->addWidget( hHandleTop );
    vbox = new QHBoxLayout( hbox );
    verHandle = new QDockWidgetHandle( this );
    vHandleLeft = new QDockWidgetResizeHandle( Qt::Vertical, this, this, "vert. handle" );
    vbox->addWidget( verHandle );
    vbox->addWidget( vHandleLeft );
    layout = new QBoxLayout( vbox, QBoxLayout::LeftToRight );
    vHandleRight = new QDockWidgetResizeHandle( Qt::Vertical, this, this, "vert. handle" );
    vbox->addWidget( vHandleRight );
    hHandleBottom = new QDockWidgetResizeHandle( Qt::Horizontal, this, this, "horz. handle" );
    hbox->addWidget( hHandleBottom );
    hHandleBottom->hide();
    vHandleRight->hide();
    hHandleTop->hide();
    vHandleLeft->hide();
    setFrameStyle( QFrame::StyledPanel | QFrame::Raised );
    setLineWidth( 2 );
    updateGui();
    stretchable[ Horizontal ] = FALSE;
    stretchable[ Vertical ] = FALSE;

    connect( titleBar, SIGNAL( doubleClicked() ), this, SLOT( doDock() ) );
    connect( verHandle, SIGNAL( doubleClicked() ), this, SLOT( doUndock() ) );
    connect( horHandle, SIGNAL( doubleClicked() ), this, SLOT( doUndock() ) );
}

QDockWidget::~QDockWidget()
{
    delete (QDockArea::DockWidgetData*)dockWidgetData;
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

QWidget *QDockWidget::areaAt( const QPoint &gp )
{
    QWidget *w = qApp->widgetAt( gp, TRUE );
    QMainWindow *mw = 0;
    while ( w ) {
	if ( w->inherits( "QDockArea" ) ) {
	    QDockArea *a = (QDockArea*)w;
	    if ( !a->isDockWidgetAccepted( this ) )
		return 0;
	    return w;
	}
	if ( w->inherits( "QMainWindow" ) )
	    mw = (QMainWindow*)w;
	w = w->parentWidget();
    }

    if ( !mw )
	return 0;

    QDockArea *a = mw->dockingArea( mw->mapFromGlobal( gp ) );
    if ( !a || !a->isDockWidgetAccepted( this ) )
	return 0;
    return a;
}

void QDockWidget::handleMoveOutsideDock( const QPoint &pos, const QPoint &gp )
{
    if ( !unclippedPainter )
	return;

    unclippedPainter->drawRect( currRect );
    currRect = QRect( realWidgetPos( this ), size() );
    QWidget *w = areaAt( gp );
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
    if ( startOrientation != ( area ? area->orientation() : Horizontal ) )
	    swapRect( currRect, orientation(), startOffset );
    unclippedPainter->setPen( QPen( gray, 1 ) );
    unclippedPainter->drawRect( currRect );
    tmpDockArea = area;
}

void QDockWidget::updateGui()
{
    if ( curPlace == OutsideDock ) {
 	horHandle->hide();
 	verHandle->hide();
	titleBar->show();
	titleBar->updateGui();
	hHandleTop->hide();
	vHandleLeft->hide();
	hHandleBottom->hide();
	vHandleRight->hide();
	setLineWidth( 2 );
    } else {
	titleBar->hide();
	if ( orientation() == Horizontal ) {
	    horHandle->hide();
	    verHandle->show();
	    verHandle->updateGui();
	} else {
	    horHandle->show();
	    horHandle->updateGui();
	    verHandle->hide();
	}
	if ( isResizeEnabled() ) {
	    if ( orientation() == Horizontal ) {
		hHandleBottom->raise();
		hHandleTop->raise();
	    } else {
		vHandleRight->raise();
		vHandleLeft->raise();
	    }
	
	    if ( area() ) {
		if ( orientation() == Horizontal ) {
		    if ( area()->gravity() == QDockArea::Normal ) {
			hHandleBottom->show();
			hHandleTop->hide();
		    } else {
			hHandleTop->show();
			hHandleBottom->hide();
		    }
		    vHandleRight->show();
		    vHandleLeft->hide();
		} else {
		    if ( area()->gravity() == QDockArea::Normal ) {
			vHandleRight->show();
			vHandleLeft->hide();
		    } else {
			vHandleLeft->show();
			vHandleRight->hide();
		    }
		    hHandleBottom->show();
		    hHandleTop->hide();
		}
	    }
	}
	setLineWidth( 1 );
    }
}

void QDockWidget::updatePosition( const QPoint &globalPos )
{
    if ( state == InDock ) {
	if ( tmpDockArea ) {
	    if ( dockArea && dockArea != tmpDockArea )
		dockArea->removeDockWidget( this, FALSE, FALSE );
	    dockArea = tmpDockArea;
	    dockArea->moveDockWidget( this, globalPos, currRect, startOrientation != orientation() );
	}
    } else {
	if ( dockArea )
	    dockArea->removeDockWidget( this, TRUE, startOrientation != Horizontal );
	dockArea = 0;
	move( currRect.topLeft() );
	show();
    }
    curPlace = state;
    updateGui();
    emit orientationChanged( orientation() );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    if ( state == OutsideDock )
	adjustSize();
    emit positionChanged();
    tmpDockArea = 0;
}

void QDockWidget::setWidget( QWidget *w )
{
    wid = w;
    boxLayout()->addWidget( w );
    updateGui();
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
    updateGui();
}

bool QDockWidget::isResizeEnabled() const
{
    return resizeEnabled;
}

void QDockWidget::setCloseMode( int m )
{
    cMode = m;
    if ( place() == InDock ) {
	horHandle->updateGui();
	verHandle->updateGui();
    } else {
	titleBar->updateGui();
    }
}

bool QDockWidget::isCloseEnabled() const
{
    return  ( ( cMode & Docked ) == Docked && place() == InDock ||
	      ( cMode & Undocked ) == Undocked && place() == OutsideDock );
}

int QDockWidget::closeMode() const
{
    return closeMode();
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

void QDockWidget::setNewLine( bool b )
{
    nl = b;
}

bool QDockWidget::newLine() const
{
    return nl;
}

QBoxLayout *QDockWidget::boxLayout()
{
    return layout;
}

QSize QDockWidget::sizeHint() const
{
    QSize sh( QFrame::sizeHint() );
    sh = sh.expandedTo( fixedExtend() );
    sh = sh.expandedTo( QSize( 16, 16 ) );
    return sh;
}

QSize QDockWidget::minimumSize() const
{
    QSize ms( QFrame::minimumSize() );
    ms = ms.expandedTo( fixedExtend() );
    ms = ms.expandedTo( QSize( 16, 16 ) );
    return ms;
}

QSize QDockWidget::minimumSizeHint() const
{
    QSize msh( QFrame::minimumSize() );
    msh = msh.expandedTo( fixedExtend() );
    msh = msh.expandedTo( QSize( 16, 16 ) );
    return msh;
}

void QDockWidget::dock()
{
    if ( place() == InDock )
	return;
    qWarning( "QDockWidget::dock() not implemented yet!" );
}

void QDockWidget::undock()
{
    if ( place() == OutsideDock )
	return;

    QPoint p( 50, 50 );
    if ( topLevelWidget() )
	p = topLevelWidget()->pos() + QPoint( 20, 20 );
    if ( dockArea ) {
	delete (QDockArea::DockWidgetData*)dockWidgetData;
	dockWidgetData = dockArea->dockWidgetData( this );
	dockArea->removeDockWidget( this, TRUE, orientation() != Horizontal );
    }
    dockArea = 0;
    if ( lastPos != QPoint( -1, -1 ) )
	move( lastPos );
    else
	move( p );
    curPlace = OutsideDock;
    updateGui();
    emit orientationChanged( orientation() );
    QApplication::sendPostedEvents( this, QEvent::LayoutHint );
    adjustSize();
    show();
}

void QDockWidget::removeFromDock()
{
    if ( dockArea )
	dockArea->removeDockWidget( this, FALSE, FALSE );
}

void QDockWidget::doUndock()
{
    undock();
}

void QDockWidget::doDock()
{
    if ( !(QDockArea::DockWidgetData*)dockWidgetData ||
	 !( (QDockArea::DockWidgetData*)dockWidgetData )->area )
	return;
    curPlace = InDock;
    lastPos = pos();
    ( (QDockArea::DockWidgetData*)dockWidgetData )->
	area->dockWidget( this, (QDockArea::DockWidgetData*)dockWidgetData );
    emit orientationChanged( orientation() );
}

void QDockWidget::hideEvent( QHideEvent *e )
{
    QFrame::hideEvent( e );
    emit visibilityChanged( FALSE );
}

void QDockWidget::showEvent( QShowEvent *e )
{
    QFrame::showEvent( e );
    emit visibilityChanged( TRUE );
}

#include "qdockwidget.moc"
