/****************************************************************************
** $Id$
**
** Definition of Aqua-like style class
**
** Created : 001129
**
** Copyright (C) 1992-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software. This file and its contents may
** not be distributed onto any other platform or included in any other licensed
** package unless explicit permission is granted.
**
** This file is not available for use under any other license without
** express written permission from the copyright holder.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qcheckbox.h"

#ifndef QT_NO_STYLE_AQUA

#include "qaquastyle.h"
#include "qapplication.h"
#include "qpainter.h"
#include "qdrawutil.h"
#include "qpixmap.h"
#include "qpalette.h"
#include "qpushbutton.h"
#include "qdockwindow.h"
#include "qtabbar.h"
#include "qscrollbar.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qtoolbutton.h"
#include "qtoolbar.h"
#include "qobjectlist.h"
#include "qlayout.h"
#include "qptrlist.h"
#include "qbuttongroup.h"
#include "qtabwidget.h"
#include "qslider.h"
#include "qwmatrix.h"
#include "qprogressbar.h"
#include "qtextedit.h"
#include "qlistview.h"
#include <limits.h>
#include "private/qtitlebar_p.h"
#include "private/qinternal_p.h"
#include "qpopupmenu.h"
#include "qguardedptr.h"
#include "qlineedit.h"
#include "qcombobox.h"
#include "qptrstack.h"
#ifdef Q_WS_MAC
#  include <string.h>
#  include <qt_mac.h>
#endif
#define QT_AQUA_XPM
#include "private/qaquastyle_p.h"

#define QMAC_QAQUASTYLE_SIZE_CONSTRAIN

/*****************************************************************************
  QAquaStyle debug facilities
 *****************************************************************************/
//#define DEBUG_SIZE_CONSTRAINT

static const int aquaSepHeight         = 10;    // separator height
static const int aquaItemFrame         = 2;    // menu item frame width
static const int aquaItemHMargin       = 3;    // menu item hor text margin
static const int aquaItemVMargin       = 2;    // menu item ver text margin
static const int aquaArrowHMargin      = 6;    // arrow horizontal margin
static const int aquaTabSpacing        = 12;   // space between text and tab
static const int aquaCheckMarkHMargin  = 2;    // horiz. margins of check mark
static const int aquaRightBorder       = 12;   // right border on aqua
static const int aquaCheckMarkWidth    = 12;   // checkmarks width on aqua
static QColor qt_aqua_highlight_active_color = QColor( 0xC2, 0xC2, 0xC2 ); //color of highlighted text
static QColor qt_aqua_highlight_inactive_color = qt_aqua_highlight_active_color.light();
#ifdef QMAC_QAQUA_MODIFY_TEXT_COLOURS
static QColor qt_aqua_text_active_color = Qt::black;
static QColor qt_aqua_text_inactive_color = Qt::black;
#endif
static bool qt_mac_scrollbar_arrows_together = FALSE; //whether scroll arrows go together
QCString p2qstring(const unsigned char *c); //qglobal.cpp

//Perhaps this QAquaFocusWidget should be defined elsewhere??
QAquaFocusWidget::QAquaFocusWidget(bool noerase)
    : QWidget( NULL, "magicFocusWidget", WResizeNoErase | (noerase ? WRepaintNoErase : 0) ), d( NULL )
{
//    setBackgroundMode(NoBackground);
}
void QAquaFocusWidget::setFocusWidget( QWidget * widget )
{
    hide();
    if (d) {
	if(d->parentWidget())
	    d->parentWidget()->removeEventFilter(this);
	d->removeEventFilter( this );
    }
    d = NULL;
    if(widget && widget->parentWidget()) {
	d = widget;
	reparent( d->parentWidget(), pos() );
	raise();
	d->installEventFilter( this );
	d->parentWidget()->installEventFilter( this );
	setGeometry( widget->x() - focusOutset(), widget->y() - focusOutset(), 
		     widget->width() + (focusOutset() * 2), 
		     widget->height() + (focusOutset() * 2) );
	setMask( QRegion( rect() ) - focusRegion() );
	show();
    }
}
bool QAquaFocusWidget::eventFilter( QObject * o, QEvent * e )
{
    if ((e->type() == QEvent::ChildInserted || e->type() == QEvent::ChildRemoved) &&
	((QChildEvent*)e)->child() == this) {
	if(e->type() == QEvent::ChildRemoved)
	    o->removeEventFilter(this); //once we're removed, stop listening
	return TRUE; //block child events
    } else if (o == d) {
	switch (e->type()) {
	case QEvent::Hide: 
	    hide();
	    break;
	case QEvent::Show:
	    show();
	    break;
	case QEvent::Move: {
	    QMoveEvent *me = (QMoveEvent*)e;
	    move( me->pos().x() - focusOutset(), me->pos().y() - focusOutset() );
	    break;
	}
	case QEvent::Resize: {
	    QResizeEvent *re = (QResizeEvent*)e;
	    resize( re->size().width() + (focusOutset() * 2), 
		    re->size().height() + (focusOutset() * 2) );
	    setMask( QRegion( rect() ) - focusRegion() );
	    break;
	}
	case QEvent::Reparent:
	    reparent( d->parentWidget(), pos() );
	    d->parentWidget()->installEventFilter( this );
	    raise();
	    break;
	default:
	    break;
	}
    }
    return FALSE;
}
void QAquaFocusWidget::paintEvent( QPaintEvent * )
{
    QPixmap pmt, pmb, pml, pmr, pmtl, pmtr, pmbl, pmbr;
    qAquaPixmap( "focus_t", pmt );
    qAquaPixmap( "focus_l", pml );
    qAquaPixmap( "focus_b", pmb );
    qAquaPixmap( "focus_r", pmr );
    qAquaPixmap( "focus_tl", pmtl );
    qAquaPixmap( "focus_bl", pmbl );
    qAquaPixmap( "focus_tr", pmtr );
    qAquaPixmap( "focus_br", pmbr );

    QSharedDoubleBuffer buffer( (bool)FALSE, (bool)FALSE );
    buffer.begin( this, rect() );
    buffer.painter()->drawTiledPixmap( 4, 0, width() - 8, pmt.height(), pmt );
    buffer.painter()->drawTiledPixmap( 4, height() - pmb.height(), width() - 8, pmb.height(), pmb );
    buffer.painter()->drawTiledPixmap( 0, 5, pml.width(), height() - 10, pml );
    buffer.painter()->drawTiledPixmap( width() - pmr.width(), 5, pml.width(), height() - 10, pmr );
    buffer.painter()->drawPixmap( 0, 0, pmtl );
    buffer.painter()->drawPixmap( 0, height() - pmbl.height(), pmbl );
    buffer.painter()->drawPixmap( width() - pmtr.width(), 0, pmtr );
    buffer.painter()->drawPixmap( width() - pmbr.width(), height() - pmbr.height(), pmbr );
}

