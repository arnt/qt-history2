/****************************************************************************
** $Id: $
**
** Definition of Aqua-style guidelines functions
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

#include <stdlib.h>
#include "private/qaquastyle_p.h"
#include <qapplication.h>
#include <qobjectlist.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qptrlist.h>
#include <qguardedptr.h>
#include <qtextedit.h>
#include <qtoolbutton.h>
#include <qsize.h>
#include <qslider.h>
#include <qlabel.h>
#include <qradiobutton.h>
#ifdef Q_WS_MAC
#  include <qt_mac.h>
#endif

QCString p2qstring(const unsigned char *c); //qglobal.cpp
#define QMAC_QAQUASTYLE_SIZE_CONSTRAIN

/*****************************************************************************
  QAquaStyle debug facilities
 *****************************************************************************/
//#define DEBUG_SIZE_CONSTRAINT

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
    if(focusable(w)) {
	if(w->hasFocus()) 
	    setFocusWidget(w);
	w->installEventFilter(this);
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
	    if(doAnimate(AquaPushButton)) 
		d->defaultButton->repaint(FALSE);
	}
    } else if( te->timerId() == d->progressTimerId && !d->progressBars.isEmpty() ) {
	if(doAnimate(AquaProgressBar)) {
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
	QWidget *top = w->parentWidget(), *p = top;
	while(!top->isTopLevel() && !top->testWFlags(WSubWindow))
	    top = top->parentWidget();
	if(top && (w->width() < top->width() - 30 || w->height() < top->height() - 40)) {
	    if(w->inherits("QLineEdit") && p->inherits("QComboBox"))
		w = p;
	    else if(w->inherits("QSpinWidget")) //transfer to the editor
		w = ((QSpinWidget*)w)->editWidget();
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
    return (w && w->parentWidget() && 
	    (w->inherits("QSpinWidget") || w->inherits("QDateTimeEditor") ||
	     (w->inherits("QLineEdit") && w->parentWidget()->inherits("QSpinWidget")) ||
	     (w->inherits("QFrame") && ((QFrame*)w)->frameStyle() != QFrame::NoFrame && 
	      ((w->inherits("QLineEdit") /* &&
	      (w->parentWidget()->inherits("QComboBox") || (((QLineEdit*)w)->frame())) */) ||
	      (w->inherits("QTextEdit") && !((QTextEdit*)w)->isReadOnly()) ||
	      w->inherits("QListBox") || w->inherits("QListView")))));
}

void qt_mac_polish_font(QWidget *w, QAquaWidgetSize size)
{
#ifdef Q_WS_MAC
    if(!w->ownFont() && (w->font() == qApp->font() || 
			 (w->parentWidget() && w->font() == w->parentWidget()->font()))) {
	short key = kThemeSystemFont;
	bool set_font = TRUE, find_small = (size == QAquaSizeSmall), find_bold = FALSE;
	if(w->inherits("QPushButton"))
	    key = kThemePushButtonFont;
	else if(w->inherits("QListView") || w->inherits("QListBox"))
	    key = kThemeViewsFont;
	else if(w->inherits("QTitleBar"))
	    key = kThemeWindowTitleFont;
	else if(w->inherits("QMenuBar"))
	    key = kThemeMenuTitleFont;
	else if(w->inherits("QPopupMenu"))
	    key = kThemeMenuItemFont;
	else if(w->inherits("QHeader") || w->inherits("QTipLabel"))
	    find_small = TRUE;
	else if((w->inherits("QLabel") && ((QLabel*)w)->buddy()) || w->inherits("QGroupBox"))
	    key = kThemeLabelFont;
	else if(w->inherits("QLabel") && w->topLevelWidget() && w->topLevelWidget()->inherits("QMessageBox"))
	    find_bold = TRUE;
#if 0
	else
	    set_font = FALSE;
#endif
	if(find_small && (key == kThemeLabelFont || key == kThemeSystemFont))
	    key = kThemeSmallSystemFont;
	if(find_bold) {
	    if(key == kThemeSystemFont)
		key = kThemeEmphasizedSystemFont;
	    else if(key == kThemeSmallSystemFont)
		key = kThemeSmallEmphasizedSystemFont;
	}
	if(set_font) {
	    Str255 f_name;
	    SInt16 f_size;
	    Style f_style;
	    GetThemeFont(key, smSystemScript, f_name, &f_size, &f_style);
	    w->setFont(QFont(p2qstring(f_name), f_size,
			     (f_style & ::bold) ? QFont::Bold : QFont::Normal,
			     (bool)(f_style & ::italic)));
	}
    }
#ifdef QMAC_QAQUA_MODIFY_TEXT_COLOURS
    if( !w->ownPalette() ) {
	ThemeBrush active = kThemeTextColorDialogActive, 
		 inactive = kThemeTextColorDialogInactive;
	bool set_colour = TRUE;
	if(w->inherits("QButton")) {
	    active = kThemeTextColorPushButtonActive;
	    inactive = kThemeTextColorPushButtonInactive;
	} else if(w->inherits("QListView") || w->inherits("QListBox")) {
	    active = inactive = kThemeTextColorListView;
	} else if(w->inherits("QLabel")) {
	    active = kThemeTextColorPlacardActive;
	    inactive = kThemeTextColorPlacardInactive;
	} else if(w->inherits("QPopupMenu")) {
	    active = kThemeTextColorPopupLabelActive;
	    inactive = kThemeTextColorPopupLabelInactive;
	} else {
	    set_colour = FALSE;
	}
	if(set_colour) {
	    QColor qc;
	    RGBColor c;
	    QPalette pal = w->palette();
	    //active
	    if(!GetThemeTextColor(active, 32, true, &c)) {
		qc = QColor(c.red / 256, c.green / 256, c.blue / 256);		
		pal.setColor(QPalette::Active, QColorGroup::Text, qc);
		pal.setColor(QPalette::Active, QColorGroup::Foreground, qc);
		pal.setColor(QPalette::Active, QColorGroup::HighlightedText, qc);
	    }
	    //inactive
	    if(!GetThemeTextColor(inactive, 32, true, &c)) {
		qc = QColor(c.red / 256, c.green / 256, c.blue / 256);		
		pal.setColor(QPalette::Inactive, QColorGroup::Text, qc);
		pal.setColor(QPalette::Disabled, QColorGroup::Text, qc);
		pal.setColor(QPalette::Inactive, QColorGroup::Foreground, qc);
		pal.setColor(QPalette::Disabled, QColorGroup::Foreground, qc);
		pal.setColor(QPalette::Inactive, QColorGroup::HighlightedText, qc);
		pal.setColor(QPalette::Disabled, QColorGroup::HighlightedText, qc);
	    }
	    w->setPalette(pal);
	}
    }
#endif
#else
    Q_UNUSED(w);
    Q_UNUSED(size);
#endif
}    

bool qt_mac_update_palette(QPalette &pal, bool do_init)
{
    static QPalette last_pal;
    if(do_init) {
	last_pal = pal;
	last_pal.setColor( QPalette::Inactive, QColorGroup::ButtonText, QColor( 148,148,148 ));
	last_pal.setColor( QPalette::Disabled, QColorGroup::ButtonText, QColor( 148,148,148 ));
#ifndef Q_WS_MAC
	last_pal.setColor(QPalette::Active, QColorGroup::Highlight, QColor(0xC2, 0xC2, 0xC2));
	last_pal.setColor(QPalette::Inactive, QColorGroup::Highlight, 
			  last_pal.color(QPalette::Active, QColorGroup::Highlight).light());
	last_pal.setColor(QPalette::Active, QColorGroup::Shadow, Qt::gray);
	last_pal.setColor(QPalette::Inactive, QColorGroup::Shadow, Qt::lightGray);
	last_pal.setColor(QColorGroup::HighlightedText, Qt::black);
#ifdef QMAC_QAQUA_MODIFY_TEXT_COLOURS
	last_pal.setColor(QColorGroup::Text, Qt::black);
	last_pal.setColor(QColorGroup::Foreground, Qt::black);
#endif
#endif
    }
    QPalette new_pal = last_pal;
#ifdef Q_WS_MAC
    RGBColor c;
#define BIND_CLR(mac, palette, group) \
       GetThemeBrushAsColor(mac, 32, true, &c ); \
       new_pal.setBrush(palette, group, QColor(c.red / 256, c.green / 256, c.blue / 256));
    //these came from carbon mailing list waiting for addition of
    //kThemeBrush[Primary|Secondary]HighlightColor in Appearance.h
    BIND_CLR(-3, QPalette::Active, QColorGroup::Highlight);
    BIND_CLR(-4, QPalette::Inactive, QColorGroup::Highlight);
    BIND_CLR(kThemeBrushButtonActiveDarkShadow, QPalette::Active, QColorGroup::Shadow);
    BIND_CLR(kThemeBrushButtonInactiveDarkShadow, QPalette::Inactive, QColorGroup::Shadow);
#ifdef QMAC_QAQUA_MODIFY_TEXT_COLOURS
    BIND_CLR(kThemeTextColorDialogActive, QPalette::Active, QColorGroup::Text);
    BIND_CLR(kThemeTextColorDialogActive, QPalette::Active, QColorGroup::Foreground);
    BIND_CLR(kThemeTextColorDialogActive, QPalette::Active, QColorGroup::HighlightedText);
    BIND_CLR(kThemeTextColorDialogInactive, QPalette::Inactive, QColorGroup::Text);
    BIND_CLR(kThemeTextColorDialogInactive, QPalette::Inactive, QColorGroup::Foreground);
    BIND_CLR(kThemeTextColorDialogInactive, QPalette::Inactive, QColorGroup::HighlightedText);
#endif
#undef BIND_CLR
#endif
    if(do_init || new_pal != last_pal) {
	new_pal.setDisabled(new_pal.inactive());
	pal = last_pal = new_pal;
	return TRUE;
    }
    return FALSE;
}


#if defined( QMAC_QAQUASTYLE_SIZE_CONSTRAIN ) || defined(DEBUG_SIZE_CONSTRAINT)
static QAquaWidgetSize qt_aqua_guess_size(const QWidget *widg, QSize large, QSize small)
{
    if(large == QSize(-1, -1)) {
	if(small == QSize(-1, -1))
	    return QAquaSizeUnknown;
	return QAquaSizeSmall;
    } else if(small == QSize(-1, -1)) {
	return QAquaSizeLarge;
    }

    if(widg->topLevelWidget()->inherits("QDockWindow") || getenv("QWIDGET_ALL_SMALL")) {
	//if(small.width() != -1 || small.height() != -1)
	    return QAquaSizeSmall;
    }

#if 0
    /* Figure out which size we're closer to, I just hacked this in, I haven't
       tested it as it would probably look pretty strange to have some widgets
       big and some widgets small in the same window?? -Sam */
    int large_delta=0;
    if(large.width() != -1) {
	int delta = large.width() - widg->width();
	large_delta += delta * delta;
    }
    if(large.height() != -1) {
	int delta = large.height() - widg->height();
	large_delta += delta * delta;
    }
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
	return QAquaSizeSmall;
#endif
    return QAquaSizeLarge;
}
#ifdef Q_WS_MAC
static int qt_mac_aqua_get_metric(ThemeMetric met)
{
    SInt32 ret;
    GetThemeMetric(met, &ret);
    return ret;
}
#endif
static QSize qt_aqua_get_known_size(QStyle::ContentsType ct, const QWidget *widg, QSize szHint, QAquaWidgetSize sz)
{
    QSize ret(-1, -1);
    if(sz != QAquaSizeSmall && sz != QAquaSizeLarge) {
	qDebug("Not sure how to return this..");
	return ret;
    }
    if(ct == QStyle::CT_CustomBase && widg) {
	if(widg->inherits("QPushButton"))
	    ct = QStyle::CT_PushButton;
	else if(widg->inherits("QRadioButton")) 
	    ct = QStyle::CT_RadioButton;
	else if(widg->inherits("QCheckBox")) 
	    ct = QStyle::CT_CheckBox;
	else if(widg->inherits("QComboBox")) 
	    ct = QStyle::CT_ComboBox;
	else if(widg->inherits("QToolButton"))
	    ct = QStyle::CT_ToolButton;
	else if(widg->inherits("QSlider"))
	    ct = QStyle::CT_Slider;
	else if(widg->inherits("QProgressBar")) 
	    ct = QStyle::CT_ProgressBar;
	else if(widg->inherits("QLineEdit")) 
	    ct = QStyle::CT_LineEdit;
	else if(widg->inherits("QHeader")) 
	    ct = QStyle::CT_Header;
	else if(widg->inherits("QMenuBar"))
	    ct = QStyle::CT_MenuBar;
	else
	    return ret;
    }

    const int shadow_height = 3;
    if(ct == QStyle::CT_PushButton) {
	QPushButton *psh = (QPushButton*)widg;
	int minw = -1;
	if(psh->text() == qApp->translate("QAquaStyle", "OK") || 
	   psh->text() == qApp->translate("QAquaStyle", "Cancel"))
	    minw = 69;
#ifdef Q_WS_MAC
	if(sz == QAquaSizeLarge)
	    ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricPushButtonHeight) + shadow_height);
	else
	    ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricSmallPushButtonHeight) + shadow_height);
