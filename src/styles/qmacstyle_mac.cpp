/****************************************************************************
** $Id$
**
** Implementation of Mac native theme
**
** Created : 001018
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Macintosh may use this file in accordance with the Qt Commercial
** License Agreement provided with the Software.
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

/* broken things:
   we don't animate yet
*/

#include "qmacstyle_mac.h"

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)

//hack, but usefull
#define private protected //I don't much like doing this.. but I need updateFont(), sorry. -Sam
#include <qpainter.h>
#undef private
class QMacPainter : public QPainter
{
public:
    QMacPainter(QPaintDevice *p) : QPainter(p) { }
    QPoint domap(int x, int y) { map(x, y, &x, &y); return QPoint(x, y); }
    void setport() { QPainter::initPaintDevice(TRUE); }
    void setfont() { QPainter::updateFont(); }
};

#include <qpushbutton.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qt_mac.h>
#include <qtabbar.h>
#include "private/qtitlebar_p.h"
#include <qbitmap.h>
#include <qprogressbar.h>
#include <qapplication.h>
#include <qlistview.h>
#include <qtoolbutton.h>
#include <qbuttongroup.h>
#include <qtoolbar.h>
#include <qlayout.h>
#include <qpopupmenu.h>
#include <qmenubar.h>
#include <qcombobox.h>
#include <qdrawutil.h>
#include <qlineedit.h>
#include <qregexp.h>
#include "qwidgetlist.h"
#include <qguardedptr.h>
#include "qmainwindow.h"
#include "private/qaquastyle_p.h"

/* I need these to simulate the pushbutton pulse */
#include <qimage.h>
#include <qpixmapcache.h>

#include <string.h>

#define QMAC_NO_MACSTYLE_ANIMATE //just disable animations for now

//externals
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp

//static utility variables
static ThemeWindowType macWinType = kThemeUtilityWindow;
static const int macItemFrame         = 2;    // menu item frame width
static const int macItemHMargin       = 3;    // menu item hor text margin
static const int macItemVMargin       = 2;    // menu item ver text margin
static const int macRightBorder       = 12;   // right border on mac