//Perhaps this QAquaAnimate should be defined elsewhere??
struct QAquaAnimatePrivate 
{
    //focus
    QWidget *focus;
    //buttons
    QGuardedPtr<QPushButton> defaultButton, noPulse;
    int buttonTimerId;
    //timers
    QPtrList<QProgressBar> progressBars;
    int progressTimerId;
};
QAquaAnimate::QAquaAnimate() 
{
    d = new QAquaAnimatePrivate;
    d->focus = d->defaultButton = d->noPulse = NULL;
    d->progressTimerId = d->buttonTimerId = -1;
}
QAquaAnimate::~QAquaAnimate()
{ 
    delete d; 
}
bool QAquaAnimate::addWidget(QWidget *w) 
{
    if( focusable(w) ) {
	if(w->hasFocus()) 
	    setFocusWidget(w);
	w->installEventFilter( this );
    }
    if(w == d->defaultButton || d->progressBars.contains((QProgressBar*)w)) //already knew of it
	return FALSE;

    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn->isDefault() || (btn->autoDefault() && btn->hasFocus()) ){
	    d->defaultButton = btn;
            btn->installEventFilter( this );
            if( btn->isVisible() && d->buttonTimerId == -1 ) 
                d->buttonTimerId = startTimer( 50 );
        }
	return TRUE;
    } else if( w->inherits("QProgressBar") ){
	w->installEventFilter( this );
	QObject::connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
	if( w->isVisible() && d->progressTimerId == -1 ) {
	    d->progressBars.append((QProgressBar*)w);
	    d->progressTimerId = startTimer( 50 );
	}
	return TRUE;
    }
    return FALSE;
}
void QAquaAnimate::removeWidget(QWidget *w) 
{
    if(focusWidget() == w) 
	setFocusWidget(NULL);

    if( w->inherits("QPushButton") ){
        QPushButton * btn = (QPushButton *) w;
        if( btn == d->defaultButton )
	    d->defaultButton = 0;
        if( d->buttonTimerId != -1 ){
            killTimer( d->buttonTimerId );
            d->buttonTimerId = -1;
        }
    }
    if( w->inherits("QProgressBar") ) {
	d->progressBars.remove((QProgressBar *) w);
	if(d->progressBars.isEmpty() && d->progressTimerId != -1) {
	    killTimer( d->progressTimerId );
	    d->progressTimerId = -1;
	}
    }
}
void QAquaAnimate::objDestroyed(QObject *o)
{
    if(o == d->focus) 
	setFocusWidget(NULL);
    while(d->progressBars.remove((QProgressBar*)o));
}
bool QAquaAnimate::animatable(QAquaAnimate::Animates as, QWidget *w)
{
    if(as == AquaPushButton && w->inherits("QPushButton")) {
	QPushButton *btn = (QPushButton *)w;
	if((!d->noPulse || (QPushButton*)d->noPulse == btn || !d->noPulse->isDown()) &&
	   btn->isEnabled() && (btn->isDefault() || (btn->autoDefault() && btn->hasFocus())) && ((QPushButton*)d->defaultButton == btn) &&
	   w == d->defaultButton)
	    return TRUE;
    } else if(as == AquaProgressBar && d->progressBars.find((QProgressBar*)w) != -1) {
	return TRUE;
    }
    return FALSE;
}
void QAquaAnimate::timerEvent( QTimerEvent * te )
{
    if( te->timerId() == d->buttonTimerId ) {
	if( d->defaultButton && d->defaultButton->isEnabled() && d->defaultButton->isVisibleTo(0) &&
	    (d->defaultButton->isDefault() || (d->defaultButton->autoDefault() && d->defaultButton->hasFocus()) ) ) {
	    doAnimate(AquaPushButton);
	    d->defaultButton->repaint(FALSE);
	}
    } else if( te->timerId() == d->progressTimerId && !d->progressBars.isEmpty() ) {
	doAnimate(AquaProgressBar);
	if( d->progressBars.count() == 1) {
	    QProgressBar *b = d->progressBars.first();
	    if(b->progress() > 0)
		b->repaint(FALSE);
	} else {
	    for( QPtrListIterator<QProgressBar> it(d->progressBars); it.current(); ++it) {
		if((*it)->progress() > 0)
		    (*it)->repaint( FALSE );
	    }
	}
    }
}
bool QAquaAnimate::eventFilter( QObject * o, QEvent * e )
{
    //focus
    if(o->isWidgetType() && focusWidget() && focusable((QWidget *)o) &&
       ((e->type() == QEvent::FocusOut && focusWidget() == o) ||
	(e->type() == QEvent::FocusIn && focusWidget() != o)))  { //restore it
	if (((QFocusEvent *)e)->reason() != QFocusEvent::Popup) 
	    setFocusWidget(NULL);
    }
    if( o && o->isWidgetType() && e->type() == QEvent::FocusIn ) {
	QWidget *w = (QWidget *)o;
	if( focusable(w) ) 
	    setFocusWidget(w);
    }
    //animate
    if( o && o->isWidgetType() && e->type() == QEvent::FocusIn ) {
	QWidget *w = (QWidget *)o;
	if( o->inherits("QPushButton") && ((QPushButton *)w)->autoDefault()) {
	    // Kb Focus received - make this the default button
	    d->defaultButton = (QPushButton *) w;
            if( w->isVisible() && d->buttonTimerId == -1 )
                d->buttonTimerId = startTimer( 50 );
	}
    } else if(e->type() == QEvent::Show && o->inherits("QProgressBar")) {
	d->progressBars.append((QProgressBar*)o);
	if(d->progressTimerId == -1)
	    d->progressTimerId = startTimer( 50 );
    } else if(e->type() == QEvent::Hide && d->progressBars.find((QProgressBar*)o) != -1) {
	while(d->progressBars.remove((QProgressBar*)o));
	if(d->progressBars.isEmpty() && d->progressTimerId != -1) {
	    killTimer(d->progressTimerId);
	    d->progressTimerId = -1;
	}
    } else if( e->type() == QEvent::Hide && d->defaultButton == o ) {
	d->defaultButton = NULL;
	if( d->buttonTimerId != -1 ) {
	    killTimer(d->buttonTimerId);
	    d->buttonTimerId = -1;
	}
    } else if( (e->type() == QEvent::MouseButtonPress || e->type() == QEvent::MouseButtonRelease) &&
	       o->inherits("QPushButton") ) {
	QMouseEvent *me = (QMouseEvent*)e;
	d->noPulse = me->type() == QEvent::MouseButtonPress && me->button() == Qt::LeftButton ? 
				    (QPushButton*)o : NULL;
    } else if( o && (e->type() == QEvent::FocusOut || e->type() == QEvent::Show) &&
	       o->inherits("QPushButton") ) {
	QPushButton *btn = (QPushButton *)o;
	// Find the correct button to use as default button
	QObjectList *list = btn->topLevelWidget()->queryList( "QPushButton" );
	QObjectListIt it( * list );
	QPushButton * pb;
	while ( (pb = (QPushButton*)it.current()) ) {
	    ++it;
	    if( ((e->type() == QEvent::FocusOut) && (pb->isDefault() ||
						     (pb->autoDefault() && pb->hasFocus())) && (pb != btn)) ||
		((e->type() == QEvent::Show) && pb->isDefault()))
	    {
		QPushButton * tmp = d->defaultButton;
		d->defaultButton = 0;
		if( tmp )
		    tmp->repaint( FALSE );
		if(pb->topLevelWidget()->isActiveWindow()) 
		    d->defaultButton = pb;
		break;
	    }
	}
	delete list;
	if(d->defaultButton) {
	    if(d->buttonTimerId == -1)
                d->buttonTimerId = startTimer( 50 );
	} else if(d->buttonTimerId != -1) {
	    killTimer(d->buttonTimerId);
	    d->buttonTimerId = -1;
	}
    }
    return FALSE;
}
QWidget *QAquaAnimate::focusWidget() const
{
    return d->focus;
}
void QAquaAnimate::setFocusWidget(QWidget *w) 
{
    if(w) {
	QWidget *p = w->parentWidget();
	while(!p->isTopLevel() && !p->testWFlags(WSubWindow))
	    p = p->parentWidget();
	if(p && (w->width() < p->width() - 30 || w->height() < p->height() - 40)) {
	    if(w->inherits("QLineEdit") && w->parentWidget()->inherits("QComboBox"))
		w = w->parentWidget();
	} else {
	    w = NULL;
	}
    }
    if(w == d->focus)
	return;
    doFocus(w);
    if(d->focus) 
	QObject::disconnect(d->focus, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
    if((d->focus = w))
	QObject::connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
}
bool QAquaAnimate::focusable(QWidget *w)
{
    return (w && w->parentWidget() && (w->inherits("QDateTimeEditor") ||
	     (w->inherits("QFrame") && ((QFrame*)w)->frameStyle() != QFrame::NoFrame && 
	      ((w->inherits("QLineEdit") /* &&
	      (w->parentWidget()->inherits("QComboBox") || (((QLineEdit*)w)->frame())) */) ||
	      (w->inherits("QTextEdit") && !((QTextEdit*)w)->isReadOnly()) ||
	      w->inherits("QListBox") || w->inherits("QListView")))));
}


class QAquaStylePrivate : public QAquaAnimate
{
public:
    QAquaStylePrivate() : QAquaAnimate() { }
    //blinking buttons
    struct buttonState {
    public:
        buttonState() : frame(0), dir(1) {}
        int frame;
        int dir;
    };
    buttonState   buttonState;
    //animated progress bars
    int progressOff;
    //big focus rects
    QGuardedPtr<QAquaFocusWidget> focusWidget;
    
protected:
    void doAnimate(QAquaAnimate::Animates);
    void doFocus(QWidget *);
};
void QAquaStylePrivate::doAnimate(QAquaAnimate::Animates as)
{
    if(as == QAquaAnimate::AquaProgressBar) 
	progressOff--;
}
void QAquaStylePrivate::doFocus(QWidget *w)
{
    if (!focusWidget)
	focusWidget = new QAquaFocusWidget();
    focusWidget->setFocusWidget( w );
}

#if defined( QMAC_QAQUASTYLE_SIZE_CONSTRAIN ) || defined(DEBUG_SIZE_CONSTRAINT)
static AquaSize qt_aqua_size(QWidget *widg, QSize large, QSize small)
{
    if(large == QSize(-1, -1) && small != QSize(-1, -1))
	return AquaSizeSmall;
    else if(small == QSize(-1, -1) && large != QSize(-1, -1))
	return AquaSizeLarge;

    if(widg->topLevelWidget()->inherits("QDockWindow") || getenv("QWIDGET_ALL_SMALL")) {
	//if(small.width() != -1 || small.height() != -1)
	    return AquaSizeSmall;
    }

    int large_delta=0;
    if(large.width() != -1) {
	int delta = large.width() - widg->width();
	large_delta += delta * delta;
    }
    if(large.height() != -1) {
	int delta = large.height() - widg->height();
	large_delta += delta * delta;
    }
#if 0
    int small_delta=0;
    if(small.width() != -1) {
	int delta = small.width() - widg->width();
	small_delta += delta * delta;
    }
    if(small.height() != -1) {
	int delta = small.height() - widg->height();
	small_delta += delta * delta;
    }
    if(small_delta < large_delta) 
	return AquaSizeSmall;
#else
    Q_UNUSED(small);
#endif
    return AquaSizeLarge;
}
static AquaSize qt_aqua_size_constrain(QWidget *widg, bool fix, QSize large, QSize small=QSize())
{
    AquaSize ret = qt_aqua_size(widg, large, small);
    QSize *sz = NULL;
    if(ret == AquaSizeSmall)
	sz = &small;
    else if(ret == AquaSizeLarge)
	sz = &large;
    if(sz && fix && ((sz->width() != -1 && sz->width() != widg->width()) || 
		     (sz->height() != -1 && sz->height() != widg->height()))) {
#ifdef DEBUG_SIZE_CONSTRAINT
	const char *size_desc = "Unknown";
	if(sz == &small)
	    size_desc = "Small";
	else if(sz == &large)
	    size_desc = "Large";
	qDebug("%s - %s: %s %staken (%d, %d) [ %d, %d ]", widg->name(), widg->className(), 
	       size_desc, fix ? "" : "not ", widg->width(), widg->height(), 
	       sz->width(), sz->height());
#endif
#ifdef QMAC_QAQUASTYLE_SIZE_CONSTRAIN
	if(sz->width() != -1)
	    widg->setFixedWidth(sz->width());
	if(sz->height() != -1)
	    widg->setFixedHeight(sz->height());
#endif
    }
    return ret;
}
#ifdef Q_WS_MAC
static int qt_mac_aqua_get_metric(ThemeMetric met)
{
    SInt32 ret;
    GetThemeMetric(met, &ret);
    return ret;
}
#endif
static QSize qt_aqua_size_constraints(QWidget *widg, AquaSize sz)
{
    QSize ret(-1, -1);
    const int shadow_height = 3;
    if(sz != AquaSizeSmall && sz != AquaSizeLarge) {
	qDebug("Not sure how to return this..");
	return ret;
    }
    if(widg->inherits("QPushButton")) {
	int minw = -1;
	if(widg->caption() == widg->tr("OK") || widg->caption() == widg->tr("Cancel"))
	    minw = 69;
#ifdef Q_WS_MAC
	if(sz == AquaSizeLarge)
	    ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricPushButtonHeight) + shadow_height);
	else
	    ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricSmallPushButtonHeight) + shadow_height);
#else
	if(sz == AquaSizeLarge)
	    ret = QSize(minw, 20 + shadow_height);
	else
	    ret = QSize(minw, 17 + shadow_height);
#endif
    } else if(widg->inherits("QRadioButton")) {
#ifdef Q_WS_MAC
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricRadioButtonHeight) + shadow_height);
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallRadioButtonHeight) + shadow_height);
#else
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, 18 + shadow_height);
	else 
	    ret = QSize(-1, 15 + shadow_height);
#endif
    } else if(widg->inherits("QCheckBox")) {
#ifdef Q_WS_MAC
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricCheckBoxHeight) + shadow_height);
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallCheckBoxHeight) + shadow_height);
#else
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, 18 + shadow_height);
	else 
	    ret = QSize(-1, 16 + shadow_height);
#endif
    } else if(widg->inherits("QComboBox")) {
#ifdef Q_WS_MAC
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricPopupButtonHeight));
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallPopupButtonHeight));
#else
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, 20);
	else 
	    ret = QSize(-1, 17);
#endif
    } else if(widg->inherits("QToolButton") && !((QToolButton*)widg)->usesTextLabel() &&
	      widg->parentWidget() && widg->parentWidget()->inherits("QToolBar")) {
	if(sz == AquaSizeSmall)
	    ret = QSize(20, 20);
    } else if(widg->inherits("QSlider")) {
	int w = -1;
	QSlider *sld = (QSlider*)widg;
#ifdef Q_WS_MAC
	if(sz == AquaSizeLarge) {
	    if(sld->orientation() == Qt::Horizontal) {
		w = qt_mac_aqua_get_metric(kThemeMetricHSliderTickHeight);
		if(sld->tickmarks() != QSlider::NoMarks) 
		    w += qt_mac_aqua_get_metric(kThemeMetricHSliderTickHeight);
	    } else {
		w = qt_mac_aqua_get_metric(kThemeMetricVSliderTickWidth);
		if(sld->tickmarks() != QSlider::NoMarks) 
		    w += qt_mac_aqua_get_metric(kThemeMetricVSliderTickWidth);
	    }
	} else {
	    if(sld->orientation() == Qt::Horizontal) {
		w = qt_mac_aqua_get_metric(kThemeMetricSmallHSliderTickHeight);
		if(sld->tickmarks() != QSlider::NoMarks) 
		    w += qt_mac_aqua_get_metric(kThemeMetricSmallHSliderTickHeight);
	    } else {
		w = qt_mac_aqua_get_metric(kThemeMetricSmallVSliderTickWidth);
		if(sld->tickmarks() != QSlider::NoMarks) 
		    w += qt_mac_aqua_get_metric(kThemeMetricSmallVSliderTickWidth);
	    }
	}
#else
	if(sld->tickmarks() == QSlider::NoMarks) {
	    if(sz == AquaSizeLarge)
		w = 18;
	    else 
		w = 16;
	} else {
	    if(sz == AquaSizeLarge)
		w = 25;
	    else 
		w = 18;
	}
	if(sld->orientation() == Qt::Horizontal) 
	    ret.setHeight(w);
	else
	    ret.setWidth(w);
#endif
    } else if(widg->inherits("QProgressBar")) {
#ifdef Q_WS_MAC
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricLargeProgressBarThickness));
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricNormalProgressBarThickness));
#else
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, 16);
	else 
	    ret = QSize(-1, 10);
