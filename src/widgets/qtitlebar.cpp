/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtitlebar.cpp#43 $
**
** Implementation of some Qt private functions.
**
** Created : 001101
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

// Get the system specific includes and defines
#include "qplatformdefs.h"
#include "qtitlebar_p.h"

#ifndef QT_NO_TITLEBAR

#include "qapplication.h"
#include "qdatetime.h"
#include "../kernel/qapplication_p.h"
#include "qtooltip.h"
#include "qimage.h"
#include "qtimer.h"
#include "qpainter.h"
#include "../kernel/qinternal_p.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessiblewidget.h"
#endif
#ifndef QT_NO_WORKSPACE
#include "qworkspace.h"
#endif


class QTitleBarTip : public QToolTip
{
public:
    QTitleBarTip( QWidget * parent ) : QToolTip( parent ) { }

    void maybeTip( const QPoint &pos )
    {
	if ( !parentWidget()->inherits( "QTitleBar" ) )
	    return;
	QTitleBar *t = (QTitleBar *)parentWidget();
	int controlWidth, controlHeight, titleHeight, titleWidth;
	t->style().titleBarMetrics(t, controlWidth, controlHeight, titleWidth, titleHeight);

	QString tipstring;
	switch(t->style().titleBarPointOver( t, pos )) {
	case QStyle::TitleNone:
	    break;
	case QStyle::TitleSysMenu:
	    tipstring = t->tr("System Menu");
	    break;
	case QStyle::TitleShadeButton:
	    tipstring = t->tr("Shade");
	    break;
	case QStyle::TitleUnshadeButton:
	    tipstring = t->tr("Unshade");
	    break;
	case QStyle::TitleNormalButton:
	case QStyle::TitleMinButton:
	    if(t->window->isMinimized())
		tipstring = t->tr("Normalize");
	    else
		tipstring = t->tr("Minimize");
	    break;
	case QStyle::TitleMaxButton:
	    tipstring = t->tr("Maximize");
	    break;
	case QStyle::TitleCloseButton:
	    tipstring = t->tr("Close");
	    break;
	case QStyle::TitleLabel:
	    if(t->cuttext != t->text)
		tipstring = t->text;
	    break;
	}
	if(!tipstring.isEmpty())
	    tip( QRect(pos, QSize(controlWidth, controlHeight)), tipstring );
    }
};

QTitleBar::QTitleBar (QWidget* w, QWidget* parent, const char* name)
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder | WResizeNoErase | WRepaintNoErase )
{
    toolTip = new QTitleBarTip( this );
    window = w;
    buttonDown = 0;
    act = 0;
    getColors();
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
}

QTitleBar::~QTitleBar()
{
    delete toolTip;
}

#ifdef Q_WS_WIN
extern QRgb qt_colorref2qrgb(COLORREF col);
#endif

void QTitleBar::getColors()
{
#ifdef Q_WS_WIN
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
	    if ( qt_winver & Qt::WV_NT_based )
		SystemParametersInfo( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
	    else
		SystemParametersInfoA( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );

	    if ( gradient ) {
		arightc = qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTACTIVECAPTION));
		irightc = qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTINACTIVECAPTION));
	    }
	}
    }
#endif // Q_WS_WIN
    setActive( act );
}

void QTitleBar::mousePressEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	emit doActivate();
	int ctrl = (int)style().titleBarPointOver(this, e->pos());
	switch(ctrl) {
	case QStyle::TitleSysMenu:
	{
	    buttonDown = 0;
	    static QTime* t = 0;
	    static QTitleBar* tc = 0;
	    if ( !t )
		t = new QTime;
	    if ( tc != this || t->elapsed() > QApplication::doubleClickInterval() ) {
		emit showOperationMenu();
		t->start();
		tc = this;
	    } else {
		tc = 0;
		emit doClose();
		return;
	    }
	    break;
	}
	case QStyle::TitleShadeButton:
	case QStyle::TitleUnshadeButton:
	case QStyle::TitleNormalButton:
	case QStyle::TitleMinButton:
	    if(window->isMinimized())
		buttonDown = QStyle::TitleNormalButton;
	    else
		buttonDown = ctrl;
	    break;
	case QStyle::TitleMaxButton:
	case QStyle::TitleCloseButton:
	    buttonDown = ctrl;
	    break;
	case QStyle::TitleLabel:
	    buttonDown = ctrl;
	    moveOffset = mapToParent( e->pos() );
	    break;
	}

	QPainter p(this);
	style().drawTitleBarControls(&p, this, buttonDown, buttonDown);
	p.end();

    }
}

void QTitleBar::contextMenuEvent( QContextMenuEvent *e )
{
    int ctrl = style().titleBarPointOver(this, e->pos());
    if(ctrl == QStyle::TitleLabel) {
	emit popupOperationMenu(e->globalPos());
	e->accept();
    }
}


void QTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	int ctrl = style().titleBarPointOver(this, e->pos());
	if(ctrl == QStyle::TitleMinButton && window && window->isMinimized())
	    ctrl = QStyle::TitleNormalButton;
	if(ctrl == buttonDown) {
	    QPainter p(this);
	    style().drawTitleBarControls(&p, this, buttonDown, 0);
	    p.end();
		
	    switch(ctrl) {
	    case QStyle::TitleSysMenu:
		break;
	    case QStyle::TitleShadeButton:
		emit doShade();
		break;
	    case QStyle::TitleUnshadeButton:
	    case QStyle::TitleNormalButton:
		emit doNormal();
		break;
	    case QStyle::TitleMinButton:
		emit doMinimize();
		break;
	    case QStyle::TitleMaxButton:
		emit doMaximize();
		break;
	    case QStyle::TitleCloseButton:
		emit doClose();
		return;
	    case QStyle::TitleLabel:
		break;
	    }
	}
	buttonDown = 0;
	releaseMouse();
    }
}