// Utility to generate correct rectangles for AppManager internals
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=NULL,
					  bool off=TRUE, const QRect &rect=QRect())
{
    static Rect r;
    bool use_rect = (rect.x() || rect.y() || rect.width() || rect.height());
    QPoint tl(qr.topLeft());
    if(pd && pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    if(use_rect)
	tl += rect.topLeft();
    int offset = 0;
    if(off)
	offset = 1;
    SetRect(&r, tl.x(), tl.y(), (tl.x() + qr.width()) - offset, (tl.y() + qr.height()) - offset);
    if(use_rect) {
	r.right -= rect.width();
	r.bottom -= rect.height();
    }
    return &r;
}
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
					  bool off=TRUE, const QRect &rect=QRect())
{
    QPoint pt = qr.topLeft();
    QRect r(((QMacPainter*)p)->domap(pt.x(), pt.y()), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

//utility to figure out the size (from the painter)
QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if(p && p->device()->devType() == QInternal::Widget)
	return qt_aqua_size_constrain((QWidget*)p->device());
    return qt_aqua_size_constrain(NULL);
}

//private
class QMacStyleFocusWidget : public QAquaFocusWidget
{
public:
    QMacStyleFocusWidget() : QAquaFocusWidget(FALSE) { }

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual int focusOutset();
};
void QMacStyleFocusWidget::paintEvent(QPaintEvent *)
{
    QMacPainter p(this);
    p.setport();
    QRect r(focusOutset(), focusOutset(),
	    width() - (focusOutset()*2),
	    height() - (focusOutset()*2));
    DrawThemeFocusRect(qt_glb_mac_rect(r, this, TRUE, QRect(1, 1, 1, 1)), true);
}
int QMacStyleFocusWidget::focusOutset()
{
    SInt32 ret = 0;
    GetThemeMetric(kThemeMetricFocusRectOutset, &ret);
    return ret;
}

class QMacStylePrivate : public QAquaAnimate
{
    ControlRef progressbar;
    QGuardedPtr<QMacStyleFocusWidget> focusWidget;
public:
    struct ButtonState {
	int frame;
	enum { ButtonDark, ButtonLight } dir;
    } buttonState;
    QMacStylePrivate();
    ~QMacStylePrivate();
    ControlRef control(QAquaAnimate::Animates);
protected:
    void doAnimate(QAquaAnimate::Animates);
    void doFocus(QWidget *);
};
ControlRef
QMacStylePrivate::control(QAquaAnimate::Animates as)
{
    if(as == QAquaAnimate::AquaProgressBar) {
	if(progressbar)
	    return progressbar;
	if(CreateProgressBarControl((WindowPtr)qt_mac_safe_pdev->handle(),
				     qt_glb_mac_rect(QRect(0, 0, 40, 10), (QPaintDevice*)0, FALSE),
				     0, 0, 10, false, 0))
	    qDebug("Unexpected error: %s:%d", __FILE__, __LINE__);
	else
	    ShowControl(progressbar);
	return progressbar;
    }
    return 0;
}
QMacStylePrivate::QMacStylePrivate() : QAquaAnimate(), progressbar(0) 
{ 
    buttonState.frame = 0;
    buttonState.dir = ButtonState::ButtonDark;
}
QMacStylePrivate::~QMacStylePrivate()
{
    buttonState.frame = 0;
    buttonState.dir = ButtonState::ButtonDark;
    if(progressbar)
	DisposeControl(progressbar);
    progressbar = NULL;
}
void QMacStylePrivate::doAnimate(QAquaAnimate::Animates as)
{
    if(as == AquaPushButton) {
	if(buttonState.frame == 25 && buttonState.dir == ButtonState::ButtonDark)
	    buttonState.dir = ButtonState::ButtonLight;
	else if(!buttonState.frame && buttonState.dir == ButtonState::ButtonLight)
	    buttonState.dir = ButtonState::ButtonDark;
	buttonState.frame += ((buttonState.dir == ButtonState::ButtonDark) ? 1 : -1);
    } 
#ifndef QMAC_NO_MACSTYLE_ANIMATE
    else {
	if(QWidgetList *list = qApp->topLevelWidgets()) {
	    for (QWidget *widget = list->first(); widget; widget = list->next()) {
		if(widget->isActiveWindow())
		    IdleControls((WindowPtr)widget->handle());
	    }
	}
    }
#endif
}
void QMacStylePrivate::doFocus(QWidget *w)
{
    if(!focusWidget)
	focusWidget = new QMacStyleFocusWidget();
    focusWidget->setFocusWidget(w);
}

#define private public //ugh, what I'll do, guess we have to wait until 4.0
                       //to access positionToValue()..
#include <qslider.h>
#undef private

static int mac_count = 0;

/*!
    \class QMacStyle qmacstyle_mac.h
    \brief The QMacStyle class implements an Appearance Manager style.

    \ingroup appearance

    This class is implemented as a wrapper to the Apple Appearance
    Manager. This allows your application to be styled by whatever
    theme your Macintosh is using. This is done by having primitives
    in QStyle implemented in terms of what the Macintosh would
    normally theme (i.e. the Finder). The implementation does have
    some limitations, generally the same ones described in the
    QAquaStyle documentation.

    Note that the functions provided by QMacStyle are
    reimplementations of QStyle functions; see QStyle for their
    documentation.
*/

/*!
    Constructs a QMacStyle object.
*/
QMacStyle::QMacStyle()  : QWindowsStyle()
{
    d = new QMacStylePrivate;
    if(!mac_count++)
	RegisterAppearanceClient();
}

/*!
    Destructs a QMacStyle object.
*/
QMacStyle::~QMacStyle()
{
    if(!(--mac_count))
	UnregisterAppearanceClient();
    delete d;
}

/*! \reimp */
void QMacStyle::polish(QApplication* app)
{
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QColor pc(black);
    {
	QPainter p(&px);
	((QMacPainter *)&p)->setport();
	SetThemeBackground(kThemeBrushDialogBackgroundActive, px.depth(), true);
	EraseRect(qt_glb_mac_rect(QRect(0, 0, px.width(), px.height()), (QPaintDevice*)0, FALSE));
	RGBColor c;
	GetThemeBrushAsColor(kThemeBrushDialogBackgroundActive, 32, true, &c);
	pc = QColor(c.red / 256, c.green / 256, c.blue / 256);
    }
    QBrush background(pc, px);
    pal.setBrush(QColorGroup::Background, background);
    pal.setBrush(QColorGroup::Button, background);
    qt_mac_update_palette(pal, TRUE);
    app->setPalette(pal, TRUE);
}

/*! \reimp */
void QMacStyle::polish(QWidget* w)
{
    qt_mac_polish_font(w, qt_aqua_size_constrain(w));
    d->addWidget(w);

    if(!w->isTopLevel() && !w->inherits("QSplitter") && 
       w->backgroundPixmap() && qApp->palette().isCopyOf(w->palette())) 
            w->setBackgroundOrigin(QWidget::WindowOrigin);

    if(w->inherits("QLineEdit")) {
	SInt32 frame_size;
	GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
	((QLineEdit *)w)->setLineWidth(frame_size);
    } else if(w->inherits("QToolButton")) {
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise(FALSE);
	if(btn->group()) {
	    btn->group()->setMargin(0);
	    btn->group()->setInsideSpacing(0);
	}
    } else if(w->inherits("QToolBar")) {
	QToolBar * bar = (QToolBar *) w;
	QBoxLayout * layout = bar->boxLayout();
	layout->setSpacing(0);
	layout->setMargin(0);
    } else if(w->inherits("QTipLabel")) {
	QLabel *label = (QLabel*)w;
	label->setFrameStyle(QFrame::NoFrame);
	label->setLineWidth(1);
    }
#ifdef Q_WS_MAC
    else if(w->inherits("QPopupMenu")) {
	QMacSavedPortInfo::setAlphaTransparancy(w, 0.9);
    }
#endif

#if 0
    else if(w->inherits("QTitleBar")) {
	w->font().setPixelSize(10);
	((QTitleBar*)w)->setAutoRaise(TRUE);
    }
#endif
}

/*! \reimp */
void QMacStyle::unPolish(QWidget* w)
{
    d->removeWidget(w);
    if(w->inherits("QToolButton")) {
        QToolButton * btn = (QToolButton *) w;
        btn->setAutoRaise(TRUE);
    }
#ifdef Q_WS_MAC
    else if(w->inherits("QPopupMenu")) {
	QMacSavedPortInfo::setAlphaTransparancy(w, 1);
    }
#endif

}

/*! \reimp */
void QMacStyle::drawItem(QPainter *p, const QRect &r,
			   int flags, const QColorGroup &cg, bool enabled,
			   const QPixmap *pixmap, const QString& text,
			   int len, const QColor* penColor) const
{
#if defined(Q_WS_MACX) && 0
    /* No accelerators drawn here! */
    bool pass_through = TRUE;
    if(!pixmap && !text.isEmpty()) {
	ThemeDrawState tds = kThemeStateActive;
	if(qAquaActive(cg)) {
	    if(!enabled)
		tds = kThemeStateUnavailable;
	} else {
	    if(enabled)
		tds = kThemeStateInactive;
	    else
		tds = kThemeStateUnavailableInactive;
	}
	int bkup = 0;
	QString rt = text;
	for(uint i = 0; i < rt.length(); i++) {
	    if(rt[i] == '&' && rt[i+1] != '&')
		bkup++;
	    else
		rt[i-bkup] = rt[i];
	}
	len = (len < 0) ? rt.length() - bkup : len - bkup;

	pass_through = FALSE;
	((QMacPainter *)p)->setport();
	((QMacPainter *)p)->setfont();
	int x = r.x(), y = r.y(), w = r.width(), h = r.height();
	QFontMetrics met = p->fontMetrics();
	if(flags & AlignRight)
	    x += w - met.width(text, len);
	else if(flags & AlignHCenter)
	    x += (w / 2) - (met.width(text, len) / 2);
	if(flags & AlignBottom)
	    y += h - met.height();
	else if(flags & AlignVCenter)
	    y += (h / 2) - (met.height() / 2);
	CFStringRef str = CFStringCreateWithCharacters(NULL, (UniChar *)rt.unicode(), len);
	DrawThemeTextBox(str, kThemeCurrentPortFont, tds, false,
			 qt_glb_mac_rect(QRect(x, y, x+w, y+h), p), 0, 0);
	CFRelease(str);
    }
    if(pass_through)
#endif
	QWindowsStyle::drawItem(p, r, flags | NoAccel, cg,
				 enabled, pixmap, text, len, penColor);
}

/*! \reimp */
void QMacStyle::drawPrimitive(PrimitiveElement pe,
			       QPainter *p,
			       const QRect &r,
			       const QColorGroup &cg,
			       SFlags flags,
			       const QStyleOption& opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if(flags & Style_Down) {
	tds = kThemeStatePressed;
    } else if(qAquaActive(cg)) {
	if(!(flags & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if(flags & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch(pe) {
    case PE_Panel:
    case PE_PanelLineEdit: {
	if(flags & Style_Sunken) {
	    SInt32 frame_size;
	    if(pe == PE_PanelLineEdit)
		GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
	    else
		GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
	    p->fillRect(r.x(), r.y(), r.x() + opt.lineWidth(), r.bottom(), cg.background()); //left
	    p->fillRect(r.right()-opt.lineWidth(), r.y(), r.right(), r.bottom(), cg.background()); //right
	    p->fillRect(r.x(), r.y(), r.right(), r.y() + opt.lineWidth(), cg.background()); //top
	    p->fillRect(r.x(), r.bottom()-opt.lineWidth(), r.right(), r.bottom(), cg.background()); //bottom

	    const Rect *rect = qt_glb_mac_rect(r, p, FALSE,
					       QRect(frame_size, frame_size, frame_size * 2, frame_size * 2));
	    ((QMacPainter *)p)->setport();
	    if(pe == PE_PanelLineEdit)
		DrawThemeEditTextFrame(rect, tds);
	    else
		DrawThemeListBoxFrame(rect, tds);
	} else {
	    QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	}
	break; }
    case PE_PanelGroupBox: {
	if(opt.isDefault())
	    break;
	QWidget *w = NULL;
	//This is terrible, if I we just passed the widget in this wouldn't be necesary!
	if(p && p->device() && p->device()->devType() == QInternal::Widget)
	    w = (QWidget*)p->device();
	((QMacPainter *)p)->setport();
	if(w && w->parentWidget() && w->parentWidget()->inherits("QGroupBox"))
	    DrawThemeSecondaryGroup(qt_glb_mac_rect(r, p), kThemeStateActive);
	else
	    DrawThemePrimaryGroup(qt_glb_mac_rect(r, p), kThemeStateActive);
	break; }
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
	p->save();
	p->setPen(cg.text());
	QPointArray a;
	if(pe == PE_ArrowDown)
	    a.setPoints(3, r.x(), r.y(), r.right(), r.y(), r.x() + (r.width() / 2) ,
			 r.bottom());
	else if(pe == PE_ArrowRight)
	    a.setPoints(3, r.x(), r.y(), r.right(), r.y() + (r.height() / 2), r.x(),
			 r.bottom());
	else if(pe == PE_ArrowUp)
	    a.setPoints(3, r.x() + (r.width() / 2), r.y(), r.right(), r.bottom(), r.x(),
			 r.bottom());
	else
	    a.setPoints(3, r.x(), r.y() + (r.height() / 2), r.right(), r.y(), r.right(),
			 r.bottom());
	p->setBrush(cg.text());
	p->drawPolygon(a);
	p->setBrush(NoBrush);
	p->restore();
	break; }
    case PE_SizeGrip: {
	const Rect *rect = qt_glb_mac_rect(r, p);
	Point orig = { rect->top, rect->left };
	((QMacPainter *)p)->setport();
	DrawThemeStandaloneGrowBox(orig, kThemeGrowRight | kThemeGrowDown, false,
				   kThemeStateActive);
	break; }
    case PE_FocusRect:
	break;     //This is not used because of the QAquaFocusWidget thingie..
    case PE_TabBarBase:
	((QMacPainter *)p)->setport();
	DrawThemeTabPane(qt_glb_mac_rect(r, p), tds);
	break;
    case PE_HeaderArrow: //drawn in HeaderSection rather than separately..
	break;
    case PE_HeaderSection: {
	ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
	if(qAquaActive(cg)) {
	    if(!(flags & Style_Enabled)) 
		info.state = kThemeStateUnavailable;
	    else if(flags & Style_Down) 
		info.state = kThemeStatePressed;
	} else {
	    if(flags & Style_Enabled)
		info.state = kThemeStateInactive;
	    else
		info.state = kThemeStateUnavailableInactive;
	}
	if(flags & Style_Sunken) 
	    info.value = kThemeButtonOn;
	if((flags & Style_Up)) 
	    info.adornment |= kThemeAdornmentHeaderButtonSortUp;
	((QMacPainter *)p)->setport();
	DrawThemeButton(qt_glb_mac_rect(r, p, FALSE), kThemeListHeaderButton,
			&info, NULL, NULL, NULL, 0);
	break; }
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_On)
	    info.value = kThemeButtonOn;
	ThemeButtonKind bkind = kThemeRadioButton;
	if(qt_mac_get_size_for_painter(p) == QAquaSizeSmall)
	    bkind = kThemeSmallRadioButton;
	if(pe == PE_ExclusiveIndicator) {
	    p->fillRect(r, white);
	    ((QMacPainter *)p)->setport();
	    DrawThemeButton(qt_glb_mac_rect(r, p), bkind,
			    &info, NULL, NULL, NULL, 0);
	} else {
	    p->save();
	    QRegion rgn;
	    GetThemeButtonRegion(qt_glb_mac_rect(r, p), bkind,
				 &info, rgn.handle(TRUE));
	    p->setClipRegion(rgn);
	    p->fillRect(r, color1);
	    p->restore();
	}
	break; }
    case PE_IndicatorMask:
    case PE_Indicator: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(flags & Style_NoChange)
	    info.value = kThemeButtonMixed;
	else if(flags & Style_On)
	    info.value = kThemeButtonOn;
	ThemeButtonKind bkind = kThemeCheckBox;
	if(qt_mac_get_size_for_painter(p) == QAquaSizeSmall)
	    bkind = kThemeSmallCheckBox;
	if(pe == PE_Indicator) {
	    p->fillRect(r, white);
	    ((QMacPainter *)p)->setport();
	    DrawThemeButton(qt_glb_mac_rect(r, p), bkind,
			    &info, NULL, NULL, NULL, 0);
	} else {
	    p->save();
	    QRegion rgn;
	    GetThemeButtonRegion(qt_glb_mac_rect(r, p), bkind,
				 &info, rgn.handle(TRUE));
	    p->setClipRegion(rgn);
	    p->fillRect(r, color1);
	    p->restore();
	}
	break; }
    default:
	QWindowsStyle::drawPrimitive(pe, p, r, cg, flags, opt);
	break;
    }
}

/*! \reimp */
void QMacStyle::drawControl(ControlElement element,
				 QPainter *p,
				 const QWidget *widget,
				 const QRect &r,
				 const QColorGroup &cg,
				 SFlags how,
				 const QStyleOption& opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if(how & Style_Down) {
	tds = kThemeStatePressed;
    } else if(qAquaActive(cg)) {
	if(!(how & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if(how & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch(element) {
    case CE_PopupMenuScroller: {
	Rect mrect = *qt_glb_mac_rect(widget->rect(), p),
	     irect = *qt_glb_mac_rect(r, p, FALSE);
	ThemeMenuState tms = kThemeMenuActive;
	if(how & Style_Active)
	    tms |= kThemeMenuSelected;
	ThemeMenuItemType tmit = kThemeMenuItemScrollUpArrow;
	if((how & Style_Down))
	    tmit = kThemeMenuItemScrollDownArrow;
	((QMacPainter *)p)->setport();
	DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, NULL, 0);
	break; }
    case CE_MenuBarEmptyArea:
	p->fillRect(r, cg.brush(QColorGroup::Button));
	((QMacPainter*)p)->setport();
	DrawThemeMenuBarBackground(qt_glb_mac_rect(r, p, FALSE), kThemeMenuBarNormal,
				   kThemeMenuSquareMenuBar);
	break;
    case CE_PopupMenuItem: {
	if(!widget || opt.isDefault())
	    break;
	QPopupMenu *popupmenu = (QPopupMenu *)widget;
	QMenuItem *mi = opt.menuItem();
	if(!mi)
	    break;

	const QColorGroup & g = cg;
	QColorGroup itemg = g;
	bool dis = !mi->isEnabled();
	int tab = opt.tabWidth();
	int maxpmw = opt.maxIconWidth();
	bool checked = mi->isChecked();
	bool checkable = popupmenu->isCheckable();
	bool act = how & Style_Active;
	int x, y, w, h;
	r.rect(&x, &y, &w, &h);
	Rect mrect = *qt_glb_mac_rect(popupmenu->rect(), p),
	     irect = *qt_glb_mac_rect(r, p, FALSE);

	if(checkable)
	    maxpmw = QMAX(maxpmw, 12); // space for the checkmarks

	int checkcol = maxpmw;
	if(mi && mi->isSeparator()) {
	    ((QMacPainter *)p)->setport();
	    DrawThemeMenuSeparator(&irect);
	    return;
	}

	ThemeMenuState tms = kThemeMenuActive;
	if(!mi->isEnabled())
	    tms |= kThemeMenuDisabled;
	if(how & Style_Active)
	    tms |= kThemeMenuSelected;
	ThemeMenuItemType tmit = kThemeMenuItemPlain;
	if(mi->popup())
	    tmit |= kThemeMenuItemHierarchical;
	if(mi->iconSet())
	    tmit |= kThemeMenuItemHasIcon;
	((QMacPainter *)p)->setport();
	DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, NULL, 0);
	bool reverse = QApplication::reverseLayout();

	int xpos = x;
	if(reverse)
	    xpos += w - checkcol;
	if(mi->iconSet()) {              // draw iconset
	    if(checked) {
		QRect vrect = visualRect(QRect(xpos, y, checkcol, h), r);
		if(act && !dis) {
		    qDrawShadePanel(p, vrect.x(), y, checkcol, h,
				     cg, TRUE, 1, &cg.brush(QColorGroup::Button));
		} else {
		    QBrush fill(cg.light(), Dense4Pattern);
		    // set the brush origin for the hash pattern to the x/y coordinate
		    // of the menu item's checkmark... this way, the check marks have
		    // a consistent look
		    QPoint origin = p->brushOrigin();
		    p->setBrushOrigin(vrect.x(), y);
		    qDrawShadePanel(p, vrect.x(), y, checkcol, h, cg, TRUE, 1,
				     &fill);
		    // restore the previous brush origin
		    p->setBrushOrigin(origin);
		}
	    } else if(!act) {
		p->fillRect(xpos, y, checkcol, h,
			    g.brush(QColorGroup::Button));
	    }

	    QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
	    if(act && !dis)
		mode = QIconSet::Active;
	    QPixmap pixmap;
	    if(checkable && mi->isChecked())
		pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode, QIconSet::On);
	    else
		pixmap = mi->iconSet()->pixmap(QIconSet::Small, mode);
	    int pixw = pixmap.width();
	    int pixh = pixmap.height();
	    if(act && !dis) {
		if(!mi->isChecked())
		    qDrawShadePanel(p, xpos, y, checkcol, h, g, FALSE, 1,
				     &g.brush(QColorGroup::Button));
	    }
	    QRect cr(xpos, y, checkcol, h);
	    QRect pmr(0, 0, pixw, pixh);
	    pmr.moveCenter(cr.center());
	    p->setPen(itemg.text());
	    p->drawPixmap(pmr.topLeft(), pixmap);
	} else  if(checkable) {  // just "checking"...
	    int mw = checkcol + macItemFrame;
	    int mh = h - 2*macItemFrame;
	    if(mi->isChecked()) {
		int xp = xpos;
		if(reverse)
		    xp -= macItemFrame;
		else
		    xp += macItemFrame;

		SFlags cflags = Style_Default;
		if(! dis)
		    cflags |= Style_Enabled;
		if(act)
		    cflags |= Style_On;
		drawPrimitive(PE_CheckMark, p, QRect(xp, y+macItemFrame, mw, mh), cg, cflags);
	    }
	}

	p->setPen(act ? Qt::white/*g.highlightedText()*/ : g.buttonText());

	QColor discol;
	if(dis) {
	    discol = itemg.text();
	    p->setPen(discol);
	}

	int xm = macItemFrame + checkcol + macItemHMargin;
	if(reverse)
	    xpos = macItemFrame + tab;
	else
	    xpos += xm;

	if(mi->custom()) {
	    int m = macItemVMargin;
	    p->save();
	    if(dis && !act) {
		p->setPen(g.light());
		mi->custom()->paint(p, itemg, act, !dis,
				     xpos+1, y+m+1, w-xm-tab+1, h-2*m);
		p->setPen(discol);
	    }
	    mi->custom()->paint(p, itemg, act, !dis,
				 x+xm, y+m, w-xm-tab+1, h-2*m);
	    p->restore();
	}
	QString s = mi->text();
	if(!s.isNull()) {                        // draw text
	    int t = s.find('\t');
	    int m = macItemVMargin;
	    const int text_flags = AlignVCenter|NoAccel | DontClip | SingleLine;
	    if(t >= 0) {                         // draw tab text
		int xp;
		if(reverse)
		    xp = x + macRightBorder+macItemHMargin+macItemFrame - 1;
		else
		    xp = x + w - tab - macRightBorder-macItemHMargin-macItemFrame+1;
		if(dis && !act) {
		    p->setPen(g.light());
		    p->drawText(xp, y+m+1, tab, h-2*m, text_flags, s.mid(t+1));
		    p->setPen(discol);
		}
		p->drawText(xp, y+m, tab, h-2*m, text_flags, s.mid(t+1));
		s = s.left(t);
	    }
	    if(dis && !act) {
		p->setPen(g.light());
		p->drawText(xpos+1, y+m+1, w-xm-tab+1, h-2*m, text_flags, s, t);
		p->setPen(discol);
	    }
	    p->drawText(xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s, t);
	} else if(mi->pixmap()) {                        // draw pixmap
	    QPixmap *pixmap = mi->pixmap();
	    if(pixmap->depth() == 1)
		p->setBackgroundMode(OpaqueMode);
	    p->drawPixmap(xpos, y+macItemFrame, *pixmap);
	    if(pixmap->depth() == 1)
		p->setBackgroundMode(TransparentMode);
	}
	break; }
    case CE_MenuBarItem: {
	if(!widget)
	    break;
	const QMenuBar *mbar = (const QMenuBar *)widget;
	QRect ir(r.x(), 0, r.width(), mbar->height());
	Rect mrect = *qt_glb_mac_rect(mbar->rect(), p),
	     irect = *qt_glb_mac_rect(ir, p, FALSE);
	ThemeMenuState tms = kThemeMenuActive;
	if(!(how & Style_Active))
	    tms |= kThemeMenuDisabled;
	if(how & Style_Down)
	    tms |= kThemeMenuSelected;
	((QMacPainter *)p)->setport();
	DrawThemeMenuTitle(&mrect, &irect, tms, 0, NULL, 0);
	QCommonStyle::drawControl(element, p, widget, r, cg, how, opt);
	break; }
    case CE_ProgressBarContents: {
	if(!widget)
	    break;
	QProgressBar *pbar = (QProgressBar *) widget;
#ifndef QMAC_NO_MACSTYLE_ANIMATE
	if(ControlRef prgctl = d->control(QAquaAnimate::AquaProgressBar)) {
	    qDebug("foo..");
	    SetControlBounds(prgctl, qt_glb_mac_rect(r, p->device()));
	    SetControlMaximum(prgctl, pbar->totalSteps());
	    SetControlValue(prgctl, pbar->progress());
	    ((QMacPainter *)p)->setport();
	    DrawControlInCurrentPort(prgctl);
	} else
#endif
	{
	    ThemeTrackDrawInfo ttdi;
	    memset(&ttdi, '\0', sizeof(ttdi));
	    ttdi.kind = kThemeLargeProgressBar;
	    if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
		ttdi.kind = kThemeMediumProgressBar;
	    ttdi.bounds = *qt_glb_mac_rect(r, p);
	    ttdi.max = pbar->totalSteps();
	    ttdi.value = pbar->progress();
	    ttdi.attributes |= kThemeTrackHorizontal;
	    if(!qAquaActive(cg))
		ttdi.enableState = kThemeTrackInactive;
	    else if(!pbar->isEnabled())
		ttdi.enableState = kThemeTrackDisabled;
	    ((QMacPainter *)p)->setport();
	    DrawThemeTrack(&ttdi, NULL, NULL, 0);
	}
	break; }
    case CE_TabBarTab: {
	if(!widget)
	    break;
	QTabBar * tb = (QTabBar *) widget;
	ThemeTabStyle tts = kThemeTabNonFront;
	if(how & Style_Selected) {
	    if(!qAquaActive(cg))
		tts = kThemeTabFrontUnavailable;
	    else if(!(how & Style_Enabled))
		tts = kThemeTabFrontInactive;
	    else
		tts = kThemeTabFront;
	} else if(!qAquaActive(cg)) {
	    tts = kThemeTabNonFrontUnavailable;
	} else if(!(how & Style_Enabled)) {
	    tts = kThemeTextColorTabNonFrontInactive;
	} else if((how & Style_Sunken) && (how & Style_MouseOver)) {
	    tts = kThemeTabNonFrontPressed;
	}
	ThemeTabDirection ttd = kThemeTabNorth;
	if(tb->shape() == QTabBar::RoundedBelow || tb->shape() == QTabBar::TriangularBelow) 
	    ttd = kThemeTabSouth;
	QRect tabr(r.x(), r.y(), r.width(), r.height() + pixelMetric(PM_TabBarBaseOverlap, widget));
	if(ttd == kThemeTabSouth)
	    tabr.moveBy(0, -pixelMetric(PM_TabBarBaseOverlap, widget));
	((QMacPainter *)p)->setport();
	DrawThemeTab(qt_glb_mac_rect(tabr, p, FALSE), tts, ttd, NULL, 0);
	if(!(how & Style_Selected)) {
	    //The "fudge" is just so the left and right side doesn't
	    //get drawn onto my widget (not sure it will really happen)
	    const int fudge = 20;
	    QRect pr = QRect(r.x() - fudge, r.bottom() - 2, 
			     r.width() + (fudge * 2), pixelMetric(PM_TabBarBaseHeight, widget));
	    if(ttd == kThemeTabSouth) 
		pr.moveBy(0, -(r.height() + 2));
	    p->save();
	    p->setClipRect(QRect(pr.x() + fudge, pr.y(), pr.width() - (fudge * 2), pr.height()));
	    ((QMacPainter *)p)->setport();
	    ThemeDrawState tabpane_tds = kThemeStateActive;
	    if(qAquaActive(cg)) {
		if(!tb->isEnabled()) 
		    tabpane_tds = kThemeStateUnavailable;
	    } else { 
		if(tb->isEnabled()) 
		    tabpane_tds = kThemeStateInactive;
		else 
		    tabpane_tds = kThemeStateUnavailableInactive;
	    }
	    DrawThemeTabPane(qt_glb_mac_rect(pr, p), tabpane_tds);
	    p->restore();
	}
	break; }
    case CE_PushButton: {
	if(!widget)
	    break;
	QPushButton *btn = (QPushButton *)widget;
	if(btn->isFlat() && !(how & Style_Down)) {
	    btn->erase();
	    break;
	}

	d->addWidget(btn);
	if(btn->isToggleButton() && btn->isOn())
	    tds = kThemeStatePressed;
	QString pmkey;
	bool do_draw = FALSE;
	QPixmap *buffer = NULL;
	QTextOStream os(&pmkey);
	int frame = ((how & Style_Down) ? 0 : d->buttonState.frame);
	os << "$qt_mac_pshbtn_" << r.width() << "x" << r.height() << "_" << how << "_" << frame;
	if(d->animatable(QAquaAnimate::AquaPushButton, (QWidget *)widget)) {
	    tds = kThemeStatePressed;
	    if(frame && !(buffer = QPixmapCache::find(pmkey))) {
		do_draw = TRUE;
		buffer = new QPixmap(r.width(), r.height(), 32);
		buffer->fill(color0);
	    }
	}
	const QRect off_rct(1, 1, 1, 2);
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	if(btn->isFlat())
	    info.adornment = kThemeAdornmentNoShadow;
	((QMacPainter *)p)->setport();
	DrawThemeButton(qt_glb_mac_rect(r, p, TRUE, off_rct), 
			kThemePushButton, &info, NULL, NULL, NULL, 0);
	if(buffer) {
	    if(do_draw && frame) {
		QMacSavedPortInfo savedInfo(buffer);
		const Rect *buff_rct = qt_glb_mac_rect(QRect(0, 0, r.width(), r.height()), 
						       buffer, TRUE, off_rct);
		DrawThemeButton(buff_rct, kThemePushButton, &info, NULL, NULL, NULL, 0);

		QPixmap buffer_mask(buffer->size(), 32);
		{
		    buffer_mask.fill(color0);
		    ThemeButtonDrawInfo mask_info = info;
		    mask_info.state = kThemeStateActive;
		    QMacSavedPortInfo savedInfo(&buffer_mask);
		    DrawThemeButton(buff_rct, kThemePushButton, &mask_info, NULL, NULL, NULL, 0);
		}

		QImage img = buffer->convertToImage(), maskimg = buffer_mask.convertToImage();
		QImage mask_out(img.width(), img.height(), 1, 2, QImage::LittleEndian);
		for(int y = 0; y < img.height(); y++) {
		    //calculate a mask
		    for(int maskx = 0; maskx < img.width(); maskx++) {
			QRgb in = img.pixel(maskx, y), out = maskimg.pixel(maskx, y);
			int diff = (((qRed(in)-qRed(out))*((qRed(in)-qRed(out)))) +
				    ((qGreen(in)-qGreen(out))*((qGreen(in)-qGreen(out)))) +
				    ((qBlue(in)-qBlue(out))*((qBlue(in)-qBlue(out)))));
			mask_out.setPixel(maskx, y, diff > 100);
		    }
		    //pulse the colours
		    uchar *bytes = img.scanLine(y);
		    for(int x = 0; x < img.bytesPerLine(); x++) 
			*(bytes + x) = (*(bytes + x) * (100 - frame)) / 100;
		}
		*buffer = img;
		{
		    QBitmap qmask;
		    qmask = mask_out;
		    buffer->setMask(qmask);
		}
	    }
	    p->drawPixmap(r, *buffer);
	    if(do_draw && !QPixmapCache::insert(pmkey, buffer))	// save in cache
		delete buffer;
	}
	break; }
    default:
	QWindowsStyle::drawControl(element, p, widget, r, cg, how, opt);
    }
}

/*! \reimp */
void QMacStyle::drawComplexControl(ComplexControl ctrl, QPainter *p,
					const QWidget *widget,
					const QRect &r,
					const QColorGroup &cg,
					SFlags flags,
					SCFlags sub,
					SCFlags subActive,
					const QStyleOption& opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if(qAquaActive(cg)) {
	if(!(flags & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else if(flags & Style_Enabled) {
	tds = kThemeStateInactive;
    } else {
	tds = kThemeStateUnavailableInactive;
    }

    switch(ctrl) {
    case CC_ToolButton: {
	if(!widget)
	    break;
	QToolButton *toolbutton = (QToolButton *) widget;

	QRect button, menuarea;
	button   = querySubControlMetrics(ctrl, widget, SC_ToolButton, opt);
	menuarea = querySubControlMetrics(ctrl, widget, SC_ToolButtonMenu, opt);
	SFlags bflags = flags,
	       mflags = flags;
	if(subActive & SC_ToolButton)
	    bflags |= Style_Down;
	if(subActive & SC_ToolButtonMenu)
	    mflags |= Style_Down;

	if(sub & SC_ToolButton) {
	    if(bflags & (Style_Down | Style_On | Style_Raised)) {
		ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
		if(toolbutton->isOn() || toolbutton->isDown())
		    info.value |= kThemeStatePressed;
		((QMacPainter *)p)->setport();
		DrawThemeButton(qt_glb_mac_rect(button, p),
				kThemeBevelButton, &info, NULL, NULL, NULL, 0);
	    } else if(toolbutton->parentWidget() &&
			toolbutton->parentWidget()->backgroundPixmap() &&
			! toolbutton->parentWidget()->backgroundPixmap()->isNull()) {
		p->drawTiledPixmap(r, *(toolbutton->parentWidget()->backgroundPixmap()),
				    toolbutton->pos());
	    }
	}

	if(sub & SC_ToolButtonMenu) {
	    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	    if(toolbutton->isOn() || toolbutton->isDown() || (subActive & SC_ToolButtonMenu))
		info.value |= kThemeStatePressed;
	    ((QMacPainter *)p)->setport();
	    DrawThemeButton(qt_glb_mac_rect(menuarea, p),
			    kThemeBevelButton, &info, NULL, NULL, NULL, 0);
	    QRect r(menuarea.x() + ((menuarea.width() / 2) - 4), menuarea.height() - 8, 8, 8);
	    DrawThemePopupArrow(qt_glb_mac_rect(r, p),
				kThemeArrowDown, kThemeArrow7pt, tds, NULL, 0);
	}
	break; }
    case CC_ListView: {
	if(sub & SC_ListView)
	    QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, opt);
	if(sub & (SC_ListViewBranch | SC_ListViewExpand)) {
	    if(opt.isDefault())
		break;
	    QListViewItem *item = opt.listViewItem();
	    int y=r.y(), h=r.height();
	    ((QMacPainter *)p)->setport();
	    {
		::RGBColor f;
		f.red = f.green = f.blue = 0;
		RGBForeColor(&f);
		f.red = f.green = f.blue = ~0;
		RGBBackColor(&f);
	    }
	    for(QListViewItem *child = item->firstChild(); child && y < h;
		y += child->totalHeight(), child = child->nextSibling()) {
		if(y + child->height() > 0) {
		    if(child->isExpandable() || child->childCount()) {
			ThemeButtonDrawInfo info = { tds, kThemeDisclosureRight, kThemeAdornmentDrawIndicatorOnly };
			if(child->isOpen())
			    info.value = kThemeDisclosureDown;
			DrawThemeButton(qt_glb_mac_rect(
			    QRect(r.right() - 10, (y + child->height()/2) - 4, 9, 9), p),
					kThemeDisclosureButton, &info, NULL, NULL, NULL, 0);
		    }
		}
	    }
	}
	break; }
    case CC_SpinWidget: {
	QSpinWidget * sw = (QSpinWidget *) widget;
	if(sub & SC_SpinWidgetFrame)
	    drawPrimitive(PE_PanelLineEdit, p, r, cg, Style_Sunken);
	if((sub & SC_SpinWidgetDown) || (sub & SC_SpinWidgetUp)) {
	    if(!sw->isUpEnabled() && !sw->isDownEnabled())
		tds = kThemeStateUnavailable;
	    if(subActive == SC_SpinWidgetDown)
		tds = kThemeStatePressedDown;
	    else if(subActive == SC_SpinWidgetUp)
		tds = kThemeStatePressedUp;
	    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	    QRect updown = sw->upRect() | sw->downRect();
	    if(sw->backgroundPixmap())
		p->drawPixmap(updown, *sw->backgroundPixmap());
	    else
		p->fillRect(updown, sw->backgroundColor());
	    ((QMacPainter *)p)->setport();
	    DrawThemeButton(qt_glb_mac_rect(updown, p), kThemeIncDecButton,
			    &info, NULL, NULL, NULL, 0);
	}
	break; }
    case CC_TitleBar: {
	if(!widget)
	    break;
	QTitleBar *tbar = (QTitleBar *) widget;
	ThemeWindowMetrics twm;
	memset(&twm, '\0', sizeof(twm));
	twm.metricSize = sizeof(twm);
	twm.titleWidth = tbar->width();
	twm.titleHeight = tbar->height();
	ThemeWindowAttributes twa = kThemeWindowHasTitleText;
	if(tbar->window())
	    twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
	else if(tbar->testWFlags(WStyle_SysMenu))
	    twa |= kThemeWindowHasCloseBox;
	if(flags & Style_MouseOver)
	    tds = kThemeStateRollover;
	//AppMan paints outside the given rectangle, so I have to adjust for the height properly!
	QRegion treg;
	GetThemeWindowRegion(macWinType, qt_glb_mac_rect(r),
			     tds, &twm, twa, kWindowTitleBarRgn, treg.handle(TRUE));
	QRect br = treg.boundingRect(), newr = r;
	newr.moveBy(newr.x() - br.x(), newr.y() - br.y());
	((QMacPainter *)p)->setport();
	DrawThemeWindowFrame(macWinType, qt_glb_mac_rect(newr, p, FALSE),
			     tds, &twm, twa, NULL, 0);
	if((sub & SC_TitleBarLabel) || 1) {
	    p->save();
	    int iw = 0;
	    if(tbar->icon()) {
		GetThemeWindowRegion(macWinType, qt_glb_mac_rect(newr),
				     tds, &twm, twa, kWindowTitleProxyIconRgn, treg.handle(TRUE));
		if(!treg.isEmpty())
		    iw = tbar->icon()->width();
	    }
	    if(!tbar->visibleText().isEmpty()) {
		GetThemeWindowRegion(macWinType, qt_glb_mac_rect(newr),
				     tds, &twm, twa, kWindowTitleTextRgn, treg.handle(TRUE));
		p->setClipRegion(treg);
		QRect br = treg.boundingRect();
		int x = br.x(), y = br.y() + ((tbar->height() / 2) - (p->fontMetrics().height() / 2));
		if(br.width() <= (p->fontMetrics().width(tbar->caption())+iw*2))
		    x += iw;
		else
		    x += (br.width() / 2) - (p->fontMetrics().width(tbar->visibleText()) / 2);
		if(iw)
		    p->drawPixmap(x - iw, y, *tbar->icon());
		p->drawText(x, y + p->fontMetrics().ascent(), tbar->visibleText());
	    }
	    p->restore();
	}
	break; }
    case CC_ScrollBar: {
	if(!widget)
	    break;
	QScrollBar *scrollbar = (QScrollBar *) widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumScrollBar;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    ttdi.kind = kThemeSmallScrollBar;
	ttdi.bounds = *qt_glb_mac_rect(r, p);
	ttdi.min = scrollbar->minValue();
	ttdi.max = scrollbar->maxValue();
	ttdi.value = scrollbar->value();
	ttdi.attributes |= kThemeTrackShowThumb;
	if(scrollbar->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(!qAquaActive(cg))
	    ttdi.enableState = kThemeTrackInactive;
	else if(!scrollbar->isEnabled())
	    ttdi.enableState = kThemeTrackDisabled;
	if(subActive == SC_ScrollBarSubLine)
	    ttdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed |
						  kThemeLeftOutsideArrowPressed;
	else if(subActive == SC_ScrollBarAddLine)
	    ttdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed |
						  kThemeRightOutsideArrowPressed;
	else if(subActive == SC_ScrollBarAddPage)
	    ttdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
	else if(subActive == SC_ScrollBarSubPage)
	    ttdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
	else if(subActive == SC_ScrollBarSlider)
	    ttdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
	ttdi.trackInfo.scrollbar.viewsize = scrollbar->pageStep();
	((QMacPainter *)p)->setport();
	DrawThemeTrack(&ttdi, NULL, NULL, 0);
	break; }
    case CC_Slider: {
	if(!widget)
	    break;
	QSlider *sldr = (QSlider *)widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumSlider;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    ttdi.kind = kThemeSmallSlider;
	ttdi.bounds = *qt_glb_mac_rect(widget->rect(), p);
	ttdi.min = sldr->minValue();
	ttdi.max = sldr->maxValue();
	ttdi.value = sldr->valueFromPosition(sldr->sliderStart());
	ttdi.attributes |= kThemeTrackShowThumb;
	if(sldr->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(widget->isEnabled())
	    ttdi.enableState |= kThemeTrackActive;
	if(!qAquaActive(cg))
	    ttdi.enableState = kThemeTrackInactive;
	else if(!sldr->isEnabled())
	    ttdi.enableState = kThemeTrackDisabled;
	if(sldr->tickmarks() == QSlider::Above)
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
	else
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
	if(subActive == SC_SliderGroove)
	    ttdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
	else if(subActive == SC_SliderHandle)
	    ttdi.trackInfo.slider.pressState = kThemeThumbPressed;
	if(sldr->backgroundPixmap())
	    p->drawPixmap(r, *sldr->backgroundPixmap());
	else
	    p->fillRect(r, sldr->backgroundColor());
	((QMacPainter *)p)->setport();
	DrawThemeTrack(&ttdi, NULL, NULL, 0);
	if(sub & SC_SliderTickmarks)
	    DrawThemeTrackTickMarks(&ttdi, sldr->maxValue() / sldr->pageStep(), NULL, 0);
	break; }
    case CC_ComboBox: {
	ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
	((QMacPainter *)p)->setport();
	DrawThemeButton(qt_glb_mac_rect(r, p, TRUE, QRect(1, 0, 0, 0)), kThemePopupButton,
			&info, NULL, NULL, NULL, 0);
	break; }
    default:
	QWindowsStyle::drawComplexControl(ctrl, p, widget, r, cg, flags, sub, subActive, opt);
    }
}

/*! \reimp */
int QMacStyle::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret = 0;
    switch(metric) {
    case PM_PopupMenuScrollerHeight:
#if 0
	SInt16 ash, asw;
	GetThemeMenuItemExtra(kThemeMenuItemScrollUpArrow, &ash, &asw);
	ret = ash;
#else
	ret = 10; //I hate having magic numbers in here...
#endif
	break;
    case PM_DefaultFrameWidth:
	if(widget &&
	   (widget->isTopLevel() || !widget->parentWidget() || 
	    (widget->parentWidget()->inherits("QMainWindow") && 
	     ((QMainWindow*)widget->parentWidget())->centralWidget() == widget)) &&
	    (widget->inherits("QScrollView") || widget->inherits("QWorkspaceChild")))
	    ret = 0;
	else if(widget && widget->inherits("QLineEdit"))
	    GetThemeMetric(kThemeMetricEditTextFrameOutset, &ret);
	else
	    ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    case PM_MaximumDragDistance:
	ret = -1;
	break;
    case PM_ScrollBarSliderMin:
	ret = 24;
	break;
    case PM_TabBarBaseHeight:
	ret = 8;
	break;
    case PM_SpinBoxFrameWidth:
	ret = 1;
	break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
	ret = 0;
	break;
    case PM_SliderLength:
	ret = 17;
	break;
    case PM_ButtonDefaultIndicator:
	ret = 0;
	break;
    case PM_TitleBarHeight: {
	if(!widget)
	    break;
	QTitleBar *tbar = (QTitleBar*)widget;
	ThemeWindowMetrics twm;
	memset(&twm, '\0', sizeof(twm));
	twm.metricSize = sizeof(twm);
	twm.titleWidth = tbar->width();
	twm.titleHeight = tbar->height();
	ThemeWindowAttributes twa = kThemeWindowHasTitleText;
	if(tbar->window())
	    twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
	else if(tbar->testWFlags(WStyle_SysMenu))
	    twa |= kThemeWindowHasCloseBox;
	QRegion treg;
	GetThemeWindowRegion(macWinType, qt_glb_mac_rect(tbar->rect()), kThemeStateActive,
			     &twm, twa, kWindowTitleBarRgn, treg.handle(TRUE));
	ret = treg.boundingRect().height();
	break; }
    case PM_TabBarTabOverlap:
	GetThemeMetric(kThemeMetricTabOverlap, &ret);
	break;
    case PM_ScrollBarExtent: {
	ThemeMetric tm = kThemeMetricScrollBarWidth;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallScrollBarWidth;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_TabBarBaseOverlap:
	GetThemeMetric(kThemeMetricTabFrameOverlap, &ret);
	break;
    case PM_IndicatorHeight: {
	ThemeMetric tm = kThemeMetricCheckBoxHeight;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallCheckBoxHeight;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_IndicatorWidth: {
	ThemeMetric tm = kThemeMetricCheckBoxWidth;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallCheckBoxWidth;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_ExclusiveIndicatorHeight: {
	ThemeMetric tm = kThemeMetricRadioButtonHeight;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallRadioButtonHeight;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_ExclusiveIndicatorWidth: {
	ThemeMetric tm = kThemeMetricRadioButtonWidth;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallRadioButtonWidth;
	GetThemeMetric(tm, &ret);
	break; }
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
}

/*! \reimp */
QRect QMacStyle::querySubControlMetrics(ComplexControl control,
					    const QWidget *w,
					    SubControl sc,
					    const QStyleOption& opt) const
{
    switch(control) {
    case CC_SpinWidget: {
	const int spinner_w = 20, spinner_h = 10; //isn't there some way to get this from the AppMan?
	int fw = pixelMetric(PM_SpinBoxFrameWidth, w), y = fw, x = w->width() - fw - spinner_w;
	switch ( sc ) {
	case SC_SpinWidgetUp:
	    return QRect(x, y + ((w->height() / 2) - spinner_h), spinner_w, spinner_h);
	case SC_SpinWidgetDown:
	    return QRect(x, y + (w->height() / 2), spinner_w, spinner_h);
	case SC_SpinWidgetButtonField:
	    return QRect(x, y, spinner_w, w->height() - (fw*2));
	case SC_SpinWidgetEditField: 
	    return QRect(fw + 3, fw, (w->width() - spinner_w - (fw*3) - 3) - 3, w->height() - (fw*2));
	case SC_SpinWidgetFrame:
	    return QRect(0, 0, w->width() - spinner_w - fw - 3, w->height());
	default:
	    break;
	}
	break; }
    case CC_TitleBar: {
	if(!w)
	    return QRect();
	QTitleBar *tbar = (QTitleBar*)w;
	ThemeWindowMetrics twm;
	memset(&twm, '\0', sizeof(twm));
	twm.metricSize = sizeof(twm);
	twm.titleWidth = tbar->width();
	twm.titleHeight = tbar->height();
	ThemeWindowAttributes twa = kThemeWindowHasTitleText;
	if(tbar->window())
	    twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox | kThemeWindowHasCollapseBox;
	else if(tbar->testWFlags(WStyle_SysMenu))
	    twa |= kThemeWindowHasCloseBox;
	WindowRegionCode wrc = kWindowGlobalPortRgn;
	if(sc & SC_TitleBarCloseButton)
	    wrc = kWindowCloseBoxRgn;
	else if(sc & SC_TitleBarMinButton)
	    wrc = kWindowCollapseBoxRgn;
	else if(sc & SC_TitleBarMaxButton)
	    wrc = kWindowZoomBoxRgn;
	else if(sc & SC_TitleBarLabel)
	    wrc = kWindowTitleTextRgn;
	else if(sc & SC_TitleBarSysMenu)
	    return QRect(-666, -666, 10, pixelMetric(PM_TitleBarHeight)); //ugh
	if(wrc != kWindowGlobalPortRgn) {
	    //AppMan paints outside the given rectangle, so I have to adjust for the height properly!
	    QRegion treg;
	    QRect r = w->rect();
	    GetThemeWindowRegion(macWinType, qt_glb_mac_rect(r),
				 kThemeStateActive, &twm, twa, kWindowTitleBarRgn, treg.handle(TRUE));
	    QRect br = treg.boundingRect();
	    r.moveBy(r.x() - br.x(), r.y() - br.y());
	    GetThemeWindowRegion(macWinType, qt_glb_mac_rect(r),
				 kThemeStateActive, &twm, twa, wrc, treg.handle(TRUE));
	    return treg.boundingRect();
	}
	break; }
    case CC_ComboBox: {
	if(sc == SC_ComboBoxEditField) 
	    return QRect(3, 3, w->width() - 22, w->height() - 5);
	break; }
    case CC_ScrollBar: {
	if(!w)
	    return QRect();
	QScrollBar *scrollbar = (QScrollBar *) w;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumScrollBar;
	if(qt_aqua_size_constrain(w) == QAquaSizeSmall)
	    ttdi.kind = kThemeSmallScrollBar;
	ttdi.bounds = *qt_glb_mac_rect(w->rect());
	ttdi.min = scrollbar->minValue();
	ttdi.max = scrollbar->maxValue();
	ttdi.value = scrollbar->value();
	ttdi.attributes |= kThemeTrackShowThumb;
	if(scrollbar->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(!qAquaActive(scrollbar->colorGroup()))
	    ttdi.enableState = kThemeTrackInactive;
	else if(!scrollbar->isEnabled())
	    ttdi.enableState = kThemeTrackDisabled;
	ttdi.trackInfo.scrollbar.viewsize = scrollbar->pageStep();
	switch(sc) {
	case SC_ScrollBarGroove: {
	    Rect mrect;
	    GetThemeTrackBounds(&ttdi, &mrect);
	    return QRect(mrect.left, mrect.top, mrect.right - mrect.left, mrect.bottom - mrect.top);
	    break; }
	case SC_ScrollBarSlider: {
	    QRegion rgn;
	    GetThemeTrackThumbRgn(&ttdi, rgn.handle(TRUE));
	    return rgn.boundingRect();
	    break; }
	default:
	    break;
	}
	break; }
    case CC_Slider: {
	if(!w)
	    return QRect();
	QSlider *sldr = (QSlider *)w;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumSlider;
	if(qt_aqua_size_constrain(w) == QAquaSizeSmall)
	    ttdi.kind = kThemeSmallSlider;
	ttdi.bounds = *qt_glb_mac_rect(w->rect());
	ttdi.min = sldr->minValue();
	ttdi.max = sldr->maxValue();
	ttdi.value = sldr->valueFromPosition(sldr->sliderStart());
	ttdi.attributes |= kThemeTrackShowThumb;
	if(sldr->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(!qAquaActive(sldr->colorGroup()))
	    ttdi.enableState = kThemeTrackInactive;
	else if(!sldr->isEnabled())
	    ttdi.enableState = kThemeTrackDisabled;
	if(sldr->tickmarks() == QSlider::Above)
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
	else
	    ttdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
	switch(sc) {
	case SC_SliderGroove: {
	    Rect mrect;
	    GetThemeTrackBounds(&ttdi, &mrect);
	    return QRect(mrect.left, mrect.top,
			mrect.right - mrect.left, mrect.bottom - mrect.top);
	    break; }
	case SC_SliderHandle: {
	    QRegion rgn;
	    GetThemeTrackThumbRgn(&ttdi, rgn.handle(TRUE));
	    return rgn.boundingRect();
	    break; }
	default:
	    break;
	}
	break; }
    default:
	break;
    }
    return QWindowsStyle::querySubControlMetrics(control, w, sc, opt);
}

/*! \reimp */
QRect QMacStyle::subRect(SubRect r, const QWidget *w) const
{
    QRect ret;
    switch(r) {
    case SR_ProgressBarLabel:
    case SR_ProgressBarGroove:
	break;
    case SR_ProgressBarContents:
	ret = w->rect();
	break;
    default:
	ret = QWindowsStyle::subRect(r, w);
	break;
    }
    return ret;
}

/*! \reimp */
QStyle::SubControl QMacStyle::querySubControl(ComplexControl control,
						 const QWidget *widget,
						 const QPoint &pos,
						 const QStyleOption& opt) const
{
    SubControl ret = SC_None;
    switch(control) {
    case CC_ScrollBar: {
	if(!widget)
	    break;
	QScrollBar *scrollbar = (QScrollBar *) widget;
	ThemeTrackDrawInfo ttdi;
	memset(&ttdi, '\0', sizeof(ttdi));
	ttdi.kind = kThemeMediumScrollBar;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    ttdi.kind = kThemeSmallScrollBar;
	ttdi.bounds = *qt_glb_mac_rect(widget->rect());
	ttdi.min = scrollbar->minValue();
	ttdi.max = scrollbar->maxValue();
	ttdi.value = scrollbar->value();
	ttdi.attributes |= kThemeTrackShowThumb;
	if(scrollbar->orientation() == Qt::Horizontal)
	    ttdi.attributes |= kThemeTrackHorizontal;
	if(!qAquaActive(scrollbar->colorGroup()))
	    ttdi.enableState = kThemeTrackInactive;
	else if(!scrollbar->isEnabled())
	    ttdi.enableState = kThemeTrackDisabled;
	ttdi.trackInfo.scrollbar.viewsize = scrollbar->pageStep();
	Point pt = { pos.y(), pos.x() };
	Rect mrect;
	GetThemeTrackBounds(&ttdi, &mrect);
	ControlPartCode cpc;
	if(HitTestThemeScrollBarArrows(&ttdi.bounds, ttdi.enableState,
				       0, scrollbar->orientation() == Qt::Horizontal,
				       pt, &mrect, &cpc)) {
	    if(cpc == kControlUpButtonPart)
		ret = SC_ScrollBarSubLine;
	    else if(cpc == kControlDownButtonPart)
		ret = SC_ScrollBarAddLine;
	} else if(HitTestThemeTrack(&ttdi, pt, &cpc)) {
	    if(cpc == kControlPageUpPart)
		ret = SC_ScrollBarSubPage;
	    else if(cpc == kControlPageDownPart)
		ret = SC_ScrollBarAddPage;
	    else
		ret = SC_ScrollBarSlider;
	}
	break; }
    default:
	ret = QWindowsStyle::querySubControl(control, widget, pos, opt);
	break;
    }
    return ret;
}

/*! \reimp */
int QMacStyle::styleHint(StyleHint sh, const QWidget *w,
			  const QStyleOption &opt,QStyleHintReturn *d) const
{
    SInt32 ret = 0;
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
    case SH_RichText_FullWidthSelection:
	ret = TRUE;
	break;
    case SH_BlinkCursorWhenTextSelected:
	ret = FALSE;
	break;
    case SH_ScrollBar_StopMouseOverSlider:
        ret = TRUE;
        break;
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
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

/*! \reimp */
QSize QMacStyle::sizeFromContents(ContentsType contents,
				       const QWidget *widget,
				       const QSize &contentsSize,
				       const QStyleOption& opt) const
{
    QSize sz(contentsSize);
    switch(contents) {
    case CT_TabBarTab: {
	SInt32 lth = kThemeLargeTabHeight;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    lth = kThemeSmallTabHeight;
	if(sz.height() > lth)
	    sz.setHeight(lth);
	break; }
    case CT_PopupMenuItem: {
	if(!widget || opt.isDefault())
	    break;
	const QPopupMenu *popup = (const QPopupMenu *) widget;
	bool checkable = popup->isCheckable();
	QMenuItem *mi = opt.menuItem();
	int maxpmw = opt.maxIconWidth();
	int w = sz.width(), h = sz.height();

	if(mi->custom()) {
	    w = mi->custom()->sizeHint().width();
	    h = mi->custom()->sizeHint().height();
	    if(! mi->custom()->fullSpan())
		h += 8;
	} else if(mi->widget()) {
	} else if(mi->isSeparator()) {
	    w = 10;
	    SInt16 ash;
	    GetThemeMenuSeparatorHeight(&ash);
	    h = ash;
	} else {
	    if(mi->pixmap())
		h = QMAX(h, mi->pixmap()->height() + 4);
	    else
		h = QMAX(h, popup->fontMetrics().height() + 8);

	    if(mi->iconSet() != 0)
		h = QMAX(h, mi->iconSet()->pixmap(QIconSet::Small,
						  QIconSet::Normal).height() + 4);
	}

	if(! mi->text().isNull()) {
	    if(mi->text().find('\t') >= 0)
		w += 12;
	}

	if(maxpmw)
	    w += maxpmw + 6;
	if(checkable && maxpmw < 20)
	    w += 20 - maxpmw;
	if(checkable || maxpmw > 0)
	    w += 2;
	w += 12;
	if(widget->parentWidget() && widget->parentWidget()->inherits("QComboBox"))
	    w = QMAX(w, widget->parentWidget()->width() - 20);
	sz = QSize(w, h);
	break; }
    case CT_PushButton:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	sz = QSize(sz.width() + 16, sz.height()); //###
	break;
    default:
	sz = QWindowsStyle::sizeFromContents(contents, widget, contentsSize, opt);
	break;
    }
    {
	QSize macsz;
	if(qt_aqua_size_constrain(widget, contents, sz, &macsz) != QAquaSizeUnknown) {
	    if(macsz.width() != -1)
		sz.setWidth(macsz.width());
	    if(macsz.height() != -1)
		sz.setHeight(macsz.height());
	}
    }
    return sz;
}

/*! \reimp */
bool QMacStyle::event(QEvent *e)
{
    if(e->type() == QEvent::Style) {
	    QPalette pal = qApp->palette();
	    if(qt_mac_update_palette(pal, FALSE))
		qApp->setPalette(pal, TRUE);
    }
    return FALSE;
}

#endif