#endif
    } else if(widg->inherits("QLineEdit")) {
	if(!widg->parentWidget() || !widg->parentWidget()->inherits("QComboBox")) {
	    //should I take into account the font dimentions of the lineedit? -Sam
	    if(sz == AquaSizeLarge)
		ret = QSize(-1, 22);
	    else 
		ret = QSize(-1, 19);
	}
    }
#ifdef Q_WS_MAC
    else if(widg->inherits("QHeader")) {
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricListHeaderHeight));
    }
#if 0
    else if(widg->inherits("QTabBar")) {
	if(sz == AquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallTabHeight));
    }
#endif
#endif
    return ret;
}
#endif
AquaSize qt_aqua_size_constrain(QWidget *widg, bool fix)
{
#if defined( QMAC_QAQUASTYLE_SIZE_CONSTRAIN ) || defined(DEBUG_SIZE_CONSTRAINT)
    if(!widg) {
	if(getenv("QWIDGET_ALL_SMALL")) 
	    return AquaSizeSmall;
	return AquaSizeUnknown;
    }
    return qt_aqua_size_constrain(widg, fix,
				  qt_aqua_size_constraints(widg, AquaSizeLarge),
				  qt_aqua_size_constraints(widg, AquaSizeSmall));
#else
    Q_UNUSED(widg);
    Q_UNUSED(fix);
    return AquaSizeUnknown;
#endif
}

/*!
  \class QAquaStyle qaquastyle.h
  \brief The QAquaStyle class implements the Aqua 'Look and Feel'.
  \ingroup appearance

  The class tries to emulate the Mac OS X Aqua GUI style using the
  QStyle system. There are additional issues that should be taken into
  consideration to make an application compatible with the
  \link http://developer.apple.com/techpubs/macosx/Carbon/HumanInterfaceToolbox/Aqua/aqua.html
  Aqua Style Guidelines \endlink. Some of these issues are outlined
  below.

  \list

  \i Layout - The restrictions on window layout are such that some
  aspects of layout that are style-dependent cannot be achieved using
  QLayout. Changes are being considered (and feedback would be
  appreciated) to make layouts QStyle-able. Some of the restrictions
  involve horizontal and vertical widget alignment and widget size
  (covered below).

  \i Widget size - Aqua allows widgets to have specific fixed sizes.
  Qt does not implement this behaviour so as to maintain
  multiplatform compatibility. As a result you may scale a widget to
  any size and QAquaStyle will respect your size, even if it is not
  appropriate in terms of the Aqua Style Guidelines.

  \i Effects - QAquaStyle is an emulating style. Not all of the
  animations and effects have been completed. If you notice any
  shortcomings please report them as bugs. We aim to ensure that
  QAquaStyle is as close to the native look and feel as possible.

  \endlist

  There are other issues that need to be considered in the feel of
  your application (including the general color scheme to match the
  Aqua colors). The Guidelines mentioned above will remain current
  with new advances and design suggestions for Mac OS X.

  QAquaStyle is currently a pretty good representation, even though it
  has different design goals from native Aqua.

  \warning The QAquaStyle code may not be distributed on any platform
  other than Mac OS X, or included in any other licensed package,
  unless explicit permission is granted by Trolltech.

  Note that the functions provided by QAquaStyle are reimplementations
  of QStyle functions; see QStyle for their documentation.
*/


/*!
  Constructs a QAquaStyle object.
*/
QAquaStyle::QAquaStyle()
{
    /* I am doing this for binary compatability */
    installEventFilter(this);

    d = new QAquaStylePrivate;
    d->progressOff = 0;
}

/*!
  Destructs a QAquaStyle object.
*/
QAquaStyle::~QAquaStyle()
{
    delete d;
}

/*! \reimp */
void QAquaStyle::polish( QApplication* app )
{
    QPixmap px;
    qAquaPixmap( "gen_back", px );
    QBrush background( Qt::black, px );
    QPalette pal = app->palette();
    pal.setBrush( QColorGroup::Background, background );
    pal.setBrush( QColorGroup::Button, background );
    qt_mac_update_palette(pal, TRUE);
    app->setPalette(pal, TRUE);
}

/*! \reimp */
void QAquaStyle::polish( QWidget * w )
{
    qt_mac_polish_font(w, qt_aqua_size_constrain(w, TRUE));
    d->addWidget(w);
#ifdef Q_WS_MAC
    if(w->inherits("QPopupMenu"))
	QMacSavedPortInfo::setAlphaTransparancy(w, 0.9);
#endif
    if(w->inherits("QLineEdit")) 
	((QLineEdit *)w)->setLineWidth(3);

    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( FALSE );
    }

    if( w->inherits("QToolBar") ){
	QToolBar * bar = (QToolBar *) w;
	QBoxLayout * layout = bar->boxLayout();
	layout->setSpacing( 0 );
	layout->setMargin( 0 );
    }

    if(w->inherits("QTipLabel")) {
	QLabel *label = (QLabel*)w;
	label->setFrameStyle(QFrame::NoFrame);
	label->setLineWidth( 1 );
    }

    if ( !w->isTopLevel() ) {
        if( !w->inherits("QSplitter") && w->backgroundPixmap() &&
            (w->backgroundMode() == QWidget::PaletteBackground) && qApp->palette().isCopyOf(w->palette()))
            w->setBackgroundOrigin( QWidget::WindowOrigin );
    }

    if( w->inherits("QTitleBar") ) {
	w->font().setPixelSize(10);
	((QTitleBar*)w)->setAutoRaise(TRUE);
    }
#ifdef Q_WS_MACX
    if( w->inherits("QPopupMenu")  && w->parentWidget() && w->parentWidget()->inherits("QComboBox") )
	((QPopupMenu*)w)->setFrameStyle(QFrame::NoFrame);
#endif
}

/*! \reimp */
void QAquaStyle::unPolish( QWidget * w )
{
    d->removeWidget(w);
#ifdef Q_WS_MAC
    if(w->inherits("QPopupMenu"))
	QMacSavedPortInfo::setAlphaTransparancy(w, 1);
#endif
    if( w->inherits("QToolButton") ){
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise( TRUE );
    }

    if( w->inherits("QTitleBar") )
	((QTitleBar*)w)->setAutoRaise(FALSE);
    if( w->inherits("QPopupMenu")  && w->parentWidget() && w->parentWidget()->inherits("QComboBox") )
	((QPopupMenu*)w)->setFrameStyle( QFrame::PopupPanel | QFrame::Raised );

    if ( !w->isTopLevel() ) {
        if( !w->inherits("QSplitter") && w->backgroundPixmap() &&
            (w->backgroundMode() == QWidget::PaletteBackground) )
            w->setBackgroundOrigin( QWidget::WidgetOrigin );
    }
}

/*! \reimp */
bool QAquaStyle::eventFilter( QObject * o, QEvent * e )
{
    if(o == this) {
#ifdef Q_WS_MAC
	if(e->type() == QEvent::Style)
	    appearanceChanged();
#endif
	return FALSE;
    }
    return FALSE;
}

/*! \reimp */
void QAquaStyle::drawItem( QPainter *p, const QRect &r,
			   int flags, const QColorGroup &g, bool enabled,
			   const QPixmap *pixmap, const QString& text,
			   int len, const QColor* penColor ) const
{
    //No accelerators drawn here!
    QWindowsStyle::drawItem( p, r, flags | NoAccel, g, enabled, pixmap, text,
			     len, penColor );
}

