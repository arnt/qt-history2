#include <qmacstyle_mac.h>
#include <qmacstylecg_mac.h>

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)

#include <private/qt_mac_p.h>
#include <private/qtitlebar_p.h>
#include <qapplication.h>
#include <qaquastyle_mac.h>
#include <qbitmap.h>
#include <qcombobox.h>
#include <qevent.h>
#include <qgroupbox.h>
#include <qhash.h>
#include <qmenu.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpointer.h>
#include <qpushbutton.h>
#include <qrubberband.h>
#include <qstyleoption.h>
#include <qtabbar.h>
#include <qviewport.h>

/*****************************************************************************
  External functions
 *****************************************************************************/
extern CGContextRef qt_macCreateCGHandle(const QPaintDevice *); //qpaintdevice_mac.cpp
extern QPixmap qt_mac_convert_iconref(IconRef, int, int); //qpixmap_mac.cpp

/*****************************************************************************
  QMacCGStyle globals
 *****************************************************************************/
const int qt_mac_hitheme_version = 0; //the HITheme version we speak
const int macSpinBoxSep        = 5;    // distance between spinwidget and the lineedit
const int macItemFrame         = 2;    // menu item frame width
const int macItemHMargin       = 3;    // menu item hor text margin
const int macItemVMargin       = 2;    // menu item ver text margin
const int macRightBorder       = 12;   // right border on mac
const ThemeWindowType QtWinType = kThemeUtilityWindow; // Window type we use for QTitleBar.

/*****************************************************************************
  QMacCGStyle utility functions
 *****************************************************************************/
static inline HIRect qt_hirectForQRect(const QRect &convertRect, QPainter *p = 0,
                                       bool useOffset = true, const QRect &rect = QRect())
{
    int x, y;
    int offset = 0;
    if (useOffset) {
        if(QRect::rectangleMode() == QRect::InclusiveRectangles)
            offset = 1;
        else
            offset = 2;
    }
    if (p) {
        p->map(convertRect.x(), convertRect.y(), &x, &y);
    } else {
        x = convertRect.x();
        y = convertRect.y();
    }
    HIRect retRect = CGRectMake(x + rect.x(), y + rect.y(), convertRect.width() - offset - rect.width(),
                                convertRect.height() - offset - rect.height());
    return retRect;
}

static inline QWidget *qt_abuse_painter_for_widget(QPainter *p)
{
    QWidget *w = 0;
    if (p && p->device()->devType() == QInternal::Widget)
        w = static_cast<QWidget *>(p->device());
    return w;
}

static inline const QRect qrectForHIRect(const HIRect &hirect)
{
    return QRect(QPoint(int(hirect.origin.x), int(hirect.origin.y)),
                 QSize(int(hirect.size.width), int(hirect.size.height)));
}

static inline bool qt_mac_is_metal(const QWidget *w)
{
    for (; w; w = w->parentWidget()) {
        if (w->testAttribute(Qt::WA_MacMetalStyle))
            return true;
        if (w->isTopLevel())
            break;
    }
    return false;
}

static inline bool qt_mac_is_metal(QPainter *p)
{
    return qt_mac_is_metal(qt_abuse_painter_for_widget(p));
}

static void getSliderInfo(QStyle::ComplexControl cc, const QStyleOptionSlider *slider,
                          HIThemeTrackDrawInfo *tdi, const QWidget *needToRemoveMe)
{
    memset(tdi, 0, sizeof(HIThemeTrackDrawInfo)); // We don't get it all for some reason or another...
    tdi->version = qt_mac_hitheme_version;
    tdi->reserved = 0;
    bool isScrollbar = (cc == QStyle::CC_ScrollBar);
    switch (qt_aqua_size_constrain(needToRemoveMe)) {
    case QAquaSizeUnknown:
    case QAquaSizeLarge:
        if (isScrollbar)
            tdi->kind = kThemeMediumScrollBar;
        else
            tdi->kind = kThemeMediumSlider;
        break;
    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
            if (isScrollbar)
                tdi->kind = kThemeMiniScrollBar;
            else
                tdi->kind = kThemeMiniSlider;
            break;
        }
#endif
    case QAquaSizeSmall:
        if (isScrollbar)
            tdi->kind = kThemeSmallScrollBar;
        else
            tdi->kind = kThemeSmallSlider;
        break;
    }
    tdi->bounds = qt_hirectForQRect(slider->rect);
    tdi->min = slider->minimum;
    tdi->max = slider->maximum;
    tdi->value = slider->sliderPosition;
    tdi->attributes = kThemeTrackShowThumb;
    if (slider->orientation == Qt::Horizontal)
        tdi->attributes |= kThemeTrackHorizontal;
    if (slider->useRightToLeft)
        tdi->attributes |= kThemeTrackRightToLeft;
    tdi->enableState = slider->state & QStyle::Style_Enabled ? kThemeTrackActive
                                                             : kThemeTrackDisabled;
    if(!qAquaActive(slider->palette))
        tdi->enableState = kThemeTrackInactive;
    if (!isScrollbar) {
        if (slider->state & QStyle::Style_HasFocus)
            tdi->attributes |= kThemeTrackHasFocus;
        if (slider->tickmarks == QSlider::NoMarks || slider->tickmarks == QSlider::Both)
            tdi->trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickmarks == QSlider::Above)
            tdi->trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi->trackInfo.slider.thumbDir = kThemeThumbDownward;
    }
}

/*****************************************************************************
  QMacCGStyle member functions
 *****************************************************************************/

class QMacStyleCGFocusWidget : public QAquaFocusWidget
{
public:
    QMacStyleCGFocusWidget(QWidget *w) : QAquaFocusWidget(false, w) { }

protected:
    void drawFocusRect(QPainter *p) const;

    virtual QRegion focusRegion() const;
    virtual void paintEvent(QPaintEvent *);
};
QRegion QMacStyleCGFocusWidget::focusRegion() const
{
    const QRgb fillColor = qRgb(192, 191, 190);
    QImage img;
    {
        QPixmap pix(size(), 32);
        pix.fill(fillColor);
        QPainter p(&pix);
        drawFocusRect(&p);
        img = pix;
    }
    QImage mask(img.width(), img.height(), 1, 2, QImage::LittleEndian);
    for(int y = 0; y < img.height(); y++) {
        for(int x = 0; x < img.width(); x++) {
            QRgb clr = img.pixel(x, y);
            int diff = (((qRed(clr)-qRed(fillColor))*((qRed(clr)-qRed(fillColor)))) +
                        ((qGreen(clr)-qGreen(fillColor))*((qGreen(clr)-qGreen(fillColor)))) +
                        ((qBlue(clr)-qBlue(fillColor))*((qBlue(clr)-qBlue(fillColor)))));
            mask.setPixel(x, y, diff < 100);
        }
    }
    QBitmap qmask;
    qmask = mask;
    return QRegion(qmask);
}
void QMacStyleCGFocusWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawFocusRect(&p);
}
void QMacStyleCGFocusWidget::drawFocusRect(QPainter *p) const
{
    int fo = focusOutset();
    HIRect rect = CGRectMake(fo, fo, width() - 2 * fo, height() - 2 * fo);
    HIThemeDrawFocusRect(&rect, true, QMacCGContext(p), kHIThemeOrientationNormal);
}

