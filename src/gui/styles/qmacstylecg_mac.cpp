#include "qmacstylecg_mac.h"

#if QT_MACOSX_VERSION >= 0x1030

#include <qapplication.h>
#include <qevent.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpushbutton.h>
#include <qpointer.h>
#include <qpopupmenu.h>
#include <qslider.h>
#include <qt_mac.h>

#include <private/qaquastyle_p.h>

QPixmap qt_mac_convert_iconref(IconRef, int, int); //qpixmap_mac.cpp

const int qt_mac_hitheme_version = 0; //the HITheme version we speak
// Utility to generate correct rectangles for AppManager internals
static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=0,
					    bool off=true, const QRect &rect=QRect())
{
    static HIRect r;
    QPoint tl(qr.topLeft());
    if (pd && pd->devType() == QInternal::Widget) {
	QWidget *w = (QWidget*)pd;
	tl = w->mapTo(w->topLevelWidget(), tl);
    }
    int offset = 0;
    if (off)
	offset = 1;
    r = CGRectMake(tl.x()+rect.x(), tl.y()+rect.y(),
		   qr.width() - offset - rect.width(), qr.height() - offset - rect.height());
    return &r;
}

static inline const HIRect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
					    bool off=true, const QRect &rect=QRect())
{
    int x, y;
    p->map(qr.x(), qr.y(), &x, &y);
    QRect r(QPoint(x, y), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if (p && p->device()->devType() == QInternal::Widget)
	return qt_aqua_size_constrain((QWidget*)p->device());
    return qt_aqua_size_constrain(0);
}

class QMacStyleCGPrivate : public QObject 
{
public:
    static const int RefreshRate = 33;  // Gives us about 30 frames a second.
    QPointer<QPushButton> defaultButton;
    CFAbsoluteTime defaultButtonStart;
    int timerID;
    
    QMacStyleCGPrivate() { defaultButtonStart = 0; timerID = -1; }
    inline void animateButton(QPushButton *btn) {
        defaultButton = btn;
        defaultButtonStart = CFAbsoluteTimeGetCurrent();
        if (timerID <= 0)
            timerID = startTimer(RefreshRate);
    }
    inline void stopButtonAnimation() {
        if (!defaultButton)
            return;
        QPushButton *tmp = defaultButton;
        defaultButton = 0; // Lazily let the other stuff be reset in timerEvent.
        if (tmp->isVisible())
            tmp->update();
    }
protected:
    inline void timerEvent(QTimerEvent *)
    {
        if (!defaultButton.isNull()) {
            defaultButton->update();
        } else {
            killTimer(timerID);
            timerID = -1;
            defaultButtonStart = 0;
        }
    }
};

//externals
QRegion qt_mac_convert_mac_region(HIShapeRef); //qregion_mac.cpp

//HITheme QMacStyle
QMacStyleCG::QMacStyleCG() : QWindowsStyle()
{
    d = new QMacStyleCGPrivate;
}

QMacStyleCG::~QMacStyleCG()
{

}

void QMacStyleCG::polish(QWidget *w)
{
    QPushButton *btn = ::qt_cast<QPushButton *>(w);
    if (btn) {
        btn->installEventFilter(this);
        if (btn->isDefault() || (btn->autoDefault() && btn->hasFocus())) {
            d->animateButton(btn);
        }
    }
    
    QPopupMenu *pop = ::qt_cast<QPopupMenu *>(w);
    if (pop) {
        QPalette pal = pop->palette();
        QPixmap px(200, 200, 32);
        QPainter p(&px);
        HIThemeMenuDrawInfo mtinfo;
        mtinfo.version = qt_mac_hitheme_version;
        mtinfo.menuType = kThemeMenuTypePopUp;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawMenuBackground(&rect, &mtinfo, static_cast<CGContextRef>(p.handle()),
                                  kHIThemeOrientationNormal);
        p.end();
        QBrush background(black, px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        pop->setPalette(pal);
    }
    QWindowsStyle::polish(w);
}

void QMacStyleCG::unPolish(QWidget *w)
{
    QWindowsStyle::unPolish(w);
}

void QMacStyleCG::polish(QApplication *app)
{
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QPainter p(&px);
    
    // This gives us the metal look, of course.
    /*
    HIThemeBackgroundDrawInfo bginfo;
    bginfo.version = qt_mac_hitheme_version;
    bginfo.state = kThemeStateActive;
    bginfo.kind = kThemeBackgroundMetal;
    HIRect rect = CGRectMake(0, 0, px.width(), px.height());
    HIThemeDrawBackground(&rect, &bginfo, static_cast<CGContextRef>(p.handle()),
                          kHIThemeOrientationNormal);
     */
    
    // Believe it or not, we get the "older" style with this pixmap
    HIThemeMenuDrawInfo mtinfo;
    mtinfo.version = qt_mac_hitheme_version;
    mtinfo.menuType = kThemeMenuTypeHierarchical;
    HIRect rect = CGRectMake(0, 0, px.width(), px.height());
    HIThemeDrawMenuBackground(&rect, &mtinfo, static_cast<CGContextRef>(p.handle()),
                              kHIThemeOrientationNormal);
    
    p.end();
    QBrush background(black, px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
    app->setPalette(pal);
}

void QMacStyleCG::drawPrimitive(PrimitiveElement pe, QPainter *p, const QRect &r,
				const QPalette &pal, SFlags flags, const QStyleOption &opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if (flags & Style_Down) {
	tds = kThemeStatePressed;
    } else if (qAquaActive(pal)) {
	if (!(flags & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if (flags & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch (pe) {
    case PE_CheckListController:
	break;
    case PE_CheckListExclusiveIndicator:
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = (qt_mac_get_size_for_painter(p) == QAquaSizeSmall) ? kThemeSmallRadioButton
	            : kThemeRadioButton;
	info.value = (flags & Style_On) ? kThemeButtonOn : kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	const HIRect *mac_r = qt_glb_mac_rect(r, p);
	if (pe == PE_ExclusiveIndicatorMask) {
	    QRegion saveRegion = p->clipRegion();
	    HIShapeRef shape;
	    HIThemeGetButtonShape(mac_r, &info, &shape);
	    p->setClipRegion(qt_mac_convert_mac_region(shape));
	    p->fillRect(r, color1);
	    p->setClipRegion(saveRegion);
	} else {
	    HIThemeDrawButton(mac_r, &info, static_cast<CGContextRef>(p->handle()),
			      kHIThemeOrientationNormal, 0);
	}
	break; }
    case PE_CheckListIndicator:
    case PE_IndicatorMask:
    case PE_Indicator: {
	HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
	info.state = tds;
	info.kind = (qt_mac_get_size_for_painter(p) == QAquaSizeSmall) ? kThemeSmallCheckBox
		    : kThemeCheckBox;
	if (flags & Style_NoChange)
	    info.value = kThemeButtonMixed;
	else if (flags & Style_On)
	    info.value = kThemeButtonOn;
	else
	    info.value = kThemeButtonOff;
	info.adornment = kThemeAdornmentNone;
	const HIRect *mac_r = qt_glb_mac_rect(r, p);
	if (pe == PE_IndicatorMask) {
	    QRegion saveRegion = p->clipRegion();
	    HIShapeRef shape;
	    HIThemeGetButtonShape(mac_r, &info, &shape);
	    p->setClipRegion(qt_mac_convert_mac_region(shape));
	    p->fillRect(r, color1);
	    p->setClipRegion(saveRegion);
	} else {
	    HIThemeDrawButton(mac_r, &info, static_cast<CGContextRef>(p->handle()),
			      kHIThemeOrientationNormal, 0);
	}
	break; }
    case PE_FocusRect:
	break;     //This is not used because of the QAquaFocusWidget thingie.
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        HIThemePopupArrowDrawInfo info;
        info.version = qt_mac_hitheme_version;
        info.state = tds;
        switch (pe) {
        case PE_ArrowUp:
            info.orientation = kThemeArrowUp;
            break;
        case PE_ArrowDown:
            info.orientation = kThemeArrowDown;
            break;
        case PE_ArrowRight:
            info.orientation = kThemeArrowRight;
            break;
        case PE_ArrowLeft:
            info.orientation = kThemeArrowLeft;
            break;
        default:     // Stupid compiler should know better.
            break;
        }
        if (r.width() < 8)
            info.size = kThemeArrow5pt;
        else
            info.size = kThemeArrow9pt;
        HIThemeDrawPopupArrow(qt_glb_mac_rect(r, p), &info, static_cast<CGContextRef>(p->handle()),
                              kHIThemeOrientationNormal);
        break; }
    case PE_TreeBranch: {
        if (!(flags & Style_Children))
            break;
        HIThemeButtonDrawInfo bi;
        bi.version = qt_mac_hitheme_version;
        bi.state = flags & Style_Enabled ? kThemeStateActive : kThemeStateInactive;
        if (flags & Style_Down)
            bi.state |= kThemeStatePressed;
        bi.kind = kThemeDisclosureButton;
        bi.value = flags & Style_Open ? kThemeDisclosureDown : kThemeDisclosureRight;
        bi.adornment = kThemeAdornmentNone;
        HIThemeDrawButton(qt_glb_mac_rect(r, p), &bi, static_cast<CGContextRef>(p->handle()),
                          kHIThemeOrientationNormal, 0);
        break; }
    default:
	QWindowsStyle::drawPrimitive(pe, p, r, pal, flags, opt);
	break;
    }
}

void QMacStyleCG::drawControl(ControlElement element, QPainter *p, const QWidget *widget,
			      const QRect &r, const QPalette &pal, SFlags how,
			      const QStyleOption &opt) const
{
    ThemeDrawState tds = kThemeStateActive;
    if (qAquaActive(pal)) {
	if (!(how & Style_Enabled))
	    tds = kThemeStateUnavailable;
    } else {
	if (how & Style_Enabled)
	    tds = kThemeStateInactive;
	else
	    tds = kThemeStateUnavailableInactive;
    }

    switch(element) {
    case CE_PushButton: {
        if (!widget)
            break;
	const QPushButton *btn = static_cast<const QPushButton *>(widget);
        if (btn->isFlat() && !(how & Style_Down))
	    return;
        HIThemeButtonDrawInfo info;
	info.version = qt_mac_hitheme_version;
        if (how & (Style_On | Style_Down))
            info.state = kThemeStatePressed;
        else
            info.state = tds;
        info.value = kThemeButtonOff;
        if (btn->isMenuButton() || btn->isFlat()) {
            info.kind = kThemeMediumBevelButton;
        } else {
            info.kind = kThemePushButton;
            info.adornment = kThemeAdornmentNone;
        }

        if (how & Style_ButtonDefault && btn == d->defaultButton) {
            info.adornment = kThemeAdornmentDefault;
            info.animation.time.start = d->defaultButtonStart;
            info.animation.time.current = CFAbsoluteTimeGetCurrent();
        }
	HIThemeDrawButton(qt_glb_mac_rect(r, p), &info, static_cast<CGContextRef>(p->handle()),
		          kHIThemeOrientationNormal, 0);
	break; }
    case CE_PushButtonLabel: {
        // ### This is wrong, we should probably have another couple of rects, the arrow shouldn't be part of the label.
#ifndef QT_NO_PUSHBUTTON
        const QPushButton *button = static_cast<const QPushButton *>(widget);
        QRect ir = r;
        if (button->isMenuButton()) {
            int mbi = pixelMetric(PM_MenuButtonIndicator, widget);
            QRect ar(ir.right() - mbi, ir.height() - 13, mbi, ir.height() - 4);
            drawPrimitive(PE_ArrowDown, p, ar, pal, how, opt);
            ir.setWidth(ir.width() - mbi);
        }
        int tf = AlignVCenter | ShowPrefix | NoAccel;
        
#ifndef QT_NO_ICONSET
        if (button->iconSet() && ! button->iconSet()->isNull()) {
            QIconSet::Mode mode = button->isEnabled() ? QIconSet::Normal : QIconSet::Disabled;
            if (mode == QIconSet::Normal && button->hasFocus())
                mode = QIconSet::Active;
            
            QIconSet::State state = QIconSet::Off;
            if (button->isToggleButton() && button->isOn())
                state = QIconSet::On;
            
            QPixmap pixmap = button->iconSet()->pixmap(QIconSet::Small, mode, state);
            int pixw = pixmap.width();
            int pixh = pixmap.height();
            
            //Center the icon if there is neither text nor pixmap
            if (button->text().isEmpty() && !button->pixmap())
                p->drawPixmap(ir.x() + ir.width() / 2 - pixw / 2,
                              ir.y() + ir.height() / 2 - pixh / 2, pixmap);
            else
                p->drawPixmap(ir.x() + 2, ir.y() + ir.height() / 2 - pixh / 2, pixmap);
            
            ir.moveBy(pixw + 4, 0);
            ir.setWidth(ir.width() - (pixw + 4));
            // left-align text if there is
            if (!button->text().isEmpty())
                tf |= AlignLeft;
            else if (button->pixmap())
                tf |= AlignHCenter;
        } else
#endif //QT_NO_ICONSET
            tf |= AlignHCenter;
        drawItem(p, ir, tf, pal,
                 how & Style_Enabled, button->pixmap(), button->text(), -1,
                 &(pal.buttonText().color()) );
#endif
        break; }
    case CE_PopupMenuItem: {
        if (!widget || opt.isDefault())
            break;
        const QPopupMenu *popup = static_cast<const QPopupMenu *>(widget);
        const QMenuItem *mi = opt.menuItem();
        bool disabled = mi ? !mi->isEnabled() : false;
        bool itemSelected = how & Style_Active;
        // Draw the background and then the text and stuff.
        HIRect menuRect = *qt_glb_mac_rect(popup->rect(), popup);
        HIRect itemRect = *qt_glb_mac_rect(r, popup);
        HIThemeMenuItemDrawInfo mdi;
        mdi.version = qt_mac_hitheme_version;
        mdi.itemType = kThemeMenuItemPlain;
        if (mi && mi->iconSet())
            mdi.itemType |= kThemeMenuItemHasIcon;
        if (mi && mi->popup())
            mdi.itemType |= kThemeMenuItemHierarchical | kThemeMenuItemHierBackground;
        else
            mdi.itemType |= kThemeMenuItemPopUpBackground;
        mdi.state = (disabled) ? kThemeMenuDisabled : kThemeMenuActive;
        if (itemSelected)
            mdi.state |= kThemeMenuSelected;
        HIRect contentRect;
        if (mi && mi->isSeparator()) {
            // ### Something strange is happening here.
            HIThemeDrawMenuSeparator(&menuRect, &itemRect, &mdi,
                                     static_cast<CGContextRef>(p->handle()),
                                     kHIThemeOrientationNormal);
            break;
        } else {
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi, static_cast<CGContextRef>(p->handle()),
                                kHIThemeOrientationNormal, &contentRect);
        }
        bool checkable = popup->isCheckable();
        bool itemChecked = mi->isChecked();
        bool reverse = QApplication::reverseLayout();
        int maxIW = opt.maxIconWidth();
        if (checkable)
            maxIW += 12;
        QRect qtContentRect(QPoint((int)contentRect.origin.x, (int)contentRect.origin.y),
                            QSize((int)contentRect.size.width, (int)contentRect.size.height));
        int xpos = qtContentRect.x() + 2;
        if (reverse)
            xpos = qtContentRect.width() - maxIW - 2;
        // Draw checks
        if (checkable && itemChecked) {
            int mw = maxIW;
            int mh = qtContentRect.height();
            int xp = xpos;
            SFlags myflags = Style_Default;
            if (!disabled)
                myflags |= Style_Enabled;
            if (itemSelected)
                myflags |= Style_On;
            // ### Need to implement PE_CheckMark, commonstyle doesn't look correct for Mac OS X.
            // It should also change color when it is selected.
            drawPrimitive(PE_CheckMark, p, QRect(xp, qtContentRect.y(), mw, mh), pal, myflags);
        }
        if (disabled)
            p->setPen(pal.text());
        else if (itemSelected)
            p->setPen(pal.highlightedText());
        else
            p->setPen(pal.buttonText());
        // All Mac popups appear to have a space for a check.
        xpos += maxIW + 5;
        // Draw text
        if (mi) {
            QString str = mi->text();
            if (!str.isEmpty()) {
                int text_flags = AlignVCenter | NoAccel | DontClip | SingleLine;
                p->drawText(xpos, qtContentRect.y(), qtContentRect.width(), qtContentRect.height(),
                            text_flags, str);
            }
        }
        break; }
    default:
	QWindowsStyle::drawControl(element, p, widget, r, pal, how, opt);
    }
}

void QMacStyleCG::drawComplexControl(ComplexControl control, QPainter *p, const QWidget *w,
				     const QRect &r, const QPalette& pal, SFlags flags, SCFlags sub,
				     SCFlags subActive, const QStyleOption &opt) const
{
    switch (control) {
    case CC_Slider: {
        const QSlider *slider = static_cast<const QSlider *>(w);
        bool tracking = slider->hasTracking();
        HIThemeTrackDrawInfo tdi;
        tdi.version = qt_mac_hitheme_version;
        tdi.kind = qt_aqua_size_constrain(w) == QAquaSizeSmall ? kThemeSmallSlider
                                                              : kThemeMediumSlider;
        //tdi.kind = kThemeSlider;
        tdi.bounds = *qt_glb_mac_rect(r, p);
        tdi.min = slider->minimum();
        tdi.max = slider->maximum();
        tdi.value = slider->sliderPosition();
        tdi.reserved = 0;
        tdi.attributes = kThemeTrackShowThumb;
        if (qMacVersion() >= Qt::MV_JAGUAR) {
            if (flags & Style_HasFocus)
                tdi.attributes |= kThemeTrackHasFocus;
        }
	if (slider->orientation() == Qt::Horizontal)
	    tdi.attributes |= kThemeTrackHorizontal;
	tdi.enableState = slider->isEnabled() ? kThemeTrackActive : kThemeTrackDisabled;
	if (slider->tickmarks() == QSlider::NoMarks || slider->tickmarks() == QSlider::Both)
	    tdi.trackInfo.slider.thumbDir = kThemeThumbPlain;
	else if (slider->tickmarks() == QSlider::Above)
	    tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;;
	if (subActive == SC_SliderGroove)
	    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
	else if (subActive == SC_SliderHandle)
	    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
        HIRect macRect;
        if (!tracking) {
            HIShapeRef shape;
            HIThemeGetTrackThumbShape(&tdi, &shape);
            HIShapeGetBounds(shape, &macRect);
            CFRelease(shape);
	    tdi.value = slider->value();
	}
        HIThemeDrawTrack(&tdi, tracking ? 0 : &macRect, static_cast<CGContextRef>(p->handle()),
                         kHIThemeOrientationNormal);
        if (sub && SC_SliderTickmarks) {
            int numMarks = slider->maximum() / slider->pageStep();
            HIThemeDrawTrackTickMarks(&tdi, numMarks, static_cast<CGContextRef>(p->handle()),
                                      kHIThemeOrientationNormal);
        }
        break; }
    default:
        QWindowsStyle::drawComplexControl(control, p, w, r, pal, flags, sub, subActive, opt);
    }
}

int QMacStyleCG::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret;
    switch(metric) {
    case PM_IndicatorWidth: {
	ThemeMetric tm = kThemeMetricCheckBoxWidth;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallCheckBoxWidth;
	GetThemeMetric(tm, &ret);
	break; }
    case PM_IndicatorHeight: {
	ThemeMetric tm = kThemeMetricCheckBoxHeight;
	if(qt_aqua_size_constrain(widget) == QAquaSizeSmall)
	    tm = kThemeMetricSmallCheckBoxHeight;
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
    case PM_MenuButtonIndicator:
        GetThemeMetric(kThemeMetricDisclosureTriangleWidth, &ret);
        break;
    case PM_ButtonShiftHorizontal:
    case PM_ButtonShiftVertical:
        ret = 0;
        break;
    case PM_SliderLength:
	ret = 17;
	break;
    default:
	ret = QWindowsStyle::pixelMetric(metric, widget);
	break;
    }
    return ret;
}


QRect QMacStyleCG::querySubControlMetrics(ComplexControl control, const QWidget *widget,
					  SubControl sc, const QStyleOption &opt) const
{
    QRect rect;
    switch (control) {
    case CC_Slider: {
        const QSlider *slider = static_cast<const QSlider *>(widget);
        HIThemeTrackDrawInfo tdi;
        tdi.version = qt_mac_hitheme_version;
        tdi.kind = kThemeSlider;
        tdi.bounds = *qt_glb_mac_rect(widget->rect());
        tdi.min = slider->minimum();
        tdi.max = slider->maximum();
	tdi.value = slider->sliderPosition();
        tdi.attributes = kThemeTrackShowThumb;
        if (qMacVersion() >= Qt::MV_JAGUAR && slider->hasFocus())
            tdi.attributes |= kThemeTrackHasFocus;
        if (slider->orientation() == Qt::Horizontal)
            tdi.attributes |= kThemeTrackHorizontal;
        tdi.enableState = slider->isEnabled() ? kThemeTrackActive : kThemeTrackDisabled;
        if (slider->tickmarks() == QSlider::Above)
            tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
        if (slider->tickmarks() == QSlider::Below)
            tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
        if (slider->tickmarks() == QSlider::NoMarks)
            tdi.trackInfo.slider.thumbDir = kThemeThumbPlain;

        HIRect macRect;
        switch (sc) {
        case SC_SliderGroove:
            HIThemeGetTrackBounds(&tdi, &macRect);
            rect = QRect(QPoint((int)macRect.origin.x, (int)macRect.origin.y),
                         QSize((int)macRect.size.width, (int)macRect.size.height));
            break;
        case SC_SliderHandle:
            HIShapeRef shape;
            HIThemeGetTrackThumbShape(&tdi, &shape);
            HIShapeGetBounds(shape, &macRect);
            rect = QRect(QPoint((int)macRect.origin.x, (int)macRect.origin.y),
                         QSize((int)macRect.size.width, (int)macRect.size.height));
            CFRelease(shape);
            break;
        }

        break; }
    default:
        rect = QWindowsStyle::querySubControlMetrics(control, widget, sc, opt);
    }
    return rect;
}

QRect QMacStyleCG::subRect(SubRect sr, const QWidget *widget) const
{
    QRect subrect;
    switch (sr) {
        case SR_PushButtonContents: {
            CGRect inRect = CGRectMake(0, 0, widget->width(), widget->height());
            CGRect outRect;
            HIThemeButtonDrawInfo info;
            info.version = qt_mac_hitheme_version;
            info.state = kThemeStateActive;
            info.kind = kThemePushButton;
            info.adornment = kThemeAdornmentNone;
            HIThemeGetButtonContentBounds(&inRect, &info, &outRect);
            subrect = QRect(QPoint((int)outRect.origin.x, (int)outRect.origin.y),
                            QSize((int)outRect.size.width, (int)outRect.size.height));
            break;
        }
        default:
            subrect = QWindowsStyle::subRect(sr, widget);
    }
    return subrect;
}

QStyle::SubControl QMacStyleCG::querySubControl(ComplexControl control, const QWidget *widget,
						const QPoint &pos, const QStyleOption &opt) const
{
    return QWindowsStyle::querySubControl(control, widget, pos, opt);
}

int QMacStyleCG::styleHint(StyleHint sh, const QWidget *widget, const QStyleOption &opt,
		       QStyleHintReturn *ret) const
{
    int returnMe;
    switch (sh) {
    case SH_UnderlineAccelerator:
        returnMe = false;
        break;
    default:
        returnMe = QWindowsStyle::styleHint(sh, widget, opt, ret);
    }
    return returnMe;
}

QSize QMacStyleCG::sizeFromContents(ContentsType contents, const QWidget *w,
				    const QSize &contentsSize, const QStyleOption &opt) const
{
    QSize sz;
    switch (contents) {
    default:
        sz = QWindowsStyle::sizeFromContents(contents, w, contentsSize, opt);
    }
    return sz;
}

QPixmap QMacStyleCG::stylePixmap(StylePixmap sp, const QWidget *widget,
				 const QStyleOption &opt) const
{
    IconRef icon = 0;
    switch (sp) {
        case SP_MessageBoxQuestion:
        case SP_MessageBoxInformation:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertNoteIcon, &icon);
            break;
        case SP_MessageBoxWarning:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertCautionIcon, &icon);
            break;
        case SP_MessageBoxCritical:
            GetIconRef(kOnSystemDisk, kSystemIconsCreator, kAlertStopIcon, &icon);
            break;
        default:
            break;
    }
    if (icon) {
	QPixmap ret = qt_mac_convert_iconref(icon, 64, 64);
	ReleaseIconRef(icon);
	return ret;
    }
    return QWindowsStyle::stylePixmap(sp, widget, opt);
}

QPixmap QMacStyleCG::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
				 const QPalette &pal, const QStyleOption &opt) const
{
    return QCommonStyle::stylePixmap(pixmaptype, pixmap, pal, opt);
}

bool QMacStyleCG::eventFilter(QObject *o, QEvent *e)
{
    QPushButton *btn = ::qt_cast<QPushButton *>(o);
    if (btn) {
        switch (e->type()) {
        default:
            break;
        case QEvent::FocusIn:
            if (btn->autoDefault())
                d->animateButton(btn);
            break;
        case QEvent::Hide:
            if (btn == d->defaultButton)
                d->stopButtonAnimation();
            break;
        case QEvent::MouseButtonPress:
            // From looking at Panther it seems that pulsing stops whenever the mouse
            // is pressed, most people will probably be happy enough if we do it
            // just for buttons.           
            d->stopButtonAnimation();
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::FocusOut:
        case QEvent::Show: {
            // Find the correct button and animate it.
            QObjectList list = btn->topLevelWidget()->queryList("QPushButton");
            for (int i = 0; i < list.size(); ++i) {
                QPushButton *pBtn = static_cast<QPushButton *>(list.at(i));
                if ((e->type() == QEvent::FocusOut 
                     && (pBtn->isDefault() || (pBtn->autoDefault() && pBtn->hasFocus()))
                     && pBtn != btn)
                    || ((e->type() == QEvent::Show || e->type() == QEvent::MouseButtonRelease)
                        && pBtn->isDefault())) {
                    if (pBtn->topLevelWidget()->isActiveWindow())
                        d->animateButton(pBtn);
                    break;
                }
            }
            break; }
        }
    }
    return QWindowsStyle::eventFilter(o, e);
}

#endif