/*!
  \reimp
*/
void QAquaStyle::drawPrimitive( PrimitiveElement pe,
				   QPainter *p,
				   const QRect &r,
				   const QColorGroup &cg,
				   SFlags flags,
				   const QStyleOption& opt ) const
{
    switch( pe ) {
    case PE_Panel:
    case PE_PanelLineEdit: {
	if(flags & Style_Sunken) {
	    if(!opt.isDefault() && opt.lineWidth() != 2) { //need to erase some..
		p->fillRect(r.x(), r.y(), r.x() + opt.lineWidth(), r.bottom(), cg.brush(QColorGroup::Background));
		p->fillRect(r.right()-opt.lineWidth(), r.y(), r.right(), r.bottom(), cg.brush(QColorGroup::Background));
		p->fillRect(r.x(), r.y(), r.right(), r.y() + opt.lineWidth(), cg.brush(QColorGroup::Background));
		p->fillRect(r.x(), r.bottom()-opt.lineWidth(), r.right(), r.bottom(), cg.brush(QColorGroup::Background));
	    }
	    //Top
	    p->setPen(QColor(120, 124, 120));
	    p->drawLine(r.x(), r.y(), r.right(), r.y());
	    p->setPen(QColor(192, 192, 192));
	    p->drawLine(r.x(), r.y() + 1, r.right(), r.y() + 1);
	    p->setPen(QColor(240, 240, 240));
	    p->drawLine(r.x() + 2, r.y() + 2, r.right() - 4, r.y() + 2);
	    //Bottom
	    p->setPen(QColor(200, 204, 200));
	    p->drawLine(r.x() + 1, r.bottom(), r.right() - 2, r.bottom());
	    p->setPen(QColor(232, 236, 232));
	    p->drawLine(r.x() + 2, r.bottom() - 1, r.right() - 4, r.bottom() - 1);
	    //Left/Right
	    p->setPen(QColor(184, 184, 184));
	    p->drawLine(r.x(), r.y() + 1, r.x(), r.bottom());
	    p->drawLine(r.right(), r.y() + 1, r.right(), r.bottom());
	    p->setPen(QColor(224, 224, 224));
	    p->drawLine(r.x() + 1, r.y() + 2, r.x() + 1, r.bottom() - 1);
	    p->drawLine(r.right() - 1, r.y() + 2, r.right() - 1, r.bottom() - 1);
	} else {
	    QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	}
	break; }

    case PE_PanelGroupBox: {
	if ( opt.isDefault() )
	    break;
	QColor clr = gray;
	//This is terrible, if I we just passed the widget in this wouldn't be necesary!
	QWidget *w = NULL;
	if(p && p->device() && p->device()->devType() == QInternal::Widget) {
	    w = (QWidget*)p->device();
	    if(w && w->parentWidget() && w->parentWidget()->inherits("QGroupBox"))
		clr = lightGray;
	}
	enum { HORIZ, VERT, SQUARE, NONE } kind = SQUARE;
	if(w && w->parentWidget() && w->parentWidget()->layout() && 
	   w->inherits("QGroupBox") && ((QGroupBox*)w)->title().isEmpty()) {
	    QPtrStack<QLayout> lays;
	    QLayoutItem *last = NULL;
	    lays.push(w->parentWidget()->layout());
	    while(QLayout *lay = lays.pop()) {
		if(lay->inherits("QBoxLayout")) {
		    QBoxLayout *blay = (QBoxLayout*)lay;
		    QLayoutIterator it = blay->iterator();
		    for(QLayoutItem *child; (child = it.current()); ++it) {
			if(child->widget() == w) {
			    if(last && last->widget() && 
			       last->widget()->inherits("QGroupBox") && 
			       ((QGroupBox*)last->widget())->title().isEmpty()) {
				if(blay->direction() == QBoxLayout::LeftToRight || 
				   blay->direction() == QBoxLayout::TopToBottom) 
				    kind = (blay->direction() == QBoxLayout::LeftToRight) ? VERT : HORIZ;
				else
				    kind = NONE;
			    } else {
				++it;
				if((child = it.current()) && child->widget() && 
				   child->widget()->inherits("QGroupBox") &&
				    ((QGroupBox*)child->widget())->title().isEmpty()) {
				    if(blay->direction() == QBoxLayout::LeftToRight || 
				       blay->direction() == QBoxLayout::TopToBottom) 
					kind = NONE;
				    else
					kind = (blay->direction() == QBoxLayout::RightToLeft) ? VERT : HORIZ;
				}
			    }
			    break;
			} else if(child->layout()) {
			    lays.push(child->layout());
			}
			if(child->widget()) //save the last child
			    last = child;
		    }
		}
	    }
	}
	p->setPen(clr);
	if(kind == VERT)
	    p->drawLine(r.x(), r.y()+15, r.x(), r.bottom()-15);
	else if(kind == HORIZ)
	    p->drawLine(r.x()+15, r.y(), r.right()-15, r.y());
	else if(kind == SQUARE)
	    p->drawRect(r);
	break; }

    case PE_HeaderArrow:
	if(flags & Style_Up)
	    drawPrimitive(PE_ArrowUp, p, QRect(r.x(), r.y()+2, r.width(), r.height()-4), cg, 0, opt);
	else
	    drawPrimitive(PE_ArrowDown, p, QRect(r.x(), r.y()+2, r.width(), r.height()-4), cg, 0, opt);
	break;

    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
	p->save();
	p->setPen( cg.text() );
	QPointArray a;
	if ( pe == PE_ArrowDown )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y(), r.x() + (r.width() / 2) , r.bottom());
	else if( pe == PE_ArrowRight )
	    a.setPoints( 3, r.x(), r.y(), r.right(), r.y() + (r.height() / 2), r.x(), r.bottom());
	else if( pe == PE_ArrowUp )
	    a.setPoints( 3, r.x() + (r.width() / 2), r.y(), r.right(), r.bottom(), r.x(), r.bottom());
	else
	    a.setPoints( 3, r.x(), r.y() + (r.height() / 2), r.right(), r.y(), r.right(), r.bottom());
	p->setBrush( cg.text() );
	p->drawPolygon( a );
	p->setBrush( NoBrush );
	p->restore();
	break; }

    case PE_HeaderSection: {
	QPixmap px;
	QString nstr = QString::number(r.height()), mod;
	if(flags & Style_Down )
	    mod = "down_";
	else if(flags & Style_Sunken && qAquaActive( cg ))
	    mod = "act_";
	qAquaPixmap( "hdr_" + mod + nstr, px );
	p->drawTiledPixmap( r, px );
	//separator
	p->save();
	p->setPen( gray );
	p->drawLine( r.right(), r.top(), r.right(), r.bottom() );
	p->restore();
	break; }

    case PE_ProgressBarChunk: {
	QPixmap px;
	qAquaPixmap( "progress_" + QString::number(r.height()), px );
	p->drawTiledPixmap( r, px, QPoint((r.x() % px.width()) - d->progressOff, 0) );
	break; }

    case PE_FocusRect:
	break;     // The Mac Aqua style doesn't use focus rectangles

    case PE_DockWindowHandle:
	{
	    p->save();
	    p->translate( r.x(), r.y() );

	    bool highlight = flags & Style_On;
	    QColor dark( cg.dark() );
	    QColor light( cg.light() );
	    unsigned int i;
	    if ( flags & Style_Horizontal ) {
		int h = r.height();
		if ( h > 6 ) {
		    if ( highlight )
			p->fillRect( 1, 1, 8, h - 2, cg.highlight() );
		    QPointArray a( 2 * ((h-6)/3) );
		    int y = 3 + (h%3)/2;
		    p->setPen( dark );
		    p->drawLine( 8, 1, 8, h-2 );
		    for( i=0; 2*i < a.size(); i ++ ) {
			a.setPoint( 2*i, 5, y+1+3*i );
			a.setPoint( 2*i+1, 2, y+2+3*i );
		    }
		    p->drawPoints( a );
		    p->setPen( light );
		    p->drawLine( 9, 1, 9, h-2 );
		    for( i=0; 2*i < a.size(); i++ ) {
			a.setPoint( 2*i, 4, y+3*i );
			a.setPoint( 2*i+1, 1, y+1+3*i );
		    }
		    p->drawPoints( a );
		}
	    } else {
		int w = r.width();
		if ( w > 6 ) {
		    if ( highlight )
			p->fillRect( 1, 1, w - 2, 9, cg.highlight() );
		    QPointArray a( 2 * ((w-6)/3) );

		    int x = 3 + (w%3)/2;
		    p->setPen( dark );
		    p->drawLine( 1, 8, w-2, 8 );
		    for( i=0; 2*i < a.size(); i ++ ) {
			a.setPoint( 2*i, x+1+3*i, 6 );
			a.setPoint( 2*i+1, x+2+3*i, 3 );
		    }
		    p->drawPoints( a );
		    p->setPen( light );
		    p->drawLine( 1, 9, w-2, 9 );
		    for( i=0; 2*i < a.size(); i++ ) {
			a.setPoint( 2*i, x+3*i, 5 );
			a.setPoint( 2*i+1, x+1+3*i, 2 );
		    }
		    p->drawPoints( a );
		}
		p->restore();
	    }
	}
	break;

    case PE_DockWindowSeparator: {
	QPixmap px;
	if( flags & Style_Horizontal )
	    qAquaPixmap( "tbar_hsep_" + QString::number(r.width())+ "_" + QString::number(r.height()), px );
	else
	    qAquaPixmap( "tbar_vsep_" + QString::number(r.width())+ "_" + QString::number(r.height()), px );

	p->drawPixmap( r.x(), r.y(), px );
	break; }

    case PE_TabBarBase: {
	QPixmap px;
	QString mod = "act";
	if( !qAquaActive( cg ) )
	    mod = "dis";
	if( flags & Style_Top )
	    qAquaPixmap( "tab_t_top_" + mod, px );
	else
	    qAquaPixmap( "tab_b_top_" + mod, px );
	p->drawTiledPixmap( r.x(), r.y(), r.width(), r.height(), px );
	break; }

    case PE_Indicator: {
	QPixmap px;
	bool down = flags & Style_Down;
	bool on  = flags  & Style_On;
	bool tri = flags & Style_NoChange;
	if( flags & Style_Enabled && qAquaActive( cg ) ){
	    if( down && on )
		qAquaPixmap("chk_psh_t", px);
	    else if( on )
		qAquaPixmap("chk_act_t", px);
	    else if ( down && tri )
		qAquaPixmap("chk_mid_psh_t", px );
	    else if ( tri )
		qAquaPixmap("chk_mid_t", px );
	    else if( !on && down )
		qAquaPixmap("chk_psh_f", px);
	    else
		qAquaPixmap("chk_act_f", px);
	} else {
	    if ( on )
		qAquaPixmap("chk_dis_t", px);
	    else if ( tri )
		qAquaPixmap("chk_mid_dis_t", px);
	    else
		qAquaPixmap("chk_act_f", px);
	}
	p->drawPixmap( r.x(), r.y(), px, 0, 1 );
	break; }

    case PE_IndicatorMask: {
	p->fillRect(r.x(), r.y(), r.width(), r.height() - 1, color1);
	break; }

    case PE_ExclusiveIndicator: {
	QPixmap px;
	bool down = flags & Style_Down;
	bool on  = flags  & Style_On;
	bool enabled = flags & Style_Enabled;
	if( enabled  && qAquaActive( cg ) ){
	    if( down && on )
		qAquaPixmap("radio_psh_t", px);
	    else if( on )
		qAquaPixmap("radio_t", px);
	    else if( !on && down )
		qAquaPixmap("radio_psh_f", px);
	    else
		qAquaPixmap("radio_f", px);
	} else {
	    if(on)
		qAquaPixmap("radio_dis_t", px) ;
	    else
		qAquaPixmap("radio_f", px);
	}
	p->drawPixmap( r.x(), r.y(), px, 0, 0 );
	break; }

    case PE_ExclusiveIndicatorMask: {
	QBitmap radio_mask( aqua_radio_mask_xbm_width,
			    aqua_radio_mask_xbm_height,
			    (const uchar *) aqua_radio_mask_xbm_bits, TRUE );
	p->drawPixmap( r.x(), r.y()+1, radio_mask, 0, 1 );
	break; }

#ifndef QT_NO_SCROLLBAR
    case PE_ScrollBarAddLine:
    case PE_ScrollBarSubLine: {
	QPixmap arrow;
	p->setBackgroundMode( OpaqueMode );
	QString mod, prefix, join;
	if(flags & Style_Down )
	    mod += "psh_";
	if( flags & Style_Horizontal ) {
	    prefix = "h";
	    if(pe == PE_ScrollBarAddLine)
		mod += "right_";
	    else
		mod += "left_";
	    mod += QString::number(r.height());
	} else {
	    prefix = "v";
	    if(pe == PE_ScrollBarAddLine)
		mod += "down_";
	    else
		mod += "up_";
	    mod += QString::number(r.width());
	}
	if( qt_mac_scrollbar_arrows_together )
	    join = "joined_";
	qAquaPixmap( prefix + "sbr_" + join + "arw_" + mod, arrow );
	p->drawPixmap( r.x(), r.y(), arrow );
	break; }

    case PE_ScrollBarSubPage:
    case PE_ScrollBarAddPage: {
	QPixmap fill;
	p->setBackgroundMode( OpaqueMode );
	QString prefix="v";
	if( flags & Style_Horizontal )
	    prefix = "h";
	qAquaPixmap( prefix + "sbr_back_fill_" + QString::number(
	    flags & Style_Horizontal ? r.height() : r.width()), fill );
	QRect fillr = r;
	if( pe == PE_ScrollBarSubPage && qt_mac_scrollbar_arrows_together) {
	    QPixmap cap;
	    qAquaPixmap( prefix + "sbr_joined_back_cap_" + QString::number(
		flags & Style_Horizontal ? r.height() : r.width()), cap );
	    p->drawPixmap( r.x(), r.y(), cap );
	    if(flags & Style_Horizontal)
		fillr.setX( r.x() + cap.width() );
	    else
		fillr.setY( r.y() + cap.height() );
	}
	p->drawTiledPixmap( fillr, fill );
	break; }

    case PE_ScrollBarSlider: {
	QPixmap tip1, tip2, fill;
	QBitmap tip1m, tip2m;
	QRect real_rect;
	QString act;
	if(!qAquaActive( cg ) )
	    act = "dis_";
	if( flags & Style_Horizontal ) {
	    QString nstr = QString::number(r.height());
	    qAquaPixmap( "hsbr_tip_" + act + "left_" + nstr, tip1 );
	    qAquaPixmap( "hsbr_tip_" + act + "right_" + nstr, tip2 );
	    qAquaPixmap( "hsbr_" + act + "fill_" + nstr, fill );
	    p->drawPixmap( r.x(), r.y(), tip1 );
	    p->drawPixmap( r.x()+r.width()-tip2.width(), r.y(), tip2 );
	    real_rect = QRect(r.x() + tip1.width(), r.y(), r.width() - tip2.width() - tip1.width(),
			      r.height());
	} else {
	    QString nstr = QString::number(r.width());
	    qAquaPixmap( "vsbr_tip_" + act + "up_" + nstr, tip1 );
	    qAquaPixmap( "vsbr_tip_" + act + "down_" + nstr, tip2 );
	    qAquaPixmap( "vsbr_" + act + "fill_" + nstr, fill );
	    p->drawPixmap( r.x(), r.y(), tip1 );
	    p->drawPixmap( r.x(), r.y()+r.height()-tip2.height(), tip2 );
	    real_rect = QRect(r.x(), r.y() + tip1.height(), r.width(),
			      r.height() - tip2.height() - tip1.height());
	}
	p->fillRect( real_rect, QBrush( Qt::black, fill ) );
	break; }