class QMacStyleCGPrivate : public QAquaAnimate
{
    QPointer<QMacStyleCGFocusWidget> focusWidget;
public:
    UInt8 progressFrame;
    CFAbsoluteTime defaultButtonStart;
    QMacStyleCGPrivate() : progressFrame(0) { defaultButtonStart = CFAbsoluteTimeGetCurrent(); }

protected:
    friend class QMacStyleCG;
    inline int animateSpeed(Animates) { return 33; }   // Gives us about 30 frames a second.
    inline bool doAnimate(Animates anim) {
        if(anim == AquaProgressBar)
            ++progressFrame;
        return true;
    }
    virtual void doFocus(QWidget *w) {
        if(!focusWidget)
            focusWidget = new QMacStyleCGFocusWidget(w);
        focusWidget->setFocusWidget(w);
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
    d->addWidget(w);
    QPixmap px(0, 0, 32);
    if (qt_mac_is_metal(w)) {
        px.resize(200, 200);
        HIThemeBackgroundDrawInfo bginfo;
        bginfo.version = qt_mac_hitheme_version;
        bginfo.state = kThemeStateActive;
        bginfo.kind = kThemeBackgroundMetal;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        CGContextRef context = qt_macCreateCGHandle(&px);
        HIThemeDrawBackground(&rect, &bginfo, context, kHIThemeOrientationNormal);
        CGContextRelease(context);
    }

    if (::qt_cast<QMenu*>(w)) {
        px.resize(200, 200);
        HIThemeMenuDrawInfo mtinfo;
        mtinfo.version = qt_mac_hitheme_version;
        mtinfo.menuType = kThemeMenuTypePopUp;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        CGContextRef context = qt_macCreateCGHandle(&px);
        HIThemeDrawMenuBackground(&rect, &mtinfo, context, kHIThemeOrientationNormal);
        CGContextRelease(context);
        w->setWindowOpacity(0.95);
    }
    if (!px.isNull()) {
        QPalette pal = w->palette();
        QBrush background(px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
    }

    if (::qt_cast<QRubberBand*>(w))
        w->setWindowOpacity(0.25);

    if (QTitleBar *tb = ::qt_cast<QTitleBar *>(w))
        tb->setAutoRaise(true);
    QWindowsStyle::polish(w);
}

void QMacStyleCG::unPolish(QWidget *w)
{
    d->removeWidget(w);
    if (::qt_cast<QMenu*>(w) || qt_mac_is_metal(w)) {
        QPalette pal = w->palette();
        QPixmap tmp;
        QBrush background(tmp);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
        w->setWindowOpacity(1.0);
    }
    if (::qt_cast<QRubberBand*>(w))
        w->setWindowOpacity(1.0);
    QWindowsStyle::unPolish(w);
}

void QMacStyleCG::polish(QApplication *app)
{
    QPalette pal = app->palette();
    QPixmap px(200, 200, 32);
    QPainter p(&px);

    HIThemeMenuDrawInfo mtinfo;
    mtinfo.version = qt_mac_hitheme_version;
    mtinfo.menuType = kThemeMenuTypeHierarchical;
    HIRect rect = CGRectMake(0, 0, px.width(), px.height());
    HIThemeDrawMenuBackground(&rect, &mtinfo, static_cast<CGContextRef>(px.handle()),
                              kHIThemeOrientationNormal);

    p.end();
    QBrush background(px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
    app->setPalette(pal);
}

int QMacStyleCG::pixelMetric(PixelMetric metric, const QWidget *widget) const
{
    SInt32 ret;
    switch(metric) {
    case PM_IndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        break; }
    case PM_IndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxHeight, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniCheckBoxHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxHeight, &ret);
            break;
        }
        break; }
    case PM_ExclusiveIndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonHeight, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniRadioButtonHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonHeight, &ret);
            break;
        }
        break; }
    case PM_ExclusiveIndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniRadioButtonWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonWidth, &ret);
            break;
        }
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
    case PM_ScrollBarExtent: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricScrollBarWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniScrollBarWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallScrollBarWidth, &ret);
            break;
        }
        break; }
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        ret = 0;
        break;
    case PM_TabBarTabVSpace:
        ret = 4;
        break;
    case PM_TabBarBaseHeight:
        ret = 8;
        break;
    case PM_TabBarTabOverlap:
        GetThemeMetric(kThemeMetricTabOverlap, &ret);
        break;
    case PM_TabBarBaseOverlap:
        GetThemeMetric(kThemeMetricTabFrameOverlap, &ret);
        break;
    case PM_SpinBoxFrameWidth:
        GetThemeMetric(kThemeMetricEditTextFrameOutset, &ret);
        ret += 2;
        break;
    case PM_MenuDesktopFrameWidth:
        ret = 15;
        break;
    case PM_MenuScrollerHeight:
        ret = 15;
        break;
    case PM_MenuVMargin:
        ret = 4;
        break;
    case PM_MenuFrameWidth:
        ret = 0;
        break;
    case PM_TitleBarHeight: {
        if (!widget)
            break;
        const QTitleBar *titlebar = static_cast<const QTitleBar *>(widget);
        HIThemeWindowDrawInfo wdi;
        wdi.version = qt_mac_hitheme_version;
        wdi.state = kThemeStateActive;
        wdi.windowType = QtWinType;
        if (titlebar->window()) {
            if (titlebar->window()->isMinimized())
                wdi.attributes |= kThemeWindowIsCollapsed;
            if (titlebar->window()->isWindowModified())
                wdi.attributes |= kThemeWindowHasDirty;
            wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                | kThemeWindowHasCollapseBox;
        } else if (titlebar->testWFlags(Qt::WStyle_SysMenu)) {
            wdi.attributes |= kThemeWindowHasCloseBox;
        }
        wdi.titleHeight = titlebar->height();
        wdi.titleWidth = titlebar->width();
        QCFType<HIShapeRef> region;
        HIRect hirect = qt_hirectForQRect(titlebar->rect());
        HIThemeGetWindowShape(&hirect, &wdi, kWindowTitleBarRgn, &region);
        HIRect rect;
        HIShapeGetBounds(region, &rect);
        ret = int(rect.size.height);
         break; }
    default:
        ret = QWindowsStyle::pixelMetric(metric, widget);
        break;
    }
    return ret;
}


int QMacStyleCG::styleHint(StyleHint sh, const QWidget *widget, const Q3StyleOption &opt,
                           QStyleHintReturn *d) const
{
    SInt32 ret = 0;
    switch (sh) {
    case SH_Menu_AllowActiveAndDisabled:
        ret = false;
        break;
    case SH_Menu_SubMenuPopupDelay:
        ret = 100;
        break;
    case SH_ScrollBar_LeftClickAbsolutePosition:
        extern bool qt_scrollbar_jump_to_pos; //qapplication_mac.cpp
        ret = qt_scrollbar_jump_to_pos;
        break;
    case SH_TabBar_PreferNoArrows:
        ret = true;
        break;
    case SH_LineEdit_PasswordCharacter:
        ret = kBulletUnicode;
        break;
    case SH_GroupBox_TextLabelColor:
        ret = (int) (widget ? widget->palette().foreground().color().rgb() : 0);
        break;
    case SH_Menu_SloppySubMenus:
        ret = true;
        break;
    case SH_GroupBox_TextLabelVerticalAlignment:
        ret = Qt::AlignTop;
        break;
    case SH_ScrollView_FrameOnlyAroundContents:
        if (widget
            && (widget->isTopLevel() || !widget->parentWidget()
                || widget->parentWidget()->isTopLevel())
            && (qt_cast<QViewport *>(widget)
#ifdef QT_COMPAT
                || widget->inherits("QScrollView")
#endif
                || widget->inherits("QWorkspaceChild")))
            ret = true;
        else
            ret = QWindowsStyle::styleHint(sh, widget, opt, d);
        break;
    case SH_Menu_FillScreenWithScroll:
        ret = (QSysInfo::MacintoshVersion < QSysInfo::MV_PANTHER);
        break;
    case SH_Menu_Scrollable:
        ret = true;
        break;
    case SH_RichText_FullWidthSelection:
        ret = true;
        break;
    case SH_BlinkCursorWhenTextSelected:
        ret = false;
        break;
    case SH_ScrollBar_StopMouseOverSlider:
        ret = true;
        break;
    case SH_ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
        break;
    case SH_ComboBox_Popup:
        ret = (!widget || !static_cast<const QComboBox *>(widget)->isEditable());
        break;
    case SH_Workspace_FillSpaceOnMaximize:
        ret = true;
        break;
    case SH_Widget_ShareActivation:
        ret = true;
        break;
    case SH_Header_ArrowAlignment:
        ret = Qt::AlignRight;
        break;
    case SH_TabBar_Alignment:
        ret = Qt::AlignHCenter;
        break;
    case SH_UnderlineShortcut:
        ret = false;
        break;
    case SH_TipLabel_Opacity:
        ret = 242; // About 95%
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, widget, opt, d);
        break;
    }
    return ret;
}

/*! \reimp */
QPixmap QMacStyleCG::stylePixmap(PixmapType pixmaptype, const QPixmap &pixmap,
                               const QPalette &pal, const Q3StyleOption &opt) const
{
    QPixmap px = pixmap;
    switch (pixmaptype) {
    case PT_Disabled: {
        QImage img = px;
        img.setAlphaBuffer(true);
        QRgb pixel;
        int imgh = img.height();
        int imgw = img.width();
        for (int y = 0; y < imgh; ++y) {
            for (int x = 0; x < imgw; ++x) {
                pixel = img.pixel(x, y);
                img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel),
                                         qAlpha(pixel) / 2));
            }
        }
        px = img;
        break; }
    case PT_Active: {
        QImage img = px;
        img.setAlphaBuffer(true);
        int imgh = img.height();
        int imgw = img.width();
        int h, s, v, a;
        QRgb pixel;
        for (int y = 0; y < imgh; ++y) {
            for (int x = 0; x < imgw; ++x) {
                pixel = img.pixel(x, y);
                a = qAlpha(pixel);
                QColor hsvColor(pixel);
                hsvColor.getHsv(&h, &s, &v);
                s = qMin(100, s * 2);
                v = v / 2;
                hsvColor.setHsv(h, s, v);
                pixel = hsvColor.rgb();
                img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel), a));
            }
        }
        px = img;
        break; }
    default:
        px = QCommonStyle::stylePixmap(pixmaptype, pixmap, pal, opt);
    }
    return px;
}

