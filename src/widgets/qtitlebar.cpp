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

#ifndef QT_NO_TOOLTIP
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
	QStyle::SubControl ctrl = t->style().querySubControl(QStyle::CC_TitleBar, t, pos);
	QSize controlSize = t->style().querySubControlMetrics(QStyle::CC_TitleBar, t, ctrl).size();

	QWidget *window = t->window;
	if ( window ) {
	    switch(ctrl) {
	    case QStyle::SC_TitleBarSysMenu:
		if ( window->testWFlags( WStyle_SysMenu ) )
		    tipstring = QTitleBar::tr( "System Menu" );
		break;

	    case QStyle::SC_TitleBarShadeButton:
		if ( window->testWFlags( WStyle_Tool ) && window->testWFlags( WStyle_MinMax ) )
		    tipstring = QTitleBar::tr( "Shade" );
		break;

	    case QStyle::SC_TitleBarUnshadeButton:
		if ( window->testWFlags( WStyle_Tool ) && window->testWFlags( WStyle_MinMax ) )
		    tipstring = QTitleBar::tr( "Unshade" );
		break;

	    case QStyle::SC_TitleBarNormalButton:
	    case QStyle::SC_TitleBarMinButton:
		if ( !window->testWFlags( WStyle_Tool ) && window->testWFlags( WStyle_Minimize ) ) {
		    if( window->isMinimized() )
			tipstring = QTitleBar::tr( "Normalize" );
		    else
			tipstring = QTitleBar::tr( "Minimize" );
		}
		break;

	    case QStyle::SC_TitleBarMaxButton:
		if ( !window->testWFlags( WStyle_Tool ) && window->testWFlags( WStyle_Maximize ) )
		    tipstring = QTitleBar::tr( "Maximize" );
		break;

	    case QStyle::SC_TitleBarCloseButton:
		if ( window->testWFlags( WStyle_SysMenu ) )
		    tipstring = QTitleBar::tr( "Close" );
		break;

	    default:
		break;
	    }
	}
	if ( tipstring.isEmpty() ) {
	    if ( t->cuttext != t->txt )
		tipstring = t->txt;
	}
	if(!tipstring.isEmpty())
	    tip( QRect(pos, controlSize), tipstring );
    }
};
#endif

QTitleBar::QTitleBar (QWidget* w, QWidget* parent, const char* name)
    : QWidget( parent, name, WStyle_Customize | WStyle_NoBorder | WResizeNoErase | WRepaintNoErase )
{
#ifndef QT_NO_TOOLTIP
    toolTip = new QTitleBarTip( this );
#endif
    window = w;
    buttonDown = QStyle::SC_None;
    act = 0;
    getColors();
    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed ) );
}

QTitleBar::~QTitleBar()
{
#ifndef QT_NO_TOOLTIP
    delete toolTip;
#endif
}

#ifdef Q_WS_WIN
extern QRgb qt_colorref2qrgb(COLORREF col);
#endif

void QTitleBar::getColors()
{
    aleftc = arightc = palette().active().highlight();
    atextc = palette().active().highlightedText();
#ifdef Q_WS_WIN
    ileftc = irightc = palette().inactive().dark();
    itextc = palette().inactive().background();
#else
    ileftc = irightc = palette().inactive().background();
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
    emit doActivate();
    if ( e->button() == LeftButton ) {
	QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	switch (ctrl) {
	case QStyle::SC_TitleBarSysMenu: 
	    if ( window && window->testWFlags( WStyle_SysMenu ) && !window->testWFlags( WStyle_Tool ) ) {
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
	    }
	    break;

	case QStyle::SC_TitleBarShadeButton:
	case QStyle::SC_TitleBarUnshadeButton:
	    if ( window && window->testWFlags( WStyle_MinMax ) && window->testWFlags( WStyle_Tool ) )
		buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarNormalButton:
	    if( window && window->testWFlags( WStyle_Minimize ) && !window->testWFlags( WStyle_Tool ) )
		buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarMinButton:
	    if( window && window->testWFlags( WStyle_Minimize ) && !window->testWFlags( WStyle_Tool ) )
		buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarMaxButton:
	    if ( window && window->testWFlags( WStyle_Maximize ) && !window->testWFlags( WStyle_Tool ) )
		buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarCloseButton:
	    if ( window && window->testWFlags( WStyle_SysMenu ) )
		buttonDown = ctrl;
	    break;

	case QStyle::SC_TitleBarLabel:
	    buttonDown = ctrl;
	    moveOffset = mapToParent( e->pos() );
	    break;
	    
	default:
	    break;
	}
	repaint( FALSE );
    }
}