#endif
    default:
	QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	break;
    }
}

/*!
  \reimp
*/
void QAquaStyle::drawControl( ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags how,
				 const QStyleOption& opt ) const
{
    SFlags flags = Style_Default;
    if (widget->isEnabled())
	flags |= Style_Enabled;

    switch(element) {
    case CE_PopupMenuScroller: {
	p->fillRect( r.x(), r.y(), r.width(), r.height(), cg.brush( QColorGroup::Button ));
	const int w = 10, x = (r.width() / 2) - (w / 2), 
		  h = 10, y = (r.height() / 2) - (h / 2);
	drawPrimitive((how & Style_Down) ? PE_ArrowDown : PE_ArrowUp, p, 
		      QRect(r.x() + x, r.y() + y, w, h), cg, flags, opt);
	break; }
    case CE_TabBarTab: {
#ifndef QT_NO_TABBAR
	if(!widget)
	    break;

	QPixmap left, mid, right;
	QTabBar * tb = (QTabBar *) widget;
	bool selected = how & Style_Selected;
	QString pos;

	if( tb->shape() == QTabBar::RoundedAbove )
	    pos = "t";
	else
	    pos = "b";

	QString hstr = QString::number( r.height() );
	if( qAquaActive( tb->colorGroup() ) ){
	    if( (how & Style_Sunken) && (how & Style_MouseOver) ) {
		qAquaPixmap( "tab_"+ pos +"_prs_left_" + hstr, left );
		qAquaPixmap( "tab_"+ pos +"_prs_mid_" + hstr, mid );
		qAquaPixmap( "tab_"+ pos +"_prs_right_" + hstr, right );
	    } else if( selected ){
		qAquaPixmap( "tab_"+ pos +"_act_left_" + hstr, left );
		qAquaPixmap( "tab_"+ pos +"_act_mid_" + hstr, mid );
		qAquaPixmap( "tab_"+ pos +"_act_right_" + hstr, right );
	    } else {
		qAquaPixmap( "tab_"+ pos +"_dis_left_" + hstr, left );
		qAquaPixmap( "tab_"+ pos +"_dis_mid_" + hstr, mid );
		qAquaPixmap( "tab_"+ pos +"_dis_right_" + hstr , right );
	    }
	} else {
	    if( selected ){
		qAquaPixmap( "tab_"+ pos +"_sel_dis_left_" + hstr, left );
		qAquaPixmap( "tab_"+ pos +"_sel_dis_mid_" + hstr, mid );
		qAquaPixmap( "tab_"+ pos +"_sel_dis_right_" + hstr, right );
	    } else {
		qAquaPixmap( "tab_"+ pos +"_usel_dis_left_" + hstr, left );
		qAquaPixmap( "tab_"+ pos +"_usel_dis_mid_" + hstr, mid );
		qAquaPixmap( "tab_"+ pos +"_usel_dis_right_" + hstr, right );
	    }
	}

	p->drawPixmap( r.x(), r.y(), left );
	p->drawTiledPixmap( r.x() + left.width(), r.y(), r.width()-left.width()*2,
			    r.height(), mid );
	p->drawPixmap( r.x() + r.width() - right.width(), r.y(), right );
#endif
	break; }

    case CE_PopupMenuItem: {
#ifndef QT_NO_POPUPMENU
	if(!widget || opt.isDefault())
	    break;
	QPopupMenu *popupmenu = (QPopupMenu *)widget;
	QMenuItem *mi = opt.menuItem();
	int x, y, w, h;
	r.rect(&x, &y, &w, &h);
	if ( !mi ) {
	    p->fillRect( x, y, w, h, cg.brush( QColorGroup::Button ));
	    break;
	}

	const QColorGroup & g = cg;
	QColorGroup itemg = g;
	bool dis = !mi->isEnabled();
	int tab = opt.tabWidth();
	int maxpmw = opt.maxIconWidth();
	bool checked = mi->isChecked();
	bool checkable = popupmenu->isCheckable();
	bool act = how & Style_Active;

	if ( checkable )
	    maxpmw = QMAX( maxpmw, 12 ); // space for the checkmarks

	int checkcol = maxpmw;
	QPixmap selectedBackground;
	qAquaPixmap( "sel_back", selectedBackground );

	if ( mi && mi->isSeparator() ) { // Aqua separators are just empty menuitems
	    p->fillRect( r, g.brush( QColorGroup::Button ) );
	    return;
	}

	QBrush fill = act && !dis ? QBrush( Qt::black, selectedBackground ) :
	    g.brush( QColorGroup::Button );
	p->fillRect( x, y, w, h, fill);
	bool reverse = QApplication::reverseLayout();

	int xpos = x;
	if ( reverse )
	    xpos += w - checkcol;

	if ( mi && mi->iconSet() ) {              // draw iconset
	    if ( checked ) {
		QRect vrect = visualRect( QRect( xpos, y, checkcol, h ), r );
		if ( act && !dis ) {
		    qDrawShadePanel( p, vrect.x(), y, checkcol, h,
				     cg, TRUE, 1, &cg.brush( QColorGroup::Button ) );
		} else {
		    QBrush fill( cg.light(), Dense4Pattern );
		    // set the brush origin for the hash pattern to the x/y coordinate
		    // of the menu item's checkmark... this way, the check marks have
		    // a consistent look
		    QPoint origin = p->brushOrigin();
		    p->setBrushOrigin( vrect.x(), y );
		    qDrawShadePanel( p, vrect.x(), y, checkcol, h, cg, TRUE, 1,
				     &fill );
		    // restore the previous brush origin
		    p->setBrushOrigin( origin );
		}
	    } else if ( !act ) {
		p->fillRect(xpos, y, checkcol, h,
			    g.brush( QColorGroup::Button ));
	    }

	    QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
	    if (act && !dis )
		mode = QIconSet::Active;
	    QPixmap pixmap;
	    if ( checkable && mi->isChecked() )
		pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode, QIconSet::On );
	    else
		pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
	    int pixw = pixmap.width();
	    int pixh = pixmap.height();
	    if ( act && !dis ) {
		if ( !mi->isChecked() )
		    qDrawShadePanel( p, xpos, y, checkcol, h, g, FALSE, 1,
				     &g.brush( QColorGroup::Button ) );
	    }
	    QRect cr( xpos, y, checkcol, h );
	    QRect pmr( 0, 0, pixw, pixh );
	    pmr.moveCenter( cr.center() );
	    p->setPen( itemg.text() );
	    p->drawPixmap( pmr.topLeft(), pixmap );

	    QBrush fill = act? QBrush( Qt::black, selectedBackground ) :
				  g.brush( QColorGroup::Button );
	    int xp;
	    if ( reverse )
		xp = x;
	    else
		xp = xpos + checkcol + 1;
	    p->fillRect( xp, y, w - checkcol - 1, h, fill);
	} else  if ( checkable ) {  // just "checking"...
	    int mw = checkcol + aquaItemFrame;
	    int mh = h - 2*aquaItemFrame;
	    if ( mi->isChecked() ) {
		int xp = xpos;
		if( reverse )
		    xp -= aquaItemFrame;
		else
		    xp += aquaItemFrame;

		SFlags cflags = Style_Default;
		if (! dis)
		    cflags |= Style_Enabled;
		if (act)
		    cflags |= Style_On;
		drawPrimitive(PE_CheckMark, p, QRect(xp, y+aquaItemFrame, mw, mh), cg, cflags);
	    }
	}

	p->setPen( act ? Qt::white/*g.highlightedText()*/ : g.buttonText() );

	QColor discol;
	if ( dis ) {
	    discol = itemg.text();
	    p->setPen( discol );
	}

	int xm = aquaItemFrame + checkcol + aquaItemHMargin;
	if ( reverse )
	    xpos = aquaItemFrame + tab;
	else
	    xpos += xm;

	if ( mi && mi->custom() ) {
	    int m = aquaItemVMargin;
	    p->save();
	    if ( dis && !act ) {
		p->setPen( g.light() );
		mi->custom()->paint( p, itemg, act, !dis,
				     xpos+1, y+m+1, w-xm-tab+1, h-2*m );
		p->setPen( discol );
	    }
	    mi->custom()->paint( p, itemg, act, !dis,
				 x+xm, y+m, w-xm-tab+1, h-2*m );
	    p->restore();
	}
	QString s = mi->text();
	if ( !s.isNull() ) {                        // draw text
	    int t = s.find( '\t' );
	    int m = aquaItemVMargin;
	    const int text_flags = AlignVCenter|NoAccel | DontClip | SingleLine;
	    if ( t >= 0 ) {                         // draw tab text
		int xp;
		if( reverse )
		    xp = x + aquaRightBorder+aquaItemHMargin+aquaItemFrame - 1;
		else
		    xp = x + w - tab - aquaRightBorder-aquaItemHMargin-aquaItemFrame+1;
		if ( dis && !act ) {
		    p->setPen( g.light() );
		    p->drawText( xp, y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
		    p->setPen( discol );
		}
		p->drawText( xp, y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
		s = s.left( t );
	    }
	    if ( dis && !act ) {
		p->setPen( g.light() );
		p->drawText( xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t );
		p->setPen( discol );
	    }
	    p->drawText( xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
	} else if ( mi && mi->pixmap() ) {                        // draw pixmap
	    QPixmap *pixmap = mi->pixmap();
	    if ( pixmap->depth() == 1 )
		p->setBackgroundMode( OpaqueMode );
	    p->drawPixmap( xpos, y+aquaItemFrame, *pixmap );
	    if ( pixmap->depth() == 1 )
		p->setBackgroundMode( TransparentMode );
	}
	if ( mi->popup() ) {                        // draw sub menu arrow
	    int dim = (h-2*aquaItemFrame) / 2;
	    PrimitiveElement arrow;
	    if ( reverse ) {
		arrow = PE_ArrowLeft;
		xpos = x + aquaArrowHMargin + aquaItemFrame;
	    } else {
		arrow = PE_ArrowRight;
		xpos = x+w - aquaArrowHMargin - aquaItemFrame - dim;
	    }
	    if ( act ) {
		if ( !dis )
		    discol = white;
		QColorGroup g2( discol, cg.highlight(),
				white, white,
				dis ? discol : white,
				discol, white );

		drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
			      g2, Style_Enabled);
	    } else {
		drawPrimitive(arrow, p, QRect(xpos, y + h / 2 - dim / 2, dim, dim),
			      cg, mi->isEnabled() ? Style_Enabled : Style_Default);
	    }
	}
#endif
	break; }

    case CE_MenuBarItem: {
	if (opt.isDefault())
	    break;

	QMenuItem *mi = opt.menuItem();
	if (!mi)
	    break;
	bool down = how & Style_Down;
	bool active = how & Style_Active;
	if( down && active ){
	    QPixmap px;
	    qAquaPixmap( "sel_back", px );
	    p->fillRect( r, QBrush( black, px ) );
	} else {
	    p->fillRect( r, cg.brush( QColorGroup::Button ) );
	}
	drawItem( p, r, AlignCenter|DontClip|SingleLine|ShowPrefix,
		  cg, mi->isEnabled(), mi->pixmap(), mi->text(), -1,
		  (down && active) ? &white : &cg.buttonText() );
	break; }

    case CE_PushButton: {
#ifndef QT_NO_PUSHBUTTON
	if(!widget)
	    break;
	QPixmap left, mid, right;
	QColorGroup g = widget->colorGroup();
	QPushButton *btn = (QPushButton *)widget;
	d->addWidget(btn);
	int x=r.x(), y=r.y(), w=r.width(), h=r.height();

	// ### What about buttons that are so small that the pixmaps don't fit?
	if( w < 33 ){
	    drawPrimitive( PE_ButtonCommand, p, r, cg, how, opt);
	    return;
	}

	QString hstr = QString::number( h - y );
	if( d->animatable(QAquaAnimate::AquaPushButton, btn) ) {
	    int & alt = d->buttonState.frame;
	    int & dir = d->buttonState.dir;
	    if( alt > 0 && !(how & Style_Down) ){
		QString num = QString::number( alt );
		qAquaPixmap( "btn_def_left" + num + "_" + hstr, left );
		qAquaPixmap( "btn_def_mid" + num + "_" + hstr, mid );
		qAquaPixmap( "btn_def_right" + num + "_" + hstr, right );
	    } else {
		qAquaPixmap( "btn_def_left_" + hstr, left );
		qAquaPixmap( "btn_def_mid_" + hstr, mid );
		qAquaPixmap( "btn_def_right_" + hstr, right );
	    }
	    // Pause animation if button is down
	    if( how & Style_Down ) {
		dir = 1;
		alt = 0;
	    } else {
		if( (dir == 1) && (alt == 9) ) dir = -1;
		if( (dir == -1) && (alt == 0) ) dir = 1;
		alt += dir;
	    }
	} else if ( how & Style_Down ) {
	    if(btn->isFlat()) {
		qAquaPixmap( "btn_def_mir_left_" + hstr, left );
		qAquaPixmap( "btn_def_mir_mid_" + hstr, mid );
		qAquaPixmap( "btn_def_mir_right_" + hstr, right );
	    } else {
		qAquaPixmap( "btn_def_left_" + hstr, left );
		qAquaPixmap( "btn_def_mid_" + hstr, mid );
		qAquaPixmap( "btn_def_right_" + hstr, right );
	    }
	} else if ( !(how & Style_Enabled) ) {
	    qAquaPixmap( "btn_dis_left_" + hstr, left );
	    qAquaPixmap( "btn_dis_mid_" + hstr, mid );
	    qAquaPixmap( "btn_dis_right_" + hstr, right );
	} else if ( how & Style_On ) {
	    qAquaPixmap( "btn_def_mir_left_" + hstr, left );
	    qAquaPixmap( "btn_def_mir_mid_" + hstr, mid );
	    qAquaPixmap( "btn_def_mir_right_" + hstr, right );
	} else if( how & Style_Raised ) {
	    qAquaPixmap( "btn_nrm_left_" + hstr, left );
	    qAquaPixmap( "btn_nrm_mid_" + hstr, mid );
	    qAquaPixmap( "btn_nrm_right_" + hstr, right );
	}

	if(!mid.isNull()) {
	    QBrush mid_f( Qt::black, mid );
	    p->drawPixmap( x, y, left );
	    p->drawTiledPixmap( x+left.width(), y, w-x-left.width()*2, h-y, mid );
	    p->drawPixmap( w-right.width(), y, right );
	}
#endif
	break; }

    case CE_PushButtonLabel: {
#ifndef QT_NO_PUSHBUTTON
	if(!widget)
	    break;
	QPushButton *btn = (QPushButton *)widget;
	bool on = btn->isDown() || btn->isOn();
	int x, y, w, h;
	r.rect( &x, &y, &w, &h );
	if ( btn->isMenuButton() ) {
	    int dx = pixelMetric(PM_MenuButtonIndicator, widget);

	    QColorGroup g( btn->colorGroup() );
	    int xx = x+w-dx-4;
	    int yy = y-3;
	    int hh = h+6;

	    if ( !on ) {
		p->setPen( g.mid() ); // mid
		p->drawLine(xx, yy+2, xx, yy+hh-3);
		p->setPen( g.button() );
		p->drawLine(xx+1, yy+1, xx+1, yy+hh-2);
		p->setPen( g.light() );
		p->drawLine(xx+2, yy+2, xx+2, yy+hh-2);
	    }
	    QRect ar(r.right() - dx, r.y() + 2, dx - 4, r.height() - 4);
	    drawPrimitive(PE_ArrowDown, p, ar, cg, how, opt);
	    w -= dx;
	}

	if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
	    QIconSet::Mode mode = btn->isEnabled()
				  ? QIconSet::Normal : QIconSet::Disabled;
	    if ( mode == QIconSet::Normal && btn->hasFocus() )
		mode = QIconSet::Active;
	    QIconSet::State state = QIconSet::Off;
	    if ( btn->isToggleButton() && btn->isOn() )
		state = QIconSet::On;
	    QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode, state );
	    int pixw = pixmap.width();
	    int pixh = pixmap.height();
	    p->drawPixmap( x+2, y+h/2-pixh/2, pixmap );
	    x += pixw + 4;
	    w -= pixw + 4;
	}

	drawItem( p, QRect(x, y, w, h), AlignCenter|ShowPrefix, cg, btn->isEnabled(),
		  btn->pixmap(), btn->text(), -1);
#endif
	break; }

    default:
	QWindowsStyle::drawControl( element, p, widget, r, cg, how, opt);
	break;
    }
}