/*! \reimp */
QPixmap QMacStyleCG::stylePixmap(StylePixmap stylepixmap, const QWidget *widget,
                                 const Q3StyleOption &opt) const
{
    IconRef icon = 0;
    switch (stylepixmap) {
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
    return QWindowsStyle::stylePixmap(stylepixmap, widget, opt);
}

void QMacStyleCG::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                                const QWidget *w) const
{
    ThemeDrawState tds = d->getDrawState(opt->state, opt->palette);
    QMacCGContext cg(p);
    switch (pe) {
    case PE_CheckListExclusiveIndicator:
    case PE_ExclusiveIndicatorMask:
    case PE_ExclusiveIndicator:
    case PE_CheckListIndicator:
    case PE_IndicatorMask:
    case PE_Indicator:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = tds;
            bdi.adornment = kThemeDrawIndicatorOnly;
            if (btn->state & Style_HasFocus
                    && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment |= kThemeAdornmentFocus;
            bool isRadioButton = (pe == PE_CheckListExclusiveIndicator
                                  || pe == PE_ExclusiveIndicatorMask
                                  || pe == PE_ExclusiveIndicator);
            switch (qt_aqua_size_constrain(w)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                if (isRadioButton)
                    bdi.kind = kThemeRadioButton;
                else
                    bdi.kind = kThemeCheckBox;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                    if (isRadioButton)
                        bdi.kind = kThemeMiniRadioButton;
                    else
                        bdi.kind = kThemeMiniCheckBox;
                    break;
                }
#endif
            case QAquaSizeSmall:
                if (isRadioButton)
                    bdi.kind = kThemeSmallRadioButton;
                else
                    bdi.kind = kThemeSmallCheckBox;
                break;
            }
            if (btn->state & Style_NoChange)
                bdi.value = kThemeButtonMixed;
            else if (btn->state & Style_On)
                bdi.value = kThemeButtonOn;
            else
                bdi.value = kThemeButtonOff;
            HIRect macRect = qt_hirectForQRect(btn->rect, p);
            if (pe == PE_IndicatorMask || pe == PE_ExclusiveIndicatorMask) {
                QRegion saveRegion = p->clipRegion();
                QCFType<HIShapeRef> macRegion;
                HIThemeGetButtonShape(&macRect, &bdi, &macRegion);
                p->setClipRegion(qt_mac_convert_mac_region(macRegion));
                p->fillRect(btn->rect, Qt::color1);
                p->setClipRegion(saveRegion);
            } else {
                HIThemeDrawButton(&macRect, &bdi, cg, kHIThemeOrientationNormal, 0);
            }
        }
        break;
    case PE_ArrowUp:
    case PE_ArrowDown:
    case PE_ArrowRight:
    case PE_ArrowLeft: {
        HIThemePopupArrowDrawInfo pdi;
        pdi.version = qt_mac_hitheme_version;
        pdi.state = tds;
        switch (pe) {
        case PE_ArrowUp:
            pdi.orientation = kThemeArrowUp;
            break;
        case PE_ArrowDown:
            pdi.orientation = kThemeArrowDown;
            break;
        case PE_ArrowRight:
            pdi.orientation = kThemeArrowRight;
            break;
        case PE_ArrowLeft:
            pdi.orientation = kThemeArrowLeft;
            break;
        default:     // Stupid compiler _should_ know better.
            break;
        }
        if (opt->rect.width() < 8)
            pdi.size = kThemeArrow5pt;
        else
            pdi.size = kThemeArrow9pt;
        HIRect macRect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawPopupArrow(&macRect, &pdi, cg, kHIThemeOrientationNormal);
        break; }
    case PE_FocusRect:
        // Use the our own focus widget stuff.
        break;
    case PE_Splitter: {
        HIThemeSplitterDrawInfo sdi;
        sdi.version = qt_mac_hitheme_version;
        sdi.state = tds;
        sdi.adornment = qt_mac_is_metal(w) ? kHIThemeSplitterAdornmentMetal
                                           : kHIThemeSplitterAdornmentNone;
        HIRect hirect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawPaneSplitter(&hirect, &sdi, cg, kHIThemeOrientationNormal);
        break; }
    case PE_TreeBranch: {
        if (!(opt->state & Style_Children))
            break;
        HIThemeButtonDrawInfo bi;
        bi.version = qt_mac_hitheme_version;
        bi.state = opt->state & Style_Enabled ?  kThemeStateActive : kThemeStateInactive;
        if (opt->state & Style_Down)
            bi.state |= kThemeStatePressed;
        bi.kind = kThemeDisclosureButton;
        bi.value = opt->state & Style_Open ? kThemeDisclosureDown : kThemeDisclosureRight;
        bi.adornment = kThemeAdornmentNone;
        HIRect hirect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawButton(&hirect, &bi, cg, kHIThemeOrientationNormal, 0);
        break; }
    case PE_RubberBandMask:
        p->fillRect(opt->rect, Qt::color1);
        break;
    case PE_RubberBand: {
        p->fillRect(opt->rect, opt->palette.brush(QPalette::Disabled, QPalette::Highlight));
        break; }
    case PE_SizeGrip: {
        HIThemeGrowBoxDrawInfo gdi;
        gdi.version = qt_mac_hitheme_version;
        gdi.state = tds;
        gdi.kind = kHIThemeGrowBoxKindNormal;
        gdi.direction = kThemeGrowLeft | kThemeGrowDown;
        switch (qt_aqua_size_constrain(w)) {
            case QAquaSizeLarge:
            case QAquaSizeUnknown:
                gdi.size = kHIThemeGrowBoxSizeNormal;
                break;
            case QAquaSizeSmall:
            case QAquaSizeMini:
                gdi.size = kHIThemeGrowBoxSizeSmall;
                break;
        }
        HIPoint pt = { opt->rect.x(), opt->rect.y() };
        HIThemeDrawGrowBox(&pt, &gdi, cg,
                           kHIThemeOrientationNormal);
        break; }
    case PE_HeaderArrow:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            if (w && w->inherits("QTable"))
                drawPrimitive(header->state & Style_Up ? PE_ArrowUp : PE_ArrowDown, header, p, w);
            // ListView header is taken care of.
        }
        break;
    case PE_HeaderSection:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = tds;
            SFlags flags = header->state;
            if (w && w->parentWidget()->inherits("QTable")) {
                bdi.kind = kThemeBevelButton;
                if (p->font().bold())
                    flags |= Style_Sunken;
                else
                    flags &= ~Style_Sunken;
            } else {
                bdi.kind = kThemeListHeaderButton;
            }
            if (flags & Style_Sunken)
                bdi.value = kThemeButtonOn;
            else
                bdi.value = kThemeButtonOff;

            bdi.adornment = kThemeAdornmentNone;

            QRect ir = header->rect;
            if (flags & Style_Off)
                ir.setRight(ir.right() + 50);  // Cheat to hide the down indicator.
            else if (flags & Style_Up)
                bdi.adornment = kThemeAdornmentHeaderButtonSortUp;

            if (flags & Style_HasFocus && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment = kThemeAdornmentFocus;
            HIRect hirect = qt_hirectForQRect(ir, p);
            HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
        }
        break;
    case PE_PanelGroupBox:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            HIThemeGroupBoxDrawInfo gdi;
            gdi.version = qt_mac_hitheme_version;
            gdi.state = tds;
            if (w && ::qt_cast<QGroupBox *>(w->parentWidget()))
                gdi.kind = kHIThemeGroupBoxKindSecondary;
            else
                gdi.kind = kHIThemeGroupBoxKindPrimary;
            HIRect hirect = qt_hirectForQRect(frame->rect, p);
            HIThemeDrawGroupBox(&hirect, &gdi, cg, kHIThemeOrientationNormal);
        }
        break;
    case PE_Panel:
    case PE_PanelLineEdit:
        if (const QStyleOptionFrame *frame = qt_cast<const QStyleOptionFrame *>(opt)) {
            if (frame->state & Style_Sunken) {
                HIThemeFrameDrawInfo fdi;
                fdi.version = qt_mac_hitheme_version;
                fdi.state = tds;
                SInt32 frame_size;
                if (pe == PE_PanelLineEdit) {
                    fdi.kind = kHIThemeFrameTextFieldSquare;
                    GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
                } else {
                    fdi.kind = kHIThemeFrameListBox;
                    GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
                }
                int lw = frame->lineWidth;
                if (lw <= 0)
                    lw = pixelMetric(PM_DefaultFrameWidth, 0);
                p->fillRect(frame->rect.x(), frame->rect.y(), lw, frame->rect.height(),
                            frame->palette.background());
                p->fillRect(frame->rect.right() - lw + 1, frame->rect.y(), lw, frame->rect.height(),
                            frame->palette.background());
                p->fillRect(frame->rect.x(), frame->rect.y(), frame->rect.width(), lw,
                            frame->palette.background());
                p->fillRect(frame->rect.x(), frame->rect.bottom() - lw + 1, frame->rect.width(), lw,
                            frame->palette.background());
                HIRect hirect = qt_hirectForQRect(frame->rect, p, false,
                                                  QRect(frame_size, frame_size,
                                                        frame_size * 2, frame_size * 2));

                HIThemeDrawFrame(&hirect, &fdi, cg, kHIThemeOrientationNormal);
                break;
            } else {
                QWindowsStyle::drawPrimitive(pe, opt, p, w);
            }
        }
    case PE_TabBarBase: {
        HIThemeTabPaneDrawInfo tpdi;
        tpdi.version = 0;
        tpdi.state = tds;
        tpdi.direction = kThemeTabNorth;
        if (opt->state & Style_Bottom)
            tpdi.direction = kThemeTabSouth;
        tpdi.size = kHIThemeTabSizeNormal;
        HIRect hirect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawTabPane(&hirect, &tpdi, cg, kHIThemeOrientationNormal);
        break; }
    default:
        QWindowsStyle::drawPrimitive(pe, opt, p, w);
    }
}