void QTitleBar::contextMenuEvent( QContextMenuEvent *e )
{
    QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
    if( ctrl == QStyle::SC_TitleBarLabel || ctrl == QStyle::SC_TitleBarSysMenu ) {
	emit popupOperationMenu(e->globalPos());
	e->accept();
    }
}

void QTitleBar::mouseReleaseEvent( QMouseEvent * e)
{
    if ( e->button() == LeftButton ) {
	QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());

	if (ctrl == buttonDown) {
	    switch(ctrl) {
	    case QStyle::SC_TitleBarShadeButton:
	    case QStyle::SC_TitleBarUnshadeButton:
		if( window && window->testWFlags( WStyle_MinMax ) && window->testWFlags( WStyle_Tool ) )
		    emit doShade();
		break;
	    
	    case QStyle::SC_TitleBarNormalButton:
		if( window && window->testWFlags( WStyle_MinMax ) && !window->testWFlags( WStyle_Tool ) )
		    emit doNormal();
		break;

	    case QStyle::SC_TitleBarMinButton:
		if( window && window->testWFlags( WStyle_Minimize ) && !window->testWFlags( WStyle_Tool ) )
		    emit doMinimize();
		break;

	    case QStyle::SC_TitleBarMaxButton:
		if( window && window->testWFlags( WStyle_Maximize ) && !window->testWFlags( WStyle_Tool ) )
		    emit doMaximize();
		break;

	    case QStyle::SC_TitleBarCloseButton:
		if( window && window->testWFlags( WStyle_SysMenu ) ) {
		    buttonDown = QStyle::SC_None;
		    emit doClose();
		    return;
		}
		break;

	    default:
		break;
	    }
	}
	buttonDown = QStyle::SC_None;
	repaint(FALSE);
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
    case QStyle::SC_TitleBarCloseButton:
	{
	    QStyle::SCFlags last_ctrl = buttonDown;
	    buttonDown = style().querySubControl(QStyle::CC_TitleBar, this, e->pos());
	    if( buttonDown != last_ctrl)
		buttonDown = QStyle::SC_None;
	    repaint(FALSE);
	    buttonDown = last_ctrl;
	}
	break; 

    case QStyle::SC_TitleBarLabel:
	if ( buttonDown == QStyle::SC_TitleBarLabel ) {
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
	} else {
	    QStyle::SCFlags last_ctrl = buttonDown;
	    buttonDown = QStyle::SC_None;
	    if( buttonDown != last_ctrl)
		repaint(FALSE);
	}
	break;
    }
}

void QTitleBar::resizeEvent( QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    cutText();
}

void QTitleBar::paintEvent(QPaintEvent *)
{
    QStyle::SCFlags ctrls = QStyle::SC_TitleBarLabel;
    if ( window && window->testWFlags( WStyle_SysMenu) ) {
	if ( window->testWFlags( WStyle_Tool ) ) {
	    ctrls |= QStyle::SC_TitleBarCloseButton;
	    if ( window->testWFlags( WStyle_MinMax ) ) {
		if ( window->isMinimized() )
		    ctrls |= QStyle::SC_TitleBarUnshadeButton;
		else
		    ctrls |= QStyle::SC_TitleBarShadeButton;
	    }
	} else {
	    ctrls |= QStyle::SC_TitleBarSysMenu | QStyle::SC_TitleBarCloseButton;
	    if ( window->testWFlags( WStyle_Minimize ) ) {
		if( window->isMinimized() )
		    ctrls |= QStyle::SC_TitleBarNormalButton;
		else
		    ctrls |= QStyle::SC_TitleBarMinButton;
	    }
	    if ( window->testWFlags( WStyle_Maximize ) && !window->isMaximized() )
		ctrls |= QStyle::SC_TitleBarMaxButton;
	}
    }

    QSharedDoubleBuffer buffer( (bool)FALSE, (bool)FALSE );
    buffer.begin( this, rect() );
    style().drawComplexControl(QStyle::CC_TitleBar, buffer.painter(), this, rect(),
			       colorGroup(),
			       isEnabled() ? QStyle::Style_Enabled :
			       QStyle::Style_Default, ctrls, buttonDown);
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
	if ( window && window->testWFlags( WStyle_SysMenu ) )
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
    if ( !window )
	maxw = width() - 20;
    cuttext = txt;
    if ( fm.width( txt+"m" ) > maxw ) {
	int i = txt.length();
	while ( (fm.width(txt.left( i ) + "...")  > maxw) && i>0 )
	    i--;
	cuttext = txt.left( i ) + "...";
    }
}

void QTitleBar::setText( const QString& title )
{
    if(txt == title)
	return;
    txt = title;
    cutText();
    repaint( FALSE );
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

    repaint(FALSE);
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