/*!
  \reimp
*/
int QAquaStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    int ret = 0;
    switch(metric) {
    case PM_PopupMenuScrollerHeight:
	ret = 10;
	break;
    case PM_DefaultFrameWidth:
	if(widget && 
	   (widget->isTopLevel() || !widget->parentWidget() || widget->parentWidget()->isTopLevel()) &&  
	   (widget->inherits("QScrollView") || widget->inherits("QWorkspaceChild"))) 
	    ret = 0;
	else if(widget && widget->inherits("QLineEdit")) 
	    ret = 2;
	else 
	    ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    case PM_TitleBarHeight:
	ret = 16;
	break;
    case PM_ScrollBarSliderMin:
	ret = 24;
	break;
    case PM_SpinBoxFrameWidth:
    case PM_ProgressBarChunkWidth:
	ret = 1;
	break;
    case PM_DockWindowSeparatorExtent:
    case PM_TabBarBaseHeight:
	ret = 8;
	break;
    case PM_TabBarBaseOverlap:
	ret = 2;
	break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;
    case PM_MaximumDragDistance:
	ret = -1;
	break;
    case PM_SliderLength:
	ret = 17;
	break;
    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;
    case PM_IndicatorHeight:
	ret = 18;
	break;
    case PM_IndicatorWidth:
	ret = 15;
	break;
    case PM_ExclusiveIndicatorWidth:
	ret = 14;
	break;
    case PM_ExclusiveIndicatorHeight:
	ret = 17;
	break;
    case PM_ScrollBarExtent:
	ret = 14;
	break;
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
}

/*!
  \reimp
*/
QSize QAquaStyle::sizeFromContents( ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       const QStyleOption& opt ) const
{
    QSize sz(contentsSize);
    switch(contents) {
    case CT_PopupMenuItem: {
#ifndef QT_NO_POPUPMENU
	if(!widget || opt.isDefault())
	    break;
	const QPopupMenu *popup = (const QPopupMenu *) widget;
	bool checkable = popup->isCheckable();
	QMenuItem *mi = opt.menuItem();
	int maxpmw = opt.maxIconWidth();
	int w = sz.width(), h = sz.height();

	if (mi->custom()) {
	    w = mi->custom()->sizeHint().width();
	    h = mi->custom()->sizeHint().height();
	    if (! mi->custom()->fullSpan())
		h += 8;
	} else if ( mi->widget() ) {
	} else if (mi->isSeparator()) {
	    w = 10;
	    h = aquaSepHeight;
	} else {
	    if (mi->pixmap())
		h = QMAX(h, mi->pixmap()->height() + 4);
	    else
		h = QMAX(h, popup->fontMetrics().height() + 8);

	    if (mi->iconSet() != 0)
		h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						  QIconSet::Normal).height() + 4);
	}

	if (! mi->text().isNull()) {
	    if (mi->text().find('\t') >= 0)
		w += 12;
	}

	if (maxpmw)
	    w += maxpmw + 6;
	if (checkable && maxpmw < 20)
	    w += 20 - maxpmw;
	if (checkable || maxpmw > 0)
	    w += 2;
	w += 12;
	if(widget->parentWidget() && widget->parentWidget()->inherits("QComboBox"))
	    w = QMAX(w, widget->parentWidget()->width() - 20);
	sz = QSize(w, h);
#endif
	break; }

    case CT_PushButton:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	if(widget->width() > 32)
	    sz.setWidth(sz.width() + 16);
	break;

    default:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	break;
    }
    return sz;
}

/*!
  \reimp
*/
QRect QAquaStyle::subRect( SubRect r, const QWidget *w ) const
{
    QRect ret, wrect(w->rect());
    switch(r) {
    case SR_PushButtonContents:
	ret = QWindowsStyle::subRect(r, w);
	if(w->width() > 32) {
	    ret.setTop( ret.top()+1);
	    ret.setLeft( ret.left()+8);
	    ret.setBottom( ret.bottom()-1);
	    ret.setRight( ret.right()-8);
	}
	break;

    case SR_ComboBoxFocusRect: {
	QRect wrect = w->rect();
	ret = QRect(wrect.x()+4, wrect.y()+4, wrect.width()-8-20, wrect.height()-8);
	break; }

    case SR_RadioButtonContents: {
	QRect ir = subRect(SR_RadioButtonIndicator, w);
	ret.setRect(ir.right() + 5, wrect.y(),
		    wrect.width() - ir.width() - 5, wrect.height());
	break; }

    case SR_CheckBoxContents: {
	QRect ir = subRect(SR_CheckBoxIndicator, w);
	ret.setRect(ir.right() + 5, wrect.y(),
		    wrect.width() - ir.width() - 5, wrect.height());
	break; }

    default:
	ret = QWindowsStyle::subRect(r, w);
	break;
    }
    return ret;
}