void QMacStyleCG::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                              const QWidget *w) const
{
    ThemeDrawState tds = d->getDrawState(opt->state, opt->palette);
    QMacCGContext cg(p);
    switch (ce) {
    case CE_PushButton:
        if (const QStyleOptionButton *btn = ::qt_cast<const QStyleOptionButton *>(opt)) {
            if (!(btn->state & (Style_Raised | Style_Down | Style_On)))
                break;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            if (btn->state & (Style_On | Style_Down))
                bdi.state = kThemeStatePressed;
            else
                bdi.state = kThemeStateActive;
            bdi.adornment = kThemeAdornmentNone;
            bdi.value = kThemeButtonOff;
            if (btn->features != QStyleOptionButton::None)
                bdi.kind = kThemeBevelButton;
            else
                bdi.kind = kThemePushButton;
            if (btn->state & Style_HasFocus
                    && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment |= kThemeAdornmentFocus;
            if (btn->state & Style_ButtonDefault
                && d->animatable(QAquaAnimate::AquaPushButton, w)) {
                bdi.adornment |= kThemeAdornmentDefault;
                bdi.animation.time.start = d->defaultButtonStart;
                bdi.animation.time.current = CFAbsoluteTimeGetCurrent();
            }
            HIRect newRect = qt_hirectForQRect(btn->rect);
            // Like Appearance Manager, HITheme draws outside my rect, so make it a bit bigger.
            QRect off_rct;
            HIRect outRect;
            HIThemeGetButtonBackgroundBounds(&newRect, &bdi, &outRect);
            off_rct.setRect(int(newRect.origin.x - outRect.origin.x),
                            int(newRect.origin.y - outRect.origin.y),
                            int(outRect.size.width - newRect.size.width),
                            int(outRect.size.height - newRect.size.height));
            newRect = qt_hirectForQRect(btn->rect, p, false, off_rct);
            HIThemeDrawButton(&newRect, &bdi, cg, kHIThemeOrientationNormal, 0);
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbi = pixelMetric(PM_MenuButtonIndicator, w);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() / 2 - 5, mbi, ir.height() / 2);
                drawPrimitive(PE_ArrowDown, &newBtn, p, w);
            }
        }
        break;
    case CE_MenuItem:
    case CE_MenuEmptyArea:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            int tabwidth = mi->tabWidth;
            int maxpmw = mi->maxIconWidth;
            bool active = mi->state & Style_Active;
            bool enabled = mi->state & Style_Enabled;
            HIRect menuRect = qt_hirectForQRect(mi->menuRect, p, false);
            HIRect itemRect = qt_hirectForQRect(mi->rect, p, false);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            mdi.itemType = kThemeMenuItemPlain;
            if (!mi->icon.isNull())
                mdi.itemType |= kThemeMenuItemHasIcon;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                mdi.itemType |= kThemeMenuItemHierarchical | kThemeMenuItemHierBackground;
            else
                mdi.itemType |= kThemeMenuItemPopUpBackground;
            if (enabled)
                mdi.state = kThemeMenuActive;
            else
                mdi.state = kThemeMenuDisabled;
            if (active)
                mdi.state |= kThemeMenuSelected;
            HIRect contentRect;
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                // First arg should be &menurect, but wacky stuff happens then.
                HIThemeDrawMenuSeparator(&itemRect, &itemRect, &mdi,
                                         cg, kHIThemeOrientationNormal);
                break;
            } else {
                HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                    cg, kHIThemeOrientationNormal, &contentRect);
                if(ce == CE_MenuEmptyArea)
                    break;
            }
            int x, y, w, h;
            mi->rect.rect(&x, &y, &w, &h);
            int xpos = x + 18;
            int checkcol = maxpmw;
            int xm = macItemFrame + maxpmw + macItemHMargin;
            if (!enabled)
                p->setPen(mi->palette.text());
            else if (active)
                p->setPen(mi->palette.highlightedText());
            else
                p->setPen(mi->palette.buttonText());

            if (mi->checkState == QStyleOptionMenuItem::Checked) {
                // Use the HIThemeTextInfo foo to draw the check mark correctly, if we do it,
                // we somehow need to use a special encoding as it doesn't look right with our
                // drawText().
                HIThemeTextInfo tti;
                tti.version = qt_mac_hitheme_version;
                tti.state = tds;
                if (active)
                    tti.state = kThemeStatePressed;
                tti.fontID = kThemeMenuItemMarkFont;
                tti.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
                tti.verticalFlushness = kHIThemeTextVerticalFlushCenter;
                tti.options = kHIThemeTextBoxOptionNone;
                tti.truncationPosition = kHIThemeTextTruncationNone;
                tti.truncationMaxLines = 1;
                QCFString checkmark = QString(QChar(kCheckUnicode));
                int mw = checkcol + macItemFrame;
                int mh = h - 2 * macItemFrame;
                int xp = x;
                xp += macItemFrame;
                float outWidth, outHeight, outBaseline;
                HIThemeGetTextDimensions(checkmark, 0, &tti, &outWidth, &outHeight,
                                         &outBaseline);
                QRect r(xp, y + macItemFrame, mw, mh);
                r.moveBy(0, p->fontMetrics().ascent() - int(outBaseline) + 1);
                HIRect bounds = qt_hirectForQRect(r);
                HIThemeDrawTextBox(checkmark, &bounds, &tti,
                                   cg,
                                   kHIThemeOrientationNormal);
            }
            if (!mi->icon.isNull()) {
                QIconSet::Mode mode = (mi->state & Style_Enabled) ? QIconSet::Normal
                                                                  : QIconSet::Disabled;
                if (active && !enabled)
                    mode = QIconSet::Active;
                QPixmap pixmap = mi->icon.pixmap(QIconSet::Small, mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(xpos, y, checkcol, h);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }

            QString s = mi->text;
            if (!s.isEmpty()) {
                int t = s.indexOf('\t');
                int m = 2;
                int text_flags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextHideMnemonic | Qt::TextSingleLine;
                p->save();
                if (t >= 0) {
                    extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp
                    p->setFont(qt_app_fonts_hash()->value("QMenuItem", p->font()));
                    int xp = x + w - tabwidth - macRightBorder
                             - macItemHMargin - macItemFrame + 1;
                    p->drawText(xp, y + m, tabwidth, h - 2 * m, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }

                text_flags ^= Qt::AlignRight;
                p->setFont(mi->font);
                p->drawText(xpos, y + m, w - xm - tabwidth + 1, h - 2 * m, text_flags, s, t);
                p->restore();
            }
        }
        break;
    case CE_MenuHMargin:
    case CE_MenuVMargin:
    case CE_MenuTearoff:
    case CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            HIRect menuRect = qt_hirectForQRect(mi->menuRect);
            HIRect itemRect = qt_hirectForQRect(mi->rect);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            if (opt->state & Style_Active)
                mdi.state = kThemeMenuSelected;
            else
                mdi.state = kThemeMenuActive;
            if (ce == CE_MenuScroller) {
                if (opt->state & Style_Down)
                    mdi.itemType = kThemeMenuItemScrollDownArrow;
                else
                    mdi.itemType = kThemeMenuItemScrollUpArrow;
            } else {
                mdi.itemType = kThemeMenuItemPlain;
            }
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                cg,
                                kHIThemeOrientationNormal, 0);
            if (ce == CE_MenuTearoff) {
                p->setPen(QPen(mi->palette.dark(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2 - 1);
                p->setPen(QPen(mi->palette.light(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2);
            }
        }
        break;
    case CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            HIThemeMenuTitleDrawInfo tdi;
            tdi.version = qt_mac_hitheme_version;
            if (!(mi->state & Style_Enabled))
                tdi.state = kThemeMenuDisabled;
            else
                tdi.state = kThemeMenuActive;
            if (mi->state & Style_Active)
                tdi.state |= kThemeMenuSelected;
            tdi.attributes = 0;
            tdi.condensedTitleExtra = 0.0;
            HIRect mbRect = qt_hirectForQRect(mi->menuRect), textRect;
            HIRect rect = qt_hirectForQRect(QRect(mi->rect.x(), 0, mi->rect.width(), mi->menuRect.height()), false);
            HIThemeDrawMenuTitle(&mbRect, &rect, &tdi, cg,
                                 kHIThemeOrientationNormal, &textRect);
            //text
            QPixmap pix = mi->icon.pixmap(QIconSet::Small, QIconSet::Normal);
            drawItem(p, qrectForHIRect(textRect),
                     Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip | Qt::TextSingleLine,
                     mi->palette, mi->state & Style_Enabled,
                     pix, mi->text, -1, &mi->palette.buttonText().color());
        }
        break;
    case CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            HIThemeMenuBarDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeMenuBarNormal;
            bdi.attributes = kThemeMenuSquareMenuBar;
            HIRect hirect = qt_hirectForQRect(mi->rect);
            HIThemeDrawMenuBarBackground(&hirect, &bdi, cg,
                                         kHIThemeOrientationNormal);
            break;
        }
    case CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qt_cast<const QStyleOptionProgressBar *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            tdi.version = qt_mac_hitheme_version;
            tdi.reserved = 0;
            // Boy, I love writing this...
            switch (qt_aqua_size_constrain(w)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                tdi.kind = pb->totalSteps ? kThemeLargeProgressBar
                                          : kThemeLargeIndeterminateBar;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                    tdi.kind = pb->totalSteps ? kThemeMiniProgressBar
                                              : kThemeMiniIndeterminateBar;
                    break;
                }
#endif
            case QAquaSizeSmall:
                tdi.kind = pb->totalSteps ? kThemeProgressBar : kThemeIndeterminateBar;
                break;
            }
            tdi.bounds = qt_hirectForQRect(pb->rect);
            tdi.max = pb->totalSteps;
            tdi.min = 0;
            tdi.value = pb->progress;
            tdi.attributes = kThemeTrackHorizontal;
            tdi.trackInfo.progress.phase = d->progressFrame;
            if (!qAquaActive(pb->palette))
                tdi.enableState = kThemeTrackInactive;
            else if (!(pb->state & Style_Enabled))
                tdi.enableState = kThemeTrackDisabled;
            else
                tdi.enableState = kThemeTrackActive;
            HIThemeDrawTrack(&tdi, 0, cg,
                             kHIThemeOrientationNormal);
        }
        break;
    case CE_ProgressBarLabel:
    case CE_ProgressBarGroove:
        break;
    case CE_HeaderLabel:
        if (const QStyleOptionHeader *header = qt_cast<const QStyleOptionHeader *>(opt)) {
            QRect textr = header->rect;
            if (!header->icon.isNull()) {
                QIconSet::Mode mode = QIconSet::Disabled;
                if (opt->state & Style_Enabled)
                    mode = QIconSet::Normal;
                QPixmap pixmap = header->icon.pixmap(QIconSet::Small, mode);

                QRect pixr = header->rect;
                pixr.setY(header->rect.center().y() - (pixmap.height() - 1) / 2);
                drawItem(p, pixr, Qt::AlignVCenter, header->palette,
                        mode != QIconSet::Disabled
                        || !header->icon.isGenerated(QIconSet::Small, mode), pixmap);
                textr.moveBy(pixmap.width() + 2, 0);
            }

            // change the color to bright text if we are a table header and selected.
            const QColor *penColor = &header->palette.buttonText().color();
            if (w && w->parentWidget()->inherits("QTable") && p->font().bold())
                penColor = &header->palette.color(QPalette::BrightText);
            drawItem(p, textr, Qt::AlignVCenter, header->palette, header->state & Style_Enabled,
                    header->text, -1, penColor);
        }
        break;
    case CE_ToolBoxTab:
        QCommonStyle::drawControl(ce, opt, p, w);
        break;
    case CE_TabBarTab:
        if (const QStyleOptionTab *tabOpt = qt_cast<const QStyleOptionTab *>(opt)) {
            HIThemeTabDrawInfo tdi;
            tdi.version = qt_mac_hitheme_version;
            tdi.style = kThemeTabNonFront;
            if (tabOpt->state & Style_Selected) {
                if(!qAquaActive(tabOpt->palette))
                    tdi.style = kThemeTabFrontUnavailable;
                else if(!(tabOpt->state & Style_Enabled))
                    tdi.style = kThemeTabFrontInactive;
                else
                    tdi.style = kThemeTabFront;
            } else if (!qAquaActive(tabOpt->palette)) {
                tdi.style = kThemeTabNonFrontUnavailable;
            } else if( !(tabOpt->state & Style_Enabled)) {
                tdi.style = kThemeTabNonFrontInactive;
            } else if ((tabOpt->state & (Style_Sunken | Style_MouseOver))
                       == (Style_Sunken | Style_MouseOver)) {
                tdi.style = kThemeTabNonFrontPressed;
            }
            if (tabOpt->shape == QTabBar::RoundedAbove || tabOpt->shape == QTabBar::TriangularAbove)
                tdi.direction = kThemeTabNorth;
            else
                tdi.direction = kThemeTabSouth;
            tdi.size = kHIThemeTabSizeNormal;
            if (tabOpt->state & Style_HasFocus)
                tdi.adornment = kHIThemeTabAdornmentFocus;
            else
                tdi.adornment = kHIThemeTabAdornmentNone;
            QRect tabrect = tabOpt->rect;
            tabrect.setHeight(tabrect.height() + pixelMetric(PM_TabBarBaseOverlap, w));
            HIRect hirect = qt_hirectForQRect(tabrect, p);
            HIThemeDrawTab(&hirect, &tdi, cg, kHIThemeOrientationNormal, 0);
            if (!(opt->state & Style_Selected)) {
                HIThemeTabPaneDrawInfo tpdi;
                tpdi.version = 0;
                tpdi.state = tds;
                tpdi.direction = tdi.direction;
                tpdi.size = tdi.size;
                const int FUDGE = 20;
                QRect panerect(tabOpt->rect.x() - FUDGE, tabOpt->rect.bottom() - 2,
                               2 * FUDGE + tabOpt->rect.width(),
                               pixelMetric(PM_TabBarBaseHeight, w));
                if (tdi.direction == kThemeTabSouth)
                    panerect.moveBy(0, (-tabOpt->rect.height() + 2));
                p->save();
                p->setClipRect(tabOpt->rect.x(), panerect.y(), tabOpt->rect.width(),
                               panerect.height());
                hirect = qt_hirectForQRect(panerect, p);
                HIThemeDrawTabPane(&hirect, &tpdi, cg, kHIThemeOrientationNormal);
                p->restore();
            }
        }
        break;
    case CE_ToolBarButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            const QRect cr = subRect(SR_ToolBarButtonContents, btn, w);
            QIconSet::Mode iconMode = (btn->state & Style_Enabled) ? QIconSet::Normal
                                                                   : QIconSet::Disabled;
            if (btn->state & Style_Down)
                iconMode = QIconSet::Active;
            QIconSet::State iconState = (btn->state & Style_On) ? QIconSet::On : QIconSet::Off;

            const QPixmap pixmap = btn->icon.pixmap(QIconSet::Large, iconMode, iconState);

            p->drawPixmap((cr.width() - pixmap.width()) / 2, 3, pixmap);
            if (!btn->text.isEmpty()) {
                if (btn->state & Style_Down)
                    p->drawText(cr.x(), cr.y() + pixmap.height(), cr.width(),
                                cr.height() - pixmap.height(), Qt::AlignCenter, btn->text);
                p->drawText(cr.x(), cr.y() + pixmap.height(), cr.width(),
                            cr.height() - pixmap.height(), Qt::AlignCenter, btn->text);
            }
            if (btn->features & QStyleOptionButton::HasMenu) {
                QStyleOption arrowOpt(0);
                arrowOpt.rect = subRect(SR_ToolBarButtonMenu, btn, w);
                arrowOpt.rect.setY(arrowOpt.rect.y() + arrowOpt.rect.height() / 2);
                arrowOpt.rect.setHeight(arrowOpt.rect.height() / 2);
                arrowOpt.state = btn->state;
                arrowOpt.palette = btn->palette;
                drawPrimitive(PE_ArrowDown, &arrowOpt, p, w);
            }
        }
        break;
    default:
        QWindowsStyle::drawControl(ce, opt, p, w);
    }
}

