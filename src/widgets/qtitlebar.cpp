/****************************************************************************
** $Id: $
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
#ifndef QT_NO_WORKSPACE
#include "qworkspace.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
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

	QString tipstring;
	QStyle::SubControl control =
	    t->style().querySubControl(QStyle::CC_TitleBar, t, pos);
	QSize controlSize =
	    t->style().querySubControlMetrics(QStyle::CC_TitleBar, t, control).size();

	switch(control) {
	case QStyle::SC_None:
	default:
	    break;

	case QStyle::SC_TitleBarSysMenu:
	    tipstring = t->tr("System Menu");
	    break;

	case QStyle::SC_TitleBarShadeButton:
	    tipstring = t->tr("Shade");
	    break;

	case QStyle::SC_TitleBarUnshadeButton:
	    tipstring = t->tr("Unshade");
	    break;

	case QStyle::SC_TitleBarNormalButton:
	case QStyle::SC_TitleBarMinButton:
	    if(t->window->isMinimized())
		tipstring = t->tr("Normalize");
	    else
		tipstring = t->tr("Minimize");
	    break;

	case QStyle::SC_TitleBarMaxButton:
	    tipstring = t->tr("Maximize");
	    break;

	case QStyle::SC_TitleBarCloseButton:
	    tipstring = t->tr("Close");
	    break;

	case QStyle::SC_TitleBarLabel:
	    if(t->cuttext != t->text)
		tipstring = t->text;
	    break;
	}

	if(!tipstring.isEmpty())
	    tip( QRect(pos, controlSize), tipstring );
    }
};

QTitleBar::QTitleBar (QWidget* w, QWidget* parent, const char* name)
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder | WResizeNoErase | WRepaintNoErase )
{
    toolTip = new QTitleBarTip( this );
    window = w;
    buttonDown = QStyle::SC_None;
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
#ifdef Q_OS_TEMP
		SystemParametersInfo( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
#else
#if defined(UNICODE)
	    if ( qt_winver & Qt::WV_NT_based )
		SystemParametersInfo( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
	    else
#endif
		SystemParametersInfoA( SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0 );
#endif

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
	QStyle::SCFlags ctrl =
	    style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	switch (ctrl) {
	case QStyle::SC_TitleBarSysMenu: {
	    buttonDown = QStyle::SC_None;
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
	    break; }

	case QStyle::SC_TitleBarShadeButton:
	case QStyle::SC_TitleBarUnshadeButton:
	case QStyle::SC_TitleBarNormalButton:
	case QStyle::SC_TitleBarMinButton:
	    if(window && window->isMinimized())
		buttonDown = QStyle::SC_TitleBarNormalButton;
	    else
		buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarMaxButton:
	case QStyle::SC_TitleBarCloseButton:
	    buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarLabel:
	    buttonDown = ctrl;
	    moveOffset = mapToParent( e->pos() );
	    break;
	}

	QPainter p(this);
	if(window && window->isMinimized())
	    ctrl |= QStyle::SC_TitleBarNormalButton;
	style().drawComplexControl(QStyle::CC_TitleBar, &p, this, rect(), colorGroup(),
				   QStyle::CStyle_Default, ctrl, buttonDown);
	p.end();
    }
}

void QTitleBar::contextMenuEvent( QContextMenuEvent *e )
{
    QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
    if(ctrl == QStyle::SC_TitleBarLabel) {
	emit popupOperationMenu(e->globalPos());
	e->accept();
    }
}


void QTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	QStyle::SCFlags ctrl =
	    style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	if (ctrl == QStyle::SC_TitleBarMinButton && window && window->isMinimized())
	    ctrl = QStyle::SC_TitleBarNormalButton;
	if (ctrl == buttonDown) {
	    QPainter p(this);
	    style().drawComplexControl(QStyle::CC_TitleBar, &p, this, rect(),
				       colorGroup(), QStyle::CStyle_Default,
				       ctrl | (window && window->isMinimized() ?
					       QStyle::SC_TitleBarNormalButton : 0),
				       QStyle::SC_None);
	    p.end();

	    switch(ctrl) {
	    case QStyle::SC_TitleBarSysMenu:
		break;
	    case QStyle::SC_TitleBarShadeButton:
		emit doShade();
		break;
	    case QStyle::SC_TitleBarUnshadeButton:
	    case QStyle::SC_TitleBarNormalButton:
		emit doNormal();
		break;
	    case QStyle::SC_TitleBarMinButton:
		emit doMinimize();
		break;
	    case QStyle::SC_TitleBarMaxButton:
		emit doMaximize();
		break;
	    case QStyle::SC_TitleBarCloseButton:
		emit doClose();
		buttonDown = QStyle::SC_None;
		return;
	    case QStyle::SC_TitleBarLabel:
		break;
	    }
	}
	buttonDown = QStyle::SC_None;
	releaseMouse();
    }
}

void QTitleBar::mouseMoveEvent( QMouseEvent * e)
{
    if ( buttonDown == QStyle::SC_None)
	return;

    switch (buttonDown) {
    case QStyle::SC_TitleBarSysMenu:
	break;

    case QStyle::SC_TitleBarShadeButton:
    case QStyle::SC_TitleBarUnshadeButton:
    case QStyle::SC_TitleBarNormalButton:
    case QStyle::SC_TitleBarMinButton:
    case QStyle::SC_TitleBarMaxButton:
    case QStyle::SC_TitleBarCloseButton: {
	static QStyle::SCFlags last_ctrl = 0;
	QStyle::SCFlags ctrl =
	    style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	if(ctrl == QStyle::SC_TitleBarMinButton && window && window->isMinimized())
	    ctrl = QStyle::SC_TitleBarNormalButton;
	if(ctrl != last_ctrl) {
	    QPainter p(this);
	    style().drawComplexControl(QStyle::CC_TitleBar, &p, this, rect(),
				       colorGroup(), QStyle::CStyle_Default,
				       ctrl | (window && window->isMinimized() ?
					       QStyle::SC_TitleBarNormalButton : 0),
				       ((ctrl == buttonDown) ? buttonDown :
					QStyle::SC_None));
	}
	last_ctrl = ctrl;
	break; }

    case QStyle::SC_TitleBarLabel:
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
	style().drawComplexControl(QStyle::CC_TitleBar, &painter, this, rect(),
				   colorGroup(), QStyle::CStyle_Default,
				   QStyle::SC_TitleBarLabel, buttonDown);
    }
}

void QTitleBar::paintEvent(QPaintEvent *)
{
    QStyle::SCFlags ctrls = (QStyle::SC_TitleBarLabel |
			     QStyle::SC_TitleBarSysMenu |
			     QStyle::SC_TitleBarMaxButton);
    ctrls |= (QStyle::SC_TitleBarCloseButton |
	      QStyle::SC_TitleBarShadeButton |
	      QStyle::SC_TitleBarUnshadeButton);
    if(window && window->isMinimized())
	ctrls |= QStyle::SC_TitleBarNormalButton;
    else
	ctrls |= QStyle::SC_TitleBarMinButton;

    QSharedDoubleBuffer buffer( (bool)FALSE, (bool)FALSE );
    buffer.begin( this, rect() );
    style().drawComplexControl(QStyle::CC_TitleBar, buffer.painter(), this, rect(),
			       colorGroup(), QStyle::CStyle_Default, ctrls, buttonDown);
}

void QTitleBar::mouseDoubleClickEvent( QMouseEvent *e )
{
    if ( e->button() != LeftButton )
	return;

    switch(style().querySubControl(QStyle::CC_TitleBar, this, e->pos())) {
    case QStyle::SC_TitleBarLabel:
	emit doubleClicked();
	break;

    case QStyle::SC_TitleBarSysMenu:
	emit doClose();
	break;

    default:
	break;
    }
}

void QTitleBar::cutText()
{
    QFontMetrics fm( font() );

    int maxw = style().querySubControlMetrics(QStyle::CC_TitleBar, this,
					      QStyle::SC_TitleBarLabel).width();
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
    style().drawComplexControl(QStyle::CC_TitleBar, &p, this, rect(), colorGroup(),
			       QStyle::CStyle_Default, QStyle::SC_TitleBarLabel);
}


void QTitleBar::setIcon( const QPixmap& icon )
{
    QRect menur = style().querySubControlMetrics(QStyle::CC_TitleBar, this,
						  QStyle::SC_TitleBarSysMenu);
    if (icon.width() > menur.width()) {
	// try to keep something close to the same aspect
	int aspect = (icon.height() * 100) / icon.width();
	int newh = (aspect * menur.width()) / 100;
	pixmap.convertFromImage( icon.convertToImage().smoothScale(menur.width(),
								   newh) );
    } else if (icon.height() > menur.height()) {
	// try to keep something close to the same aspect
	int aspect = (icon.width() * 100) / icon.height();
	int neww = (aspect * menur.height()) / 100;
	pixmap.convertFromImage( icon.convertToImage().smoothScale(neww,
								   menur.height()) );
    } else
	pixmap = icon;

    QPainter p(this);
    style().drawComplexControl(QStyle::CC_TitleBar, &p, this, rect(), colorGroup(),
			       QStyle::CStyle_Default, QStyle::SC_TitleBarSysMenu);
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
    QRect menur = style().querySubControlMetrics(QStyle::CC_TitleBar, this,
						 QStyle::SC_TitleBarSysMenu);
    return QSize( menur.width(), QMAX( QMAX(menur.height(), 18),
				       fontMetrics().lineSpacing() ) );
}

#endif //QT_NO_TITLEBAR