/*!
  \reimp
*/
void QAquaStyle::drawComplexControl( ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt ) const
{
    switch(ctrl) {
#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	if(!widget)
	    break;
	QScrollBar *scrollbar = (QScrollBar *) widget;
	if(!scrollbar->minValue() && !scrollbar->maxValue()) { //just a "groove"
	    QPixmap fill;
	    QString prefix="v";
	    int size = r.width();
	    if( flags & Style_Horizontal ) {
		prefix = "h";
		size = r.height();
	    }
	    qAquaPixmap(prefix + "sbr_back_fill_" + QString::number(size), fill);
	    p->drawTiledPixmap(r, fill);
	    break;
	}

	sub = 0xFFFFFFF; //bleh, must paint all?
	QRect addline, subline, addpage, subpage, slider, first, last;
	bool maxedOut = (scrollbar->minValue() == scrollbar->maxValue());

	subline = querySubControlMetrics(ctrl, widget, SC_ScrollBarSubLine, opt);
	addline = querySubControlMetrics(ctrl, widget, SC_ScrollBarAddLine, opt);
	subpage = querySubControlMetrics(ctrl, widget, SC_ScrollBarSubPage, opt);
	addpage = querySubControlMetrics(ctrl, widget, SC_ScrollBarAddPage, opt);
	slider  = querySubControlMetrics(ctrl, widget, SC_ScrollBarSlider,  opt);
	first   = querySubControlMetrics(ctrl, widget, SC_ScrollBarFirst,   opt);
	last    = querySubControlMetrics(ctrl, widget, SC_ScrollBarLast,    opt);

	if ((sub & SC_ScrollBarSubPage) && subpage.isValid())
	    drawPrimitive(PE_ScrollBarSubPage, p, subpage, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarSubPage) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarAddPage) && addpage.isValid())
	    drawPrimitive(PE_ScrollBarAddPage, p, addpage, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarAddPage) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarFirst) && first.isValid())
	    drawPrimitive(PE_ScrollBarFirst, p, first, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarFirst) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarLast) && last.isValid())
	    drawPrimitive(PE_ScrollBarLast, p, last, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarLast) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarSubLine) && subline.isValid())
	    drawPrimitive(PE_ScrollBarSubLine, p, subline, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarSubLine) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarAddLine) && addline.isValid())
	    drawPrimitive(PE_ScrollBarAddLine, p, addline, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarAddLine) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));
	if ((sub & SC_ScrollBarSlider) && slider.isValid()) {
	    //cleanup
	    QRect eraserect(slider);
	    if(scrollbar->orientation() == Qt::Vertical) {
		if(!qt_mac_scrollbar_arrows_together && eraserect.y() < subline.height())
		    eraserect.setY(subline.height());
		if(eraserect.bottom() > addline.y())
		    eraserect.setBottom(addline.y());
	    } else {
		if(!qt_mac_scrollbar_arrows_together && eraserect.x() < subline.width())
		    eraserect.setX(subline.width());
		if(eraserect.right() > addline.x())
		    eraserect.setRight(addline.x());
	    }
	    if(eraserect.isValid())
		drawPrimitive(PE_ScrollBarAddPage, p, eraserect, cg,
			      ((maxedOut) ? Style_Default : Style_Enabled) |
			      ((subActive == SC_ScrollBarAddPage) ?
			       Style_Down : Style_Default) |
			      ((scrollbar->orientation() == Qt::Horizontal) ?
			       Style_Horizontal : 0));
	    //now draw
	    drawPrimitive(PE_ScrollBarSlider, p, slider, cg,
			  ((maxedOut) ? Style_Default : Style_Enabled) |
			  ((subActive == SC_ScrollBarSlider) ?
			   Style_Down : Style_Default) |
			  ((scrollbar->orientation() == Qt::Horizontal) ?
			   Style_Horizontal : 0));

	    // ### perhaps this should not be able to accept focus if maxedOut?
	    if (scrollbar->hasFocus()) {
		QRect fr(slider.x() + 2, slider.y() + 2,
			 slider.width() - 5, slider.height() - 5);
		drawPrimitive(PE_FocusRect, p, fr, cg, Style_Default);
	    }
	}
	break; }
#endif

    case CC_TitleBar: {
	if(!widget)
	    break;
	if(sub) {
	    QTitleBar *tb = (QTitleBar *)widget;
	    int ctrlw = 0;
	    if(tb->window())
		ctrlw = 55;
	    else if(tb->testWFlags( WStyle_SysMenu))
		ctrlw = 17;
	    if ( sub & (SC_TitleBarMinButton|SC_TitleBarCloseButton|SC_TitleBarMaxButton) ) {
		QPixmap ctrl;
		if(flags & Style_MouseOver)
		    qAquaPixmap( "win_act_controls", ctrl );
		else
		    qAquaPixmap( "win_dis_controls", ctrl );
		p->drawPixmap(0, 0, ctrl, 0, 0, ctrlw, 16);
	    }

	    if(sub & SC_TitleBarLabel) {
		int x = ctrlw, y = 0, iw = 0;
		if(tb->icon())
		   iw = tb->icon()->width() + 3;
		QPixmap fill;
		qAquaPixmap( "win_fill", fill );
		p->drawTiledPixmap(x, 0, tb->width() - x, fill.height(), fill);
		QColorGroup cgroup = tb->usesActiveColor() ?
				     tb->palette().active() : tb->palette().inactive();
		p->setPen( cgroup.highlightedText() );
		p->save();
		p->setClipRect( x, 0, tb->width() - x, tb->height() );
		if((tb->width() - x) <= (p->fontMetrics().width(tb->caption())+iw*2))
		    x += iw;
		else
		    x += ((tb->width() - x) / 2) - ( p->fontMetrics().width(tb->visibleText()) / 2);
		y = (tb->height() / 2) - ( p->fontMetrics().height() / 2 );
		if(tb->icon() && !tb->caption().isEmpty())
		    p->drawPixmap(x - iw, y, *tb->icon());
		p->drawText( x, y + p->fontMetrics().ascent(), tb->visibleText() );
		p->restore();
	    }
	}
	break; }

    case CC_ListView: {
	if ( sub & SC_ListView ) {
	    QWindowsStyle::drawComplexControl( ctrl, p, widget, r, cg, flags, sub, subActive, opt );
	}
	if ( sub & ( SC_ListViewBranch | SC_ListViewExpand ) ) {
	    if (opt.isDefault())
		break;
	    QListViewItem *item = opt.listViewItem();
	    int y=r.y(), h=r.height();
	    QColorGroup cg2 = cg;
	    cg2.setColor(QColorGroup::Text, gray); //ick want grayish expansions
	    for(QListViewItem *child = item->firstChild(); child && y < h;
		y += child->totalHeight(), child = child->nextSibling()) {
		if(y + child->height() > 0) {
		    if ( child->isExpandable() || child->childCount() )
			drawPrimitive( child->isOpen() ? PE_ArrowDown : PE_ArrowRight, p,
				       QRect(r.right() - 10, (y + child->height()/2) - 4, 9, 9), cg2 );
		}
	    }
	}
	break; }

    case CC_SpinWidget: {
	QPixmap btn;
	if(sub & SC_SpinWidgetUp) {
	    QRect sr = visualRect( querySubControlMetrics( CC_SpinWidget, widget,
							   SC_SpinWidgetUp ), widget );
	    QString wstr = QString::number( sr.width() );
	    QString hstr = QString::number( sr.height() );
	    if ( subActive & SC_SpinWidgetUp )
		qAquaPixmap( "spinbtn_up_on_" + wstr + "_" + hstr, btn );
	    else
		qAquaPixmap( "spinbtn_up_off_" + wstr + "_" + hstr, btn );
	    p->drawPixmap( sr.x(), sr.y(), btn );
	}
	if(sub & SC_SpinWidgetDown) {
	    QRect sr = visualRect( querySubControlMetrics( CC_SpinWidget, widget,
							   SC_SpinWidgetDown ), widget );
	    QString wstr = QString::number( sr.width() );
	    QString hstr = QString::number( sr.height() );
	    if ( subActive & SC_SpinWidgetDown )
		qAquaPixmap( "spinbtn_down_on_" + wstr + "_" + hstr, btn );
	    else
		qAquaPixmap( "spinbtn_down_off_" + wstr + "_" + hstr, btn );
	    p->drawPixmap( sr.x(), sr.y(), btn );
	}
	if(sub & SC_SpinWidgetFrame)
	    QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, SC_SpinWidgetFrame, subActive, opt);
	break; }

    case CC_Slider: {
	if(!widget)
	    break;
	QSlider *sldr = (QSlider *)widget;

	if ( sub == SC_None )
	    sub = SC_SliderGroove | SC_SliderTickmarks | SC_SliderHandle;

	if ( sub & SC_SliderGroove ) {
	    int offset = 3;
	    QPixmap sldr_l, sldr_mid, sldr_r;
	    QString dir("h");
	    QSlider * s = (QSlider *) widget;
	    if ( s->orientation() == Vertical )
		dir = "v";
	    if ( s->tickmarks() == QSlider::Above )
		offset = 10;
	    qAquaPixmap( "sldr_" + dir + "grv_tip_left", sldr_l );
	    qAquaPixmap( "sldr_" + dir + "grv_mid", sldr_mid );
	    qAquaPixmap( "sldr_" + dir + "grv_tip_right", sldr_r );

	    if ( s->orientation() == Horizontal ){
		p->drawPixmap( r.x(), r.y() + offset, sldr_l );
		p->drawTiledPixmap( r.x() + sldr_l.width(), r.y() + offset, r.width() - sldr_l.width()*2,
				    sldr_l.height(), sldr_mid );
		p->drawPixmap( r.x() + r.width() - sldr_r.width(), r.y() + offset, sldr_r );
	    } else {
		p->drawPixmap( r.x() + offset, r.y(), sldr_r );
		p->drawTiledPixmap( r.x() + offset, r.y() + sldr_l.height(), sldr_mid.width(),
				    r.height() - sldr_l.height()*2, sldr_mid );
		p->drawPixmap( r.x() + offset, r.y() + r.height() - sldr_l.height(), sldr_l );
	    }
	}
	if ( sub & SC_SliderTickmarks ) {
	    QColorGroup cg2 = cg;
	    cg2.setColor(QColorGroup::Foreground, gray); //ick, just want grayish ticks
	    QWindowsStyle::drawComplexControl( ctrl, p, widget, r, cg2, flags,
					       SC_SliderTickmarks, subActive,
					       opt );
	}
	if ( sub & SC_SliderHandle ) {
	    QRect re = querySubControlMetrics( CC_Slider, widget, SC_SliderHandle, opt );
	    QPixmap px;
	    QString hstr = QString::number( re.height() );
	    QString dir;
	    if ( sldr->orientation() == Horizontal ){
		if ( sldr->tickmarks() == QSlider::Above )
		    dir = "up";
		else
		    dir = "down";
	    } else if ( sldr->orientation() == Vertical ) {
		if ( sldr->tickmarks() == QSlider::Above )
		    dir = "left";
		else
		    dir = "right";
	    }
	    if( qAquaActive( cg ) ) {
		qAquaPixmap( "sldr_act_pty_" + dir + "_" + hstr, px );
	    } else {
		qAquaPixmap( "sldr_dis_pty_" + dir + "_" + hstr, px );
	    }
	    sldr->erase(re.x(), re.y(), px.width(), px.height());
	    p->drawPixmap( re.x(), re.y(), px );
	}
	break; }

    case CC_ComboBox:
	{
	    QPixmap left, mid, right;
	    QString hstr = QString::number( r.height() );
	    bool active = qAquaActive( cg );
	    qAquaPixmap( "cmb_act_left_" + hstr, left );
	    qAquaPixmap( "cmb_act_mid_" + hstr, mid );
	    if( active )
		qAquaPixmap( "cmb_act_right_" + hstr, right );
	    else
		qAquaPixmap( "cmb_dis_right_" + hstr, right );

	    p->drawPixmap( r.x(), r.y(), left );
	    p->drawTiledPixmap( r.x() + left.width(), r.y(), r.width() - left.width()*2, r.height(), mid );
	    p->drawPixmap( r.x() + r.width() - right.width(), r.y(), right );
	    break;
	}

    case CC_ToolButton: {
	if(!widget)
	    break;
	QToolButton *toolbutton = (QToolButton *) widget;

	QRect button, menuarea;
	button   = querySubControlMetrics(ctrl, widget, SC_ToolButton, opt);
	menuarea = querySubControlMetrics(ctrl, widget, SC_ToolButtonMenu, opt);

	bool on = toolbutton->isOn();
	bool down = toolbutton->isDown();

	SFlags bflags = flags,
	       mflags = flags;

	if (subActive & SC_ToolButton)
	    bflags |= Style_Down;
	if (subActive & SC_ToolButtonMenu)
	    mflags |= Style_Down;

	if (sub & SC_ToolButton) {
	    if (bflags & (Style_Down | Style_On | Style_Raised)) {
		QPixmap px;
		QString w = QString::number( r.width() );
		QString h = QString::number( r.height() );

		QWidget *btn_prnt = toolbutton->parentWidget();
		QString mod = (down || on) ? "on_" : "off_";
		if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		    QToolBar * bar  = (QToolBar *) btn_prnt;
		    if( bar->orientation() == Qt::Vertical )
			mod += "v";
		    QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		    QObjectListIt it( *l );
		    if ( it.toFirst() == toolbutton )
			mod += "left";
		    else if( it.toLast() == toolbutton && !toolbutton->popup() )
			mod += "right";
		    else
			mod += "mid";
		    delete l;
		} else {
		    mod += "mid";
		}
		qAquaPixmap( "toolbtn_" + mod + "_" + w + "_" + h, px );
		p->drawPixmap( r.x(), r.y(), px );
	    } else if ( toolbutton->parentWidget() &&
			toolbutton->parentWidget()->backgroundPixmap() &&
			! toolbutton->parentWidget()->backgroundPixmap()->isNull() ) {
		p->drawTiledPixmap( r, *(toolbutton->parentWidget()->backgroundPixmap()),
				    toolbutton->pos() );
	    }
	}

	if (sub & SC_ToolButtonMenu) {
	    QPixmap px;
	    QString w = QString::number( menuarea.width() );
	    QString h = QString::number( menuarea.height() );
	    QWidget *btn_prnt = toolbutton->parentWidget();
	    QString mod = (down || on || (subActive & SC_ToolButtonMenu)) ? "on_" : "off_";
	    if ( btn_prnt && btn_prnt->inherits("QToolBar") ) {
		QToolBar * bar  = (QToolBar *) btn_prnt;
		if( bar->orientation() == Qt::Vertical )
		    mod += "v";
		QObjectList * l = bar->queryList( "QToolButton", 0, FALSE, FALSE );
		QObjectListIt it( *l );
		if ( it.toFirst() == toolbutton ) {
		    if( bar->orientation() == Qt::Horizontal )
			mod += "left";
		} else if( it.toLast() == toolbutton && !toolbutton->popup() ){
		    if( bar->orientation() == Qt::Horizontal )
			mod += "right";
		} else {
		    mod += "mid";
		}
		delete l;
	    } else {
		mod += "mid";
	    }
	    qAquaPixmap( "toolbtn_" + mod + "_" + w + "_" + h, px );
	    p->drawPixmap( menuarea.x(), menuarea.y(), px );
	    drawPrimitive(PE_ArrowDown, p, QRect(menuarea.x()+2,
						 menuarea.y()+(menuarea.height()-menuarea.width()-4),
						 menuarea.width() - 4, menuarea.width()-2),
			  cg, mflags, opt);
	}

	if (toolbutton->hasFocus() && !toolbutton->focusProxy()) {
	    QRect fr = toolbutton->rect();
	    fr.addCoords(3, 3, -3, -3);
	    drawPrimitive(PE_FocusRect, p, fr, cg);
	}
	break; }

    default:
	QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, opt);
    }
}