QRect QMacStyleCG::subRect(SubRect sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect r;
    switch (sr) {
    case SR_PushButtonContents:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            HIRect inRect = CGRectMake(0, 0, btn->rect.width(), btn->rect.height());
            HIRect outRect;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeStateActive;
            if (btn->features & QStyleOptionButton::None)
                bdi.kind = kThemePushButton;
            else
                bdi.kind = kThemeBevelButton;
            bdi.adornment = kThemeAdornmentNone;
            HIThemeGetButtonContentBounds(&inRect, &bdi, &outRect);
            r.setRect(int(outRect.origin.x), int(outRect.origin.y - 2),
                      int(qMin(btn->rect.width() - 2 * outRect.origin.x, outRect.size.width)),
                      int(qMin(btn->rect.height() - 2 * outRect.origin.y, outRect.size.height)));
        }
        break;
    case SR_ProgressBarGroove:
    case SR_ProgressBarLabel:
        break;
    case SR_ProgressBarContents:
        r = opt->rect;
        break;
    case SR_ToolBarButtonContents:
    case SR_ToolBarButtonMenu:
        r = QWindowsStyle::subRect(sr, opt, w);
        break;
    default:
        r = QWindowsStyle::subRect(sr, opt, w);
    }
    return r;
}

void QMacStyleCG::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                     QPainter *p, const QWidget *widget) const
{
    ThemeDrawState tds = d->getDrawState(opt->state, opt->palette);
    QMacCGContext cg(p);
    switch (cc) {
    case CC_Slider:
    case CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            if (cc == CC_Slider) {
                if (slider->activeParts == SC_SliderHandle)
                    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
                else if (slider->activeParts == SC_SliderGroove)
                    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
            } else {
                tdi.trackInfo.scrollbar.viewsize = slider->pageStep;
                if (slider->activeParts == SC_ScrollBarSubLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                         | kThemeLeftOutsideArrowPressed;
                else if (slider->activeParts == SC_ScrollBarAddLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                         | kThemeRightOutsideArrowPressed;
                else if (slider->activeParts == SC_ScrollBarAddPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
                else if (slider->activeParts == SC_ScrollBarSubPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
                else if (slider->activeParts == SC_ScrollBarSlider)
                    tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            }
            HIRect macRect;
            bool tracking = slider->sliderPosition == slider->sliderValue;
            if (!tracking) {
                // Small optimization, the same as querySubControlMetrics
                QCFType<HIShapeRef> shape;
                HIThemeGetTrackThumbShape(&tdi, &shape);
                HIShapeGetBounds(shape, &macRect);
                tdi.value = slider->sliderValue;
            }
            HIThemeDrawTrack(&tdi, tracking ? 0 : &macRect, cg,
                             kHIThemeOrientationNormal);
            if (cc == CC_Slider && slider->parts & SC_SliderTickmarks) {
                int numMarks;
                if (slider->tickInterval) {
                    if (slider->orientation == Qt::Horizontal)
                        numMarks = slider->rect.width() / slider->tickInterval;
                    else
                        numMarks = slider->rect.height() / slider->tickInterval;
                } else {
                    numMarks = (slider->maximum - slider->minimum + 1) / slider->pageStep;
                }
                if (tdi.trackInfo.slider.thumbDir == kThemeThumbPlain) {
                    // They asked for both, so we'll give it to them.
                    tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
                    HIThemeDrawTrackTickMarks(&tdi, numMarks,
                                              cg,
                                              kHIThemeOrientationNormal);
                    tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
                    HIThemeDrawTrackTickMarks(&tdi, numMarks,
                                              cg,
                                               kHIThemeOrientationNormal);
                } else {
                    HIThemeDrawTrackTickMarks(&tdi, numMarks,
                                              cg,
                                              kHIThemeOrientationNormal);

                }
            }
        }
        break;
    case CC_ListView:
        if (const QStyleOptionListView *lv = qt_cast<const QStyleOptionListView *>(opt)) {
            if (lv->parts & SC_ListView)
                QWindowsStyle::drawComplexControl(cc, lv, p, widget);
            if (lv->parts & (SC_ListViewBranch | SC_ListViewExpand)) {
                int y = lv->rect.y();
                int h = lv->rect.height();
                int x = lv->rect.right() - 10;
                for (int i = 1; i < lv->items.size() && y < h; ++i) {
                    QStyleOptionListViewItem item = lv->items.at(i);
                    if (y + item.height > 0 && (item.childCount > 0
                        || item.features & QStyleOptionListViewItem::Expandable)) {
                        QStyleOption treeOpt(0);
                        treeOpt.rect.setRect(x, y + item.height / 2 - 4, 9, 9);
                        treeOpt.palette = lv->palette;
                        treeOpt.state = lv->state;
                        treeOpt.state |= Style_Children;
                        if (item.state & Style_Open)
                            treeOpt.state |= Style_Open;
                        drawPrimitive(PE_TreeBranch, &treeOpt, p, widget);
                    }
                    y += item.totalHeight;
                }
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox newSB = *sb;
            if (sb->parts & SC_SpinBoxFrame) {
                QStyleOptionFrame lineedit(0);
                lineedit.rect = QStyle::visualRect(querySubControlMetrics(CC_SpinBox, sb,
                                                                          SC_SpinBoxFrame, widget),
                                                   widget);
                lineedit.palette = sb->palette;
                lineedit.state = Style_Sunken;
                lineedit.lineWidth = 0;
                lineedit.midLineWidth = 0;
                drawPrimitive(PE_PanelLineEdit, &lineedit, p, widget);
            }
            if (sb->parts & (SC_SpinBoxUp | SC_SpinBoxDown)) {
                HIThemeButtonDrawInfo bdi;
                bdi.version = qt_mac_hitheme_version;
                switch (qt_aqua_size_constrain(widget)) {
                    case QAquaSizeUnknown:
                    case QAquaSizeLarge:
                        bdi.kind = kThemeIncDecButton;
                        break;
                    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                            bdi.kind = kThemeIncDecButtonMini;
                            break;
                        }
#endif
                    case QAquaSizeSmall:
                        bdi.kind = kThemeIncDecButtonSmall;
                        break;
                }
                if (!(sb->stepEnabled & (QAbstractSpinBox::StepUpEnabled
                                        | QAbstractSpinBox::StepDownEnabled)))
                    tds = kThemeStateUnavailable;
                if (sb->activeParts == SC_SpinBoxDown)
                    tds = kThemeStatePressedDown;
                else if (sb->activeParts == SC_SpinBoxUp)
                    tds = kThemeStatePressedUp;
                bdi.state = tds;
                bdi.value = kThemeButtonOff;
                if (sb->state & Style_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    bdi.adornment = kThemeAdornmentFocus;
                else
                    bdi.adornment = kThemeAdornmentNone;
                QRect updown = visualRect(querySubControlMetrics(CC_SpinBox, sb, SC_SpinBoxUp,
                                                                 widget), widget);
                updown |= visualRect(querySubControlMetrics(CC_SpinBox, sb, SC_SpinBoxDown, widget),
                                     widget);
                if (widget) {
                    QPalette::ColorRole bgRole = widget->backgroundRole();
                    if (sb->palette.brush(bgRole).pixmap())
                        p->drawPixmap(updown, *sb->palette.brush(bgRole).pixmap());
                    else
                        p->fillRect(updown, sb->palette.color(bgRole));
                }
                HIRect hirect = qt_hirectForQRect(updown, p);
                HIThemeDrawButton(&hirect, &bdi, cg,
                                  kHIThemeOrientationNormal, 0);
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            QRect comborect(cmb->rect);
            bdi.adornment = opt->state & Style_HasFocus
                                ? kThemeAdornmentFocus : kThemeAdornmentNone;
            bdi.state = opt->activeParts & SC_ComboBoxArrow
                                ? ThemeDrawState(kThemeStatePressed) : tds;
            if (cmb->editable) {
                bdi.adornment |= kThemeAdornmentArrowDownArrow;
                comborect = querySubControlMetrics(CC_ComboBox, cmb, SC_ComboBoxArrow ,widget);
                switch (qt_aqua_size_constrain(widget)) {
                    case QAquaSizeUnknown:
                    case QAquaSizeLarge:
                        bdi.kind = kThemeArrowButton;
                        break;
                    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                            bdi.kind = kThemeArrowButtonMini;
                            break;
                        }
#endif
                    case QAquaSizeSmall:
                        bdi.kind = kThemeArrowButtonSmall;
                        break;
                }
                QRect lineeditRect(cmb->rect);
                lineeditRect.setWidth(cmb->rect.width() - comborect.width());
                QStyleOptionFrame lineedit(0);
                lineedit.rect = lineeditRect;
                lineedit.palette = cmb->palette;
                lineedit.state = cmb->state;
                lineedit.lineWidth = 0;
                lineedit.midLineWidth = 0;
                drawPrimitive(PE_PanelLineEdit, &lineedit, p, widget);
            } else {
                bdi.adornment |= kThemeAdornmentArrowLeftArrow;
                bdi.kind = kThemePopupButton;
            }
            HIRect hirect = qt_hirectForQRect(comborect, p);
            HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titlebar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            wdi.version = qt_mac_hitheme_version;
            wdi.state = tds;
            wdi.windowType = QtWinType;
            wdi.titleHeight = titlebar->rect.height();
            wdi.titleWidth = titlebar->rect.width();
            wdi.attributes = kThemeWindowHasTitleText;
            if (titlebar->titleBarState) {
                if (titlebar->titleBarState & Qt::WindowMinimized)
                    wdi.attributes |= kThemeWindowIsCollapsed;
                /*
                if (titlebar->window()->isWindowModified())
                    wdi.attributes |= kThemeWindowHasDirty;
                    */
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            } else if (titlebar->titleBarFlags & Qt::WStyle_SysMenu) {
                wdi.attributes |= kThemeWindowHasCloseBox;
            }
            QCFType<HIShapeRef> titleRegion;
            HIRect titleRect = qt_hirectForQRect(titlebar->rect, p);
            QRect newr = titlebar->rect;
            HIThemeGetWindowShape(&titleRect, &wdi, kWindowTitleBarRgn, &titleRegion);
            HIShapeGetBounds(titleRegion, &titleRect);
            newr.moveBy(newr.x() - (int)titleRect.origin.x, newr.y() - (int)titleRect.origin.y);
            HIRect finalRect = qt_hirectForQRect(newr, p, false);
            HIThemeDrawWindowFrame(&finalRect, &wdi, cg, kHIThemeOrientationNormal, 0);
            if (titlebar->parts & SC_TitleBarLabel) {
                int iw = 0;
                if (!titlebar->icon.isNull()) {
                    QCFType<HIShapeRef> titleRegion2;
                    HIThemeGetWindowShape(&finalRect, &wdi, kWindowTitleProxyIconRgn,
                                          &titleRegion2);
                    HIShapeGetBounds(titleRegion2, &titleRect);
                    if (titleRect.size.width != 1)
                        iw = titlebar->icon.width();
                }
                if (!titlebar->text.isEmpty()) {
                    p->save();
                    QCFType<HIShapeRef> titleRegion3;
                    HIThemeGetWindowShape(&finalRect, &wdi, kWindowTitleTextRgn, &titleRegion3);
                    HIShapeGetBounds(titleRegion3, &titleRect);
                    p->setClipRect(qrectForHIRect(titleRect));
                    QRect br = p->clipRegion().boundingRect();
                    int x = br.x(),
                    y = br.y() + (titlebar->rect.height() / 2 - p->fontMetrics().height() / 2);
                    if (br.width() <= (p->fontMetrics().width(titlebar->text) + iw * 2))
                        x += iw;
                    else
                        x += br.width() / 2 - p->fontMetrics().width(titlebar->text) / 2;
                    if (iw)
                        p->drawPixmap(x - iw, y, titlebar->icon);
                    p->drawText(x, y + p->fontMetrics().ascent(), titlebar->text);
                    p->restore();
                }
            }
            if (titlebar->parts & (SC_TitleBarCloseButton | SC_TitleBarMaxButton
                                   | SC_TitleBarMinButton | SC_TitleBarNormalButton)) {
                HIThemeWindowWidgetDrawInfo wwdi;
                wwdi.version = qt_mac_hitheme_version;
                wwdi.widgetState = tds;
                if (titlebar->state & Style_MouseOver)
                    wwdi.widgetState = kThemeStateRollover;
                wwdi.windowType = QtWinType;
                wwdi.attributes = wdi.attributes;
                wwdi.windowState = wdi.state;
                wwdi.titleHeight = wdi.titleHeight;
                wwdi.titleWidth = wdi.titleWidth;
                ThemeDrawState savedControlState = wwdi.widgetState;
                uint sc = SC_TitleBarMinButton;
                ThemeTitleBarWidget tbw = kThemeWidgetCollapseBox;
                bool active = qAquaActive(titlebar->palette);
                while (sc <= SC_TitleBarCloseButton) {
                    uint tmp = sc;
                    wwdi.widgetState = savedControlState;
                    wwdi.widgetType = tbw;
                    if (sc == SC_TitleBarMinButton)
                        tmp |= SC_TitleBarNormalButton;
                    if (active && (titlebar->activeParts & tmp))
                        wwdi.widgetState = kThemeStatePressed;
                    /*
                    if (titlebar->window() && titlebar->window()->isWindowModified()
                            && tbw == kThemeWidgetCloseBox)
                        wwdi.widgetType = kThemeWidgetDirtyCloseBox;
                            */
                    HIThemeDrawTitleBarWidget(&finalRect, &wwdi, cg, kHIThemeOrientationNormal);
                    sc = sc << 1;
                    tbw = tbw >> 1;
                }
            }
        }
        break;
    default:
        QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
}

QStyle::SubControl QMacStyleCG::querySubControl(ComplexControl cc, const QStyleOptionComplex *opt,
                                   const QPoint &pt, const QWidget *widget) const
{
    SubControl sc = SC_None;
    switch (cc) {
    case CC_Slider:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            ControlPartCode part;
            HIPoint pos = CGPointMake(pt.x(), pt.y());
            if (HIThemeHitTestTrack(&tdi, &pos, &part)) {
                if (part == kControlPageUpPart || part == kControlPageDownPart)
                    sc = SC_SliderGroove;
                else
                    sc = SC_SliderHandle;
            }
        }
        break;
    case CC_ScrollBar:
        if (const QStyleOptionSlider *sb = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIScrollBarTrackInfo sbi;
            sbi.version = qt_mac_hitheme_version;
            if (!qAquaActive(sb->palette))
                sbi.enableState = kThemeTrackInactive;
            else if (sb->state & Style_Enabled)
                sbi.enableState = kThemeTrackDisabled;
            else
                sbi.enableState = kThemeTrackActive;
            sbi.viewsize = sb->pageStep;
            HIPoint pos = CGPointMake(pt.x(), pt.y());
            HIRect macSBRect = qt_hirectForQRect(sb->rect);
            ControlPartCode part;
            if (HIThemeHitTestScrollBarArrows(&macSBRect, &sbi, sb->orientation == Qt::Horizontal,
                        &pos, 0, &part)) {
                if (part == kControlUpButtonPart)
                    sc = SC_ScrollBarSubLine;
                else if (part == kControlDownButtonPart)
                    sc = SC_ScrollBarAddLine;

            } else {
                HIThemeTrackDrawInfo tdi;
                getSliderInfo(cc, sb, &tdi, widget);
                if (HIThemeHitTestTrack(&tdi, &pos, &part)) {
                    if (part == kControlPageUpPart)
                        sc = SC_ScrollBarSubPage;
                    else if (part == kControlPageDownPart)
                        sc = SC_ScrollBarAddPage;
                    else
                        sc = SC_ScrollBarSlider;
                }
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            sc = QWindowsStyle::querySubControl(cc, cmb, pt, widget);
            if (!cmb->editable && sc != SC_None)
                sc = SC_ComboBoxArrow;  // A bit of a lie, but what we want
        }
        break;
/*
    I don't know why, but we only get kWindowContentRgn here, which isn't what we want at all.
    It would be very nice if this would work.
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            memset(&wdi, 0, sizeof(wdi));
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            wdi.titleWidth = tbar->rect.width();
            wdi.titleHeight = tbar->rect.height();
            if (tbar->titleBarState)
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            else if (tbar->titleBarFlags & Qt::WStyle_SysMenu)
                wdi.attributes |= kThemeWindowHasCloseBox;
            QRect tmpRect = tbar->rect;
            tmpRect.setHeight(tmpRect.height() + 100);
            HIRect hirect = qt_hirectForQRect(tmpRect);
            WindowRegionCode hit;
            HIPoint hipt = CGPointMake(pt.x(), pt.y());
            if (HIThemeGetWindowRegionHit(&hirect, &wdi, &hipt, &hit)) {
                switch (hit) {
                case kWindowCloseBoxRgn:
                    sc = SC_TitleBarCloseButton;
                    break;
                case kWindowCollapseBoxRgn:
                    sc = SC_TitleBarMinButton;
                    break;
                case kWindowZoomBoxRgn:
                    sc = SC_TitleBarMaxButton;
                    break;
                case kWindowTitleTextRgn:
                    sc = SC_TitleBarLabel;
                    break;
                default:
                    qDebug("got something else %d", hit);
                    break;
                }
            }
        }
        break;
*/
    default:
        sc = QWindowsStyle::querySubControl(cc, opt, pt, widget);
    }
    return sc;
}

QRect QMacStyleCG::querySubControlMetrics(ComplexControl cc, const QStyleOptionComplex *opt,
                                          SubControl sc, const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case CC_Slider:
    case CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qt_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            HIRect macRect;
            // Luckily, the slider and scrollbar parts don't overlap so we can do it in one go.
            switch (sc) {
            case SC_SliderGroove:
                HIThemeGetTrackBounds(&tdi, &macRect);
                ret = qrectForHIRect(macRect);
                break;
            case SC_ScrollBarGroove:
                HIThemeGetTrackDragRect(&tdi, &macRect);
                ret = qrectForHIRect(macRect);
                break;
            case SC_SliderHandle:
            case SC_ScrollBarSlider:
                QCFType<HIShapeRef> shape;
                HIThemeGetTrackThumbShape(&tdi, &shape);
                HIShapeGetBounds(shape, &macRect);
                ret = qrectForHIRect(macRect);
                break;
            }
        }
        break;
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qt_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 10,
            spinner_h = 15;
            int fw = pixelMetric(PM_SpinBoxFrameWidth, widget),
            y = fw,
            x = spin->rect.width() - fw - spinner_w;
            switch (sc) {
                case SC_SpinBoxUp:
                    ret.setRect(x, y + ((spin->rect.height() - fw * 2) / 2 - spinner_h),
                                spinner_w, spinner_h);
                    break;
                case SC_SpinBoxDown:
                    ret.setRect(x, y + (spin->rect.height() - fw * 2) / 2, spinner_w, spinner_h);
                    break;
                case SC_SpinBoxButtonField:
                    ret.setRect(x, y, spinner_w, spin->rect.height() - fw * 2);
                    break;
                case SC_SpinBoxEditField:
                    ret.setRect(fw, fw, spin->rect.width() - spinner_w - fw * 2 - macSpinBoxSep,
                                spin->rect.height() - fw * 2);
                    break;
                case SC_SpinBoxFrame:
                    ret.setRect(0, 0, spin->rect.width() - spinner_w - macSpinBoxSep,
                                spin->rect.height());
                    break;
                default:
                    ret = QWindowsStyle::querySubControlMetrics(cc, spin, sc, widget);
                    break;
            }
        }
        break;
    case CC_TitleBar:
        if (const QStyleOptionTitleBar *titlebar = qt_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            memset(&wdi, 0, sizeof(wdi));
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            wdi.titleHeight = titlebar->rect.height();
            wdi.titleWidth = titlebar->rect.width();
            wdi.attributes = kThemeWindowHasTitleText;
            if (titlebar->titleBarState)
                wdi.attributes |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            else if (titlebar->titleBarFlags & Qt::WStyle_SysMenu)
                wdi.attributes |= kThemeWindowHasCloseBox;
            WindowRegionCode wrc = kWindowGlobalPortRgn;
            if (sc == SC_TitleBarCloseButton)
                wrc = kWindowCloseBoxRgn;
            else if (sc == SC_TitleBarMinButton)
                wrc = kWindowCollapseBoxRgn;
            else if (sc == SC_TitleBarMaxButton)
                wrc = kWindowZoomBoxRgn;
            else if (sc == SC_TitleBarLabel)
                wrc = kWindowTitleTextRgn;
            else if (sc == SC_TitleBarSysMenu)
                ret.setRect(-1024, -1024, 10, pixelMetric(PM_TitleBarHeight, 0));
            if (wrc != kWindowGlobalPortRgn) {
                QCFType<HIShapeRef> region;
                QRect tmpRect = titlebar->rect;
                HIRect titleRect = qt_hirectForQRect(tmpRect);
                HIThemeGetWindowShape(&titleRect, &wdi, kWindowTitleBarRgn, &region);
                HIShapeGetBounds(region, &titleRect);
                CFRelease(region);
                tmpRect.moveBy(tmpRect.x() - int(titleRect.origin.x),
                               tmpRect.y() - int(titleRect.origin.y));
                titleRect = qt_hirectForQRect(tmpRect);
                HIThemeGetWindowShape(&titleRect, &wdi, wrc, &region);
                HIShapeGetBounds(region, &titleRect);
                ret = qrectForHIRect(titleRect);
            }
        }
        break;
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qt_cast<const QStyleOptionComboBox *>(opt)) {
            if (cmb->editable) {
                if (sc == SC_ComboBoxEditField)
                    ret.setRect(0, 0, cmb->rect.width() - 20, cmb->rect.height());
                else if (sc == SC_ComboBoxArrow)
                    ret.setRect(cmb->rect.width() - 24, 0, 24, cmb->rect.height());
            } else {
                ret = QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
                if (sc == SC_ComboBoxEditField)
                    ret.setWidth(ret.width()-5);
            }
        }
        break;
    default:
        ret = QWindowsStyle::querySubControlMetrics(cc, opt, sc, widget);
    }
    return ret;
}

QSize QMacStyleCG::sizeFromContents(ContentsType ct, const QStyleOption *opt, const QSize &csz,
                                    const QFontMetrics &fm, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case CT_SpinBox:
        sz.setWidth(sz.width() + macSpinBoxSep);
        break;
    case CT_TabWidget:
        sz.setWidth(sz.width() + 15);
        break;
    case CT_TabBarTab: {
        SInt32 tabh = sz.height();
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge: {
            GetThemeMetric(kThemeLargeTabHeight, &tabh);
            SInt32 overlap;
            GetThemeMetric(kThemeMetricTabFrameOverlap, &overlap);
            tabh += overlap;
            break; }
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_PANTHER) {
                GetThemeMetric(kThemeMetricMiniTabHeight, &tabh);
                SInt32 overlap;
                GetThemeMetric(kThemeMetricMiniTabFrameOverlap, &overlap);
                tabh += overlap;
                break;
            }
#endif
        case QAquaSizeSmall: {
            GetThemeMetric(kThemeSmallTabHeight, &tabh);
            SInt32 overlap;
            GetThemeMetric(kThemeMetricSmallTabFrameOverlap, &overlap);
            tabh += overlap;
            break; }
        }
        if(sz.height() > tabh)
            sz.setHeight(tabh);
        break; }
    case CT_PushButton:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, fm, widget);
        sz = QSize(sz.width() + 16, sz.height());
        break;
    case CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qt_cast<const QStyleOptionMenuItem *>(opt)) {
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(),
                h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                SInt16 ash;
                GetThemeMenuSeparatorHeight(&ash);
                h = ash;
            } else {
                h = qMax(h, fm.height() + 2);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(QIconSet::Small, QIconSet::Normal).height() + 4);
            }
            if (mi->text.contains('\t'))
                w += 12;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 20;
            if (maxpmw)
                w += maxpmw + 6;
            // add space for a check. All items have place for a check too.
            w += 20;
            if (widget && ::qt_cast<QComboBox*>(widget->parentWidget())
                    && widget->parentWidget()->isVisible()) {
                QStyleOptionComboBox cmb(0);
                cmb.init(widget->parentWidget());
                cmb.editable = false;
                cmb.parts = SC_ComboBoxEditField;
                cmb.activeParts = SC_None;
                w = qMax(w, querySubControlMetrics(CC_ComboBox, &cmb, SC_ComboBoxEditField,
                                                   widget->parentWidget())
                            .width());
            } else {
                w += 12;
            }
            sz = QSize(w, h);
        }
        break;
    case CT_ToolBarButton:
        if (const QStyleOptionButton *btn = qt_cast<const QStyleOptionButton *>(opt)) {
            sz = btn->icon.pixmap(QIconSet::Large, QIconSet::Normal).size() + QSize(7, 7);
            if (!btn->text.isEmpty()) {
                sz.rheight() += fm.lineSpacing();
                sz.rwidth() = qMax(sz.width(), fm.width(btn->text));
            }
            if (btn->features & QStyleOptionButton::HasMenu)
                sz.rwidth() += 12;
        }
        break;
    default:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, fm, widget);
    }
    QSize macsz;
    if (qt_aqua_size_constrain(widget, ct, sz, &macsz) != QAquaSizeUnknown) {
        if (macsz.width() != -1)
            sz.setWidth(macsz.width());
        if (macsz.height() != -1)
            sz.setHeight(macsz.height());
    }

    // Adjust size to within Aqua guidelines
    if (ct == CT_PushButton || ct == CT_ToolButton) {
        ThemeButtonKind bkind = kThemePushButton;
        if (ct == CT_ToolButton)
            bkind = kThemeBevelButton;
        if (qt_aqua_size_constrain(widget) == QAquaSizeSmall) {
            if(bkind == kThemeBevelButton)
                bkind = kThemeSmallBevelButton;
        }
        HIThemeButtonDrawInfo bdi;
        bdi.version = qt_mac_hitheme_version;
        bdi.state = kThemeStateActive;
        bdi.kind = bkind;
        bdi.value = kThemeButtonOff;
        bdi.adornment = kThemeAdornmentNone;
        HIRect macRect, myRect;
        myRect = CGRectMake(0, 0, sz.width(), sz.height());
        HIThemeGetButtonBackgroundBounds(&myRect, &bdi, &macRect);
        sz.setWidth(sz.width() + int(macRect.size.width - myRect.size.width));
        sz.setHeight(sz.height() + int(macRect.size.height - myRect.size.height));
    }

    return sz;
}


#endif