void QTitleBar::mouseMoveEvent( QMouseEvent * e)
{
    if ( !buttonDown )
	return;

    switch(buttonDown) {
    case QStyle::TitleSysMenu:
	break;
    case QStyle::TitleShadeButton:
    case QStyle::TitleUnshadeButton:
    case QStyle::TitleNormalButton:
    case QStyle::TitleMinButton:
    case QStyle::TitleMaxButton:
    case QStyle::TitleCloseButton:
    {
	int ctrl = style().titleBarPointOver(this, e->pos());
	if(window && ctrl == QStyle::TitleMinButton && window->isMinimized())
	    ctrl = QStyle::TitleNormalButton;

	QPainter p(this);
	if(ctrl != buttonDown)
	    style().drawTitleBarControls(&p, this, buttonDown, QStyle::TitleNone);
	else
	    style().drawTitleBarControls(&p, this, buttonDown, buttonDown);
	break;
    }
    case QStyle::TitleLabel:
	if ( (moveOffset - mapToParent( e->pos() ) ).manhattanLength() >= 4 ) {
	    QPoint p = mapFromGlobal(e->globalPos());
#ifndef QT_NO_WORKSPACE
	    if(window && window->parentWidget()->inherits("QWorkspaceChild")) {
		QWidget *w = window->parentWidget()->parentWidget();
		if(w && w->inherits("QWorkspace")) {
		    QWorkspace *workspace = (QWorkspace*)w;
		    p = workspace->mapFromGlobal( e->globalPos() );
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
		}
	    }
#endif
	    QPoint pp = p - moveOffset;
	    parentWidget()->move( pp );
	}
	break;
    }
}

void QTitleBar::resizeEvent( QResizeEvent *r)
{
    QWidget::resizeEvent(r);

    QString oldt = cuttext;
    cutText();
    if(oldt != cuttext) {
	QPainter painter(this);
	style().drawTitleBarControls(&painter, this, QStyle::TitleLabel, buttonDown);
    }
}

void QTitleBar::paintEvent(QPaintEvent *)
{
    int ctrls = QStyle::TitleLabel | QStyle::TitleSysMenu | QStyle::TitleMaxButton;
    ctrls |= QStyle::TitleCloseButton | QStyle::TitleShadeButton | QStyle::TitleUnshadeButton;
    if(window && window->isMinimized())
	ctrls |= QStyle::TitleNormalButton;
    else
	ctrls |= QStyle::TitleMinButton;

    QSharedDoubleBuffer buffer( (bool)FALSE, (bool)FALSE );
    buffer.begin( this, rect() );
    style().drawTitleBarControls(buffer.painter(), this, ctrls, buttonDown);
}

void QTitleBar::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( e->button() == LeftButton ) {
	switch(style().titleBarPointOver(this, e->pos())) {
	case QStyle::TitleLabel:
	    if ( window && window->testWFlags( WStyle_MinMax ) )
		emit doMaximize();
	    else
		emit doShade();
	    break;
	case QStyle::TitleSysMenu:
	    emit doClose();
	    break;
	default:
	    break;
	}
    }
}

void QTitleBar::cutText()
{
    QFontMetrics fm( font() );

    int maxw, tmp;
    style().titleBarMetrics(this, tmp, tmp, maxw, tmp);
    cuttext = text;
    if ( fm.width( text+"m" ) > maxw ) {
	int i = text.length();
	while ( (fm.width(text.left( i ) + "...")  > maxw) && i>0 )
	    i--;
	cuttext = text.left( i ) + "...";
    }
}

void QTitleBar::setText( const QString& title )
{
    if(text == title)
	return;
    text = title;
    cutText();

    QPainter p(this);
    style().drawTitleBarControls(&p, this, QStyle::TitleLabel, 0);
}


void QTitleBar::setIcon( const QPixmap& icon )
{
    int controlWidth, controlHeight, titleHeight, titleWidth;
    style().titleBarMetrics(this, controlWidth, controlHeight, titleWidth, titleHeight);
    if(icon.width() > controlWidth || icon.height() > controlHeight)
	pixmap.convertFromImage( icon.convertToImage().smoothScale( controlWidth, controlHeight ) );
    else
	pixmap = icon;

    QPainter p(this);
    style().drawTitleBarControls(&p, this, QStyle::TitleSysMenu, 0);
}

void QTitleBar::enterEvent( QEvent * )
{
    QEvent e( QEvent::Leave );
    QApplication::sendEvent( parentWidget(), &e );
}

void QTitleBar::setActive( bool active )
{
    if ( act == active )
	return ;

    act = active;
    update();
}

bool QTitleBar::isActive() const
{
    return act;
}

bool QTitleBar::event( QEvent* e )
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

    return QWidget::event( e );
}

QSize QTitleBar::sizeHint() const
{
    constPolish();
    int controlWidth, controlHeight, titleHeight, titleWidth;
    style().titleBarMetrics(this, controlWidth, controlHeight, titleWidth, titleHeight);
    return QSize( 128, QMAX( QMAX(controlHeight, titleHeight), fontMetrics().lineSpacing() ) );;
}

#if defined(QT_ACCESSIBILITY_SUPPORT)
QAccessibleInterface *QTitleBar::accessibleInterface()
{
    return new QAccessibleWidget( this, QAccessible::TitleBar );
}
#endif

#endif //QT_NO_TITLEBAR