#else
	if(sz == QAquaSizeLarge)
	    ret = QSize(minw, 20 + shadow_height);
	else
	    ret = QSize(minw, 17 + shadow_height);
#endif
    } else if(ct == QStyle::CT_RadioButton) {
	QRadioButton *rdo = (QRadioButton*)widg;
        // Exception for case where multiline radiobutton text requires no size constrainment
	if (rdo->text().find( '\n' ) != -1) 
	    return QSize(-1, -1);
#ifdef Q_WS_MAC
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricRadioButtonHeight) + shadow_height);
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallRadioButtonHeight) + shadow_height);
#else
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, 18 + shadow_height);
	else 
	    ret = QSize(-1, 15 + shadow_height);
#endif
    } else if(ct == QStyle::CT_CheckBox) {
#ifdef Q_WS_MAC
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricCheckBoxHeight) + shadow_height);
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallCheckBoxHeight) + shadow_height);
#else
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, 18 + shadow_height);
	else 
	    ret = QSize(-1, 16 + shadow_height);
#endif
    } else if(ct == QStyle::CT_ComboBox) {
#ifdef Q_WS_MAC
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricPopupButtonHeight));
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallPopupButtonHeight));
#else
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, 20);
	else 
	    ret = QSize(-1, 17);
#endif
    } else if(ct == QStyle::CT_ToolButton && sz == QAquaSizeSmall) {
	int width = 0, height = 0;
	if(szHint == QSize(-1, -1)) { //just 'guess'..
	    QToolButton *bt = (QToolButton*)widg;
	    if(!bt->iconSet().isNull()) {
		QIconSet::Size sz = QIconSet::Small;
		if ( bt->usesBigPixmap() ) 
		    sz = QIconSet::Large;
		QSize iconSize = QIconSet::iconSize(sz);
		QPixmap pm = bt->iconSet().pixmap(sz, QIconSet::Normal);
		width = QMAX(width, QMAX(iconSize.width(), pm.width()));
		height = QMAX(height, QMAX(iconSize.height(), pm.height()));
	    }
	    if (!bt->text().isNull() && bt->usesTextLabel()) {
		int text_width = bt->fontMetrics().width(bt->text()),
		   text_height = bt->fontMetrics().height();
		if(bt->textPosition() == QToolButton::Under) {
		    width = QMAX(width, text_width);
		    height += text_height;
		} else {
		    width += text_width;
		    width = QMAX(height, text_height);
		}
	    }
	} else {
	    width = szHint.width();
	    height = szHint.height();
	}
	width =  QMAX(20, width +  5); //border
	height = QMAX(20, height + 5); //border
	ret = QSize(width, height);
    } else if(ct == QStyle::CT_Slider) {
	int w = -1;
	QSlider *sld = (QSlider*)widg;
#ifdef Q_WS_MAC
	if(sz == QAquaSizeLarge) {
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
	    if(sz == QAquaSizeLarge)
		w = 18;
	    else 
		w = 16;
	} else {
	    if(sz == QAquaSizeLarge)
		w = 25;
	    else 
		w = 18;
	}
	if(sld->orientation() == Qt::Horizontal) 
	    ret.setHeight(w);
	else
	    ret.setWidth(w);
#endif
    } else if(ct == QStyle::CT_ProgressBar) {
#ifdef Q_WS_MAC
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricLargeProgressBarThickness));
	else 
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricNormalProgressBarThickness));
#else
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, 16);
	else 
	    ret = QSize(-1, 10);