/*!
  \reimp
*/
QRect QAquaStyle::querySubControlMetrics( ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    const QStyleOption& opt ) const
{
    QRect rect;
    switch( control ) {
    case CC_ComboBox: {
	rect = QWindowsStyle::querySubControlMetrics(control, w, sc, opt);
	if(sc == SC_ComboBoxEditField)
	    rect.setWidth(rect.width() - 5);
	break; }

    case CC_TitleBar: {
	if(sc & SC_TitleBarCloseButton) {
	    rect = QRect(6, 2, 11, 12);
	} else if(sc & SC_TitleBarMinButton) {
	    rect = QRect(23, 2, 11, 12);
	} else if(sc & SC_TitleBarMaxButton) {
	    rect = QRect(42, 2, 11, 12);
	} else if(sc & SC_TitleBarLabel) {
	    QTitleBar *tb = (QTitleBar *)w;
	    int wd = 0;
	    if(tb->window())
		wd = 55;
	    else if(tb->testWFlags( WStyle_SysMenu))
		wd = 17;
	    if(tb->icon())
		wd += tb->icon()->width() + 3;
	    rect = QRect(wd, 0, w->width() - wd, 16);
	} else if(sc & SC_TitleBarSysMenu) {
	    rect = QRect(-666, -666, 10, 10 ); //ugh
	}
	break; }

#ifndef QT_NO_SCROLLBAR
    case CC_ScrollBar: {
	if(!w)
	    break;
	QScrollBar *scr = (QScrollBar *)w;
	int sbextent = pixelMetric(PM_ScrollBarExtent, scr);
	int maxlen = ((scr->orientation() == Qt::Horizontal) ?
		      scr->width() : scr->height()) - (17 * 2) - 2;
	int sliderlen;
	int sliderStart = scr->sliderStart();

	// calculate slider length
	if (scr->maxValue() != scr->minValue()) {
	    uint range = scr->maxValue() - scr->minValue();
	    sliderlen = (scr->pageStep() * maxlen) /
			(range + scr->pageStep());

	    int slidermin = pixelMetric( PM_ScrollBarSliderMin, scr );
	    if ( sliderlen < slidermin || range > INT_MAX / 2 )
		sliderlen = slidermin;
	    if ( sliderlen > maxlen )
		sliderlen = maxlen;
	} else
	    sliderlen = maxlen;

	if( qt_mac_scrollbar_arrows_together ) {
	    switch(sc) {
	    case SC_ScrollBarAddLine:
		if(scr->orientation() == Horizontal)
		    rect.setRect( scr->width() - 17, 0, 17, 15 );
		else
		    rect.setRect( 0, scr->height() - 17, 16, 17 );
		break;
	    case SC_ScrollBarSubLine:
		if(scr->orientation() == Horizontal)
		    rect.setRect( scr->width() - (22 + 17), 0, 22, 15 );
		else
		    rect.setRect( 0, scr->height() - (20 + 17), 16, 20 );
		break;
	    case SC_ScrollBarSlider:
		if (scr->orientation() == Qt::Horizontal)
		    rect.setRect(sliderStart, 0, sliderlen, sbextent);
		else
		    rect.setRect(0, sliderStart, sbextent, sliderlen);
		break;
	    case SC_ScrollBarGroove: {
		if(scr->orientation() == Horizontal)
		    rect.setRect( 6, 0, scr->width() - 38, scr->height());
		else
		    rect.setRect(0, 5, scr->width(), scr->height() - 37 );
		break;
	    }
  	    case SC_ScrollBarSubPage:
		// between top/left button and slider
		if (scr->orientation() == Qt::Horizontal)
		    rect.setRect(0, 0, sliderStart, sbextent);
		else
		    rect.setRect(0, 0, sbextent, sliderStart );
		break;
	    case SC_ScrollBarAddPage:
		// between bottom/right button and slider
		if (scr->orientation() == Qt::Horizontal)
		    rect.setRect(sliderStart + sliderlen, 0,
				 maxlen - sliderStart - sliderlen, sbextent);
		else
		    rect.setRect(0, sliderStart + sliderlen,
				 sbextent, maxlen - sliderStart - sliderlen );
		break;
	    default:
		rect = QWindowsStyle::querySubControlMetrics( control, w, sc, opt);
	    }
	} else {
	    switch(sc) {
	    case SC_ScrollBarAddLine:
		if(scr->orientation() == Horizontal)
		    rect.setRect( scr->width() - 26, 0, 26, 14 );
		else
		    rect.setRect( 0, scr->height() - 26, 14, 26 );
		break;
	    case SC_ScrollBarSubLine:
		if(scr->orientation() == Horizontal)
		    rect.setRect( 0, 0, 26, 14 );
		else
		    rect.setRect( 0, 0, 14, 26 );
		break;
	    case SC_ScrollBarGroove:
		if (scr->orientation() == Qt::Horizontal)
		    rect.setRect(16, 0, scr->width() - 32, scr->height());
		else
		    rect.setRect(0, 15, scr->width(), scr->height() - 31 );
		break;
	    default:
		rect = QWindowsStyle::querySubControlMetrics( control, w, sc, opt);
	    }
	}

	break; }
#endif

    default:
	rect = QWindowsStyle::querySubControlMetrics( control, w, sc, opt);
	break;
    }
    return rect;
}

/*!
  \reimp
*/
int QAquaStyle::styleHint(StyleHint sh, const QWidget *w, const QStyleOption &opt,QStyleHintReturn *d) const
{
    int ret = 0;
    switch(sh) {
    case SH_GroupBox_TextLabelVerticalAlignment:
	ret = Qt::AlignTop;
	break;
    case SH_ScrollView_FrameOnlyAroundContents:
	if(w && (w->isTopLevel() || !w->parentWidget() || w->parentWidget()->isTopLevel()) &&  
	   (w->inherits("QScrollView") || w->inherits("QWorkspaceChild")))
	    ret = TRUE;
	else
	    ret = QWindowsStyle::styleHint(sh, w, opt, d);
	break;
    case SH_PopupMenu_Scrollable:
	ret = TRUE;
	break;
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
        break;
    case SH_BlinkCursorWhenTextSelected:
	ret = FALSE;
	break;
    case SH_RichText_FullWidthSelection:
	ret = TRUE;
	break;
    case SH_ScrollBar_StopMouseOverSlider:
        ret = TRUE;
        break;
    case SH_ComboBox_Popup:
        ret = TRUE;
        break;
    case SH_Workspace_FillSpaceOnMaximize:
        ret = TRUE;
        break;
    case SH_Widget_ShareActivation:
        ret = TRUE;
        break;
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignRight;
        break;
    case SH_TabBar_Alignment:
        ret = Qt::AlignHCenter;
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, w, opt, d);
        break;
    }
    return ret;
}

#ifdef Q_WS_MAC
/*!
    \internal
*/
void QAquaStyle::appearanceChanged()
{
    bool changed = FALSE;
    AquaMode m;
    {
	Collection c=NewCollection();
	GetTheme(c);
	Str255 str;
	long int s = 256;
	if(!GetCollectionItem(c, kThemeVariantNameTag, 0, &s, &str)) {
	    if(!strncmp((char *)str+1, "Blue", str[0]))
		m = AquaModeAqua;
	    else if(!strncmp((char *)str+1, "Graphite", str[0]))
		m = AquaModeGraphite;
	    else
		m = AquaModeUnknown;
	} else {
	    qDebug("Shouldn't happen %s:%d", __FILE__, __LINE__);
	}
	if(m != aquaMode) {
	    aquaMode = m;
	    changed = TRUE;
	}

	QPalette pal = qApp->palette();
	if(qt_mac_update_palette(pal, FALSE))
	    qApp->setPalette( pal, TRUE );

	ThemeScrollBarArrowStyle arrows;
	GetThemeScrollBarArrowStyle(&arrows);
	bool sat = (arrows == kThemeScrollBarArrowsLowerRight);
	if(sat != qt_mac_scrollbar_arrows_together) {
	    qt_mac_scrollbar_arrows_together = sat;
	    changed = TRUE;
	}

	//cleanup
	DisposeCollection(c);
    }
    if(changed && qApp && !qApp->startingUp() && qApp->style().inherits("QAquaStyle"))
	qApp->setStyle(new QAquaStyle);
}
#endif
#endif /* QT_NO_STYLE_AQUA */