#endif
    } else if(ct == QStyle::CT_LineEdit) {
	if(!widg || !widg->parentWidget() || !widg->parentWidget()->inherits("QComboBox")) {
	    //should I take into account the font dimentions of the lineedit? -Sam
	    if(sz == QAquaSizeLarge)
		ret = QSize(-1, 22);
	    else 
		ret = QSize(-1, 19);
	}
    }
#ifdef Q_WS_MAC
    else if(ct == QStyle::CT_Header) {
	if(sz == QAquaSizeLarge)
	    ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricListHeaderHeight));
    } else if(ct == QStyle::CT_MenuBar) {
	if(sz == QAquaSizeLarge) {
	    SInt16 size;
	    if(!GetThemeMenuBarHeight(&size))
		ret = QSize(-1, size);
	}
    }
#endif
    return ret;
}
#endif
QAquaWidgetSize qt_aqua_size_constrain(const QWidget *widg, QStyle::ContentsType ct, 
				       QSize szHint, QSize *insz)
{
#if defined( QMAC_QAQUASTYLE_SIZE_CONSTRAIN ) || defined(DEBUG_SIZE_CONSTRAINT)
    if(!widg) {
	if(insz)
	    *insz = QSize();
	if(getenv("QWIDGET_ALL_SMALL")) 
	    return QAquaSizeSmall;
	return QAquaSizeUnknown;
    }
    QSize large = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeLarge),
	  small = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeSmall);
    QAquaWidgetSize ret = qt_aqua_guess_size(widg, large, small);
    QSize *sz = NULL;
    if(ret == QAquaSizeSmall)
	sz = &small;
    else if(ret == QAquaSizeLarge)
	sz = &large;
    if(insz)
	*insz = sz ? *sz : QSize(-1, -1);
#ifdef DEBUG_SIZE_CONSTRAINT
    if(sz) {

	const char *size_desc = "Unknown";
	if(sz == &small)
	    size_desc = "Small";
	else if(sz == &large)
	    size_desc = "Large";
	qDebug("%s - %s: %s taken (%d, %d) [ %d, %d ]", widg ? widg->name() : "*Unknown*", 
	       widg ? widg->className() : "*Unknown*", size_desc, widg->width(), widg->height(), 
	       sz->width(), sz->height());
    }
#endif
    return ret;
#else
    if(insz)
	*insz = QSize();
    Q_UNUSED(widg);
    Q_UNUSED(fix);
    return QAquaSizeUnknown;
#endif
}
