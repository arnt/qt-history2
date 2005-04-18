/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qmacstyle_mac.h"

#if defined(Q_WS_MAC) && !defined(QT_NO_STYLE_MAC)
#define QMAC_QAQUASTYLE_SIZE_CONSTRAIN
//#define DEBUG_SIZE_CONSTRAINT

#include <private/qmacstylepixmaps_mac_p.h>
#include <private/qpaintengine_mac_p.h>
#include <private/qpainter_p.h>
#include <private/qprintengine_mac_p.h>
#include <qapplication.h>
#include <qbitmap.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdockwidget.h>
#include <qevent.h>
#include <qfocusframe.h>
#include <qgroupbox.h>
#include <qhash.h>
#include <qheaderview.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qpaintdevice.h>
#include <qpainter.h>
#include <qpixmapcache.h>
#include <qpointer.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qrubberband.h>
#include <qsizegrip.h>
#include <qspinbox.h>
#include <qsplitter.h>
#include <qstyleoption.h>
#include <qtextedit.h>
#include <qtextstream.h>
#include <qtimer.h>
#include <qtoolbar.h>
#include <qtoolbutton.h>
#include <qtreeview.h>

#include <QtCore/qdebug.h>

extern QRegion qt_mac_convert_mac_region(RgnHandle); //qregion_mac.cpp
extern QHash<QByteArray, QFont> *qt_app_fonts_hash(); // qapplication.cpp

static inline bool isQDPainter(const QPainter *p)
{
    QPaintEngine *engine = p->paintEngine();
    if (engine->type() == QPaintEngine::MacPrinter)
        engine = static_cast<QMacPrintEngine*>(engine)->paintEngine();
    return engine && engine->type() == QPaintEngine::QuickDraw;
}

static void qt_mac_set_port(const QPainter *p)
{
    QPaintEngine *engine = p->paintEngine();
    if (engine->type() == QPaintEngine::MacPrinter)
        engine = static_cast<QMacPrintEngine*>(engine)->paintEngine();

    QQuickDrawPaintEngine *mpe = 0;
    if (engine && (engine->type() == QPaintEngine::QuickDraw
                   || engine->type() == QPaintEngine::CoreGraphics))
        mpe = static_cast<QQuickDrawPaintEngine *>(engine);
    if (mpe) {
        mpe->syncState();
        if (mpe->type() == QPaintEngine::QuickDraw) {
            mpe->setupQDPort(true);
        } else {
            QRegion rgn;
            mpe->setupQDPort(true, 0, &rgn);
            QMacSavedPortInfo::setClipRegion(rgn);
        }
    }
    NormalizeThemeDrawingState();
}

static inline QPoint domap(const QPainter *p, QPoint pt)
{
    pt = pt * p->matrix();
    return pt;
}
#if (MAC_OS_X_VERSION_MAX_ALLOWED == MAC_OS_X_VERSION_10_2)
// It's really silly, the define isn't there, but you can pass it and it works, dumb luck I guess.
enum { kThemeComboBox = 16 }; // From the 10.3 Appearance.h file.
#endif

// Utility to generate correct rectangles for AppManager internals
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPaintDevice *pd=0,
                                          bool useOffset=true, const QRect &rect=QRect())
{
    static Rect r;
    bool use_rect = (rect.x() || rect.y() || rect.width() || rect.height());
    QPoint tl(qr.topLeft());
    if (pd && pd->devType() == QInternal::Widget) {
        QWidget *w = (QWidget*)pd;
        tl = w->mapTo(w->window(), tl);
    }
    if (use_rect)
        tl += rect.topLeft();
    int offset = 0;
    if (useOffset)
        offset = 1;
    SetRect(&r, tl.x(), tl.y(), (tl.x() + qr.width()) - offset, (tl.y() + qr.height()) - offset);
    if (use_rect) {
        r.right -= rect.width();
        r.bottom -= rect.height();
    }
    return &r;
}
static inline const Rect *qt_glb_mac_rect(const QRect &qr, const QPainter *p,
                                          bool off=true, const QRect &rect=QRect())
{
    QPoint pt = qr.topLeft();
    QRect r(domap(p, pt), qr.size());
    return qt_glb_mac_rect(r, p->device(), off, rect);
}

static inline ThemeTabDirection getTabDirection(QTabBar::Shape shape)
{
    ThemeTabDirection ttd;
    switch (shape) {
    case QTabBar::RoundedSouth:
    case QTabBar::TriangularSouth:
        ttd = kThemeTabSouth;
        break;
    default:  // Added to remove the warning, since all values are taken care of, really!
    case QTabBar::RoundedNorth:
    case QTabBar::TriangularNorth:
        ttd = kThemeTabNorth;
        break;
    case QTabBar::RoundedWest:
    case QTabBar::TriangularWest:
        ttd = kThemeTabWest;
        break;
    case QTabBar::RoundedEast:
    case QTabBar::TriangularEast:
        ttd = kThemeTabEast;
        break;
    }
    return ttd;
}


class QMacStylePrivate : public QObject
{
    Q_OBJECT
public:
    QMacStylePrivate(QMacStyle *style);

    // Stuff from QAquaAnimate:
    bool addWidget(QWidget *);
    void removeWidget(QWidget *);

    enum Animates { AquaPushButton, AquaProgressBar, AquaListViewItemOpen };
    bool animatable(Animates, const QWidget *) const;
    void stopAnimate(Animates, QWidget *);
    void startAnimate(Animates, QWidget *);
    static ThemeDrawState getDrawState(QStyle::State flags);

    bool focusable(const QWidget *) const;

    bool doAnimate(Animates);
    inline int animateSpeed(Animates) const { return 33; }

    struct PolicyState {
        static QMap<const QWidget*, QMacStyle::FocusRectPolicy> focusMap;
        static QMap<const QWidget*, QMacStyle::WidgetSizePolicy> sizeMap;
        static void watchObject(const QObject *o);
    };

    // HITheme-based functions
    void HIThemePolish(QWidget *w);
    void HIThemeUnpolish(QWidget *w);
    void HIThemeDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                              const QWidget *w = 0) const;
    void HIThemeDrawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p,
                            const QWidget *w = 0) const;
    QRect HIThemeSubElementRect(QStyle::SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;
    void HIThemeDrawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                   QPainter *p, const QWidget *w = 0) const;
    QStyle::SubControl HIThemeHitTestComplexControl(QStyle::ComplexControl cc,
                                              const QStyleOptionComplex *opt,
                                              const QPoint &pt, const QWidget *w = 0) const;
    QRect HIThemeSubControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                        QStyle::SubControl sc, const QWidget *w = 0) const;
    void HIThemeAdjustButtonSize(QStyle::ContentsType ct, QSize &sz, const QWidget *w);
    int HIThemePixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                           const QWidget *widget) const;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    void HIThemeDrawColorlessButton(const HIRect &macRect, const HIThemeButtonDrawInfo &bdi,
                                       QPainter *p, const QStyleOption *opt) const;
#endif

    // Appearance Manager-based functions
    void AppManPolish(QWidget *w);
    void AppManUnpolish(QWidget *w);

    void AppManDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                             const QWidget *w = 0) const;
    void AppManDrawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p,
                           const QWidget *w = 0) const;
    QRect AppManSubElementRect(QStyle::SubElement r, const QStyleOption *opt, const QWidget *widget = 0) const;
    void AppManDrawComplexControl(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                  QPainter *p, const QWidget *w = 0) const;
    QStyle::SubControl AppManHitTestComplexControl(QStyle::ComplexControl cc,
                                             const QStyleOptionComplex *opt, const QPoint &pt,
                                             const QWidget *w = 0) const;
    QRect AppManSubControlRect(QStyle::ComplexControl cc, const QStyleOptionComplex *opt,
                                       QStyle::SubControl sc, const QWidget *w = 0) const;
    void AppManAdjustButtonSize(QStyle::ContentsType ct, QSize &sz, const QWidget *w);
    int AppManPixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                          const QWidget *widget) const;
    void AppManDrawColorlessButton(const Rect &macRect, const ThemeButtonKind bkind,
                                      const ThemeButtonDrawInfo &bdi, QPainter *p,
                                      const QStyleOption *opt) const;
    void drawPantherTab(const QStyleOptionTab *tab, QPainter *p, const QWidget *w = 0) const;

protected:
    bool eventFilter(QObject *, QEvent *);
    void timerEvent(QTimerEvent *);

private slots:
    void startAnimationTimer();

public:
    bool useHITheme;
    QPointer<QPushButton> defaultButton; //default pushbuttons
    int timerID;
    QList<QPointer<QWidget> > progressBars; //existing progressbars that need animation

    struct ButtonState {
        int frame;
        enum { ButtonDark, ButtonLight } dir;
    } buttonState;
    UInt8 progressFrame;
    QPointer<QFocusFrame> focusWidget;
    CFAbsoluteTime defaultButtonStart;
    QMacStyle *q;
};

QPixmap qt_mac_convert_iconref(IconRef icon, int width, int height);
QMap<const QWidget*, QMacStyle::FocusRectPolicy> QMacStylePrivate::PolicyState::focusMap;
QMap<const QWidget*, QMacStyle::WidgetSizePolicy> QMacStylePrivate::PolicyState::sizeMap;

class QMacStylePrivateObjectWatcher : public QObject
{
    Q_OBJECT
public:
    QMacStylePrivateObjectWatcher(QObject *p) : QObject(p) {}
public slots:
    void destroyedObject(QObject *o);
};

#include "qmacstyle_mac.moc"

/*****************************************************************************
  External functions
 *****************************************************************************/
extern CGContextRef qt_mac_cg_context(const QPaintDevice *); //qpaintdevice_mac.cpp
extern QPixmap qt_mac_convert_iconref(IconRef, int, int); //qpixmap_mac.cpp
extern QRegion qt_mac_convert_mac_region(HIShapeRef); //qregion_mac.cpp
RgnHandle qt_mac_get_rgn(); //qregion_mac.cpp
void qt_mac_dispose_rgn(RgnHandle r); //qregion_mac.cpp
extern QPaintDevice *qt_mac_safe_pdev; //qapplication_mac.cpp

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
static inline int qt_mac_hitheme_tab_version()
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4)
        return 1;
#endif
    return 0;
}

static inline HIRect qt_hirectForQRect(const QRect &convertRect, QPainter *p = 0,
                                       bool useOffset = true, const QRect &rect = QRect())
{
    int x, y;
    int offset = 0;
    if (useOffset)
        offset = 1;
    if (p) {
        QPoint pt = domap(p, convertRect.topLeft());
        x = pt.x();
        y = pt.y();
    } else {
        x = convertRect.x();
        y = convertRect.y();
    }
    HIRect retRect = CGRectMake(x + rect.x(), y + rect.y(),
                                convertRect.width() - offset - rect.width(),
                                convertRect.height() - offset - rect.height());
    return retRect;
}

static inline const QRect qt_qrectForHIRect(const HIRect &hirect)
{
    return QRect(QPoint(int(hirect.origin.x), int(hirect.origin.y)),
                 QSize(int(hirect.size.width), int(hirect.size.height)));
}

static inline bool qt_mac_is_metal(const QWidget *w)
{
    for (; w; w = w->parentWidget()) {
        if (w->testAttribute(Qt::WA_MacMetalStyle))
            return true;
        if (w->isWindow())
            break;
    }
    return false;
}

static int qt_mac_aqua_get_metric(ThemeMetric met)
{
    SInt32 ret;
    GetThemeMetric(met, &ret);
    return ret;
}

enum QAquaWidgetSize { QAquaSizeLarge, QAquaSizeSmall, QAquaSizeMini, QAquaSizeUnknown };

static QSize qt_aqua_get_known_size(QStyle::ContentsType ct, const QWidget *widg, QSize szHint,
                                    QAquaWidgetSize sz)
{
    QSize ret(-1, -1);
    if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3 && sz == QAquaSizeMini)
        return ret;

    if (sz != QAquaSizeSmall && sz != QAquaSizeLarge && sz != QAquaSizeMini) {
        qDebug("Not sure how to return this...");
        return ret;
    }
    if (widg && widg->testAttribute(Qt::WA_SetFont)) //if you're using a custom font, no constraints for you!
        return ret;

    if (ct == QStyle::CT_CustomBase && widg) {
        if (qobject_cast<const QPushButton *>(widg))
            ct = QStyle::CT_PushButton;
        else if (qobject_cast<const QRadioButton *>(widg))
            ct = QStyle::CT_RadioButton;
        else if (qobject_cast<const QCheckBox *>(widg))
            ct = QStyle::CT_CheckBox;
        else if (qobject_cast<const QComboBox *>(widg))
            ct = QStyle::CT_ComboBox;
        else if (qobject_cast<const QToolButton *>(widg))
            ct = QStyle::CT_ToolButton;
        else if (qobject_cast<const QSlider *>(widg))
            ct = QStyle::CT_Slider;
        else if (qobject_cast<const QProgressBar *>(widg))
            ct = QStyle::CT_ProgressBar;
        else if (qobject_cast<const QLineEdit *>(widg))
            ct = QStyle::CT_LineEdit;
        else if (qobject_cast<const QHeaderView *>(widg)
#ifdef QT3_SUPPORT
                 || widg->inherits("Q3Header")
#endif
                )
            ct = QStyle::CT_HeaderSection;
        else if (qobject_cast<const QMenuBar *>(widg)
#ifdef QT3_SUPPORT
                || widg->inherits("Q3MenuBar")
#endif
               )
            ct = QStyle::CT_MenuBar;
        else if (qobject_cast<const QSizeGrip *>(widg))
            ct = QStyle::CT_SizeGrip;
        else
            return ret;
    }

    if (ct == QStyle::CT_PushButton) {
        const QPushButton *psh = static_cast<const QPushButton *>(widg);
        int minw = -1;
        // Aqua Style guidelines restrict the size of OK and Cancel buttons to 68 pixels.
        // However, this doesn't work for German, therefore only do it for English,
        // I suppose it would be better to do some sort of lookups for languages
        // that like to have really long words.
        if (psh->text() == "OK" || psh->text() == "Cancel")
            minw = 69;
        if (psh->text().contains('\n'))
            ret = QSize(minw, -1);
        else if (sz == QAquaSizeLarge)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricPushButtonHeight));
        else if (sz == QAquaSizeSmall)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricSmallPushButtonHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if (sz == QAquaSizeMini)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricMiniPushButtonHeight));
#endif
#if 0 //Not sure we are applying the rules correctly for RadioButtons/CheckBoxes --Sam
    } else if (ct == QStyle::CT_RadioButton) {
        QRadioButton *rdo = static_cast<QRadioButton *>(widg);
        // Exception for case where multiline radiobutton text requires no size constrainment
        if (rdo->text().find('\n') != -1)
            return ret;
        if (sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricRadioButtonHeight));
        else if (sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallRadioButtonHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if (sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniRadioButtonHeight));
#endif
    } else if (ct == QStyle::CT_CheckBox) {
        if (sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricCheckBoxHeight));
        else if (sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallCheckBoxHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if (sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniCheckBoxHeight));
#endif
#endif
    } else if (ct == QStyle::CT_SizeGrip) {
        if (sz == QAquaSizeLarge || sz == QAquaSizeSmall) {
            Rect r;
            Point p = { 0, 0 };
            ThemeGrowDirection dir = kThemeGrowRight | kThemeGrowDown;
            if (QApplication::isRightToLeft())
                dir = kThemeGrowLeft | kThemeGrowDown;
            if (GetThemeStandaloneGrowBoxBounds(p, dir, sz != QAquaSizeSmall, &r) == noErr)
                ret = QSize(r.right - r.left, r.bottom - r.top);
        }
    } else if (ct == QStyle::CT_ComboBox) {
        if (sz == QAquaSizeLarge ||
            (sz != QAquaSizeLarge && QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)) {
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricPopupButtonHeight));
        } else if (sz == QAquaSizeSmall) {
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallPopupButtonHeight));
        }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if (sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniPopupButtonHeight));
#endif

    } else if (ct == QStyle::CT_ToolButton && sz == QAquaSizeSmall) {
        int width = 0, height = 0;
        if (szHint == QSize(-1, -1)) { //just 'guess'..
            const QToolButton *bt = static_cast<const QToolButton *>(widg);
            if (!bt->icon().isNull()) {
                QSize iconSize = bt->iconSize();
                QSize pmSize = bt->icon().actualSize(QSize(32, 32), QIcon::Normal);
                width = qMax(width, qMax(iconSize.width(), pmSize.width()));
                height = qMax(height, qMax(iconSize.height(), pmSize.height()));
            }
            if (!bt->text().isNull() && bt->toolButtonStyle() != Qt::ToolButtonIconOnly) {
                int text_width = bt->fontMetrics().width(bt->text()),
                   text_height = bt->fontMetrics().height();
                if (bt->toolButtonStyle() == Qt::ToolButtonTextUnderIcon) {
                    width = qMax(width, text_width);
                    height += text_height;
                } else {
                    width += text_width;
                    width = qMax(height, text_height);
                }
            }
        } else {
            width = szHint.width();
            height = szHint.height();
        }
        width =  qMax(20, width +  5); //border
        height = qMax(20, height + 5); //border
        ret = QSize(width, height);
    } else if (ct == QStyle::CT_Slider) {
        int w = -1;
        const QSlider *sld = static_cast<const QSlider *>(widg);
        if (sz == QAquaSizeLarge) {
            if (sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricHSliderHeight);
                if (sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricVSliderWidth);
                if (sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricVSliderTickWidth);
            }
        } else if (sz == QAquaSizeSmall) {
            if (sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricSmallHSliderHeight);
                if (sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricSmallHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricSmallVSliderWidth);
                if (sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricSmallVSliderTickWidth);
            }
        }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if (sz == QAquaSizeMini) {
            if (sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricMiniHSliderHeight);
                if (sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricMiniHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricMiniVSliderWidth);
                if (sld->tickPosition() != QSlider::NoTicks)
                    w += qt_mac_aqua_get_metric(kThemeMetricMiniVSliderTickWidth);
            }
        }
#endif
        if (sld->orientation() == Qt::Horizontal)
            ret.setHeight(w);
        else
            ret.setWidth(w);
    } else if (ct == QStyle::CT_ProgressBar) {
        if (sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricLargeProgressBarThickness));
        else if (sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricNormalProgressBarThickness));
    } else if (ct == QStyle::CT_LineEdit) {
        if (!widg || !qobject_cast<QComboBox *>(widg->parentWidget())) {
            //should I take into account the font dimentions of the lineedit? -Sam
            if (sz == QAquaSizeLarge)
                ret = QSize(-1, 22);
            else
                ret = QSize(-1, 19);
        }
    } else if (ct == QStyle::CT_HeaderSection) {
        if (sz == QAquaSizeLarge && (widg && widg->parentWidget() &&
                                     (qobject_cast<const QTreeView *>(widg->parentWidget())
#ifdef QT3_SUPPORT
                        || widg->parentWidget()->inherits("Q3ListView")
#endif
                        )))
           ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricListHeaderHeight));
    } else if (ct == QStyle::CT_MenuBar) {
        if (sz == QAquaSizeLarge) {
            SInt16 size;
            if (!GetThemeMenuBarHeight(&size))
                ret = QSize(-1, size);
        }
    }
    return ret;
}


#if defined(QMAC_QAQUASTYLE_SIZE_CONSTRAIN) || defined(DEBUG_SIZE_CONSTRAINT)
static QAquaWidgetSize qt_aqua_guess_size(const QWidget *widg, QSize large, QSize small, QSize mini)
{
    if (large == QSize(-1, -1)) {
        if (small != QSize(-1, -1))
            return QAquaSizeSmall;
        if (mini != QSize(-1, -1))
            return QAquaSizeMini;
        return QAquaSizeUnknown;
    } else if (small == QSize(-1, -1)) {
        if (mini != QSize(-1, -1))
            return QAquaSizeMini;
        return QAquaSizeLarge;
    } else if (mini == QSize(-1, -1)) {
        return QAquaSizeLarge;
    }

#ifndef QT_NO_MAINWINDOW
    if (qobject_cast<QDockWidget *>(widg->window()) || qgetenv("QWIDGET_ALL_SMALL")) {
        //if (small.width() != -1 || small.height() != -1)
        return QAquaSizeSmall;
    } else if (qgetenv("QWIDGET_ALL_MINI")) {
        return QAquaSizeMini;
    }
#endif

#if 0
    /* Figure out which size we're closer to, I just hacked this in, I haven't
       tested it as it would probably look pretty strange to have some widgets
       big and some widgets small in the same window?? -Sam */
    int large_delta=0;
    if (large.width() != -1) {
        int delta = large.width() - widg->width();
        large_delta += delta * delta;
    }
    if (large.height() != -1) {
        int delta = large.height() - widg->height();
        large_delta += delta * delta;
    }
    int small_delta=0;
    if (small.width() != -1) {
        int delta = small.width() - widg->width();
        small_delta += delta * delta;
    }
    if (small.height() != -1) {
        int delta = small.height() - widg->height();
        small_delta += delta * delta;
    }
    int mini_delta=0;
    if (mini.width() != -1) {
        int delta = mini.width() - widg->width();
        mini_delta += delta * delta;
    }
    if (mini.height() != -1) {
        int delta = mini.height() - widg->height();
        mini_delta += delta * delta;
    }
    if (mini_delta < small_delta && mini_delta < large_delta)
        return QAquaSizeMini;
    else if (small_delta < large_delta)
        return QAquaSizeSmall;
#endif
    return QAquaSizeLarge;
}
#endif

QAquaWidgetSize qt_aqua_size_constrain(const QWidget *widg,
                                       QStyle::ContentsType ct = QStyle::CT_CustomBase,
                                       QSize szHint=QSize(-1, -1), QSize *insz = 0)
{
#if defined(QMAC_QAQUASTYLE_SIZE_CONSTRAIN) || defined(DEBUG_SIZE_CONSTRAINT)
    if (!widg) {
        if (insz)
            *insz = QSize();
        if (qgetenv("QWIDGET_ALL_SMALL"))
            return QAquaSizeSmall;
        if (qgetenv("QWIDGET_ALL_MINI"))
            return QAquaSizeMini;
        return QAquaSizeUnknown;
    }
    QSize large = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeLarge),
          small = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeSmall),
          mini  = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeMini);
    bool guess_size = false;
    QAquaWidgetSize ret = QAquaSizeUnknown;
    QMacStyle *macStyle = qobject_cast<QMacStyle *>(widg->style());
    if (macStyle) {
        QMacStyle::WidgetSizePolicy wsp = macStyle->widgetSizePolicy(widg);
        if (wsp == QMacStyle::SizeDefault)
            guess_size = true;
        else if (wsp == QMacStyle::SizeMini)
            ret = QAquaSizeMini;
        else if (wsp == QMacStyle::SizeSmall)
            ret = QAquaSizeSmall;
        else if (wsp == QMacStyle::SizeLarge)
            ret = QAquaSizeLarge;
    }
    if (guess_size)
        ret = qt_aqua_guess_size(widg, large, small, mini);

    QSize *sz = 0;
    if (ret == QAquaSizeSmall)
        sz = &small;
    else if (ret == QAquaSizeLarge)
        sz = &large;
    else if (ret == QAquaSizeMini)
        sz = &mini;
    if (insz)
        *insz = sz ? *sz : QSize(-1, -1);
#ifdef DEBUG_SIZE_CONSTRAINT
    if (sz) {
        const char *size_desc = "Unknown";
        if (sz == &small)
            size_desc = "Small";
        else if (sz == &large)
            size_desc = "Large";
        else if (sz == &mini)
            size_desc = "Mini";
        qDebug("%s - %s: %s taken (%d, %d) [%d, %d]",
               widg ? widg->objectName().toLatin1().constData() : "*Unknown*",
               widg ? widg->className() : "*Unknown*", size_desc, widg->width(), widg->height(),
               sz->width(), sz->height());
    }
#endif
    return ret;
#else
    if (insz)
        *insz = QSize();
    Q_UNUSED(widg);
    Q_UNUSED(ct);
    Q_UNUSED(szHint);
    return QAquaSizeUnknown;
#endif
}

//utility to figure out the size (from the painter)
static QAquaWidgetSize qt_mac_get_size_for_painter(QPainter *p)
{
    if (p && p->device()->devType() == QInternal::Widget)
        return qt_aqua_size_constrain(static_cast<QWidget *>(p->device()));
    return qt_aqua_size_constrain(0);
}

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
static void getSliderInfo(QStyle::ComplexControl cc, const QStyleOptionSlider *slider,
                          HIThemeTrackDrawInfo *tdi, const QWidget *needToRemoveMe)
{
    memset(tdi, 0, sizeof(HIThemeTrackDrawInfo)); // We don't get it all for some reason or another...
    tdi->version = qt_mac_hitheme_version;
    tdi->reserved = 0;
    tdi->filler1 = 0;
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
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            if (isScrollbar)
                tdi->kind = kThemeMiniScrollBar;
            else
                tdi->kind = kThemeMiniSlider;
            break;
        }
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
    if (slider->upsideDown)
        tdi->attributes |= kThemeTrackRightToLeft;
    tdi->enableState = slider->state & QStyle::State_Enabled ? kThemeTrackActive
                                                             : kThemeTrackDisabled;
    if (!(slider->state & QStyle::State_Active))
        tdi->enableState = kThemeTrackInactive;
    if (!isScrollbar) {
        if (slider->state & QStyle::QStyle::State_HasFocus)
            tdi->attributes |= kThemeTrackHasFocus;
        if (slider->tickPosition == QSlider::NoTicks || slider->tickPosition == QSlider::TicksBothSides)
            tdi->trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickPosition == QSlider::TicksAbove)
            tdi->trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi->trackInfo.slider.thumbDir = kThemeThumbDownward;
    } else {
        tdi->trackInfo.scrollbar.viewsize = slider->pageStep;
    }
}
#endif

static void getSliderInfo(QStyle::ComplexControl cc, const QStyleOptionSlider *slider,
                          const QPainter *p, ThemeTrackDrawInfo *tdi, const QWidget *needToRemove)
{
    memset(tdi, 0, sizeof(ThemeTrackDrawInfo));
    tdi->filler1 = 0;
    bool isScrollbar = (cc == QStyle::CC_ScrollBar);
    switch (qt_aqua_size_constrain(needToRemove)) {
    case QAquaSizeUnknown:
    case QAquaSizeLarge:
        if (isScrollbar)
            tdi->kind = kThemeMediumScrollBar;
        else
            tdi->kind = kThemeMediumSlider;
        break;
    case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
    // Grr... this is dumb
    if (!p) {
        tdi->bounds = *qt_glb_mac_rect(slider->rect);
    } else {
        tdi->bounds = *qt_glb_mac_rect(slider->rect, p);
    }
    tdi->min = slider->minimum;
    tdi->max = slider->maximum;
    tdi->value = slider->sliderPosition;
    tdi->attributes = kThemeTrackShowThumb;
    if (slider->state & QStyle::State_HasFocus)
        tdi->attributes |= kThemeTrackHasFocus;
    if (slider->orientation == Qt::Horizontal)
        tdi->attributes |= kThemeTrackHorizontal;
    if (slider->upsideDown)
        tdi->attributes |= kThemeTrackRightToLeft;
    tdi->enableState = slider->state & QStyle::State_Enabled ? kThemeTrackActive
                                                             : kThemeTrackDisabled;
    if (!(slider->state & QStyle::State_Active))
        tdi->enableState = kThemeTrackDisabled;
    if (!isScrollbar) {
        if (slider->tickPosition == QSlider::NoTicks || slider->tickPosition == QSlider::TicksBothSides)
            tdi->trackInfo.slider.thumbDir = kThemeThumbPlain;
        else if (slider->tickPosition == QSlider::TicksAbove)
            tdi->trackInfo.slider.thumbDir = kThemeThumbUpward;
        else
            tdi->trackInfo.slider.thumbDir = kThemeThumbDownward;
    } else {
        tdi->trackInfo.scrollbar.viewsize = slider->pageStep;
    }
}

QMacStylePrivate::QMacStylePrivate(QMacStyle *style)
    : useHITheme(false), timerID(-1), progressFrame(0), q(style)
{
#if !defined(QMAC_NO_COREGRAPHICS) && (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3 && !qgetenv("QT_MAC_USE_APPMANAGER")
            && !qgetenv("QT_MAC_USE_QUICKDRAW"))
        useHITheme = true;
#endif
    defaultButtonStart = CFAbsoluteTimeGetCurrent();
    memset(&buttonState, 0, sizeof(ButtonState));
}

bool QMacStylePrivate::animatable(QMacStylePrivate::Animates as, const QWidget *w) const
{
    if (as == AquaPushButton) {
        if (static_cast<const QPushButton *>(w) == defaultButton) {
            return true;
        }
    } else if (as == AquaProgressBar) {
        if (progressBars.contains((const_cast<QWidget *>(w))))
            return true;
    }
    return false;
}

void QMacStylePrivate::stopAnimate(QMacStylePrivate::Animates as, QWidget *w)
{
    if (as == AquaPushButton && defaultButton) {
        QPushButton *tmp = defaultButton;
        defaultButton = 0;
        if (tmp->isVisible())
            tmp->update();
    } else if (as == AquaProgressBar) {
        progressBars.removeAll(w);
    }
}

void QMacStylePrivate::startAnimate(QMacStylePrivate::Animates as, QWidget *w)
{
    if (as == AquaPushButton)
        defaultButton = static_cast<QPushButton *>(w);
    else if (as == AquaProgressBar)
        progressBars.append(w);
    startAnimationTimer();
}

void QMacStylePrivate::startAnimationTimer()
{
    if ((defaultButton || !progressBars.isEmpty()) && timerID <= -1)
        timerID = startTimer(animateSpeed(AquaListViewItemOpen));
}

bool QMacStylePrivate::focusable(const QWidget *w) const
{
    QMacStyle::FocusRectPolicy fp = q->focusRectPolicy(w);
    if (fp == QMacStyle::FocusEnabled)
        return true;
    else if (fp == QMacStyle::FocusDisabled)
        return false;
    const QLineEdit *le = qobject_cast<const QLineEdit *>(w);
    return (w && !w->isWindow() && w->parentWidget() &&
            (qobject_cast<const QAbstractSpinBox *>(w))
             || (le && qobject_cast<const QAbstractSpinBox*>(le->parentWidget()))
             || (le && !qobject_cast<const QComboBox *>(le->parentWidget()))
             || (qobject_cast<const QTextEdit *>(w) && !static_cast<const QTextEdit *>(w)->isReadOnly())
#ifdef QT3_SUPPORT
             || w->inherits("QListBox") || w->inherits("QListView")
#endif
           );
}

enum { TabNormalLeft, TabNormalMid, TabNormalRight, TabSelectedActiveLeft,
       TabSelectedActiveMid, TabSelectedActiveRight, TabSelectedInactiveLeft,
       TabSelectedInactiveMid, TabSelectedInactiveRight, TabSelectedActiveGraphiteLeft,
       TabSelectedActiveGraphiteMid, TabSelectedActiveGraphiteRight,
       TabPressedLeft, TabPressedMid, TabPressedRight };

static const char * const * const PantherTabXpms[] = {
                                    qt_mac_tabnrm_left,
                                    qt_mac_tabnrm_mid,
                                    qt_mac_tabnrm_right,
                                    qt_mac_tabselected_active_left,
                                    qt_mac_tabselected_active_mid,
                                    qt_mac_tabselected_active_right,
                                    qt_mac_tabselected_inactive_left,
                                    qt_mac_tabselected_inactive_mid,
                                    qt_mac_tabselected_inactive_right,
                                    qt_mac_tab_selected_active_graph_left,
                                    qt_mac_tab_selected_active_graph_mid,
                                    qt_mac_tab_selected_active_graph_right,
                                    qt_mac_tab_press_left,
                                    qt_mac_tab_press_mid,
                                    qt_mac_tab_press_right};

void qt_mac_draw_tab(QPainter *p, const QWidget *w, const QRect &ir, ThemeTabStyle tts,
                     ThemeTabDirection ttd)
{
    if (ir.height() > kThemeLargeTabHeightMax) {
        QPixmap tabPix(ir.width(), kThemeLargeTabHeightMax);
        QPainter pixPainter(&tabPix);
        if (w)
            pixPainter.fillRect(QRect(QPoint(0, 0), tabPix.size()),
                                w->palette().brush(w->backgroundRole()));
        else
            tabPix.fill(QColor(255, 255, 255, 255));

        Rect pixRect = *qt_glb_mac_rect(QRect(0, 0, ir.width(), kThemeLargeTabHeightMax),
                                        &pixPainter, false);
        qt_mac_set_port(&pixPainter);
        DrawThemeTab(&pixRect, tts, ttd, 0, 0);
        p->drawPixmap(ir, tabPix);
    } else {
        qt_mac_set_port(p);
        DrawThemeTab(qt_glb_mac_rect(ir, p, false), tts, ttd, 0, 0);
    }
}

void QMacStylePrivate::drawPantherTab(const QStyleOptionTab *tabOpt, QPainter *p,
                                      const QWidget *) const
{
    QString tabKey = QLatin1String("$qt_mac_style_tab_");
    int pantherTabStart;
    int pantherTabMid;
    int pantherTabEnd;

    ThemeTabDirection ttd = getTabDirection(tabOpt->shape);

    if (tabOpt->state & QStyle::State_Selected) {
        if (!(tabOpt->state & QStyle::State_Active)) {
            pantherTabStart = TabSelectedInactiveLeft;
        } else {
            // Draw into a pixmap to determine which version we use, Aqua or Graphite.
            QPixmap tabPix(20, 20);
            QPainter pixPainter(&tabPix);
            qt_mac_draw_tab(&pixPainter, 0, QRect(0, 0, 20, 20), kThemeTabFront, kThemeTabNorth);
            pixPainter.end();
            const QRgb GraphiteColor = 0xffa7b1b9;
            QImage img = tabPix.toImage();
            if (img.pixel(10, 10) == GraphiteColor)
                pantherTabStart = TabSelectedActiveGraphiteLeft;
            else
                pantherTabStart = TabSelectedActiveLeft;
        }
    } else if (tabOpt->state & QStyle::State_Sunken) {
        pantherTabStart = TabPressedLeft;
    } else {
        pantherTabStart = TabNormalLeft;
    }


    bool doLine;
    bool verticalTabs = ttd == kThemeTabWest || ttd == kThemeTabEast;

    QStyleOptionTab::TabPosition tp = tabOpt->position;
    if (ttd == kThemeTabWest
        || ((ttd == kThemeTabNorth || ttd == kThemeTabSouth)
            && tabOpt->direction == Qt::RightToLeft)) {
        if (tp == QStyleOptionTab::Beginning)
            tp = QStyleOptionTab::End;
        else if (tp == QStyleOptionTab::End)
            tp = QStyleOptionTab::Beginning;
    }

    switch (tp) {
    default:  // Stupid GCC, being overly pedantic
    case QStyleOptionTab::Beginning:
        doLine = false;
        pantherTabMid = pantherTabEnd = pantherTabStart + 1;
        break;
    case QStyleOptionTab::Middle:
        doLine = true;
        pantherTabMid = pantherTabEnd = ++pantherTabStart;
        break;
    case QStyleOptionTab::End:
        doLine = true;
        pantherTabMid = ++pantherTabStart;
        pantherTabEnd = pantherTabMid + 1;
        break;
    case QStyleOptionTab::OnlyOneTab:
        doLine = false;
        pantherTabMid = pantherTabStart + 1;
        pantherTabEnd = pantherTabMid + 1;
        break;
    }

    QPixmap pmStart;
    if (!QPixmapCache::find(tabKey + QString::number(pantherTabStart), pmStart)) {
        pmStart = QPixmap(PantherTabXpms[pantherTabStart]);
        QPixmapCache::insert(tabKey + QString::number(pantherTabStart), pmStart);
    }

    QPixmap pmMid;
    if (!QPixmapCache::find(tabKey + QString::number(pantherTabMid), pmMid)) {
        pmMid = QPixmap(PantherTabXpms[pantherTabMid]);
        QPixmapCache::insert(tabKey + QString::number(pantherTabMid), pmMid);
    }

    QPixmap pmEnd;
    if (!QPixmapCache::find(tabKey + QString::number(pantherTabEnd), pmEnd)) {
        pmEnd = QPixmap(PantherTabXpms[pantherTabEnd]);
        QPixmapCache::insert(tabKey + QString::number(pantherTabEnd), pmEnd);
    }
    QRect tr = tabOpt->rect;
    if (verticalTabs) {
        p->save();
        int newX, newY, newRot;
        if (tabOpt->shape == QTabBar::RoundedEast || tabOpt->shape == QTabBar::TriangularEast) {
            newX = tr.width();
            newY = tr.y();
            newRot = 90;
        } else {
            newX = 0;
            newY = tr.y() + tr.height();
            newRot = -90;
        }
        tr.setRect(0, 0, tr.height(), tr.width());
        QMatrix m;
        if (ttd == kThemeTabEast) {
            // It's lame but Apple inverts these on the East side.
            m.scale(-1, 1);
            m.translate(-tabOpt->rect.width(), 0);
        }
        m.translate(newX, newY);
        m.rotate(newRot);
        p->setMatrix(m, true);
    }

    int x = tr.x();
    int y = tr.y();
    int endX = x + tr.width() - pmEnd.width();

    p->drawPixmap(x, y, pmStart.width(), tr.height(), pmStart);
    if (doLine) {
        QPen oldPen = p->pen();
        p->setPen(QColor(0, 0, 0, 0x35));
        p->drawLine(x, y + (verticalTabs ? 0 : 1), x, tr.height() - 2);
    }

    for (x = x + pmStart.width(); x < endX; x += pmMid.width())
        p->drawPixmap(x, y, pmMid.width(), tr.height(), pmMid);
    p->drawPixmap(endX, y, pmEnd.width(), tr.height(), pmEnd);
    if (verticalTabs)
        p->restore();
}

bool QMacStylePrivate::addWidget(QWidget *w)
{
    //already knew of it
    if (static_cast<QPushButton*>(w) == defaultButton
            || progressBars.contains(static_cast<QProgressBar*>(w)))
        return false;

    if (QPushButton *btn = qobject_cast<QPushButton *>(w)) {
        btn->installEventFilter(this);
        if (btn->isDefault() || (btn->autoDefault() && btn->hasFocus()))
            startAnimate(AquaPushButton, btn);
        return true;
    } else {
        bool isProgressBar = (qobject_cast<QProgressBar *>(w)
#ifdef QT3_SUPPORT
                || w->inherits("Q3ProgressBar")
#endif
            );
        if (isProgressBar) {
            w->installEventFilter(this);
            startAnimate(AquaProgressBar, w);
            return true;
        }
    }
    return false;
}

void QMacStylePrivate::removeWidget(QWidget *w)
{
    QPushButton *btn = qobject_cast<QPushButton *>(w);
    if (btn && btn == defaultButton) {
        stopAnimate(AquaPushButton, btn);
    } else if (qobject_cast<QProgressBar *>(w)
#ifdef QT3_SUPPORT
            || w->inherits("Q3ProgressBar")
#endif
            ) {
        stopAnimate(AquaProgressBar, w);
    }
}

ThemeDrawState QMacStylePrivate::getDrawState(QStyle::State flags)
{
    ThemeDrawState tds = kThemeStateActive;
    if (flags & QStyle::State_Sunken) {
        tds = kThemeStatePressed;
    } else if (flags & QStyle::State_Active) {
        if (!(flags & QStyle::State_Enabled))
            tds = kThemeStateUnavailable;
    } else {
        if (flags & QStyle::State_Enabled)
            tds = kThemeStateInactive;
        else
            tds = kThemeStateUnavailableInactive;
    }
    return tds;
}

void QMacStylePrivate::PolicyState::watchObject(const QObject *o)
{
    static QPointer<QMacStylePrivateObjectWatcher> watcher;
    if (!watcher)
        watcher = new QMacStylePrivateObjectWatcher(0);
    QObject::connect(o, SIGNAL(destroyed(QObject*)), watcher, SLOT(destroyedObject(QObject*)));
}

void QMacStylePrivateObjectWatcher::destroyedObject(QObject *o)
{
    QMacStylePrivate::PolicyState::focusMap.remove(static_cast<QWidget *>(o));
    QMacStylePrivate::PolicyState::sizeMap.remove(static_cast<QWidget *>(o));
}

void QMacStylePrivate::timerEvent(QTimerEvent *)
{
    int animated = 0;
    if (defaultButton && defaultButton->isEnabled() && defaultButton->isVisibleTo(0)
       && (defaultButton->isDefault()
           || (defaultButton->autoDefault() && defaultButton->hasFocus()))
       && doAnimate(AquaPushButton)) {
        ++animated;
        defaultButton->update();
    }
    if (!progressBars.isEmpty()) {
        int i = 0;
        while (i < progressBars.size()) {
            QWidget *maybeProgress = progressBars.at(i);
            if (!maybeProgress) {
                progressBars.removeAt(i);
            } else {
                if (QProgressBar *pb = qobject_cast<QProgressBar *>(maybeProgress)) {
                    if (pb->maximum() == 0 || pb->value() > 0
                        && pb->value() < pb->maximum()) {
                        if (doAnimate(AquaProgressBar))
                            pb->update();
                    }
                }
#ifdef QT3_SUPPORT
                else {
                    // Watch me now...
                    QVariant progress = maybeProgress->property("progress");
                    QVariant totalSteps = maybeProgress->property("totalSteps");
                    if (progress.isValid() && totalSteps.isValid()) {
                        int intProgress = progress.toInt();
                        int intTotalSteps = totalSteps.toInt();
                        if (intTotalSteps == 0 || intProgress > 0 && intProgress < intTotalSteps) {
                            if (doAnimate(AquaProgressBar))
                                maybeProgress->update();
                        }
                    }
                }
#endif
                ++i;
            }
        }
        animated += i;
    }
    if (animated <= 0) {
        killTimer(timerID);
        timerID = -1;
    }
}

bool QMacStylePrivate::eventFilter(QObject *o, QEvent *e)
{
    //animate
    if (QProgressBar *pb = qobject_cast<QProgressBar *>(o)) {
        switch (e->type()) {
        default:
            break;
        case QEvent::Show:
            if (!progressBars.contains(pb))
                startAnimate(AquaProgressBar, pb);
            break;
        case QEvent::Hide:
            progressBars.removeAll(pb);
        }
    } else if (QPushButton *btn = qobject_cast<QPushButton *>(o)) {
        switch (e->type()) {
        default:
            break;
        case QEvent::FocusIn:
            if (btn->autoDefault())
                startAnimate(AquaPushButton, btn);
            break;
        case QEvent::Hide:
            if (btn == defaultButton)
                stopAnimate(AquaPushButton, btn);
            break;
        case QEvent::MouseButtonPress:
            // It is very confusing to keep the button pulsing, so just stop the animation.
            stopAnimate(AquaPushButton, btn);
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::FocusOut:
        case QEvent::Show: {
            QList<QPushButton *> list = qFindChildren<QPushButton *>(btn->window());
            for (int i = 0; i < list.size(); ++i) {
                QPushButton *pBtn = list.at(i);
                if ((e->type() == QEvent::FocusOut
                     && (pBtn->isDefault() || (pBtn->autoDefault() && pBtn->hasFocus()))
                     && pBtn != btn)
                    || ((e->type() == QEvent::Show || e->type() == QEvent::MouseButtonRelease)
                        && pBtn->isDefault())) {
                    if (pBtn->window()->isActiveWindow())
                        startAnimate(AquaPushButton, pBtn);
                    break;
                }
            }
            break; }
        }
    }
    return false;
}

bool QMacStylePrivate::doAnimate(QMacStylePrivate::Animates as)
{
    if (as == AquaPushButton && !useHITheme) {
        const int MaxFrames = QSysInfo::MacintoshVersion == QSysInfo::MV_10_2 ? 13 : 15;
        if (buttonState.frame == MaxFrames && buttonState.dir == ButtonState::ButtonDark)
            buttonState.dir = ButtonState::ButtonLight;
        else if (!buttonState.frame && buttonState.dir == ButtonState::ButtonLight)
            buttonState.dir = ButtonState::ButtonDark;
        buttonState.frame += ((buttonState.dir == ButtonState::ButtonDark) ? 1 : -1);
    } else if (as == AquaProgressBar) {
        ++progressFrame;
    } else if (as == AquaListViewItemOpen) {
        // To be revived later...
    }
    return true;
}

void QMacStylePrivate::HIThemePolish(QWidget *w)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    addWidget(w);
    QPixmap px;
    if (qt_mac_is_metal(w)) {
        px = QPixmap(200, 200);
        HIThemeBackgroundDrawInfo bginfo;
        bginfo.version = qt_mac_hitheme_version;
        bginfo.state = kThemeStateActive;
        bginfo.kind = kThemeBackgroundMetal;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawBackground(&rect, &bginfo, QCFType<CGContextRef>(qt_mac_cg_context(&px)),
                              kHIThemeOrientationNormal);
    }

    if (::qobject_cast<QMenu*>(w)) {
        px = QPixmap(200, 200);
        HIThemeMenuDrawInfo mtinfo;
        mtinfo.version = qt_mac_hitheme_version;
        mtinfo.menuType = kThemeMenuTypePopUp;
        HIRect rect = CGRectMake(0, 0, px.width(), px.height());
        HIThemeDrawMenuBackground(&rect, &mtinfo, QCFType<CGContextRef>(qt_mac_cg_context(&px)),
                                  kHIThemeOrientationNormal);
        w->setWindowOpacity(0.95);
    }
    if (!px.isNull()) {
        QPalette pal = w->palette();
        QBrush background(px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
    } else if (QRubberBand *rubber = qobject_cast<QRubberBand*>(w)) {
        rubber->setWindowOpacity(0.25);
    }
    q->QWindowsStyle::polish(w);
#else
    Q_UNUSED(w);
#endif
}

void QMacStylePrivate::HIThemeUnpolish(QWidget *w)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    removeWidget(w);
    if (::qobject_cast<QMenu*>(w) || qt_mac_is_metal(w)) {
        QPalette pal = w->palette();
        QPixmap tmp;
        QBrush background(tmp);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
        w->setWindowOpacity(1.0);
    }
    if (::qobject_cast<QRubberBand*>(w))
        w->setWindowOpacity(1.0);
    q->QWindowsStyle::unpolish(w);
#else
    Q_UNUSED(w);
#endif
}

void QMacStylePrivate::AppManDrawColorlessButton(const Rect &macRect,
                                                    const ThemeButtonKind bkind,
                                                    const ThemeButtonDrawInfo &bdi, QPainter *p,
                                                    const QStyleOption *opt) const
{
    int x, y, width, height;
    if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
        x = cmb->rect.x() + (cmb->editable ? 0 : 2);
        y = cmb->rect.y() + (cmb->editable ? 0 : 2);
        width = macRect.right - macRect.left;
        height = (macRect.bottom - macRect.top) + (cmb->editable ? 2 : 0);
    } else {
        x = opt->rect.x();
        y = opt->rect.y();
        width = macRect.right - macRect.left + 1;
        height = macRect.bottom - macRect.top + 1;
    }
    QPixmap pm(width, height);
    QString key = QLatin1String("$qt_mac_style_am_ctb_") + QString::number(bkind) + QLatin1Char('_')
                  + QString::number(bdi.value) + QLatin1Char('_') + QString::number(width)
                  + QLatin1Char('_') + QString::number(height);
    if (!QPixmapCache::find(key, pm)) {
        QPainter pixPainter(&pm);
        pixPainter.fillRect(0, 0, pm.width(), pm.height(), opt->palette.brush(QPalette::Background));
        qt_mac_set_port(&pixPainter);
        Rect pixRect = *qt_glb_mac_rect(QRect(0, 0, pm.width(), pm.height()), &pixPainter);
        DrawThemeButton(&pixRect, bkind, &bdi, 0, 0, 0, 0);
        QImage img = pm.toImage();

        for (int y = 0; y < img.height(); ++y) {
            QRgb *scanline = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (int x = 0; x < img.width(); ++x) {
                uint pixel = scanline[x];
                int distance = qAbs(qRed(pixel) - qGreen(pixel))
                    + qAbs(qRed(pixel) - qBlue(pixel))
                    + qAbs(qGreen(pixel) - qBlue(pixel));
                if (distance > 20) {
                    int tmp;
                    if (qRed(pixel) > qGreen(pixel) && qRed(pixel) > qBlue(pixel))
                        tmp = qRed(pixel);
                    else if (qGreen(pixel) > qRed(pixel) && qGreen(pixel) > qBlue(pixel))
                        tmp = qGreen(pixel);
                    else
                        tmp = qBlue(pixel);
                    pixel = qRgba(tmp, tmp, tmp, qAlpha(pixel));
                    scanline[x] = pixel;
                }
            }
        }
        pm = QPixmap::fromImage(img);
        QPixmapCache::insert(key, pm);
    }
    p->drawPixmap(x, y, width, height, pm);
}

#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
void QMacStylePrivate::HIThemeDrawColorlessButton(const HIRect &macRect,
                                                     const HIThemeButtonDrawInfo &bdi,
                                                     QPainter *p, const QStyleOption *opt) const
{
    int xoff = 0,
        yoff = 0,
        extraWidth = 0,
        extraHeight = 0,
        finalyoff = 0;
    if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
        yoff = combo->editable ? 3 : 2;
        extraWidth = 1;
        extraHeight = yoff;
    } else {
        extraHeight = 1;
        finalyoff = -1;
    }

    int width = int(macRect.size.width) + extraWidth;
    int height = int(macRect.size.height) + extraHeight;

    QString key = QLatin1String("$qt_mac_style_ctb_") + QString::number(bdi.kind) + QLatin1Char('_')
                  + QString::number(bdi.value) + QLatin1Char('_') + QString::number(width)
                  + QLatin1Char('_') + QString::number(height);
    QPixmap pm;
    if (!QPixmapCache::find(key, pm)) {
        int bytesPerLine = width * 4;
        int size = bytesPerLine * height;
        void *data = calloc(1, size);
        QCFType<CGColorSpaceRef> colorspace = CGColorSpaceCreateDeviceRGB();
        QCFType<CGContextRef> cg2 = CGBitmapContextCreate(data, width, height, 8, bytesPerLine,
                                                          colorspace, kCGImageAlphaPremultipliedFirst);
        HIRect newRect = CGRectMake(xoff, yoff, macRect.size.width, macRect.size.height);
        HIThemeDrawButton(&newRect, &bdi, cg2, kHIThemeOrientationInverted, 0);
        QImage img(width, height, QImage::Format_ARGB32);
        for (int y = 0; y < height; ++y) {
            QRgb *scanline = reinterpret_cast<QRgb *>(static_cast<char *>(data) + y * bytesPerLine);
            for (int x = 0; x < width; ++x) {
                uint pixel = scanline[x];
                int distance = qAbs(qRed(pixel) - qGreen(pixel)) + qAbs(qRed(pixel) - qBlue(pixel))
                                    + qAbs(qGreen(pixel) - qBlue(pixel));
                if (distance > 20) {
                    int tmp;
                    if (qRed(pixel) > qGreen(pixel) && qRed(pixel) > qBlue(pixel))
                        tmp = qRed(pixel);
                    else if (qGreen(pixel) > qRed(pixel) && qGreen(pixel) > qBlue(pixel))
                        tmp = qGreen(pixel);
                    else
                        tmp = qBlue(pixel);
                    pixel = qRgba(tmp, tmp, tmp, qAlpha(pixel));
                    scanline[x] = pixel;
                }
            }
            memcpy(img.scanLine(y), scanline, bytesPerLine);
        }
        free(data);

        pm = QPixmap::fromImage(img);
        QPixmapCache::insert(key, pm);
    }
    p->drawPixmap(int(macRect.origin.x), int(macRect.origin.y) + finalyoff, width, height, pm);
}
#endif

void QMacStylePrivate::HIThemeDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt,
                                            QPainter *p, const QWidget *w) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeDrawState tds = getDrawState(opt->state);
    QMacCGContext cg(p);
    switch (pe) {
    case QStyle::PE_Q3CheckListExclusiveIndicator:
    case QStyle::PE_Q3CheckListIndicator:
    case QStyle::PE_IndicatorRadioButton:
    case QStyle::PE_IndicatorCheckBox: {
    bool drawColorless = (!(opt->state & QStyle::State_Active))
                                    && opt->palette.currentColorGroup() == QPalette::Active;
        HIThemeButtonDrawInfo bdi;
        bdi.version = qt_mac_hitheme_version;
        bdi.state = tds;
        if (drawColorless && tds == kThemeStateInactive)
            bdi.state = kThemeStateActive;
        bdi.adornment = kThemeDrawIndicatorOnly;
        if (opt->state & QStyle::State_HasFocus
                && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
            bdi.adornment |= kThemeAdornmentFocus;
        bool isRadioButton = (pe == QStyle::PE_Q3CheckListExclusiveIndicator
                              || pe == QStyle::PE_IndicatorRadioButton);
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
        if (opt->state & QStyle::State_NoChange)
            bdi.value = kThemeButtonMixed;
        else if (opt->state & QStyle::State_On)
            bdi.value = kThemeButtonOn;
        else
            bdi.value = kThemeButtonOff;
        HIRect macRect = qt_hirectForQRect(opt->rect, p);
        if (!drawColorless)
            HIThemeDrawButton(&macRect, &bdi, cg, kHIThemeOrientationNormal, 0);
        else
            HIThemeDrawColorlessButton(macRect, bdi, p, opt);
        break; }
    case QStyle::PE_IndicatorArrowUp:
    case QStyle::PE_IndicatorArrowDown:
    case QStyle::PE_IndicatorArrowRight:
    case QStyle::PE_IndicatorArrowLeft: {
        HIThemePopupArrowDrawInfo pdi;
        pdi.version = qt_mac_hitheme_version;
        pdi.state = tds;
        switch (pe) {
        case QStyle::PE_IndicatorArrowUp:
            pdi.orientation = kThemeArrowUp;
            break;
        case QStyle::PE_IndicatorArrowDown:
            pdi.orientation = kThemeArrowDown;
            break;
        case QStyle::PE_IndicatorArrowRight:
            pdi.orientation = kThemeArrowRight;
            break;
        case QStyle::PE_IndicatorArrowLeft:
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
    case QStyle::PE_FrameFocusRect:
        // Use the our own focus widget stuff.
        break;
    case QStyle::PE_IndicatorBranch: {
        if (!(opt->state & QStyle::State_Children))
            break;
        HIThemeButtonDrawInfo bi;
        bi.version = qt_mac_hitheme_version;
        bi.state = tds;
        if (tds == kThemeStateInactive && opt->palette.currentColorGroup() == QPalette::Active)
            bi.state = kThemeStateActive;
        if (opt->state & QStyle::State_Sunken)
            bi.state |= kThemeStatePressed;
        bi.kind = kThemeDisclosureButton;
        bi.value = opt->state & QStyle::State_Open ? kThemeDisclosureDown : kThemeDisclosureRight;
        bi.adornment = kThemeAdornmentNone;
        HIRect hirect = qt_hirectForQRect(opt->rect); // ### passing the painter causes bad stuff in Q3ListView...
        HIThemeDrawButton(&hirect, &bi, cg, kHIThemeOrientationNormal, 0);
        break; }
    case QStyle::PE_IndicatorHeaderArrow:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            if (w && (qobject_cast<QTreeView *>(w->parentWidget())
#ifdef QT3_SUPPORT
                        || w->parentWidget()->inherits("Q3ListView")
#endif
                ))
                break; // ListView-type header is taken care of.
            q->drawPrimitive(header->state & QStyle::State_UpArrow ? QStyle::PE_IndicatorArrowUp : QStyle::PE_IndicatorArrowDown, header, p, w);
        }
        break;
    case QStyle::PE_FrameGroupBox:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            HIThemeGroupBoxDrawInfo gdi;
            gdi.version = qt_mac_hitheme_version;
            gdi.state = tds;
            if (w && qobject_cast<QGroupBox *>(w->parentWidget()))
                gdi.kind = kHIThemeGroupBoxKindSecondary;
            else
                gdi.kind = kHIThemeGroupBoxKindPrimary;
            HIRect hirect = qt_hirectForQRect(frame->rect, p);
            HIThemeDrawGroupBox(&hirect, &gdi, cg, kHIThemeOrientationNormal);
        }
        break;
    case QStyle::PE_Frame:
    case QStyle::PE_FrameLineEdit:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (frame->state & QStyle::State_Sunken) {
                QColor baseColor(frame->palette.background().color());
                HIThemeFrameDrawInfo fdi;
                fdi.version = qt_mac_hitheme_version;
                fdi.state = tds;
                SInt32 frame_size;
                if (pe == QStyle::PE_FrameLineEdit) {
                    fdi.kind = kHIThemeFrameTextFieldSquare;
                    GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
                } else {
                    baseColor = QColor(150, 150, 150); //hardcoded since no query function --Sam
                    fdi.kind = kHIThemeFrameListBox;
                    GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
                }
                fdi.isFocused = (frame->state & QStyle::State_HasFocus);
                int lw = frame->lineWidth;
                if (lw <= 0)
                    lw = q->pixelMetric(QStyle::PM_DefaultFrameWidth, frame, w);
                { //clear to base color
                    p->save();
                    p->setPen(QPen(baseColor, lw));
                    p->setBrush(Qt::NoBrush);
                    p->drawRect(frame->rect);
                    p->restore();
                }
                HIRect hirect = qt_hirectForQRect(frame->rect, p, false,
                                                  QRect(frame_size, frame_size,
                                                        frame_size * 2, frame_size * 2));

                HIThemeDrawFrame(&hirect, &fdi, cg, kHIThemeOrientationNormal);
                break;
            } else {
                q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
                break;
            }
        }
    case QStyle::PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf
                = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            HIRect hirect = qt_hirectForQRect(twf->rect, p);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4) {
                HIThemeTabPaneDrawInfo tpdi;
                tpdi.version = qt_mac_hitheme_tab_version();
                tpdi.state = tds;
                tpdi.direction = getTabDirection(twf->shape);
                tpdi.size = kHIThemeTabSizeNormal;
                if (tpdi.version == 1) {
                    tpdi.kind = kHIThemeTabKindNormal;
                    tpdi.adornment = kHIThemeTabPaneAdornmentNormal;
                }
                HIThemeDrawTabPane(&hirect, &tpdi, cg, kHIThemeOrientationNormal);
            } else
#endif
            {
                HIThemeGroupBoxDrawInfo gdi;
                gdi.version = qt_mac_hitheme_version;
                gdi.state = tds;
                gdi.kind = kHIThemeGroupBoxKindSecondary;
                HIThemeDrawGroupBox(&hirect, &gdi, cg, kHIThemeOrientationNormal);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
#else
    q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
#endif
}

void QMacStylePrivate::HIThemeDrawControl(QStyle::ControlElement ce, const QStyleOption *opt,
                                          QPainter *p, const QWidget *w) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeDrawState tds = getDrawState(opt->state);
    QMacCGContext cg(p);
    switch (ce) {
    case QStyle::CE_FocusFrame: {
        SInt32 fo;
        GetThemeMetric(kThemeMetricFocusRectOutset, &fo);
        HIRect hirect = CGRectMake(fo, fo, opt->rect.width() - 2 * fo,
                                   opt->rect.height() - 2 * fo);
        HIThemeDrawFocusRect(&hirect, true, QMacCGContext(p), kHIThemeOrientationNormal);
        break; }
    case QStyle::CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = ::qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            if (!(btn->state & (QStyle::State_Raised | QStyle::State_Sunken | QStyle::State_On)))
                break;
            bool drawColorless = btn->palette.currentColorGroup() == QPalette::Active;
            if (btn->state & QStyle::State_On)
                tds = kThemeStatePressed;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = tds;
            if (drawColorless && tds == kThemeStateInactive)
                bdi.state = kThemeStateActive;
            bdi.adornment = kThemeAdornmentNone;
            bdi.value = kThemeButtonOff;
            if ((btn->features & ((QStyleOptionButton::Flat | QStyleOptionButton::HasMenu)))
                || (btn->rect.width() < 50 || btn->rect.height() < 30))
                bdi.kind = kThemeBevelButton;
            else
                bdi.kind = kThemePushButton;
            if (btn->state & QStyle::State_HasFocus
                    && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment |= kThemeAdornmentFocus;
            if (btn->features & QStyleOptionButton::DefaultButton && animatable(AquaPushButton, w)) {
                bdi.adornment |= kThemeAdornmentDefault;
                bdi.animation.time.start = defaultButtonStart;
                bdi.animation.time.current = CFAbsoluteTimeGetCurrent();
                if (timerID <= -1) {
                    QTimer::singleShot(0, const_cast<QMacStylePrivate *>(this),
                                       SLOT(startAnimationTimer()));
                }
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
                int mbi = q->pixelMetric(QStyle::PM_MenuButtonIndicator, btn, w);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() / 2 - 5, mbi, ir.height() / 2);
                if (drawColorless && tds == kThemeStateInactive)
                    newBtn.state |= QStyle::State_Active;

                q->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, p, w);
            }
        }
        break;
    case QStyle::CE_MenuItem:
    case QStyle::CE_MenuEmptyArea:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(mi->rect, opt->palette.background());

            int tabwidth = mi->tabWidth;
            int maxpmw = mi->maxIconWidth;
            bool active = mi->state & QStyle::State_Selected;
            bool enabled = mi->state & QStyle::State_Enabled;
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
            QRect contentRect;
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                // First arg should be &menurect, but wacky stuff happens then.
                HIThemeDrawMenuSeparator(&itemRect, &itemRect, &mdi,
                                         cg, kHIThemeOrientationNormal);
                break;
            } else {
                HIRect cr;
                HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                    cg, kHIThemeOrientationNormal, &cr);
                if (ce == QStyle::CE_MenuEmptyArea)
                    break;
                contentRect = qt_qrectForHIRect(cr);
            }
            int xpos = contentRect.x() + 18;
            int checkcol = maxpmw;
            if (!enabled)
                p->setPen(mi->palette.text().color());
            else if (active)
                p->setPen(mi->palette.highlightedText().color());
            else
                p->setPen(mi->palette.buttonText().color());

            if (mi->checked) {
                // Use the HIThemeTextInfo foo to draw the check mark correctly, if we do it,
                // we somehow need to use a special encoding as it doesn't look right with our
                // drawText().
                HIThemeTextInfo tti;
                tti.version = qt_mac_hitheme_version;
                tti.state = tds;
                if (active && enabled)
                    tti.state = kThemeStatePressed;
                tti.fontID = kThemeMenuItemMarkFont;
                tti.horizontalFlushness = kHIThemeTextHorizontalFlushLeft;
                tti.verticalFlushness = kHIThemeTextVerticalFlushCenter;
                tti.options = kHIThemeTextBoxOptionNone;
                tti.truncationPosition = kHIThemeTextTruncationNone;
                tti.truncationMaxLines = 1;
                QCFString checkmark;
#if 0
                if (mi->checkType == QStyleOptionMenuItem::Exclusive)
                    checkmark = QString(QChar(kDiamondUnicode));
                else
#endif
                    checkmark = QString(QChar(kCheckUnicode));
                int mw = checkcol + macItemFrame;
                int mh = contentRect.height() - 2 * macItemFrame;
                int xp = contentRect.x();
                xp += macItemFrame;
                float outWidth, outHeight, outBaseline;
                HIThemeGetTextDimensions(checkmark, 0, &tti, &outWidth, &outHeight,
                                         &outBaseline);
                QRect r(xp, contentRect.y(), mw, mh);
                r.translate(0, p->fontMetrics().ascent() - int(outBaseline) + 1);
                HIRect bounds = qt_hirectForQRect(r);
                HIThemeDrawTextBox(checkmark, &bounds, &tti,
                                   cg, kHIThemeOrientationNormal);
            }
            if (!mi->icon.isNull()) {
                QIcon::Mode mode = (mi->state & QStyle::State_Enabled) ? QIcon::Normal
                                                                       : QIcon::Disabled;
                // Always be normal or disabled to follow the Mac style.
                QPixmap pixmap = mi->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(xpos, contentRect.y(), checkcol, contentRect.height());
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }

            QString s = mi->text;
            if (!s.isEmpty()) {
                int t = s.indexOf('\t');
                int text_flags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextHideMnemonic
                                 | Qt::TextSingleLine | Qt::AlignAbsolute;
                p->save();
                if (t >= 0) {
                    p->setFont(qt_app_fonts_hash()->value("QMenuItem", p->font()));
                    int xp = contentRect.right() - tabwidth - macRightBorder
                             - macItemHMargin - macItemFrame + 1;
                    p->drawText(xp, contentRect.y(), tabwidth, contentRect.height(), text_flags,
                                s.mid(t + 1));
                    s = s.left(t);
                }

                p->setFont(mi->font);
                const int xm = macItemFrame + maxpmw + macItemHMargin;
                p->drawText(xpos, contentRect.y(), contentRect.width() - xm - tabwidth + 1,
                            contentRect.height(), text_flags ^ Qt::AlignRight, s);
                p->restore();
            }
        }
        break;
    case QStyle::CE_MenuHMargin:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(mi->rect, opt->palette.background());

            HIRect menuRect = qt_hirectForQRect(mi->menuRect);
            HIRect itemRect = qt_hirectForQRect(mi->rect);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            if (!(opt->state & QStyle::State_Enabled))
                mdi.state = kThemeMenuDisabled;
            else if (opt->state & QStyle::State_Selected)
                mdi.state = kThemeMenuSelected;
            else
                mdi.state = kThemeMenuActive;
            if (ce == QStyle::CE_MenuScroller) {
                if (opt->state & QStyle::State_DownArrow)
                    mdi.itemType = kThemeMenuItemScrollDownArrow;
                else
                    mdi.itemType = kThemeMenuItemScrollUpArrow;
            } else {
                mdi.itemType = kThemeMenuItemPlain;
            }
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                cg,
                                kHIThemeOrientationNormal, 0);
            if (ce == QStyle::CE_MenuTearoff) {
                p->setPen(QPen(mi->palette.dark().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2 - 1);
                p->setPen(QPen(mi->palette.light().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2);
            }
        }
        break;
    case QStyle::CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            p->fillRect(mi->rect, opt->palette.background());

            HIRect menuRect = qt_hirectForQRect(mi->menuRect);
            HIRect itemRect = qt_hirectForQRect(mi->rect);
            HIThemeMenuItemDrawInfo mdi;
            mdi.version = qt_mac_hitheme_version;
            if (!(opt->state & QStyle::State_Enabled))
                mdi.state = kThemeMenuDisabled;
            else if ((opt->state & QStyle::State_Selected) && (opt->state & QStyle::State_Sunken))
                mdi.state = kThemeMenuSelected;
            else
                mdi.state = kThemeMenuActive;
            mdi.itemType = kThemeMenuItemPlain;
            HIThemeDrawMenuItem(&menuRect, &itemRect, &mdi,
                                cg, kHIThemeOrientationNormal, 0);

            if (!mi->icon.isNull()) {
                q->drawItemPixmap(p, mi->rect,
                                  Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip
                                  | Qt::TextSingleLine,
                                  mi->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize),
                          (mi->state & QStyle::State_Enabled) ? QIcon::Normal : QIcon::Disabled));
            } else {
                q->drawItemText(p, mi->rect,
                                Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip
                                | Qt::TextSingleLine,
                                mi->palette, mi->state & QStyle::State_Enabled,
                                mi->text, QPalette::ButtonText);
            }
        }
        break;
    case QStyle::CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            HIThemeMenuBarDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeMenuBarNormal;
            bdi.attributes = 0;
            HIRect hirect = qt_hirectForQRect(mi->rect);
            HIThemeDrawMenuBarBackground(&hirect, &bdi, cg,
                                         kHIThemeOrientationNormal);
            break;
        }
    case QStyle::CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            tdi.version = qt_mac_hitheme_version;
            tdi.reserved = 0;
            bool isIndeterminate = (pb->minimum == 0 && pb->maximum == 0);
            switch (qt_aqua_size_constrain(w)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                tdi.kind = !isIndeterminate ? kThemeLargeProgressBar
                                            : kThemeLargeIndeterminateBar;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    tdi.kind = !isIndeterminate ? kThemeMiniProgressBar
                                                : kThemeMiniIndeterminateBar;
                    break;
                }
#endif
            case QAquaSizeSmall:
                tdi.kind = !isIndeterminate ? kThemeProgressBar : kThemeIndeterminateBar;
                break;
            }
            tdi.bounds = qt_hirectForQRect(pb->rect);
            tdi.max = pb->maximum;
            tdi.min = pb->minimum;
            tdi.value = pb->progress;
            tdi.attributes = kThemeTrackHorizontal;
            tdi.trackInfo.progress.phase = progressFrame;
            if (!(pb->state & QStyle::State_Active))
                tdi.enableState = kThemeTrackInactive;
            else if (!(pb->state & QStyle::State_Enabled))
                tdi.enableState = kThemeTrackDisabled;
            else
                tdi.enableState = kThemeTrackActive;
            HIThemeDrawTrack(&tdi, 0, cg, kHIThemeOrientationNormal);
        }
        break;
    case QStyle::CE_ProgressBarLabel:
    case QStyle::CE_ProgressBarGroove:
        break;
    case QStyle::CE_TabBarTabShape:
        if (const QStyleOptionTab *tabOpt = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
            HIThemeTabDrawInfo tdi;
            tdi.version = 1;
            tdi.style = kThemeTabNonFront;
            bool selected = tabOpt->state & QStyle::State_Selected;
            if (selected) {
                if (!(tabOpt->state & QStyle::State_Active))
                    tdi.style = kThemeTabFrontUnavailable;
                else if (!(tabOpt->state & QStyle::State_Enabled))
                    tdi.style = kThemeTabFrontInactive;
                else
                    tdi.style = kThemeTabFront;
            } else if (!(tabOpt->state & QStyle::State_Active)) {
                tdi.style = kThemeTabNonFrontUnavailable;
            } else if (!(tabOpt->state & QStyle::State_Enabled)) {
                tdi.style = kThemeTabNonFrontInactive;
            } else if ((tabOpt->state & (QStyle::State_Sunken | QStyle::State_MouseOver))
                       == (QStyle::State_Sunken | QStyle::State_MouseOver)) {
                tdi.style = kThemeTabNonFrontPressed;
            }
            tdi.direction = getTabDirection(tabOpt->shape);
            bool verticalTabs = tdi.direction == kThemeTabWest || tdi.direction == kThemeTabEast;
            if (tabOpt->state & QStyle::State_HasFocus)
                tdi.adornment = kHIThemeTabAdornmentFocus;
            else
                tdi.adornment = kHIThemeTabAdornmentNone;
            QRect tabRect = tabOpt->rect;
            tdi.kind = kHIThemeTabKindNormal;
            if (!verticalTabs)
                tabRect.setY(tabRect.y() - 1);
            else
                tabRect.setX(tabRect.x() - 1);
            QStyleOptionTab::TabPosition tp = tabOpt->position;
            QStyleOptionTab::SelectedPosition sp = tabOpt->selectedPosition;
            if (tabOpt->direction == Qt::RightToLeft && !verticalTabs) {
                if (sp == QStyleOptionTab::NextIsSelected)
                    sp = QStyleOptionTab::PreviousIsSelected;
                else if (sp == QStyleOptionTab::PreviousIsSelected)
                    sp = QStyleOptionTab::NextIsSelected;
                switch (tp) {
                case QStyleOptionTab::Beginning:
                    tp = QStyleOptionTab::End;
                    break;
                case QStyleOptionTab::End:
                    tp = QStyleOptionTab::Beginning;
                    break;
                default:
                    break;
                }
            }
            switch (tp) {
            case QStyleOptionTab::Beginning:
                tdi.position = kHIThemeTabPositionFirst;
                tabRect.adjust(0, 0, !verticalTabs ? 1 : 0, verticalTabs ? 1 : 0);
                if (sp != QStyleOptionTab::NextIsSelected)
                    tdi.adornment |= kHIThemeTabAdornmentTrailingSeparator;
                break;
            case QStyleOptionTab::Middle:
                tdi.position = kHIThemeTabPositionMiddle;
                tabRect.adjust(0, 0, !verticalTabs ? 1 : 0, verticalTabs ? 1 : 0);
                if (selected)
                    tdi.adornment |= kHIThemeTabAdornmentLeadingSeparator;
                if (sp != QStyleOptionTab::NextIsSelected)  // Also when we're selected.
                    tdi.adornment |= kHIThemeTabAdornmentTrailingSeparator;
                break;
            case QStyleOptionTab::End:
                tdi.position = kHIThemeTabPositionLast;
                if (selected)
                    tdi.adornment |= kHIThemeTabAdornmentLeadingSeparator;
                break;
            case QStyleOptionTab::OnlyOneTab:
                tdi.position = kHIThemeTabPositionOnly;
                break;
            }
            HIRect hirect = qt_hirectForQRect(tabRect, p);
            HIThemeDrawTab(&hirect, &tdi, cg, kHIThemeOrientationNormal, 0);
#else
            drawPantherTab(tabOpt, p, w);
#endif
        }
        break;
    case QStyle::CE_SizeGrip: {
        HIThemeGrowBoxDrawInfo gdi;
        gdi.version = qt_mac_hitheme_version;
        gdi.state = tds;
        gdi.kind = kHIThemeGrowBoxKindNormal;
        gdi.direction = kThemeGrowRight | kThemeGrowDown;
        gdi.size = kHIThemeGrowBoxSizeNormal;
        HIPoint pt = CGPointMake(opt->rect.x(), opt->rect.y());
        HIThemeDrawGrowBox(&pt, &gdi, cg, kHIThemeOrientationNormal);
        break; }
    case QStyle::CE_Splitter: {
        HIThemeSplitterDrawInfo sdi;
        sdi.version = qt_mac_hitheme_version;
        sdi.state = tds;
        sdi.adornment = qt_mac_is_metal(w) ? kHIThemeSplitterAdornmentMetal
                                           : kHIThemeSplitterAdornmentNone;
        HIRect hirect = qt_hirectForQRect(opt->rect, p);
        HIThemeDrawPaneSplitter(&hirect, &sdi, cg, kHIThemeOrientationNormal);
        break; }
    case QStyle::CE_RubberBand:
        if (const QStyleOptionRubberBand *rubber = qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            QColor highlight(opt->palette.color(QPalette::Disabled, QPalette::Highlight));
            if(!rubber->opaque)
                highlight.setAlphaF(0.75);
            p->fillRect(opt->rect, highlight);
        }
        break;
    case QStyle::CE_HeaderSection:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            bool scaleHeader = false;
            SInt32 headerHeight = 0;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeStateActive;
            QStyle::State flags = header->state;
            QRect ir = header->rect;
            QWidget *pw = w ? w->parentWidget() : 0;
            if (pw && (qobject_cast<QTreeView *>(pw)
#ifdef QT3_SUPPORT
                       || pw->inherits("Q3ListView")
#endif
                )) {
                bdi.kind = kThemeListHeaderButton;
                GetThemeMetric(kThemeMetricListHeaderHeight, &headerHeight);
                if (ir.height() > headerHeight)
                    scaleHeader = true;
            } else {
                bdi.kind = kThemeBevelButton;
                if (p->font().bold())
                    flags |= QStyle::State_On;
                else
                    flags &= ~QStyle::State_On;
            }

            if (flags & QStyle::State_Active) {
                if (!(flags & QStyle::State_Enabled))
                    bdi.state = kThemeStateUnavailable;
                else if (flags & QStyle::State_Sunken)
                    bdi.state = kThemeStatePressed;
            } else {
                if (flags & QStyle::State_Enabled)
                    bdi.state = kThemeStateInactive;
                else
                    bdi.state = kThemeStateUnavailableInactive;
            }

            if (flags & QStyle::State_On)
                bdi.value = kThemeButtonOn;
            else
                bdi.value = kThemeButtonOff;

            bdi.adornment = kThemeAdornmentNone;
            if (bdi.kind == kThemeListHeaderButton
                && header->selectedPosition != QStyleOptionHeader::NotAdjacent) {
                if (header->selectedPosition == QStyleOptionHeader::PreviousIsSelected)
                    bdi.adornment = kThemeAdornmentHeaderButtonRightNeighborSelected;
                else if (header->selectedPosition == QStyleOptionHeader::PreviousIsSelected)
                    bdi.adornment = kThemeAdornmentHeaderButtonLeftNeighborSelected;
            }

            if (header->sortIndicator != QStyleOptionHeader::None) {
                bdi.value = kThemeButtonOn;
                if (header->sortIndicator == QStyleOptionHeader::SortUp)
                    bdi.adornment = kThemeAdornmentHeaderButtonSortUp;
            }
            if (flags & QStyle::State_HasFocus && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
                bdi.adornment = kThemeAdornmentFocus;
            // The ListViewHeader button is only drawn one size, so draw into a pixmap and scale it
            // Otherwise just draw it normally.
            if (scaleHeader) {
                QPixmap headerPix(ir.width(), headerHeight);
                QPainter pixPainter(&headerPix);
                QMacCGContext pixCG(&pixPainter);
                HIRect pixRect = CGRectMake(0, 0, ir.width(), headerHeight);
                HIThemeDrawButton(&pixRect, &bdi, pixCG, kHIThemeOrientationNormal, 0);
                p->drawPixmap(ir, headerPix);
            } else {
                HIRect hirect = qt_hirectForQRect(ir, p);
                HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawControl(ce, opt, p, w);
    }
#else
    q->QWindowsStyle::drawControl(ce, opt, p, w);
#endif
}

QRect QMacStylePrivate::HIThemeSubElementRect(QStyle::SubElement sr, const QStyleOption *opt, const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QRect r;
    switch (sr) {
    case QStyle::SE_ToolBoxTabContents:
        r = q->QCommonStyle::subElementRect(sr, opt, widget);
        break;
    case QStyle::SE_PushButtonContents:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            HIRect inRect = CGRectMake(btn->rect.x(), btn->rect.y(),
                                       btn->rect.width(), btn->rect.height());
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
                      int(qMin(qAbs(btn->rect.width() - 2 * outRect.origin.x), outRect.size.width)),
                      int(qMin(qAbs(btn->rect.height() - 2 * outRect.origin.y), outRect.size.height)));
        }
        break;
    case QStyle::SE_ProgressBarGroove:
    case QStyle::SE_ProgressBarLabel:
        break;
    case QStyle::SE_ProgressBarContents:
        r = opt->rect;
        break;
    default:
        r = q->QWindowsStyle::subElementRect(sr, opt, widget);
        break;
    }
    return r;
#else
    return q->QWindowsStyle::subElementRect(sr, opt, widget);
#endif
}

void QMacStylePrivate::HIThemeDrawComplexControl(QStyle::ComplexControl cc,
                                                 const QStyleOptionComplex *opt,
                                                 QPainter *p, const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeDrawState tds = getDrawState(opt->state);
    QMacCGContext cg(p);
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            if (cc == QStyle::CC_Slider) {
                if (slider->activeSubControls == QStyle::SC_SliderHandle)
                    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
                else if (slider->activeSubControls == QStyle::SC_SliderGroove)
                    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
            } else {
                if (slider->activeSubControls == QStyle::SC_ScrollBarSubLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                         | kThemeLeftOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                         | kThemeRightOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarSubPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarSlider)
                    tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            }
            HIRect macRect;
            bool tracking = slider->sliderPosition == slider->sliderValue;
            if (!tracking) {
                // Small optimization, the same as q->subControlRect
                QCFType<HIShapeRef> shape;
                HIThemeGetTrackThumbShape(&tdi, &shape);
                HIShapeGetBounds(shape, &macRect);
                tdi.value = slider->sliderValue;
            }
            HIThemeDrawTrack(&tdi, tracking ? 0 : &macRect, cg,
                             kHIThemeOrientationNormal);
            if (cc == QStyle::CC_Slider && slider->subControls & QStyle::SC_SliderTickmarks) {
                int numMarks;
                if (slider->tickInterval)
                        numMarks = ((slider->maximum - slider->minimum + 1)
                                        / slider->tickInterval) + 1;
                else
                    numMarks = ((slider->maximum - slider->minimum + 1) / slider->pageStep) + 1;
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
    case QStyle::CC_Q3ListView:
        if (const QStyleOptionQ3ListView *lv = qstyleoption_cast<const QStyleOptionQ3ListView *>(opt)) {
            if (lv->subControls & QStyle::SC_Q3ListView)
                q->QWindowsStyle::drawComplexControl(cc, lv, p, widget);
            if (lv->subControls & (QStyle::SC_Q3ListViewBranch | QStyle::SC_Q3ListViewExpand)) {
                int y = lv->rect.y();
                int h = lv->rect.height();
                int x = lv->rect.right() - 10;
                for (int i = 1; i < lv->items.size() && y < h; ++i) {
                    QStyleOptionQ3ListViewItem item = lv->items.at(i);
                    if (y + item.height > 0 && (item.childCount > 0
                        || (item.features & (QStyleOptionQ3ListViewItem::Expandable
                                            | QStyleOptionQ3ListViewItem::Visible))
                            == (QStyleOptionQ3ListViewItem::Expandable
                                | QStyleOptionQ3ListViewItem::Visible))) {
                        QStyleOption treeOpt(0);
                        treeOpt.rect.setRect(x, y + item.height / 2 - 4, 9, 9);
                        treeOpt.palette = lv->palette;
                        treeOpt.state = lv->state;
                        treeOpt.state |= QStyle::State_Children;
                        if (item.state & QStyle::State_Open)
                            treeOpt.state |= QStyle::State_Open;
                        q->drawPrimitive(QStyle::PE_IndicatorBranch, &treeOpt, p, widget);
                    }
                    y += item.totalHeight;
                }
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox newSB = *sb;
            if (sb->frame && (sb->subControls & QStyle::SC_SpinBoxFrame)) {
                QRect lineeditRect = q->subControlRect(QStyle::CC_SpinBox, sb,
                                                       QStyle::SC_SpinBoxFrame, widget);

                HIThemeFrameDrawInfo fdi;
                fdi.version = qt_mac_hitheme_version;
                fdi.state = tds;
                fdi.kind = kHIThemeFrameTextFieldSquare;
                fdi.isFocused = false;
                HIRect hirect = qt_hirectForQRect(lineeditRect, p, false);
                HIThemeDrawFrame(&hirect, &fdi, cg, kHIThemeOrientationNormal);
            }
            if (sb->subControls & (QStyle::SC_SpinBoxUp | QStyle::SC_SpinBoxDown)) {
                HIThemeButtonDrawInfo bdi;
                bdi.version = qt_mac_hitheme_version;
                QAquaWidgetSize aquaSize = qt_aqua_size_constrain(widget);
                switch (aquaSize) {
                    case QAquaSizeUnknown:
                    case QAquaSizeLarge:
                        bdi.kind = kThemeIncDecButton;
                        break;
                    case QAquaSizeMini:
                    case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                            if (aquaSize == QAquaSizeMini)
                                bdi.kind = kThemeIncDecButtonMini;
                            else
                                bdi.kind = kThemeIncDecButtonSmall;
                        } else {
                            bdi.kind = kThemeIncDecButton;
                        }
                        break;
#endif
                }
                if (!(sb->stepEnabled & (QAbstractSpinBox::StepUpEnabled
                                        | QAbstractSpinBox::StepDownEnabled)))
                    tds = kThemeStateUnavailable;
                if (sb->activeSubControls == QStyle::SC_SpinBoxDown
                    && (sb->state & QStyle::State_Sunken))
                    tds = kThemeStatePressedDown;
                else if (sb->activeSubControls == QStyle::SC_SpinBoxUp
                         && (sb->state & QStyle::State_Sunken))
                    tds = kThemeStatePressedUp;
                bdi.state = tds;
                if (!(sb->state & QStyle::State_Active)
                        && sb->palette.currentColorGroup() == QPalette::Active
                        && tds == kThemeStateInactive)
                    bdi.state = kThemeStateActive;
                bdi.value = kThemeButtonOff;
                if (sb->state & QStyle::State_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    bdi.adornment = kThemeAdornmentFocus;
                else
                    bdi.adornment = kThemeAdornmentNone;
                QRect updown = q->subControlRect(QStyle::CC_SpinBox, sb, QStyle::SC_SpinBoxUp,
                                                 widget);
                updown |= q->subControlRect(QStyle::CC_SpinBox, sb, QStyle::SC_SpinBoxDown, widget);
                HIRect newRect = qt_hirectForQRect(updown);
                QRect off_rct;
                HIRect outRect;
                HIThemeGetButtonBackgroundBounds(&newRect, &bdi, &outRect);
                off_rct.setRect(int(newRect.origin.x - outRect.origin.x),
                                int(newRect.origin.y - outRect.origin.y),
                                int(outRect.size.width - newRect.size.width),
                                int(outRect.size.height - newRect.size.height));
                newRect = qt_hirectForQRect(updown, p, false, off_rct);
                HIThemeDrawButton(&newRect, &bdi, cg, kHIThemeOrientationNormal, 0);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.adornment = kThemeAdornmentArrowLeftArrow;
            bdi.value = kThemeButtonOff;
            bool hasFocus = combo->state & QStyle::State_HasFocus;
            if (hasFocus)
                bdi.adornment = kThemeAdornmentFocus;
            bool drawColorless = combo->palette.currentColorGroup() == QPalette::Active
                                   && tds == kThemeStateInactive;
            if (combo->activeSubControls & QStyle::SC_ComboBoxArrow)
                bdi.state = kThemeStatePressed;
            else if (drawColorless)
                bdi.state = kThemeStateActive;
            else
                bdi.state = tds;

            QAquaWidgetSize aSize = qt_aqua_size_constrain(widget);
            switch (aSize) {
            case QAquaSizeMini:
            case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    if (aSize == QAquaSizeMini)
                        bdi.kind = combo->editable ? ThemeButtonKind(kThemeComboBoxMini)
                                                   : ThemeButtonKind(kThemePopupButtonMini);
                    else
                        bdi.kind = combo->editable ? ThemeButtonKind(kThemeComboBoxSmall)
                                                   : ThemeButtonKind(kThemePopupButtonSmall);
                } else {
                    bdi.kind = combo->editable ? kThemeComboBox : kThemePopupButton;
                }
                break;
#endif
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bdi.kind = combo->editable ? kThemeComboBox : kThemePopupButton;
                break;
            }
            HIRect hirect = qt_hirectForQRect(combo->rect, p);
            QRect off_rct;
            HIRect outRect;
            HIThemeGetButtonBackgroundBounds(&hirect, &bdi, &outRect);
            int offSet = (hasFocus && !combo->editable) ? -1 : 0;
            off_rct.setRect(int(hirect.origin.x - outRect.origin.x),
                            int(hirect.origin.y - outRect.origin.y + offSet),
                            int(outRect.size.width - hirect.size.width),
                            int(outRect.size.height - hirect.size.height + offSet));
            hirect = qt_hirectForQRect(combo->rect, p, false, off_rct);
            if (!drawColorless)
                HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
            else
                HIThemeDrawColorlessButton(hirect, bdi, p, opt);
        }
        break;
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *titlebar
                = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            wdi.version = qt_mac_hitheme_version;
            wdi.state = tds;
            wdi.windowType = QtWinType;
            wdi.titleHeight = titlebar->rect.height();
            wdi.titleWidth = titlebar->rect.width();
            wdi.attributes = kThemeWindowHasTitleText;
            if (titlebar->subControls & QStyle::SC_TitleBarCloseButton)
                wdi.attributes |= kThemeWindowHasCloseBox;
            if (titlebar->subControls & QStyle::SC_TitleBarMaxButton
                                        | QStyle::SC_TitleBarNormalButton)
                wdi.attributes |= kThemeWindowHasFullZoom;
            if (titlebar->subControls & QStyle::SC_TitleBarMinButton)
                wdi.attributes |= kThemeWindowHasCollapseBox;

            HIRect titleBarRect;
            HIRect tmpRect = qt_hirectForQRect(titlebar->rect, p);
            {
                QCFType<HIShapeRef> titleRegion;
                QRect newr = titlebar->rect;
                HIThemeGetWindowShape(&tmpRect, &wdi, kWindowTitleBarRgn, &titleRegion);
                HIShapeGetBounds(titleRegion, &tmpRect);
                newr.translate(newr.x() - int(tmpRect.origin.x), newr.y() - int(tmpRect.origin.y));
                titleBarRect = qt_hirectForQRect(newr, p);
            }
            HIThemeDrawWindowFrame(&titleBarRect, &wdi, cg, kHIThemeOrientationNormal, 0);
            if (titlebar->subControls & (QStyle::SC_TitleBarCloseButton
                                         | QStyle::SC_TitleBarMaxButton
                                         | QStyle::SC_TitleBarMinButton
                                         | QStyle::SC_TitleBarNormalButton)) {
                HIThemeWindowWidgetDrawInfo wwdi;
                wwdi.version = qt_mac_hitheme_version;
                wwdi.widgetState = tds;
                if (titlebar->state & QStyle::State_MouseOver)
                    wwdi.widgetState = kThemeStateRollover;
                wwdi.windowType = QtWinType;
                wwdi.attributes = wdi.attributes;
                wwdi.windowState = wdi.state;
                wwdi.titleHeight = wdi.titleHeight;
                wwdi.titleWidth = wdi.titleWidth;
                ThemeDrawState savedControlState = wwdi.widgetState;
                uint sc = QStyle::SC_TitleBarMinButton;
                ThemeTitleBarWidget tbw = kThemeWidgetCollapseBox;
                bool active = titlebar->state & QStyle::State_Active;
                while (sc <= QStyle::SC_TitleBarCloseButton) {
                    uint tmp = sc;
                    wwdi.widgetState = savedControlState;
                    wwdi.widgetType = tbw;
                    if (sc == QStyle::SC_TitleBarMinButton)
                        tmp |= QStyle::SC_TitleBarNormalButton;
                    if (active && (titlebar->activeSubControls & tmp)
                            && (titlebar->state & QStyle::State_Sunken))
                        wwdi.widgetState = kThemeStatePressed;
                    /*
                    if (titlebar->window() && titlebar->window()->isWindowModified()
                            && tbw == kThemeWidgetCloseBox)
                        wwdi.widgetType = kThemeWidgetDirtyCloseBox;
                            */
                    HIThemeDrawTitleBarWidget(&titleBarRect, &wwdi, cg, kHIThemeOrientationNormal);
                    p->paintEngine()->syncState();
                    sc = sc << 1;
                    tbw = tbw >> 1;
                }
            }
            p->paintEngine()->syncState();
            if (titlebar->subControls & QStyle::SC_TitleBarLabel) {
                int iw = 0;
                if (!titlebar->icon.isNull()) {
                    QCFType<HIShapeRef> titleRegion2;
                    HIThemeGetWindowShape(&titleBarRect, &wdi, kWindowTitleProxyIconRgn,
                                          &titleRegion2);
                    HIShapeGetBounds(titleRegion2, &tmpRect);
                    if (tmpRect.size.width != 1)
                        iw = titlebar->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize),
                                                   QIcon::Normal).width();
                }
                if (!titlebar->text.isEmpty()) {
                    p->save();
                    QCFType<HIShapeRef> titleRegion3;
                    HIThemeGetWindowShape(&titleBarRect, &wdi, kWindowTitleTextRgn, &titleRegion3);
                    HIShapeGetBounds(titleRegion3, &tmpRect);
                    p->setClipRect(qt_qrectForHIRect(tmpRect));
                    QRect br = p->clipRegion().boundingRect();
                    int x = br.x(),
                    y = br.y() + (titlebar->rect.height() / 2 - p->fontMetrics().height() / 2);
                    if (br.width() <= (p->fontMetrics().width(titlebar->text) + iw * 2))
                        x += iw;
                    else
                        x += br.width() / 2 - p->fontMetrics().width(titlebar->text) / 2;
                    if (iw)
                        p->drawPixmap(x - iw, y, titlebar->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize), QIcon::Normal));

                    p->drawText(x, y + p->fontMetrics().ascent(), titlebar->text);
                    p->restore();
                }
            }
        }
        break;
    case QStyle::CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            ThemeButtonKind bkind = kThemeBevelButton;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bkind = kThemeBevelButton;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    bkind = kThemeMiniBevelButton;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bkind = kThemeSmallBevelButton;
                break;
            }

            QRect button, menuarea;
            button   = q->subControlRect(cc, tb, QStyle::SC_ToolButton, widget);
            menuarea = q->subControlRect(cc, tb, QStyle::SC_ToolButtonMenu, widget);
            QStyle::State bflags = tb->state,
            mflags = tb->state;
            if (tb->subControls & QStyle::SC_ToolButton)
                bflags |= QStyle::State_Sunken;
            if (tb->subControls & QStyle::SC_ToolButtonMenu)
                mflags |= QStyle::State_Sunken;

            if (tb->subControls & QStyle::SC_ToolButton) {
                if (bflags & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Raised)) {
                    HIThemeButtonDrawInfo bdi;
                    bdi.version = qt_mac_hitheme_version;
                    bdi.state = tds;
                    bdi.adornment = kThemeAdornmentNone;
                    bdi.kind = bkind;
                    bdi.value = kThemeButtonOff;
                    if (tb->state & QStyle::State_HasFocus && QMacStyle::focusRectPolicy(widget)
                            != QMacStyle::FocusDisabled)
                        bdi.adornment |= kThemeAdornmentFocus;
                    if (tb->state & (QStyle::State_On | QStyle::State_Sunken))
                        bdi.value |= kThemeStatePressed;

                    QRect off_rct(0, 0, 0, 0);
                    HIRect myRect, macRect;
                    myRect = CGRectMake(tb->rect.x(), tb->rect.y(),
                                        tb->rect.width(), tb->rect.height());
                    HIThemeGetButtonBackgroundBounds(&myRect, &bdi, &macRect);
                    off_rct.setRect(int(myRect.origin.x - macRect.origin.x),
                                    int(myRect.origin.y - macRect.origin.y),
                                    int(macRect.size.width - myRect.size.width),
                                    int(macRect.size.height - myRect.size.height));

                    myRect = qt_hirectForQRect(button, p, false, off_rct);
                    HIThemeDrawButton(&myRect, &bdi, cg, kHIThemeOrientationNormal, 0);
                }
            }

            if (tb->subControls & QStyle::SC_ToolButtonMenu) {
                HIThemeButtonDrawInfo bdi;
                bdi.version = qt_mac_hitheme_version;
                bdi.state = tds;
                bdi.value = kThemeButtonOff;
                bdi.adornment = kThemeAdornmentNone;
                bdi.value = bkind;
                if (tb->state & QStyle::State_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    bdi.adornment |= kThemeAdornmentFocus;
                if (tb->state & (QStyle::State_On | QStyle::State_Sunken)
                                 || (tb->activeSubControls & QStyle::SC_ToolButtonMenu))
                    bdi.value |= kThemeStatePressed;
                HIRect hirect = qt_hirectForQRect(menuarea, p, false);
                HIThemeDrawButton(&hirect, &bdi, cg, kHIThemeOrientationNormal, 0);
                QRect r(menuarea.x() + ((menuarea.width() / 2) - 4), menuarea.height() - 8, 8, 8);
                HIThemePopupArrowDrawInfo padi;
                padi.version = qt_mac_hitheme_version;
                padi.state = tds;
                padi.orientation = kThemeArrowDown;
                padi.size = kThemeArrow7pt;
                hirect = qt_hirectForQRect(r, p);
                HIThemeDrawPopupArrow(&hirect, &padi, cg, kHIThemeOrientationNormal);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
#else
    q->QWindowsStyle::drawComplexControl(cc, opt, p, widget);
#endif
}

QStyle::SubControl QMacStylePrivate::HIThemeHitTestComplexControl(QStyle::ComplexControl cc,
                                                            const QStyleOptionComplex *opt,
                                                            const QPoint &pt,
                                                            const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QStyle::SubControl sc = QStyle::SC_None;
    switch (cc) {
    case QStyle::CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            ControlPartCode part;
            HIPoint pos = CGPointMake(pt.x(), pt.y());
            if (HIThemeHitTestTrack(&tdi, &pos, &part)) {
                if (part == kControlPageUpPart || part == kControlPageDownPart)
                    sc = QStyle::SC_SliderGroove;
                else
                    sc = QStyle::SC_SliderHandle;
            }
        }
        break;
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *sb = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            HIScrollBarTrackInfo sbi;
            sbi.version = qt_mac_hitheme_version;
            if (!(sb->state & QStyle::State_Active))
                sbi.enableState = kThemeTrackInactive;
            else if (sb->state & QStyle::State_Enabled)
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
                    sc = QStyle::SC_ScrollBarSubLine;
                else if (part == kControlDownButtonPart)
                    sc = QStyle::SC_ScrollBarAddLine;
            } else {
                HIThemeTrackDrawInfo tdi;
                getSliderInfo(cc, sb, &tdi, widget);
                if (HIThemeHitTestTrack(&tdi, &pos, &part)) {
                    if (part == kControlPageUpPart)
                        sc = QStyle::SC_ScrollBarSubPage;
                    else if (part == kControlPageDownPart)
                        sc = QStyle::SC_ScrollBarAddPage;
                    else
                        sc = QStyle::SC_ScrollBarSlider;
                }
            }
        }
        break;
/*
    I don't know why, but we only get kWindowContentRgn here, which isn't what we want at all.
    It would be very nice if this would work.
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
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
                    sc = QStyle::SC_TitleBarCloseButton;
                    break;
                case kWindowCollapseBoxRgn:
                    sc = QStyle::SC_TitleBarMinButton;
                    break;
                case kWindowZoomBoxRgn:
                    sc = QStyle::SC_TitleBarMaxButton;
                    break;
                case kWindowTitleTextRgn:
                    sc = QStyle::SC_TitleBarLabel;
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
        sc = q->QWindowsStyle::hitTestComplexControl(cc, opt, pt, widget);
    }
    return sc;
#else
    return q->QWindowsStyle::hitTestComplexControl(cc, opt, pt, widget);
#endif
}


QRect QMacStylePrivate::HIThemeSubControlRect(QStyle::ComplexControl cc,
                                                      const QStyleOptionComplex *opt,
                                                      QStyle::SubControl sc,
                                                      const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    QRect ret;
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            HIThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, &tdi, widget);
            HIRect macRect;
            QCFType<HIShapeRef> shape;
            bool scrollBar = cc == QStyle::CC_ScrollBar;
            if ((scrollBar && sc == QStyle::SC_ScrollBarSlider)
                    || (!scrollBar && sc == QStyle::SC_SliderHandle)) {
                HIThemeGetTrackThumbShape(&tdi, &shape);
                HIShapeGetBounds(shape, &macRect);
            } else if (!scrollBar && sc == QStyle::SC_SliderGroove) {
                HIThemeGetTrackBounds(&tdi, &macRect);
            } else if (sc == QStyle::SC_ScrollBarGroove) { // Only scrollbar parts available...
                HIThemeGetTrackDragRect(&tdi, &macRect);
            } else {
                ControlPartCode cpc;
                if (sc == QStyle::SC_ScrollBarSubPage || sc == QStyle::SC_ScrollBarAddPage)
                    cpc = sc == QStyle::SC_ScrollBarSubPage ? kControlPageDownPart
                                                            : kControlPageUpPart;
                else
                    cpc = sc == QStyle::SC_ScrollBarSubLine ? kControlUpButtonPart
                                                            : kControlDownButtonPart;
                HIThemeGetTrackPartBounds(&tdi, cpc, &macRect);
            }
            ret = qt_qrectForHIRect(macRect);
        }
        break;
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *titlebar
                = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            memset(&wdi, 0, sizeof(wdi));
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            wdi.titleHeight = titlebar->rect.height();
            wdi.titleWidth = titlebar->rect.width();
            wdi.attributes = kThemeWindowHasTitleText;
            if (titlebar->subControls & QStyle::SC_TitleBarCloseButton)
                wdi.attributes |= kThemeWindowHasCloseBox;
            if (titlebar->subControls & QStyle::SC_TitleBarMaxButton
                                        | QStyle::SC_TitleBarNormalButton)
                wdi.attributes |= kThemeWindowHasFullZoom;
            if (titlebar->subControls & QStyle::SC_TitleBarMinButton)
                wdi.attributes |= kThemeWindowHasCollapseBox;
            WindowRegionCode wrc = kWindowGlobalPortRgn;
            if (sc == QStyle::SC_TitleBarCloseButton)
                wrc = kWindowCloseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMinButton)
                wrc = kWindowCollapseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMaxButton)
                wrc = kWindowZoomBoxRgn;
            else if (sc == QStyle::SC_TitleBarLabel)
                wrc = kWindowTitleTextRgn;
            else if (sc == QStyle::SC_TitleBarSysMenu)
                ret.setRect(-1024, -1024, 10, q->pixelMetric(QStyle::PM_TitleBarHeight,
                                                             titlebar, widget));
            if (wrc != kWindowGlobalPortRgn) {
                QCFType<HIShapeRef> region;
                QRect tmpRect = titlebar->rect;
                HIRect titleRect = qt_hirectForQRect(tmpRect);
                HIThemeGetWindowShape(&titleRect, &wdi, kWindowTitleBarRgn, &region);
                HIShapeGetBounds(region, &titleRect);
                CFRelease(region);
                tmpRect.translate(tmpRect.x() - int(titleRect.origin.x),
                               tmpRect.y() - int(titleRect.origin.y));
                titleRect = qt_hirectForQRect(tmpRect);
                HIThemeGetWindowShape(&titleRect, &wdi, wrc, &region);
                HIShapeGetBounds(region, &titleRect);
                ret = qt_qrectForHIRect(titleRect);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *combo
                    = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            // Always figure out the edit field area
            HIRect hirect, outrect;
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.state = kThemeStateActive;
            bdi.kind = kThemePopupButton;
            bdi.value = kThemeButtonOff;
            bdi.adornment = kThemeAdornmentArrowLeftArrow;
            hirect = qt_hirectForQRect(combo->rect);
            HIThemeGetButtonContentBounds(&hirect, &bdi, &outrect);
            ret = qt_qrectForHIRect(outrect);
            if (combo->editable) {
                ret.adjust(-5, 2, 8, -3);
            } else {
                ret.adjust(0, -1, 0, 0);
            }
            switch (sc) {
            default:
                // I undo the layout that Windows rect has.
                ret = QStyle::visualRect(combo->direction, combo->rect,
                                         q->QWindowsStyle::subControlRect(cc, opt, sc, widget));
                break;
            case QStyle::SC_ComboBoxEditField:
                // ret = ret; <-- Already done
                break;
            case QStyle::SC_ComboBoxArrow:
                ret.setX(ret.x() + ret.width());
                ret.setWidth(combo->rect.width() - ret.width());
                break;
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 18,
            y = q->pixelMetric(QStyle::PM_SpinBoxFrameWidth, spin, widget),
            x = spin->rect.width() - spinner_w + y;
            ret.setRect(x, y, spinner_w, spin->rect.height() - y * 2);
            HIThemeButtonDrawInfo bdi;
            bdi.version = qt_mac_hitheme_version;
            bdi.kind = kThemeIncDecButton;
            QAquaWidgetSize aquaSize = qt_aqua_size_constrain(widget);
            switch (aquaSize) {
                case QAquaSizeUnknown:
                case QAquaSizeLarge:
                    bdi.kind = kThemeIncDecButton;
                    break;
                case QAquaSizeMini:
                case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                        if (aquaSize == QAquaSizeMini)
                            bdi.kind = kThemeIncDecButtonMini;
                        else
                            bdi.kind = kThemeIncDecButtonSmall;
                    } else {
                        bdi.kind = kThemeIncDecButton;
                    }
                    break;
#endif
            }
            bdi.state = kThemeStateActive;
            bdi.value = kThemeButtonOff;
            bdi.adornment = kThemeAdornmentNone;
            HIRect hirect = qt_hirectForQRect(ret);
            HIRect outRect;
            HIThemeGetButtonBackgroundBounds(&hirect, &bdi, &outRect);
            ret = qt_qrectForHIRect(outRect);
            switch (sc) {
            case QStyle::SC_SpinBoxUp:
                ret.setHeight(ret.height() / 2);
                break;
            case QStyle::SC_SpinBoxDown:
                ret.setY(ret.y() + ret.height() / 2);
                break;
            default:
                Q_ASSERT(0);
                break;
            }
            ret = QStyle::visualRect(spin->direction, spin->rect, ret);
        }
        break;
    default:
        ret = q->QWindowsStyle::subControlRect(cc, opt, sc, widget);
    }
    return ret;
#else
    return q->QWindowsStyle::subControlRect(cc, opt, sc, widget);
#endif
}

int QMacStylePrivate::HIThemePixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                                         const QWidget *widget) const
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    SInt32 ret = 0;
    switch(metric) {
    case QStyle::PM_TitleBarHeight:
        if (const QStyleOptionTitleBar *tb = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
            HIThemeWindowDrawInfo wdi;
            wdi.version = qt_mac_hitheme_version;
            wdi.state = kThemeStateActive;
            wdi.windowType = QtWinType;
            if (tb->titleBarState)
                wdi.attributes = kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                                  | kThemeWindowHasCollapseBox;
            else if (tb->titleBarFlags & Qt::WStyle_SysMenu)
                wdi.attributes = kThemeWindowHasCloseBox;
            else
                wdi.attributes = 0;
            wdi.titleHeight = tb->rect.height();
            wdi.titleWidth = tb->rect.width();
            QCFType<HIShapeRef> region;
            HIRect hirect = qt_hirectForQRect(tb->rect);
            HIThemeGetWindowShape(&hirect, &wdi, kWindowTitleBarRgn, &region);
            HIRect rect;
            HIShapeGetBounds(region, &rect);
            ret = int(rect.size.height);
        }
        break;
    default:
        ret = q->QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    }
    return ret;
#else
    return q->QWindowsStyle::pixelMetric(metric, opt, widget);
#endif
}

void QMacStylePrivate::HIThemeAdjustButtonSize(QStyle::ContentsType ct, QSize &sz,
                                               const QWidget *widget)
{
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
    ThemeButtonKind bkind = kThemePushButton;
    if (ct == QStyle::CT_ToolButton)
        bkind = kThemeBevelButton;
    if (qt_aqua_size_constrain(widget) == QAquaSizeSmall) {
        if (bkind == kThemeBevelButton)
            bkind = kThemeSmallBevelButton;
    } else if (ct == QStyle::CT_ComboBox) {
        bkind = kThemePopupButton;
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
#else
    Q_UNUSED(ct);
    Q_UNUSED(sz);
    Q_UNUSED(widget);
#endif
}

void QMacStylePrivate::AppManPolish(QWidget *w)
{
    addWidget(w);

#ifdef QMAC_DO_SECONDARY_GROUPBOXES
    if (w->parentWidget() && qobject_cast<QGroupBox*>(w->parentWidget())
            && !w->testAttribute(Qt::WA_SetPalette)
            && w->parentWidget()->parentWidget()
            && qobject_cast<QGroupBox*>(w->parentWidget()->parentWidget())) {
        QPalette pal = w->palette();
        QPixmap px(200, 200, 32);
        QColor pc(Qt::black);
        {
            QPainter p(&px);
            qt_mac_set_port(&p);
            Rect r; SetRect(&r, 0, 0, px.width(), px.height());
            ApplyThemeBackground(kThemeBackgroundSecondaryGroupBox, &r,
                                 kThemeStateActive, px.depth(), true);
            EraseRect(&r);
        }
        QBrush background(pc, px);
        pal.setBrush(QPalette::Background, background);
        pal.setBrush(QPalette::Button, background);
        w->setPalette(pal);
    }
#endif

    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_2) {
        if (qobject_cast<QGroupBox*>(w))
            w->setAttribute(Qt::WA_ContentsPropagated, true);
    }
        /*
    } else if (QDialogButtons *btns = qobject_cast<QDialogButtons*>(w)) {
        if (btns->buttonText(QDialogButtons::Help).isNull())
            btns->setButtonText(QDialogButtons::Help, "?");
            */
#ifndef QT_NO_MAINWINDOW
    else if (QToolBar *bar = qobject_cast<QToolBar*>(w)) {
        QLayout *layout = bar->layout();
        layout->setSpacing(0);
        layout->setMargin(0);
    }
#endif
    else if (QRubberBand *rubber = qobject_cast<QRubberBand*>(w)) {
        rubber->setWindowOpacity(0.25);
    } else if (QMenu *menu = qobject_cast<QMenu*>(w)) {
        menu->setWindowOpacity(0.95);
    }
    q->QWindowsStyle::polish(w);
}

void QMacStylePrivate::AppManUnpolish(QWidget *w)
{
    removeWidget(w);
    if (QRubberBand *rubber = qobject_cast<QRubberBand*>(w))
        rubber->setWindowOpacity(1.0);
    else if (qobject_cast<QMenu*>(w))
        w->setWindowOpacity(1.0);
}

void QMacStylePrivate::AppManDrawPrimitive(QStyle::PrimitiveElement pe, const QStyleOption *opt,
                                           QPainter *p, const QWidget *w) const
{
    ThemeDrawState tds = getDrawState(opt->state);
    switch (pe) {
    case QStyle::PE_Q3CheckListExclusiveIndicator:
    case QStyle::PE_Q3CheckListIndicator:
    case QStyle::PE_IndicatorRadioButton:
    case QStyle::PE_IndicatorCheckBox: {
        bool isRadioButton = (pe == QStyle::PE_Q3CheckListIndicator
                || pe == QStyle::PE_IndicatorRadioButton);
        ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentDrawIndicatorOnly };
        bool drawColorless = (!(opt->state & QStyle::State_Active))
                                    && opt->palette.currentColorGroup() == QPalette::Active;
        if (drawColorless && tds == kThemeStateInactive)
            info.state = kThemeStateActive;
        if (opt->state & QStyle::State_HasFocus
                && QMacStyle::focusRectPolicy(w) != QMacStyle::FocusDisabled)
            info.adornment |= kThemeAdornmentFocus;
        if (opt->state & QStyle::State_NoChange)
            info.value = kThemeButtonMixed;
        else if (opt->state & QStyle::State_On)
            info.value = kThemeButtonOn;
        ThemeButtonKind bkind;
        switch (qt_mac_get_size_for_painter(p)) {
            default:
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                if (isRadioButton)
                    bkind = kThemeRadioButton;
                else
                    bkind = kThemeCheckBox;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    if (isRadioButton)
                        bkind = kThemeMiniRadioButton;
                    else
                        bkind = kThemeMiniCheckBox;
                    break;
                }
#endif
            case QAquaSizeSmall:
                if (isRadioButton)
                    bkind = kThemeSmallRadioButton;
                else
                    bkind = kThemeSmallCheckBox;
                break;
        }
        if (!drawColorless) {
            qt_mac_set_port(p);
            DrawThemeButton(qt_glb_mac_rect(opt->rect, p, false), bkind, &info, 0, 0, 0, 0);
        } else {
            AppManDrawColorlessButton(*qt_glb_mac_rect(opt->rect, p, false), bkind, info, p, opt);
        }
        break; }
    case QStyle::PE_FrameFocusRect:
        break;     //This is not used because of the QAquaFocusWidget thingie..
    case QStyle::PE_IndicatorBranch:
        if (!(opt->state & QStyle::State_Children))
            break;
        ThemeButtonDrawInfo currentInfo;
        currentInfo.state = tds;
        if (tds == kThemeStateInactive && opt->palette.currentColorGroup() == QPalette::Active)
            currentInfo.state = kThemeStateActive;
        if (opt->state & QStyle::State_Sunken)
            currentInfo.state |= kThemeStatePressed;
        currentInfo.value = opt->state & QStyle::State_Open ? kThemeDisclosureDown
                                                            : kThemeDisclosureRight;
        currentInfo.adornment = kThemeAdornmentNone;
        qt_mac_set_port(p);
        DrawThemeButton(qt_glb_mac_rect(opt->rect, p), kThemeDisclosureButton, &currentInfo,
                        0, 0, 0, 0);
        break;
    case QStyle::PE_IndicatorHeaderArrow:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            if (w && (qobject_cast<QTreeView *>(w->parentWidget())
#ifdef QT3_SUPPORT
                        || w->parentWidget()->inherits("Q3ListView")
#endif
                ))
                break; // ListView-type header is taken care of.
            q->drawPrimitive(header->state & QStyle::State_UpArrow ? QStyle::PE_IndicatorArrowUp
                                                                   : QStyle::PE_IndicatorArrowDown,
                                                                   header, p, w);
        }
        break;
    case QStyle::PE_Frame:
    case QStyle::PE_FrameLineEdit:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            if (opt->state & QStyle::State_Sunken) {
                SInt32 frame_size;
                QColor baseColor(frame->palette.background().color());
                if (pe == QStyle::PE_FrameLineEdit) {
                    GetThemeMetric(kThemeMetricEditTextFrameOutset, &frame_size);
                } else {
                    baseColor = QColor(150, 150, 150); //hardcoded since no query function --Sam
                    GetThemeMetric(kThemeMetricListBoxFrameOutset, &frame_size);
                }

                int lw = frame->lineWidth;
                if (lw <= 0)
                    q->pixelMetric(QStyle::PM_DefaultFrameWidth, frame, w);
                { //clear to base color
                    p->save();
                    p->setPen(QPen(baseColor, lw));
                    p->setBrush(Qt::NoBrush);
                    p->drawRect(frame->rect);
                    p->restore();
                }

                const Rect *rect = qt_glb_mac_rect(frame->rect, p, false,
                                                   QRect(frame_size, frame_size, frame_size * 2,
                                                         frame_size * 2));
                qt_mac_set_port(p);
                if (pe == QStyle::PE_FrameLineEdit)
                    DrawThemeEditTextFrame(rect, tds);
                else
                    DrawThemeListBoxFrame(rect, tds);
            } else {
                q->QWindowsStyle::drawPrimitive(pe, frame, p, w);
            }
        }
        break;
    case QStyle::PE_FrameGroupBox:
        if (const QStyleOptionFrame *frame = qstyleoption_cast<const QStyleOptionFrame *>(opt)) {
            qt_mac_set_port(p);
#ifdef QMAC_DO_SECONDARY_GROUPBOXES
            if (w && qobject_cast<QGroupBox *>(w->parentWidget()))
                DrawThemeSecondaryGroup(qt_glb_mac_rect(frame->rect, p), kThemeStateActive);
            else
#endif
                DrawThemePrimaryGroup(qt_glb_mac_rect(frame->rect, p), kThemeStateActive);
        }
        break;
    case QStyle::PE_IndicatorArrowUp:
    case QStyle::PE_IndicatorArrowDown:
    case QStyle::PE_IndicatorArrowRight:
    case QStyle::PE_IndicatorArrowLeft: {
        ThemeArrowOrientation orientation;
        switch (pe) {
        case QStyle::PE_IndicatorArrowUp:
            orientation = kThemeArrowUp;
            break;
        case QStyle::PE_IndicatorArrowDown:
            orientation = kThemeArrowDown;
            break;
        case QStyle::PE_IndicatorArrowRight:
            orientation = kThemeArrowRight;
            break;
        default:
        case QStyle::PE_IndicatorArrowLeft:
            orientation = kThemeArrowLeft;
            break;
        }
        ThemePopupArrowSize size;
        if (opt->rect.width() < 8)
            size = kThemeArrow5pt;
        else
            size = kThemeArrow9pt;
        qt_mac_set_port(p);
        DrawThemePopupArrow(qt_glb_mac_rect(opt->rect, p), orientation, size, tds, 0, 0);
        break; }
    case QStyle::PE_FrameTabWidget:
        if (const QStyleOptionTabWidgetFrame *twf
                = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
            QRect wholePane = twf->rect;
            if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3) {
                // This tab pane with Appearance Manager is a real pain (pun not intended).
                // First, it can't handle drawing tabs at different positions.
                // Second, it draws outside my rectangle AND provides no function to return the area.
                // So, I must "guess" the size of the panel and do some transformations to make it work.
                const int TabPaneShadowWidth = 2; // The offset where we really start drawing the pane.
                const int TabPaneShadowHeight = 4; // The amount of pixels to for the drop shadow
                ThemeTabDirection ttd = getTabDirection(twf->shape);
                // we need to draw the whole thing, so add in the height.
                if (ttd != kThemeTabNorth) {
                    p->save();
                    QMatrix m;
                    QSize pixSize;
                    Rect finalRect;
                    switch (ttd) {
                    default:
                        break;
                    case kThemeTabSouth:
                        wholePane.setHeight(wholePane.height() - 1);
                        m.scale(1, -1);
                        m.translate(0, -wholePane.height());
                        pixSize = wholePane.size();
                        SetRect(&finalRect, TabPaneShadowWidth, 0,
                                pixSize.width() - 2 * TabPaneShadowWidth, pixSize.height() - 1);
                        break;
                    case kThemeTabWest:
                    case kThemeTabEast:
                        if (ttd == kThemeTabWest) {
                            m.translate(wholePane.x(), wholePane.y() + wholePane.height());
                            m.rotate(-90);
                        } else {
                            m.translate(wholePane.x() + wholePane.width(), wholePane.y());
                            m.rotate(90);
                        }
                        pixSize = wholePane.size();
                        SetRect(&finalRect, TabPaneShadowWidth, 0,
                                pixSize.width() - 2 * TabPaneShadowWidth,
                                pixSize.height() - TabPaneShadowHeight);
                        wholePane.setRect(wholePane.x(), wholePane.y(),
                                          wholePane.height(), wholePane.width());
                        break;
                    }
                    QPixmap pix(pixSize);
                    QPainter pixPainter(&pix);
                    qt_mac_set_port(&pixPainter);
                    Rect macRect;
                    SetRect(&macRect, 0, 0, pix.width(), pix.height());
                    ApplyThemeBackground(kThemeBackgroundTabPane, &macRect, tds, 32, true);
                    EraseRect(&macRect);
                    DrawThemeTabPane(&finalRect, tds);
                    pixPainter.end();
                    p->setMatrix(m);
                    p->drawPixmap(wholePane, pix);
                    p->restore();
                } else {
                    wholePane.adjust(TabPaneShadowWidth, 0,
                                        -TabPaneShadowWidth, -TabPaneShadowHeight);
                    qt_mac_set_port(p);
                    DrawThemeTabPane(qt_glb_mac_rect(wholePane, p), tds);
                }
            } else {
                DrawThemeSecondaryGroup(qt_glb_mac_rect(wholePane, p), tds);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawPrimitive(pe, opt, p, w);
        break;
    }
}

void QMacStylePrivate::AppManDrawControl(QStyle::ControlElement ce, const QStyleOption *opt,
                                         QPainter *p, const QWidget *widget) const
{
    ThemeDrawState tds = getDrawState(opt->state);
    switch (ce) {
    case QStyle::CE_FocusFrame: {
        SInt32 fo;
        GetThemeMetric(kThemeMetricFocusRectOutset, &fo);
        QRect r(fo, fo,  opt->rect.width() - (fo*2), opt->rect.height() - (fo*2));
        qt_mac_set_port(p);
        DrawThemeFocusRect(qt_glb_mac_rect(r, p, true, QRect(1, 1, 1, 1)), true);
        break; }
    case QStyle::CE_PushButtonBevel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            if (!(btn->state & (QStyle::State_Raised | QStyle::State_Sunken | QStyle::State_On)))
                break;
            bool drawColorless = btn->palette.currentColorGroup() == QPalette::Active;
            QString pmkey;
            bool do_draw = false;
            QPixmap buffer;
            bool darken = animatable(AquaPushButton, widget);
            int frame = buttonState.frame;
            if (btn->state & QStyle::State_On) {
                darken = true;
                frame = 12;
                if (btn->state & QStyle::State_Sunken)
                    frame += 8;
            } else if (btn->state & QStyle::State_Sunken) {
                darken = false;
                frame = 0;
            }
            if (darken && (btn->state & QStyle::State_Active)) {
                QTextStream os(&pmkey, QIODevice::WriteOnly);
                os << "$qt_mac_pshbtn_" << opt->rect.width() << "x" << opt->rect.height() << "_"
                   << opt->state << "_" << frame;
                tds = kThemeStatePressed;
                if (frame && !QPixmapCache::find(pmkey, buffer)) {
                    do_draw = true;
                    buffer = QPixmap(opt->rect.width(), opt->rect.height());
                    buffer.fill(Qt::color0);
                }
                if (timerID <= -1) {
                    QTimer::singleShot(0, const_cast<QMacStylePrivate *>(this),
                                       SLOT(startAnimationTimer()));
                }
            }
            ThemeButtonKind bkind;
            if (((btn->features & (QStyleOptionButton::Flat | QStyleOptionButton::HasMenu)))
                || (btn->rect.width() < 50 || btn->rect.height() < 30))
                bkind = kThemeBevelButton;
            else
                bkind = kThemePushButton;
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
            if (tds == kThemeStateInactive && drawColorless)
                info.state = kThemeStateActive;
            if (opt->state & QStyle::State_HasFocus
                    && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled) {
                info.adornment |= kThemeAdornmentFocus;
            }
            QRect off_rct;
            { //The AppManager draws outside my rectangle, so account for that difference..
                Rect macRect, myRect;
                SetRect(&myRect, btn->rect.x(), btn->rect.y(), btn->rect.width(),
                        btn->rect.height());
                GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                off_rct.setRect(myRect.left - macRect.left, myRect.top - macRect.top,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
            }
            qt_mac_set_port(p);
            DrawThemeButton(qt_glb_mac_rect(opt->rect, p, false, off_rct), bkind, &info, 0, 0, 0, 0);
            if (!buffer.isNull() && bkind == kThemePushButton) {
                if (do_draw && frame) {
                    QMacSavedPortInfo savedInfo(&buffer);
                    const Rect *buff_rct = qt_glb_mac_rect(QRect(0, 0, btn->rect.width(),
                                                           btn->rect.height()), &buffer, false,
                                                           off_rct);
                    DrawThemeButton(buff_rct, bkind, &info, 0, 0, 0, 0);
                    QPixmap buffer_mask(buffer.size());
                    buffer_mask.fill(Qt::color0);
                    ThemeButtonDrawInfo mask_info = info;
                    mask_info.state = kThemeStateActive;
                    {
                        QMacSavedPortInfo savedInfo(&buffer_mask);
                        DrawThemeButton(buff_rct, bkind, &mask_info, 0, 0, 0, 0);
                    }
                    QImage img = buffer.toImage(), maskimg = buffer_mask.toImage();
                    QImage mask_out(img.width(), img.height(), 1, 2, QImage::LittleEndian);
                    for (int y = 0; y < img.height(); ++y) {
                        //calculate a mask
                        for (int maskx = 0; maskx < img.width(); ++maskx) {
                            QRgb in = img.pixel(maskx, y), out = maskimg.pixel(maskx, y);
                            int diff = (((qRed(in)-qRed(out))*((qRed(in)-qRed(out)))) +
                                    ((qGreen(in)-qGreen(out))*((qGreen(in)-qGreen(out)))) +
                                    ((qBlue(in)-qBlue(out))*((qBlue(in)-qBlue(out)))));
                            mask_out.setPixel(maskx, y, diff > 100);
                        }
                        //pulse the colors
                        uchar *bytes = img.scanLine(y);
                        for (int x = 0; x < img.bytesPerLine(); ++x)
                            *(bytes + x) = 255 - (((255 - *(bytes + x)) * (128 - (frame<<2))) >> 7);
                    }
                    buffer = QPixmap::fromImage(img);
                    QBitmap qmask(mask_out);
                    buffer.setMask(qmask);
                }
                p->drawPixmap(opt->rect, buffer);
                if (do_draw)
                    QPixmapCache::insert(pmkey, buffer);
            }
            if (btn->features & QStyleOptionButton::HasMenu) {
                int mbi = q->pixelMetric(QStyle::PM_MenuButtonIndicator, btn, widget);
                QRect ir = btn->rect;
                QStyleOptionButton newBtn = *btn;
                newBtn.rect = QRect(ir.right() - mbi, ir.height() / 2 - 5, mbi, ir.height() / 2);
                q->drawPrimitive(QStyle::PE_IndicatorArrowDown, &newBtn, p, widget);
            }
        }
        break;
    case QStyle::CE_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            bool dis = !(mi->state & QStyle::State_Enabled);
            int tab = mi->tabWidth;
            int maxpmw = mi->maxIconWidth;
            bool checkable = mi->menuHasCheckableItems;
            bool act = mi->state & QStyle::State_Selected;
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
            irect = *qt_glb_mac_rect(mi->rect, p, false);

            if (checkable)
                maxpmw = qMax(maxpmw, 12); // space for the checkmarks

            ThemeMenuState tms = kThemeMenuActive;
            if (dis)
                tms |= kThemeMenuDisabled;
            if (act)
                tms |= kThemeMenuSelected;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                tmit |= kThemeMenuItemHierarchical;
            if (!mi->icon.isNull())
                tmit |= kThemeMenuItemHasIcon;
            qt_mac_set_port(p);
            if (mi->menuItemType != QStyleOptionMenuItem::Separator) {
                DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);
            } else {
                DrawThemeMenuSeparator(&irect);
                break;
            }

            int x, y, w, h;
            mi->rect.getRect(&x, &y, &w, &h);
            int checkcol = maxpmw;
            bool reverse = QApplication::isRightToLeft();
            int xpos = x + 18;
            if (reverse)
                xpos += w - checkcol;

            if (dis)
                p->setPen(mi->palette.text().color());
            else if (act)
                p->setPen(mi->palette.highlightedText().color());
            else
                p->setPen(mi->palette.buttonText().color());

            if (mi->checkType != QStyleOptionMenuItem::NotCheckable && mi->checked) {
                ThemeDrawState menuTDS = tds;
                if (act)
                    menuTDS = kThemeStatePressed;
                int xp = x;
                QCFString checkmark;
                if (mi->checkType == QStyleOptionMenuItem::Exclusive)
                    checkmark = QString(QChar(kDiamondUnicode));
                else
                    checkmark = QString(QChar(kCheckUnicode));
                Point macpt;
                SInt16 macbaseline;
                GetThemeTextDimensions(checkmark, kThemeMenuItemMarkFont, menuTDS, false, &macpt,
                                       &macbaseline);
                xp += macItemFrame;
                int mw = checkcol + macItemFrame;
                int mh = h - 2 * macItemFrame;
                QRect r(xp, y + macItemFrame, mw, mh);
                int translate = p->fontMetrics().ascent() - macbaseline + 1;
                if (macbaseline)
                    translate = p->fontMetrics().ascent() - macbaseline + 1;
                else
                    translate = 3;
                r.translate(0, translate);
                DrawThemeTextBox(checkmark, kThemeMenuItemMarkFont, menuTDS, false,
                                 qt_glb_mac_rect(r, p), teFlushDefault, 0);
            }
            if (!mi->icon.isNull()) {              // draw icon
                // Always be normal or disabled to follow the Mac style.
                QIcon::Mode mode = dis ? QIcon::Disabled : QIcon::Normal;
                QPixmap pixmap;
                pixmap = mi->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize), mode);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QRect cr(xpos, y, checkcol, h);
                QRect pmr(0, 0, pixw, pixh);
                pmr.moveCenter(cr.center());
                p->drawPixmap(pmr.topLeft(), pixmap);
                xpos += pixw + 6;
            }
            int xm = macItemFrame + checkcol + macItemHMargin;
            /*
            if (reverse)
                xpos = macItemFrame + tab;
            else
                xpos += xm;
                */
            QString s = mi->text;
            if (!s.isEmpty()) {                        // draw text
                int t = s.indexOf('\t');
                int m = macItemVMargin;
                int text_flags = Qt::AlignRight | Qt::AlignVCenter | Qt::TextHideMnemonic
                                 | Qt::TextSingleLine;
                p->save();
                if (t >= 0) {                         // draw tab text
                    int xp;
                    p->setFont(qt_app_fonts_hash()->value("QMenuItem", p->font()));
                    if (reverse)
                        xp = x + macRightBorder + macItemHMargin + macItemFrame - 1;
                    else
                        xp = x + w - tab - macRightBorder - macItemHMargin - macItemFrame + 1;
                    p->drawText(xp, y + m, tab, h - 2 * m, text_flags, s.mid(t + 1));
                    s = s.left(t);
                }
                text_flags ^= Qt::AlignRight;
                p->setFont(mi->font);
                p->drawText(xpos, y+m, w-xm-tab+1, h-2*m, text_flags, s);
                p->restore();
            }
        }
        break;
    case QStyle::CE_MenuHMargin:
    case QStyle::CE_MenuVMargin:
    case QStyle::CE_MenuTearoff:
    case QStyle::CE_MenuScroller:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
                 irect = *qt_glb_mac_rect(mi->rect, p, false);
            ThemeMenuState tms = kThemeMenuActive;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if (opt->state & QStyle::State_Selected)
                tms |= kThemeMenuSelected;
            if (ce == QStyle::CE_MenuScroller) {
                if (opt->state & QStyle::State_DownArrow)
                    tmit = kThemeMenuItemScrollDownArrow;
                else
                    tmit = kThemeMenuItemScrollUpArrow;
            }
            qt_mac_set_port(p);
            DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);
            if (ce == QStyle::CE_MenuTearoff) {
                p->setPen(QPen(mi->palette.dark().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2 - 1,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2 - 1);
                p->setPen(QPen(mi->palette.light().color(), 1, Qt::DashLine));
                p->drawLine(mi->rect.x() + 2, mi->rect.y() + mi->rect.height() / 2,
                            mi->rect.x() + mi->rect.width() - 4,
                            mi->rect.y() + mi->rect.height() / 2);
            }
        }
        break;
    case QStyle::CE_MenuBarItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            Rect mrect = *qt_glb_mac_rect(mi->menuRect, p),
                 irect = *qt_glb_mac_rect(mi->rect, p, false);
            ThemeMenuState tms = kThemeMenuActive;
            ThemeMenuItemType tmit = kThemeMenuItemPlain;
            if ((opt->state & QStyle::State_Selected) && (opt->state & QStyle::State_Sunken))
                tms |= kThemeMenuSelected;
            qt_mac_set_port(p);
            DrawThemeMenuItem(&mrect, &irect, mrect.top, mrect.bottom, tms, tmit, 0, 0);

            if (!mi->icon.isNull()) {
            q->drawItemPixmap(p, mi->rect,
                        Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip | Qt::TextSingleLine,
                              mi->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize)));
            } else {
                q->drawItemText(p, mi->rect,
                                Qt::AlignCenter | Qt::TextHideMnemonic | Qt::TextDontClip
                                | Qt::TextSingleLine,
                                mi->palette, mi->state & QStyle::State_Enabled,
                                mi->text, QPalette::ButtonText);
            }
        }
        break;
    case QStyle::CE_MenuBarEmptyArea:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            qt_mac_set_port(p);
            DrawThemeMenuBarBackground(qt_glb_mac_rect(mi->rect, p, false), kThemeMenuBarNormal,
                                       kThemeMenuSquareMenuBar);
        }
        break;
    case QStyle::CE_ProgressBarGroove:
    case QStyle::CE_ProgressBarLabel:
        break;
    case QStyle::CE_ProgressBarContents:
        if (const QStyleOptionProgressBar *pb = qstyleoption_cast<const QStyleOptionProgressBar *>(opt)) {
            ThemeTrackDrawInfo tdi;
            tdi.filler1 = 0;
            bool isIndeterminate = (pb->minimum == 0 && pb->maximum == 0);
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                tdi.kind = !isIndeterminate ? kThemeLargeProgressBar : kThemeLargeIndeterminateBar;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    tdi.kind = !isIndeterminate ? kThemeMiniProgressBar : kThemeMiniIndeterminateBar;
                    break;
                }
#endif
            case QAquaSizeSmall:
                tdi.kind = !isIndeterminate ? kThemeProgressBar : kThemeIndeterminateBar;
                break;
            }
            tdi.bounds = *qt_glb_mac_rect(opt->rect, p);
            tdi.max = pb->maximum;
            tdi.min = pb->minimum;
            tdi.value = pb->progress;
            tdi.attributes = kThemeTrackHorizontal;
            tdi.trackInfo.progress.phase = progressFrame;
            if (!(pb->state & QStyle::State_Active))
                tdi.enableState = kThemeTrackInactive;
            else if (!(pb->state & QStyle::State_Enabled))
                tdi.enableState = kThemeTrackDisabled;
            else
                tdi.enableState = kThemeTrackActive;
            qt_mac_set_port(p);
            DrawThemeTrack(&tdi, 0, 0, 0);
        }
        break;
    case QStyle::CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            ThemeTabStyle tts = kThemeTabNonFront;
            if (tab->state & QStyle::State_Selected) {
                if (!(tab->state & QStyle::State_Active))
                    tts = kThemeTabFrontUnavailable;
                else if (!(tab->state & QStyle::State_Enabled))
                    tts = kThemeTabFrontInactive;
                else
                    tts = kThemeTabFront;
            } else if (!(tab->state  &QStyle::State_Active)) {
                tts = kThemeTabNonFrontUnavailable;
            } else if (!(tab->state & QStyle::State_Enabled)) {
                tts = kThemeTabNonFrontInactive;
            } else if ((tab->state & (QStyle::State_Sunken | QStyle::State_MouseOver))
                       == (QStyle::State_Sunken | QStyle::State_MouseOver)) {
                tts = kThemeTabNonFrontPressed;
            }
            ThemeTabDirection ttd = getTabDirection(tab->shape);
            QRect tabr = tab->rect;
            qt_mac_draw_tab(p, widget, tabr, tts, ttd);
        }
        break;
    case QStyle::CE_SizeGrip: {
        const Rect *rect = qt_glb_mac_rect(opt->rect, p);
        Point orig = { rect->top, rect->left };
        qt_mac_set_port(p);
        ThemeGrowDirection dir = kThemeGrowRight | kThemeGrowDown;
#if 0
        if (QApplication::isRightToLeft())
            dir = kThemeGrowLeft | kThemeGrowDown;
#endif
        DrawThemeStandaloneGrowBox(orig, dir, false, kThemeStateActive);
        break; }

    case QStyle::CE_RubberBand:
        if (const QStyleOptionRubberBand *rubber = qstyleoption_cast<const QStyleOptionRubberBand *>(opt)) {
            QColor highlight(opt->palette.color(QPalette::Disabled, QPalette::Highlight));
            if(!rubber->opaque)
                highlight.setAlphaF(0.75);
            p->fillRect(opt->rect, highlight);
        }
        break;
    case QStyle::CE_HeaderSection:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            ThemeButtonKind bkind;
            QStyle::State flags = header->state;
            QRect ir = header->rect;
            bool scaleHeader = false;
            SInt32 headerHeight = 0;
            QWidget *pw = widget ? widget->parentWidget() : 0;
            if (pw && (qobject_cast<QTreeView *>(pw)
#ifdef QT3_SUPPORT
                       || pw->inherits("Q3ListView")
#endif
                )) {
                bkind = kThemeListHeaderButton;
                GetThemeMetric(kThemeMetricListHeaderHeight, &headerHeight);
                if (ir.height() > headerHeight)
                    scaleHeader = true;
            } else {
                bkind = kThemeBevelButton;
                if (p->font().bold())
                    flags |= QStyle::State_On;
                else
                    flags &= ~QStyle::State_On;
            }
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            if (flags & QStyle::State_HasFocus
                    && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                info.adornment |= kThemeAdornmentFocus;

            if (flags & QStyle::State_Active) {
                if (!(flags & QStyle::State_Enabled))
                    info.state = kThemeStateUnavailable;
                else if (flags & QStyle::State_Sunken)
                    info.state = kThemeStatePressed;
            } else {
                if (flags & QStyle::State_Enabled)
                    info.state = kThemeStateInactive;
                else
                    info.state = kThemeStateUnavailableInactive;
            }

            if (flags & QStyle::State_On)
                info.value = kThemeButtonOn;

            if (bkind == kThemeListHeaderButton
                && header->selectedPosition != QStyleOptionHeader::NotAdjacent) {
                if (header->selectedPosition == QStyleOptionHeader::PreviousIsSelected)
                    info.adornment = kThemeAdornmentHeaderButtonRightNeighborSelected;
                else if (header->selectedPosition == QStyleOptionHeader::PreviousIsSelected)
                    info.adornment = kThemeAdornmentHeaderButtonLeftNeighborSelected;
            }
            if (header->sortIndicator != QStyleOptionHeader::None) {
                info.value = kThemeButtonOn;
                if (header->sortIndicator == QStyleOptionHeader::SortUp)
                    info.adornment |= kThemeAdornmentHeaderButtonSortUp;
            } else if (bkind == kThemeListHeaderButton) {
                ir.setRight(ir.right() + 50);
            }
            if (scaleHeader) {
                QPixmap headerPix(ir.width(), headerHeight);
                QPainter pixPainter(&headerPix);
                Rect pixRect = *qt_glb_mac_rect(QRect(0, 0, ir.width(), headerHeight),
                                                &pixPainter, false);
                qt_mac_set_port(&pixPainter);
                DrawThemeButton(&pixRect, bkind, &info, 0, 0, 0, 0);
                p->drawPixmap(ir, headerPix);
            } else {
                qt_mac_set_port(p);
                DrawThemeButton(qt_glb_mac_rect(ir, p, false), bkind, &info, 0, 0, 0, 0);
            }
        }
        break;
    default:
        q->QWindowsStyle::drawControl(ce, opt, p, widget);
    }
}

QRect QMacStylePrivate::AppManSubElementRect(QStyle::SubElement sr, const QStyleOption *opt,
                                      const QWidget *widget) const
{
    QRect r = QRect();
    switch (sr) {
    case QStyle::SE_ToolBoxTabContents:
        r = q->QCommonStyle::subElementRect(sr, opt, widget);
        break;
    case QStyle::SE_PushButtonContents:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            Rect macRect, myRect;
            SetRect(&myRect, btn->rect.left(), btn->rect.top(), btn->rect.right(), btn->rect.bottom());
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            GetThemeButtonContentBounds(&myRect, kThemePushButton, &info, &macRect);
            r = QRect(macRect.left, macRect.top - 2,
                      qMin(qAbs(btn->rect.width() - 2 * macRect.left),
                           macRect.right - macRect.left),
                      qMin(qAbs(btn->rect.height() - 2 * macRect.top),
                           macRect.bottom - macRect.top));
        }
        break;
    case QStyle::SE_ProgressBarContents:
        r = opt->rect;
        break;
    case QStyle::SE_ProgressBarGroove:
    case QStyle::SE_ProgressBarLabel:
        break;
    default:
        r = q->QWindowsStyle::subElementRect(sr, opt, widget);
        break;
    }
    return r;
}

void QMacStylePrivate::AppManDrawComplexControl(QStyle::ComplexControl cc,
                                                const QStyleOptionComplex *opt, QPainter *p,
                                                const QWidget *widget) const
{
    ThemeDrawState tds = getDrawState(opt->state);
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, p, &tdi, widget);
            if (cc == QStyle::CC_Slider) {
                if (slider->activeSubControls == QStyle::SC_SliderGroove)
                    tdi.trackInfo.slider.pressState = kThemeLeftTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_SliderHandle)
                    tdi.trackInfo.slider.pressState = kThemeThumbPressed;
            } else {
                if (slider->activeSubControls == QStyle::SC_ScrollBarSubLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightInsideArrowPressed
                                                         | kThemeLeftOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddLine)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftInsideArrowPressed
                                                         | kThemeRightOutsideArrowPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarAddPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeRightTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarSubPage)
                    tdi.trackInfo.scrollbar.pressState = kThemeLeftTrackPressed;
                else if (slider->activeSubControls == QStyle::SC_ScrollBarSlider)
                    tdi.trackInfo.scrollbar.pressState = kThemeThumbPressed;
            }

            //The AppManager draws outside my rectangle, so account for that difference..
            Rect macRect;
            GetThemeTrackBounds(&tdi, &macRect);
            tdi.bounds.left  += tdi.bounds.left  - macRect.left;
            tdi.bounds.right -= macRect.right - tdi.bounds.right;

            bool tracking = slider->sliderPosition == slider->sliderValue;
            RgnHandle r = 0;
            if (!tracking) {
                r = qt_mac_get_rgn();
                GetThemeTrackThumbRgn(&tdi, r);
                tdi.value = slider->sliderValue;
            }

            qt_mac_set_port(p);
            DrawThemeTrack(&tdi, r, 0, 0);
            if (!tracking)
                qt_mac_dispose_rgn(r);
            if (slider->subControls & QStyle::SC_SliderTickmarks) {
                int numTicks;
                if (slider->tickInterval)
                        numTicks = ((slider->maximum - slider->minimum + 1)
                                        / slider->tickInterval) + 1;
                else
                    numTicks = ((slider->maximum - slider->minimum + 1) / slider->pageStep) + 1;
                if (tdi.trackInfo.slider.thumbDir == kThemeThumbPlain) {
                    tdi.trackInfo.slider.thumbDir = kThemeThumbDownward;
                    DrawThemeTrackTickMarks(&tdi, numTicks, 0, 0);
                    tdi.trackInfo.slider.thumbDir = kThemeThumbUpward;
                    DrawThemeTrackTickMarks(&tdi, numTicks, 0, 0);
                } else {
                    DrawThemeTrackTickMarks(&tdi, numTicks, 0, 0);
                }
            }
        }
        break;
    case QStyle::CC_Q3ListView:
        if (const QStyleOptionQ3ListView *lv = qstyleoption_cast<const QStyleOptionQ3ListView *>(opt)) {
            if (lv->subControls & QStyle::SC_Q3ListView)
                q->QWindowsStyle::drawComplexControl(cc, lv, p, widget);

            if (lv->subControls & (QStyle::SC_Q3ListViewBranch | QStyle::SC_Q3ListViewExpand)) {
                int y = lv->rect.y(),
                h = lv->rect.height(),
                x = lv->rect.right() - 10;
                for (int i = 1; i < lv->items.size() && y < h; ++i) {
                    QStyleOptionQ3ListViewItem child = lv->items.at(i);
                    if (y + child.height > 0 && (child.childCount > 0
                        || (child.features & (QStyleOptionQ3ListViewItem::Expandable
                                            | QStyleOptionQ3ListViewItem::Visible))
                            == (QStyleOptionQ3ListViewItem::Expandable
                                | QStyleOptionQ3ListViewItem::Visible))) {
                        QStyleOption treeOpt(0);
                        treeOpt.rect.setRect(x, y + child.height / 2 - 4, 9, 9);
                        treeOpt.palette = lv->palette;
                        treeOpt.state = lv->state;
                        treeOpt.state |= QStyle::State_Children;
                        if (child.state & QStyle::State_Open)
                            treeOpt.state |= QStyle::State_Open;
                        q->drawPrimitive(QStyle::PE_IndicatorBranch, &treeOpt, p, widget);
                    }
                    y += child.totalHeight;
                }
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *sb = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            QStyleOptionSpinBox newSB = *sb;
            if (sb->subControls & QStyle::SC_SpinBoxFrame) {
                QStyleOptionFrame lineedit;
                QRect lineeditRect = q->subControlRect(QStyle::CC_SpinBox, sb,
                                                       QStyle::SC_SpinBoxFrame, widget);
                qt_mac_set_port(p);
                const Rect *rect = qt_glb_mac_rect(lineeditRect, p, false);
                DrawThemeEditTextFrame(rect, tds);
            }
            if (sb->subControls & (QStyle::SC_SpinBoxDown | QStyle::SC_SpinBoxUp)) {
                if (!(sb->stepEnabled & (QAbstractSpinBox::StepUpEnabled
                                        | QAbstractSpinBox::StepDownEnabled)))
                    tds = kThemeStateUnavailable;
                if ((sb->activeSubControls == QStyle::SC_SpinBoxDown)
                    && (sb->state & QStyle::State_Sunken))
                    tds = kThemeStatePressedDown;
                else if ((sb->activeSubControls == QStyle::SC_SpinBoxUp)
                         && (sb->state & QStyle::State_Sunken))
                    tds = kThemeStatePressedUp;
                ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                if (sb->state & QStyle::State_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    info.adornment |= kThemeAdornmentFocus;
                QRect updown = q->subControlRect(QStyle::CC_SpinBox, sb, QStyle::SC_SpinBoxUp,
                                                 widget);
                updown |= q->subControlRect(QStyle::CC_SpinBox, sb, QStyle::SC_SpinBoxDown, widget);
                if (widget) {
                    QPalette::ColorRole bgRole = widget->backgroundRole();
                    QPixmap pm = sb->palette.brush(bgRole).texture();
                    if (!pm.isNull())
                        p->drawPixmap(updown, pm);
                    else
                        p->fillRect(updown, sb->palette.color(bgRole));
                }
                ThemeButtonKind kind = kThemeIncDecButton;
                switch (qt_aqua_size_constrain(widget)) {
                case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                        kind = kThemeIncDecButtonMini;
                        break;
                    }
#endif
                case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                        kind = kThemeIncDecButtonSmall;
                        break;
                    }
#endif
                case QAquaSizeUnknown:
                case QAquaSizeLarge:
                    kind = kThemeIncDecButton;
                    break;
                }
                if (!(sb->state & QStyle::State_Active)
                        && sb->palette.currentColorGroup() == QPalette::Active
                        && tds == kThemeStateInactive)
                    info.state = kThemeStateActive;
                QRect off_rct;
                Rect macRect, myRect;
                SetRect(&myRect, updown.x(), updown.y(),
                        updown.x() + updown.width(), updown.y() + updown.height());
                GetThemeButtonBackgroundBounds(&myRect, kind, &info, &macRect);
                off_rct.setRect(myRect.left - macRect.left, myRect.top - macRect.top,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
                qt_mac_set_port(p);
                DrawThemeButton(qt_glb_mac_rect(updown, p, false, off_rct), kind, &info,
                                0, 0, 0, 0);
            }
        }
        break;
    case QStyle::CC_ToolButton:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            ThemeButtonKind bkind = kThemeBevelButton;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bkind = kThemeBevelButton;
                break;
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    bkind = kThemeMiniBevelButton;
                    break;
                }
#endif
            case QAquaSizeSmall:
                bkind = kThemeSmallBevelButton;
                break;
            }

            QRect button, menuarea;
            button   = q->subControlRect(cc, tb, QStyle::SC_ToolButton, widget);
            menuarea = q->subControlRect(cc, tb, QStyle::SC_ToolButtonMenu, widget);
            QStyle::State bflags = tb->state,
            mflags = tb->state;
            if (tb->activeSubControls & QStyle::SC_ToolButton)
                bflags |= QStyle::State_Sunken;
            if (tb->activeSubControls & QStyle::SC_ToolButtonMenu)
                mflags |= QStyle::State_Sunken;

            if (tb->subControls & QStyle::SC_ToolButton) {
                if (bflags & (QStyle::State_Sunken | QStyle::State_On | QStyle::State_Raised)) {
                    ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                    if (tb->state & QStyle::State_HasFocus && QMacStyle::focusRectPolicy(widget)
                            != QMacStyle::FocusDisabled)
                        info.adornment |= kThemeAdornmentFocus;
                    if (tb->state & (QStyle::State_On | QStyle::State_Sunken))
                        info.value |= kThemeStatePressed;

                    QRect off_rct(0, 0, 0, 0);
                    { //The AppManager draws outside my rectangle, so account for that difference..
                        Rect macRect, myRect;
                        SetRect(&myRect, tb->rect.x(), tb->rect.y(), tb->rect.width(),
                                tb->rect.height());
                        GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                        off_rct = QRect(myRect.left - macRect.left, myRect.top - macRect.top,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
                    }

                    qt_mac_set_port(p);
                    DrawThemeButton(qt_glb_mac_rect(button, p, false, off_rct),
                                    bkind, &info, 0, 0, 0, 0);
                }
            }

            if (tb->subControls & QStyle::SC_ToolButtonMenu) {
                ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
                if (tb->state & QStyle::State_HasFocus
                        && QMacStyle::focusRectPolicy(widget) != QMacStyle::FocusDisabled)
                    info.adornment |= kThemeAdornmentFocus;
                if (tb->state & (QStyle::State_On | QStyle::State_Sunken)
                        || (tb->activeSubControls & QStyle::SC_ToolButtonMenu))
                    info.value |= kThemeStatePressed;
                qt_mac_set_port(p);
                DrawThemeButton(qt_glb_mac_rect(menuarea, p, false), bkind, &info, 0, 0, 0, 0);
                QRect r(menuarea.x() + ((menuarea.width() / 2) - 4), menuarea.height() - 8, 8, 8);
                DrawThemePopupArrow(qt_glb_mac_rect(r, p), kThemeArrowDown, kThemeArrow7pt, tds,
                                    0, 0);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            ThemeButtonDrawInfo info = { tds, kThemeButtonOff, kThemeAdornmentNone };
            bool drawColorless = (!(combo->state & QStyle::State_Active))
                                        && combo->palette.currentColorGroup() == QPalette::Active;
            if (drawColorless && tds == kThemeStateInactive)
                info.state = kThemeStateActive;
            bool hasFocus = combo->state & QStyle::State_HasFocus;
            if (hasFocus)
                info.adornment |= kThemeAdornmentFocus;
            if (combo->activeSubControls & QStyle::SC_ComboBoxArrow)
                info.state = kThemeStatePressed;
            p->fillRect(combo->rect, combo->palette.brush(QPalette::Button)); //make sure it is filled
            ThemeButtonKind bkind;
            QAquaWidgetSize aSize = qt_aqua_size_constrain(widget);
            switch (aSize) {
            default:  // Stupid GCC, being overly pedantic
            case QAquaSizeMini:
            case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    if (aSize == QAquaSizeMini)
                        bkind = combo->editable ? ThemeButtonKind(kThemeComboBoxMini)
                                                : ThemeButtonKind(kThemePopupButtonMini);
                    else
                        bkind = combo->editable ? ThemeButtonKind(kThemeComboBoxSmall)
                                                : ThemeButtonKind(kThemePopupButtonSmall);
                } else {
                    bkind = combo->editable ? kThemeComboBox : kThemePopupButton;
                }
                break;
#endif
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                bkind = combo->editable ? ThemeButtonKind(kThemeComboBox)
                                        : ThemeButtonKind(kThemePopupButton);
                break;
            }
            info.adornment |= kThemeAdornmentArrowLeftArrow;

            QRect off_rct;
            { //The AppManager draws outside my rectangle, so account for that difference..
                Rect macRect, myRect;
                SetRect(&myRect, combo->rect.x(), combo->rect.y(), combo->rect.width(),
                        combo->rect.height());
                GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
                int offset = hasFocus && !combo->editable ? -1 : 0;
                off_rct.setRect(myRect.left - macRect.left, (myRect.top - macRect.top) + offset,
                                (myRect.left - macRect.left) + (macRect.right - myRect.right),
                                (myRect.top - macRect.top)
                                 + (macRect.bottom - myRect.bottom) + offset);
            }
            if (!drawColorless){
                qt_mac_set_port(p);
                DrawThemeButton(qt_glb_mac_rect(combo->rect, p, true, off_rct), bkind, &info,
                                0, 0, 0, 0);
            } else {
                AppManDrawColorlessButton(*qt_glb_mac_rect(combo->rect, p, false, off_rct),
                                             bkind, info, p, opt);
            }
        }
        break;
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
        ThemeWindowMetrics twm;
        memset(&twm, 0, sizeof(twm));
        twm.metricSize = sizeof(twm);
        twm.titleWidth = tbar->rect.width();
        twm.titleHeight = tbar->rect.height();
        ThemeWindowAttributes twa = kThemeWindowHasTitleText;
        if (tbar->subControls & QStyle::SC_TitleBarCloseButton)
            twa |= kThemeWindowHasCloseBox;
        if (tbar->subControls & QStyle::SC_TitleBarMaxButton
                | QStyle::SC_TitleBarNormalButton)
            twa |= kThemeWindowHasFullZoom;
        if (tbar->subControls & QStyle::SC_TitleBarMinButton)
            twa |= kThemeWindowHasCollapseBox;

        //AppMan paints outside the given rectangle, so I have to adjust for the height properly!
        QRect newr = tbar->rect;
        {
            Rect br;
            RgnHandle rgn = qt_mac_get_rgn();
            GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(tbar->rect), tds, &twm, twa,
                                 kWindowTitleBarRgn, rgn);
            GetRegionBounds(rgn, &br);
            newr.translate(newr.x() - br.left, newr.y() - br.top);
            qt_mac_dispose_rgn(rgn);
        }
        qt_mac_set_port(p);
        Rect r = *qt_glb_mac_rect(newr, p, false);
        DrawThemeWindowFrame(QtWinType, &r, tds, &twm, twa, 0, 0);
        if (tbar->subControls & (QStyle::SC_TitleBarCloseButton | QStyle::SC_TitleBarMaxButton
                                 | QStyle::SC_TitleBarMinButton | QStyle::SC_TitleBarNormalButton)) {
            ThemeDrawState wtds = tds;
            if (tbar->state & QStyle::State_MouseOver)
                wtds = kThemeStateRollover;
            struct {
                unsigned int qt_type;
                ThemeTitleBarWidget mac_type;
            } types[] = {
                { QStyle::SC_TitleBarCloseButton, kThemeWidgetCloseBox },
                { QStyle::SC_TitleBarMaxButton, kThemeWidgetZoomBox },
                { QStyle::SC_TitleBarMinButton | QStyle::SC_TitleBarNormalButton, kThemeWidgetCollapseBox },
                { 0, 0 } };
            ThemeWindowMetrics tm;
            tm.metricSize = sizeof(tm);
            const Rect *wm_rect = qt_glb_mac_rect(newr, p, false);
            bool active = tbar->state & QStyle::State_Active;
            qt_mac_set_port(p);
            for (int i = 0; types[i].qt_type; ++i) {
                ThemeDrawState ctrl_tds = wtds;
                if (active && (tbar->activeSubControls & types[i].qt_type)
                        && (tbar->state & QStyle::State_Sunken))
                    ctrl_tds = kThemeStatePressed;
                ThemeTitleBarWidget twt = types[i].mac_type;
                /*
                if (tbar->window() && tbar->window()->isWindowModified() && twt == kThemeWidgetCloseBox)
                    twt = kThemeWidgetDirtyCloseBox;
                    */
                DrawThemeTitleBarWidget(QtWinType, wm_rect, ctrl_tds, &tm, twa, twt);
            }
        }

        if (tbar->subControls & QStyle::SC_TitleBarLabel) {
            int iw = 0;
            if (!tbar->icon.isNull()) {
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(newr, p), tds, &twm, twa,
                                     kWindowTitleProxyIconRgn, rgn);
                if (!EmptyRgn(rgn))
                    iw = tbar->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize), QIcon::Normal).width();
                qt_mac_dispose_rgn(rgn);
            }
            if (!tbar->text.isEmpty()) {
                p->save();
                {
                    RgnHandle rgn = qt_mac_get_rgn();
                    GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(newr), tds, &twm, twa,
                                         kWindowTitleTextRgn, rgn);
                    p->setClipRegion(qt_mac_convert_mac_region(rgn));
                    qt_mac_dispose_rgn(rgn);
                }
                QRect br = p->clipRegion().boundingRect();
                int x = br.x(),
                    y = br.y() + (tbar->rect.height() / 2 - p->fontMetrics().height() / 2);
                if (br.width() <= p->fontMetrics().width(tbar->text) + iw * 2)
                    x += iw;
                else
                    x += (br.width() / 2) - (p->fontMetrics().width(tbar->text) / 2);
                if (iw)
                    p->drawPixmap(x - iw, y, tbar->icon.pixmap(q->pixelMetric(QStyle::PM_SmallIconSize), QIcon::Normal));
                p->drawText(x, y + p->fontMetrics().ascent(), tbar->text);
            }
        }
        break; }
    default:
        q->QWindowsStyle::drawComplexControl(cc, opt, p, widget);
    }
}

QStyle::SubControl QMacStylePrivate::AppManHitTestComplexControl(QStyle::ComplexControl cc,
                                                           const QStyleOptionComplex *opt,
                                                           const QPoint &pt,
                                                           const QWidget *widget) const
{
    QStyle::SubControl sc = QStyle::SC_None;
    switch (cc) {
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *scrollbar = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, scrollbar, 0, &tdi, widget);
            Point pos = { (short)pt.y(), (short)pt.x() };
            Rect mrect;
            GetThemeTrackBounds(&tdi, &mrect);
            ControlPartCode cpc;
            if (HitTestThemeScrollBarArrows(&tdi.bounds, tdi.enableState,
                                            0, scrollbar->orientation == Qt::Horizontal,
                                            pos, &mrect, &cpc)) {
                if (cpc == kControlUpButtonPart)
                    sc = QStyle::SC_ScrollBarSubLine;
                else if (cpc == kControlDownButtonPart)
                    sc = QStyle::SC_ScrollBarAddLine;
            } else if (HitTestThemeTrack(&tdi, pos, &cpc)) {
                if (cpc == kControlPageUpPart)
                    sc = QStyle::SC_ScrollBarSubPage;
                else if (cpc == kControlPageDownPart)
                    sc = QStyle::SC_ScrollBarAddPage;
                else
                    sc = QStyle::SC_ScrollBarSlider;
            }
        }
        break;
    case QStyle::CC_Slider:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, 0, &tdi, widget);
            ControlPartCode hit = 0;
            Point macpt = { (short)pt.y(), (short)pt.x() };
            if (HitTestThemeTrack(&tdi, macpt, &hit) == true) {
                if (hit == kControlPageDownPart || hit == kControlPageUpPart)
                    sc = QStyle::SC_SliderGroove;
                else
                    sc = QStyle::SC_SliderHandle;
            }

        }
        break;
/*
    I don't know why, but we only get kWindowContentRgn here, which isn't what we want at all.
    It would be very nice if this would work.
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
            ThemeWindowMetrics twm;
            memset(&twm, 0, sizeof(twm));
            twm.metricSize = sizeof(twm);
            twm.titleWidth = tbar->rect.width();
            twm.titleHeight = tbar->rect.height();
            ThemeWindowAttributes twa = kThemeWindowHasTitleText;
            if (tbar->titleBarState)
                twa |= kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                       | kThemeWindowHasCollapseBox;
            else if (tbar->titleBarFlags & Qt::WStyle_SysMenu)
                twa |= kThemeWindowHasCloseBox;
            WindowRegionCode hit;
            Point macpt = { (short)pt.y(), (short)pt.x() };
            if (GetThemeWindowRegionHit(QtWinType, qt_glb_mac_rect(tbar->rect), kThemeStateActive,
                                        &twm, twa, macpt, &hit)) {
                switch (hit) {
                case kWindowCloseBoxRgn:
                    sc = QStyle::SC_TitleBarCloseButton;
                    break;
                case kWindowCollapseBoxRgn:
                    sc = QStyle::SC_TitleBarMinButton;
                    break;
                case kWindowZoomBoxRgn:
                    sc = QStyle::SC_TitleBarMaxButton;
                    break;
                case kWindowTitleTextRgn:
                    sc = QStyle::SC_TitleBarLabel;
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
        sc = q->QWindowsStyle::hitTestComplexControl(cc, opt, pt, widget);
    }
    return sc;
}
QRect QMacStylePrivate::AppManSubControlRect(QStyle::ComplexControl cc,
                                                     const QStyleOptionComplex *opt,
                                                     QStyle::SubControl sc,
                                                     const QWidget *widget) const
{
    QRect ret;
    switch (cc) {
    case QStyle::CC_Slider:
    case QStyle::CC_ScrollBar:
        if (const QStyleOptionSlider *slider = qstyleoption_cast<const QStyleOptionSlider *>(opt)) {
            ThemeTrackDrawInfo tdi;
            getSliderInfo(cc, slider, 0, &tdi, widget);
            Rect macRect;
            GetThemeTrackBounds(&tdi, &macRect);
            tdi.bounds.left  += tdi.bounds.left  - macRect.left;
            tdi.bounds.right -= macRect.right - tdi.bounds.right;
            bool scrollBar = cc == QStyle::CC_ScrollBar;
            if (!scrollBar && sc == QStyle::SC_SliderGroove) {
                GetThemeTrackBounds(&tdi, &macRect);
                ret.setRect(macRect.left, macRect.top, macRect.right - macRect.left,
                            macRect.bottom - macRect.top);
            } else if ((scrollBar && sc == QStyle::SC_ScrollBarSlider)
                        || (!scrollBar && sc == QStyle::SC_SliderHandle)) {
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeTrackThumbRgn(&tdi, rgn);
                GetRegionBounds(rgn, &macRect);
                ret.setRect(macRect.left, macRect.top, (macRect.right - macRect.left) + 1,
                            (macRect.bottom - macRect.top) + 1);
                qt_mac_dispose_rgn(rgn);
            } else if (scrollBar && sc == QStyle::SC_ScrollBarGroove) {
                GetThemeTrackDragRect(&tdi, &macRect);
                ret.setRect(macRect.left, macRect.top, macRect.right - macRect.left,
                            macRect.bottom - macRect.top);
            } else {
                ret = slider->rect; // Give them everything, since I can't get the
                                    // parts otherwise.
            }
        }
        break;
    case QStyle::CC_TitleBar:
        if (const QStyleOptionTitleBar *tbar = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
            ThemeWindowMetrics twm;
            memset(&twm, 0, sizeof(twm));
            twm.metricSize = sizeof(twm);
            twm.titleWidth = tbar->rect.width();
            twm.titleHeight = tbar->rect.height();
            ThemeWindowAttributes twa = kThemeWindowHasTitleText;
            if (tbar->subControls & QStyle::SC_TitleBarCloseButton)
                twa |= kThemeWindowHasCloseBox;
            if (tbar->subControls & QStyle::SC_TitleBarMaxButton
                                        | QStyle::SC_TitleBarNormalButton)
                twa |= kThemeWindowHasFullZoom;
            if (tbar->subControls & QStyle::SC_TitleBarMinButton)
                twa |= kThemeWindowHasCollapseBox;
            WindowRegionCode wrc = kWindowGlobalPortRgn;
            if (sc == QStyle::SC_TitleBarCloseButton)
                wrc = kWindowCloseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMinButton)
                wrc = kWindowCollapseBoxRgn;
            else if (sc == QStyle::SC_TitleBarMaxButton)
                wrc = kWindowZoomBoxRgn;
            else if (sc == QStyle::SC_TitleBarLabel)
                wrc = kWindowTitleTextRgn;
            else if (sc == QStyle::SC_TitleBarSysMenu) // We currently don't have this on Mac OS X.
                break;

            if (wrc != kWindowGlobalPortRgn) {
                // AppMan paints outside the given rectangle,
                // so I have to adjust for the height properly!
                Rect br;
                QRect r = tbar->rect;
                RgnHandle rgn = qt_mac_get_rgn();
                GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(r), kThemeStateActive, &twm, twa,
                                     kWindowTitleBarRgn, rgn);
                GetRegionBounds(rgn, &br);
                r.translate(r.x() - br.left, r.y() - br.top);
                GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(r), kThemeStateActive, &twm, twa,
                                     wrc, rgn);
                GetRegionBounds(rgn, &br);
                ret.setRect(br.left, br.top, (br.right - br.left), (br.bottom - br.top));
                qt_mac_dispose_rgn(rgn);
            }
        }
        break;
    case QStyle::CC_ComboBox:
        if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (sc == QStyle::SC_ComboBoxEditField) {
                Rect macRect, outRect;
                SetRect(&macRect, 0, 0, combo->rect.width(), combo->rect.height());
                ThemeButtonDrawInfo bdi = { kThemeStateActive, kThemeButtonOff,
                                            kThemeAdornmentNone };
                GetThemeButtonContentBounds(&macRect, kThemePopupButton, &bdi, &outRect);
                if (combo->editable) {
                    ret.setRect(outRect.left - 6, outRect.top + 2, (outRect.right - outRect.left) + 10,
                                (outRect.bottom - outRect.top) - 3);
                } else {
                    ret.setRect(outRect.left, outRect.top - 1, outRect.right - outRect.left,
                                outRect.bottom - outRect.top);
                }
            } else {
                ret = q->QWindowsStyle::subControlRect(cc, opt, sc, widget);
            }
        }
        break;
    case QStyle::CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 18,
            y = q->pixelMetric(QStyle::PM_SpinBoxFrameWidth, spin, widget),
            x = spin->rect.width() - spinner_w + y;
            ret.setRect(x, y, spinner_w, spin->rect.height() - y * 2);
            ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
            ThemeButtonKind kind = kThemeIncDecButton;
            switch (qt_aqua_size_constrain(widget)) {
            case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    kind = kThemeIncDecButtonMini;
                    break;
                }
#endif
            case QAquaSizeSmall:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                    kind = kThemeIncDecButtonSmall;
                    break;
                }
#endif
            case QAquaSizeUnknown:
            case QAquaSizeLarge:
                kind = kThemeIncDecButton;
                break;
            }
            Rect macRect;
            GetThemeButtonBackgroundBounds(qt_glb_mac_rect(ret), kind, &info, &macRect);

            ret.setRect(macRect.left, macRect.top,
                        macRect.right - macRect.left, macRect.bottom - macRect.top);
            switch (sc) {
            case QStyle::SC_SpinBoxUp:
                ret.setHeight(ret.height() / 2);
                break;
            case QStyle::SC_SpinBoxDown:
                ret.setY(ret.y() + ret.height() / 2);
                break;
            default:
                Q_ASSERT(0);
                break;
            }
            ret = QStyle::visualRect(spin->direction, spin->rect, ret);
        }
        break;
    default:
        ret = q->QWindowsStyle::subControlRect(cc, opt, sc, widget);
    }
    return ret;
}

int QMacStylePrivate::AppManPixelMetric(QStyle::PixelMetric metric, const QStyleOption *opt,
                                        const QWidget *widget) const
{
    SInt32 ret = 0;
    switch (metric) {
    case QStyle::PM_TitleBarHeight:
        if (const QStyleOptionTitleBar *tbar = qstyleoption_cast<const QStyleOptionTitleBar *>(opt)) {
            ThemeWindowMetrics twm;
            twm.metricSize = sizeof(twm);
            twm.titleWidth = tbar->rect.width();
            twm.titleHeight = tbar->rect.height();
            ThemeWindowAttributes twa = kThemeWindowHasTitleText;
            if (tbar->titleBarState)
                twa = kThemeWindowHasFullZoom | kThemeWindowHasCloseBox
                       | kThemeWindowHasCollapseBox;
            else if (tbar->titleBarFlags & Qt::WStyle_SysMenu)
                twa = kThemeWindowHasCloseBox;
            else
                twa = 0;

            Rect r;
            RgnHandle rgn = qt_mac_get_rgn();
            GetThemeWindowRegion(QtWinType, qt_glb_mac_rect(tbar->rect), kThemeStateActive,
                                 &twm, twa, kWindowTitleBarRgn, rgn);
            GetRegionBounds(rgn, &r);
            ret = (r.bottom - r.top);
            qt_mac_dispose_rgn(rgn);
        }
        break;
    default:
        ret = q->QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    }
    return ret;
}

void QMacStylePrivate::AppManAdjustButtonSize(QStyle::ContentsType ct, QSize &sz,
                                              const QWidget *widget)
{
    ThemeButtonKind bkind = kThemePushButton;
    if (ct == QStyle::CT_ToolButton)
        bkind = kThemeBevelButton;
    else if (ct == QStyle::CT_ComboBox) {
        if (!useHITheme)
            sz.rheight() += 1;
        bkind = kThemePopupButton;
    }
    if (qt_aqua_size_constrain(widget) == QAquaSizeSmall) {
        if (bkind == kThemeBevelButton)
            bkind = kThemeSmallBevelButton;
    }
    ThemeButtonDrawInfo info = { kThemeStateActive, kThemeButtonOff, kThemeAdornmentNone };
    Rect macRect, myRect;
    SetRect(&myRect, 0, 0, sz.width(), sz.height());
    GetThemeButtonBackgroundBounds(&myRect, bkind, &info, &macRect);
    sz.setWidth(sz.width() + (myRect.left - macRect.left) + (macRect.right - myRect.right));
    sz.setHeight(sz.height() + (myRect.top - macRect.top) + (macRect.bottom - myRect.bottom));
}

/*!
    \class QMacStyle qmacstyle_mac.h
    \brief The QMacStyle class implements an Appearance Manager style.

    \ingroup appearance

    This class is implemented as a wrapper to the Apple Appearance
    Manager. This allows your application to be styled by whatever
    theme your Macintosh is using. This is done by having primitives
    in QStyle implemented in terms of what the Macintosh would
    normally theme (i.e. the Finder).

    There are additional issues that should be taken
    into consideration to make an application compatible with the
    \link http://developer.apple.com/documentation/UserExperience/Conceptual/OSXHIGuidelines/index.html
    Aqua Style Guidelines \endlink. Some of these issues are outlined
    below.

    \list

    \i Layout - The restrictions on window layout are such that some
    aspects of layout that are style-dependent cannot be achieved
    using QLayout. Changes are being considered (and feedback would be
    appreciated) to make layouts QStyle-able. Some of the restrictions
    involve horizontal and vertical widget alignment and widget size
    (covered below).

    \i Widget size - Aqua allows widgets to have specific fixed sizes.  Qt
    does not fully implement this behavior so as to maintain cross-platform
    compatibility. As a result some widgets sizes may be inappropriate (and
    subsequently not rendered correctly by the Appearance Manager).The
    QWidget::sizeHint() will return the appropriate size for many
    managed widgets (widgets enumerated in \l QStyle::ContentsType).

    \i Effects - QMacStyle uses Appearance Manager for performing most of
    the drawing, but also uses emulation in a few cases where Appearance
    Manager does not provide the required functionality (for example, QPushButton
    pulsing effects). We tried to make the emulation as close to the original
    as possible. Please report any issues you see in effects or non-standard
    widgets.

    \endlist

    There are other issues that need to be considered in the feel of
    your application (including the general color scheme to match the
    Aqua colors). The Guidelines mentioned above will remain current
    with new advances and design suggestions for Mac OS X.

    Note that the functions provided by QMacStyle are
    reimplementations of QStyle functions; see QStyle for their
    documentation.
*/


/*!
    \enum QMacStyle::WidgetSizePolicy

    \value SizeSmall
    \value SizeLarge
    \value SizeMini
    \value SizeDefault
    \omitvalue SizeNone
*/

/*!
    Constructs a QMacStyle object.
*/
QMacStyle::QMacStyle()
    : QWindowsStyle()
{
    d = new QMacStylePrivate(this);
}

/*!
    Destructs a QMacStyle object.
*/
QMacStyle::~QMacStyle()
{
    delete d;
}

/*! \reimp */
void QMacStyle::polish(QPalette &pal)
{
    QPixmap px(200, 200);
    QColor pc(Qt::black);
    {
        QPainter p(&px);
        qt_mac_set_port(&p);
        SetThemeBackground(kThemeBrushDialogBackgroundActive, px.depth(), true);
        EraseRect(qt_glb_mac_rect(QRect(0, 0, px.width(), px.height()),
                  static_cast<QPaintDevice *>(0), false));
        RGBColor c;
        GetThemeBrushAsColor(kThemeBrushDialogBackgroundActive, 32, true, &c);
        pc = QColor(c.red / 256, c.green / 256, c.blue / 256);
    }
    QBrush background(pc, px);
    pal.setBrush(QPalette::Background, background);
    pal.setBrush(QPalette::Button, background);
}

/*! \reimp */
void QMacStyle::polish(QApplication *)
{
}

/*! \reimp */
void QMacStyle::unpolish(QApplication *)
{
}

/*! \reimp */
void QMacStyle::polish(QWidget* w)
{
    if (QLineEdit *lined = qobject_cast<QLineEdit*>(w)) {
        if (qobject_cast<QComboBox*>(lined->parentWidget())
                && !lined->testAttribute(Qt::WA_SetFont))
            lined->setFont(*qt_app_fonts_hash()->find("QComboLineEdit"));
    }

    if (d->useHITheme)
        d->HIThemePolish(w);
    else
        d->AppManPolish(w);
}

/*! \reimp */
void QMacStyle::unpolish(QWidget* w)
{
    if (d->useHITheme)
        d->HIThemeUnpolish(w);
    else
        d->AppManUnpolish(w);
}

/*! \reimp */
int QMacStyle::pixelMetric(PixelMetric metric, const QStyleOption *opt, const QWidget *widget) const
{
    SInt32 ret = 0;
    switch (metric) {
    case PM_ToolBarIconSize:
        ret = pixelMetric(PM_LargeIconSize);
        break;

    case PM_FocusFrameVMargin:
    case PM_FocusFrameHMargin:
        GetThemeMetric(kThemeMetricFocusRectOutset, &ret);
        break;

    case PM_CheckListControllerSize:
        ret = 0;
        break;
    case PM_CheckListButtonSize: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        break; }
    case PM_DialogButtonsSeparator:
        ret = -5;
        break;
    case PM_DialogButtonsButtonHeight: {
        QSize sz;
        ret = qt_aqua_size_constrain(0, QStyle::CT_PushButton, QSize(-1, -1), &sz);
        if (sz == QSize(-1, -1))
            ret = 32;
        else
            ret = sz.height();
        break; }
    case PM_DialogButtonsButtonWidth: {
        QSize sz;
        ret = qt_aqua_size_constrain(0, QStyle::CT_PushButton, QSize(-1, -1), &sz);
        if (sz == QSize(-1, -1))
            ret = 70;
        else
            ret = sz.width();
        break; }

    case PM_MenuBarHMargin:
        ret = 8;
        break;

    case PM_MenuBarVMargin:
        ret = 0;
        break;

    case QStyle::PM_MenuDesktopFrameWidth:
        ret = 15;
        break;
    case PM_MenuScrollerHeight:
#if 0
        SInt16 ash, asw;
        GetThemeMenuItemExtra(kThemeMenuItemScrollUpArrow, &ash, &asw);
        ret = ash;
#else
        ret = 15; //I hate having magic numbers in here...
#endif
        break;
    case PM_DefaultFrameWidth:
#ifndef QT_NO_MAINWINDOW
        if (widget && (widget->isWindow() || !widget->parentWidget()
                || (qobject_cast<const QMainWindow*>(widget->parentWidget())
                   && static_cast<QMainWindow *>(widget->parentWidget())->centralWidget() == widget))
                && (qobject_cast<const QAbstractScrollArea *>(widget)
#ifdef QT3_SUPPORT
                    || widget->inherits("QScrollView")
#endif
                    || widget->inherits("QWorkspaceChild")))
            ret = 0;
        else
#endif
            ret = QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    case PM_MaximumDragDistance:
        ret = -1;
        break;
    case PM_ScrollBarSliderMin:
        ret = 24;
        break;
    case PM_SpinBoxFrameWidth:
        GetThemeMetric(kThemeMetricEditTextFrameOutset, &ret);
        ret += 2;
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
    case PM_TitleBarHeight:
        ret = d->useHITheme ? d->HIThemePixelMetric(metric, opt, widget)
                            : d->AppManPixelMetric(metric, opt, widget);
        break;
    case PM_TabBarTabVSpace:
        ret = 4;
        break;
    case PM_TabBarTabShiftHorizontal:
    case PM_TabBarTabShiftVertical:
        ret = 0;
        break;
    case PM_TabBarBaseHeight:
        if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)
            ret = 7;
        else
            ret = 0;
        break;
    case PM_TabBarTabOverlap:
        if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3)
            GetThemeMetric(kThemeMetricTabOverlap, &ret);
        else
            ret = 0;
        break;
    case PM_TabBarBaseOverlap:
        if (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3) {
            GetThemeMetric(kThemeMetricTabFrameOverlap, &ret);
            --ret;
        } else {
            ret = 11;
        }
        break;
    case PM_ScrollBarExtent: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricScrollBarWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3) && 0
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniScrollBarWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallScrollBarWidth, &ret);
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniCheckBoxHeight, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxHeight, &ret);
            break;
        }
        break; }
    case PM_IndicatorWidth: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricCheckBoxWidth, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniCheckBoxWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallCheckBoxWidth, &ret);
            break;
        }
        ++ret;
        break; }
    case PM_ExclusiveIndicatorHeight: {
        switch (qt_aqua_size_constrain(widget)) {
        case QAquaSizeUnknown:
        case QAquaSizeLarge:
            GetThemeMetric(kThemeMetricRadioButtonHeight, &ret);
            break;
        case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
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
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                GetThemeMetric(kThemeMetricMiniRadioButtonWidth, &ret);
                break;
            }
#endif
        case QAquaSizeSmall:
            GetThemeMetric(kThemeMetricSmallRadioButtonWidth, &ret);
            break;
        }
        ++ret;
        break; }
    case PM_MenuVMargin:
        ret = 4;
        break;
    case PM_MenuPanelWidth:
        ret = 0;
        break;
    case PM_ToolTipLabelFrameWidth:
        ret = 0;
        break;
    default:
        ret = QWindowsStyle::pixelMetric(metric, opt, widget);
        break;
    }
    return ret;
}

/*! \reimp */
int QMacStyle::styleHint(StyleHint sh, const QStyleOption *opt, const QWidget *w,
                         QStyleHintReturn *hret) const
{
    SInt32 ret = 0;
    switch(sh) {
    case SH_TitleBar_AutoRaise:
        ret = true;
        break;
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
        /*
    case SH_DialogButtons_DefaultButton:
        ret = QDialogButtons::Reject;
        break;
        */
    case SH_Menu_SloppySubMenus:
        ret = true;
        break;
    case SH_GroupBox_TextLabelVerticalAlignment:
        ret = Qt::AlignTop;
        break;
    case SH_ScrollView_FrameOnlyAroundContents:
        if (w && (w->isWindow() || !w->parentWidget() || w->parentWidget()->isWindow())
            && (qobject_cast<const QAbstractScrollArea *>(w)
#ifdef QT3_SUPPORT
                || w->inherits("QScrollView")
#endif
                || w->inherits("QWorkspaceChild")))
            ret = true;
        else
            ret = QWindowsStyle::styleHint(sh, opt, w, hret);
        break;
    case SH_Menu_FillScreenWithScroll:
        ret = (QSysInfo::MacintoshVersion < QSysInfo::MV_10_3);
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
    case SH_Q3ListViewExpand_SelectMouseType:
    case SH_TabBar_SelectMouseType:
        ret = QEvent::MouseButtonRelease;
        break;
    case SH_ComboBox_Popup:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt))
            ret = !cmb->editable;
        else
            ret = 0;
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
        ret = Qt::AlignCenter;
        break;
    case SH_UnderlineShortcut:
        ret = false;
        break;
    case SH_ToolTipLabel_Opacity:
        ret = 242; // About 95%
        break;
    case SH_Button_FocusPolicy:
        ret = Qt::TabFocus;
        break;
    case SH_EtchDisabledText:
        ret = false;
        break;
    case SH_FocusFrame_Mask: {
        ret = true;
        if(QStyleHintReturnMask *mask = qstyleoption_cast<QStyleHintReturnMask*>(hret)) {
            const uchar fillR = 192, fillG = 191, fillB = 190;
            QImage img;
            {
                QPixmap pix(opt->rect.size());
                pix.fill(QColor(fillR, fillG, fillB));
                QPainter pix_paint(&pix);
                drawControl(CE_FocusFrame, opt, &pix_paint, w);
                pix_paint.end();
                img = pix.toImage();
            }

            const QRgb *sptr = (QRgb*)img.bits(), *srow;
            const int sbpl = img.bytesPerLine();
            const int w = sbpl/4, h = img.height();

            QImage img_mask(img.width(), img.height(), 32);
            QRgb *dptr = (QRgb*)img_mask.bits(), *drow;
            const int dbpl = img_mask.bytesPerLine();

            for (int y = 0; y < h; ++y) {
                srow = sptr+((y*sbpl)/4);
                drow = dptr+((y*dbpl)/4);
                for (int x = 0; x < w; ++x) {
                    ++srow;
                    const int diff = (((qRed(*srow)-qRed(fillR))*(qRed(*srow)-qRed(fillR))) +
                                      ((qGreen(*srow)-qGreen(fillG))*((qGreen(*srow)-qGreen(fillG)))) +
                                      ((qBlue(*srow)-qBlue(fillB))*((qBlue(*srow)-qBlue(fillB)))));
                    (*drow++) = (diff < 100) ? Qt::black : Qt::white;
                }
            }
            QBitmap qmask = QBitmap::fromImage(img_mask);
            mask->region = QRegion(qmask);
        }
        break; }
    case SH_RubberBand_Mask:
        ret = 0;
        break;
    case SH_ComboBox_LayoutDirection:
        ret = Qt::LeftToRight;
        break;
    default:
        ret = QWindowsStyle::styleHint(sh, opt, w, hret);
        break;
    }
    return ret;
}

/*! \reimp */
QPixmap QMacStyle::generatedIconPixmap(QIcon::Mode iconMode, const QPixmap &pixmap,
                                       const QStyleOption *opt) const
{
    switch (iconMode) {
    case QIcon::Disabled: {
        QImage img = pixmap.toImage();
        img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
        int imgh = img.height();
        int imgw = img.width();
        QRgb pixel;
        for (int y = 0; y < imgh; ++y) {
            for (int x = 0; x < imgw; ++x) {
                pixel = img.pixel(x, y);
                img.setPixel(x, y, qRgba(qRed(pixel), qGreen(pixel), qBlue(pixel),
                                         qAlpha(pixel) / 2));
            }
        }
        return QPixmap::fromImage(img);
    }
    default:
        ;
    }
    return QWindowsStyle::generatedIconPixmap(iconMode, pixmap, opt);
}

/*! \reimp */
QPixmap QMacStyle::standardPixmap(StandardPixmap standardPixmap, const QStyleOption *opt,
                                  const QWidget *widget) const
{
    IconRef icon = 0;
    OSType iconType = 0;
    switch (standardPixmap) {
    case QStyle::SP_MessageBoxQuestion:
    case QStyle::SP_MessageBoxInformation:
        iconType = kAlertNoteIcon;
        break;
    case QStyle::SP_MessageBoxWarning:
        iconType = kAlertCautionIcon;
        break;
    case QStyle::SP_MessageBoxCritical:
        iconType = kAlertStopIcon;
        break;
    case SP_DesktopIcon:
        iconType = kDesktopIcon;
        break;
    case SP_TrashIcon:
        iconType = kTrashIcon;
        break;
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
    case SP_ComputerIcon:
        iconType = kComputerIcon;
        break;
#endif
    case SP_DriveFDIcon:
        iconType = kGenericFloppyIcon;
        break;
    case SP_DriveHDIcon:
        iconType = kGenericHardDiskIcon;
        break;
    case SP_DriveCDIcon:
    case SP_DriveDVDIcon:
        iconType = kGenericCDROMIcon;
        break;
    case SP_DriveNetIcon:
        iconType = kGenericNetworkIcon;
        break;
    case SP_DirOpenIcon:
        iconType = kOpenFolderIcon;
        break;
    case SP_DirClosedIcon:
    case SP_DirLinkIcon:
        iconType = kGenericFolderIcon;
        break;
    case SP_FileLinkIcon:
    case SP_FileIcon:
        iconType = kGenericDocumentIcon;
        break;
    default:
        break;
    }
    if (iconType != 0)
        GetIconRef(kOnSystemDisk, kSystemIconsCreator, iconType, &icon);
    if (icon) {
        QPixmap ret = qt_mac_convert_iconref(icon, 64, 64);
        ReleaseIconRef(icon);
        return ret;
    }
    return QWindowsStyle::standardPixmap(standardPixmap, opt, widget);
}

/*!
    \enum QMacStyle::FocusRectPolicy

    This type is used to signify a widget's focus rectangle policy.

    \value FocusEnabled  show a focus rectangle when the widget has focus.
    \value FocusDisabled  never show a focus rectangle for the widget.
    \value FocusDefault  show a focus rectangle when the widget has
    focus and the widget is a QSpinWidget, QDateTimeEdit, QLineEdit,
    QListBox, QListView, editable QTextEdit, or one of their
    subclasses.
*/

/*!
    Sets the focus rectangle policy of \a w. The \a policy can be one of
    \l{QMacStyle::FocusRectPolicy}.

    \sa focusRectPolicy()
*/
void QMacStyle::setFocusRectPolicy(QWidget *w, FocusRectPolicy policy)
{
    QMacStylePrivate::PolicyState::focusMap.insert(w, policy);
    QMacStylePrivate::PolicyState::watchObject(w);
    if (w->hasFocus()) {
        w->clearFocus();
        w->setFocus();
    }
}

/*!
    Returns the focus rectangle policy for the widget \a w.

    The focus rectangle policy can be one of \l{QMacStyle::FocusRectPolicy}.

    \sa setFocusRectPolicy()
*/
QMacStyle::FocusRectPolicy QMacStyle::focusRectPolicy(const QWidget *w)
{
    if (QMacStylePrivate::PolicyState::focusMap.contains(w))
        return QMacStylePrivate::PolicyState::focusMap[w];
    return FocusDefault;
}

/*!
    Sets the widget size policy of \a w. The \a policy can be one of
    \l{QMacStyle::WidgetSizePolicy}.

    \sa widgetSizePolicy()
*/
void QMacStyle::setWidgetSizePolicy(const QWidget *w, WidgetSizePolicy policy)
{
    QMacStylePrivate::PolicyState::sizeMap.insert(w, policy);
    QMacStylePrivate::PolicyState::watchObject(w);
}

/*!
    Returns the widget size policy for the widget \a w.

    The widget size policy can be one of \l{QMacStyle::WidgetSizePolicy}.

    \sa setWidgetSizePolicy()
*/
QMacStyle::WidgetSizePolicy QMacStyle::widgetSizePolicy(const QWidget *w)
{
    WidgetSizePolicy ret = SizeDefault;
    if (w) {
        if (QMacStylePrivate::PolicyState::sizeMap.contains(w))
            ret = QMacStylePrivate::PolicyState::sizeMap[w];
        if (ret == SizeDefault) {
            for (QWidget *p = w->parentWidget(); p; p = p->parentWidget()) {
                if (QMacStylePrivate::PolicyState::sizeMap.contains(p)) {
                    ret = QMacStylePrivate::PolicyState::sizeMap[p];
                    if (ret != SizeDefault)
                        break;
                }
                if (p->isWindow())
                    break;
            }
        }
    }
    return ret;
}

/*! \reimp */
void QMacStyle::drawPrimitive(PrimitiveElement pe, const QStyleOption *opt, QPainter *p,
                              const QWidget *w) const
{
    switch (pe) {
    case PE_FrameTabBarBase:
        if (const QStyleOptionTabBarBase *tbb
                = qstyleoption_cast<const QStyleOptionTabBarBase *>(opt)) {
            QRegion region(tbb->rect);
            region -= tbb->tabBarRect;
            p->save();
            p->setClipRegion(region);
            QStyleOptionTabWidgetFrame twf;
            twf.QStyleOption::operator=(*tbb);
            twf.shape  = tbb->shape;
            switch (getTabDirection(twf.shape)) {
            case kThemeTabNorth:
                twf.rect = twf.rect.adjusted(0, 0, 0, 10);
                break;
            case kThemeTabSouth:
                twf.rect = twf.rect.adjusted(0, -10, 0, 0);
                break;
            case kThemeTabWest:
                twf.rect = twf.rect.adjusted(0, 0, 10, 0);
                break;
            case kThemeTabEast:
                twf.rect = twf.rect.adjusted(0, -10, 0, 0);
                break;
            }
            drawPrimitive(PE_FrameTabWidget, &twf, p, w);
            p->restore();
        }
        break;
    case PE_PanelTipLabel:
        p->fillRect(opt->rect, QColor(255, 255, 199));
        break;
    default:
        if (d->useHITheme)
            d->HIThemeDrawPrimitive(pe, opt, p, w);
        else
            d->AppManDrawPrimitive(pe, opt, p, w);
    }
}

static inline QPixmap darkenPixmap(const QPixmap &pixmap)
{
    QImage img = pixmap.toImage();
    img.convertToFormat(QImage::Format_ARGB32_Premultiplied);
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
    return QPixmap::fromImage(img);
}



/*! \reimp */
void QMacStyle::drawControl(ControlElement ce, const QStyleOption *opt, QPainter *p,
                            const QWidget *w) const
{
    switch (ce) {
    case CE_HeaderLabel:
        if (const QStyleOptionHeader *header = qstyleoption_cast<const QStyleOptionHeader *>(opt)) {
            QRect textr = header->rect;
            if (!header->icon.isNull()) {
                QIcon::Mode mode = QIcon::Disabled;
                if (opt->state & QStyle::State_Enabled)
                    mode = QIcon::Normal;
                QPixmap pixmap = header->icon.pixmap(pixelMetric(PM_SmallIconSize), mode);

                QRect pixr = header->rect;
                pixr.setY(header->rect.center().y() - (pixmap.height() - 1) / 2);
                drawItemPixmap(p, pixr, Qt::AlignVCenter, pixmap);
                textr.translate(pixmap.width() + 2, 0);
            }

            QPalette::ColorRole textRole = QPalette::ButtonText;
            if (p->font().bold()) {
                // If it's a table, use the bright text instead.
                if (!(w && (qobject_cast<QTreeView *>(w->parentWidget())
#ifdef QT3_SUPPORT
                            || w->parentWidget()->inherits("Q3ListView")
#endif
                          )))
                    textRole = QPalette::BrightText;
            }
            drawItemText(p, textr, Qt::AlignVCenter, header->palette,
                         header->state & QStyle::State_Enabled, header->text, textRole);
        }
        break;
    case CE_ToolButtonLabel:
        if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
            QStyleOptionToolButton myTb = *tb;
            myTb.state &= ~State_AutoRaise;
            if (w && qobject_cast<QToolBar *>(w->parentWidget())) {
                QRect cr = tb->rect;
                int shiftX = 0;
                int shiftY = 0;
                if (tb->state & (State_Sunken | State_On)) {
                    shiftX = pixelMetric(PM_ButtonShiftHorizontal, tb, w);
                    shiftY = pixelMetric(PM_ButtonShiftVertical, tb, w);
                }
                // The down state is special for QToolButtons in a toolbar on the Mac
                // The text is a bit bolder and gets a drop shadow and the icons are also darkened.
                // This doesn't really fit into any particular case in QIcon, so we
                // do the majority of the work ourselves.
                if (tb->state & QStyle::State_Sunken
                        && !(tb->features & QStyleOptionToolButton::Arrow)) {
                    Qt::ToolButtonStyle tbstyle = tb->toolButtonStyle;
                    if (tb->icon.isNull() && !tb->text.isEmpty())
                        tbstyle = Qt::ToolButtonTextOnly;

                    switch (tbstyle) {
                    case Qt::ToolButtonTextOnly:
                        drawItemText(p, cr, Qt::AlignCenter, tb->palette,
                                     tb->state & QStyle::State_Enabled, tb->text);
                        break;
                    case Qt::ToolButtonIconOnly:
                    case Qt::ToolButtonTextBesideIcon:
                    case Qt::ToolButtonTextUnderIcon: {
                        QRect pr = cr;
                        QIcon::Mode iconMode = (tb->state & QStyle::State_Enabled) ? QIcon::Normal
                                                                                   : QIcon::Disabled;
                        QIcon::State iconState = (tb->state & QStyle::State_On) ? QIcon::On
                                                                                : QIcon::Off;
                        QPixmap pixmap = tb->icon.pixmap(tb->rect.size().boundedTo(tb->iconSize), iconMode, iconState);

                        // Draw the text if it's needed.
                        if (tb->toolButtonStyle != Qt::ToolButtonIconOnly) {
                            int alignment = 0;
                            if (tb->toolButtonStyle == Qt::ToolButtonTextUnderIcon) {
                                int fh = p->fontMetrics().height();
                                pr.adjust(0, 3, 0, -fh - 3);
                                cr.adjust(0, pr.bottom(), 0, -3);
                                alignment |= Qt::AlignCenter;
                            } else {
                                pr.setWidth(pixmap.width() + 8);
                                cr.adjust(pr.right(), 0, 0, 0);
                                alignment |= Qt::AlignLeft | Qt::AlignVCenter;
                            }
                            cr.translate(shiftX, shiftY);
                            drawItemText(p, cr, alignment, tb->palette,
                                         tb->state & QStyle::State_Enabled, tb->text);
                            cr.adjust(0, 3, 0, -3); // the drop shadow
                            drawItemText(p, cr, alignment, tb->palette,
                                         tb->state & QStyle::State_Enabled, tb->text);
                        }
                        pr.translate(shiftX, shiftY);
                        pixmap = darkenPixmap(pixmap);
                        drawItemPixmap(p, pr, Qt::AlignCenter, pixmap);
                        break; }
                    }
                } else {
                    QWindowsStyle::drawControl(ce, &myTb, p, w);
                }
            } else {
                QWindowsStyle::drawControl(ce, &myTb, p, w);
            }
        }
        break;
    case CE_ToolBoxTab:
        QCommonStyle::drawControl(ce, opt, p, w);
        break;
    case CE_TabBarTabShape:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            // Make sure we draw Panther-style tabs on Panther
            if (QSysInfo::MacintoshVersion == QSysInfo::MV_10_3
                || (!d->useHITheme && QSysInfo::MacintoshVersion != QSysInfo::MV_10_2)) {
                d->drawPantherTab(tab, p, w);
            } else {
                if (d->useHITheme)
                    d->HIThemeDrawControl(ce, opt, p, w);
                else
                    d->AppManDrawControl(ce, opt, p, w);
            }
        }
        break;
    case CE_PushButtonLabel:
        if (const QStyleOptionButton *btn = qstyleoption_cast<const QStyleOptionButton *>(opt)) {
            // We really don't want the label to be drawn the same as on
            // windows style if it has an icon, then it should be more like a
            // tab. So, cheat a little here.
            if (btn->icon.isNull()) {
                QWindowsStyle::drawControl(ce, btn, p, w);
            } else {
                QRect br = p->boundingRect(btn->rect, Qt::AlignCenter, btn->text);
                QIcon::Mode mode = btn->state & State_Enabled ? QIcon::Normal
                                                              : QIcon::Disabled;
                if (mode == QIcon::Normal && btn->state & State_HasFocus)
                    mode = QIcon::Active;
                QIcon::State state = QIcon::Off;
                if (btn->state & State_On)
                    state = QIcon::On;
                QPixmap pixmap = btn->icon.pixmap(pixelMetric(PM_SmallIconSize), mode, state);
                int pixw = pixmap.width();
                int pixh = pixmap.height();
                QPoint btl = br.isEmpty() ? QPoint(btn->rect.center().x() + pixw / 2 + 2, btn->rect.center().y())
                                          : QPoint(br.x(), br.y() + br.height() / 2);
                QPoint pixTL(btl.x() - pixw - 2, btl.y() - pixh / 2);
                p->drawPixmap(pixTL, pixmap);
                p->drawText(br, btn->text);
            }
        }
        break;
    case CE_ComboBoxLabel:
        if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            QStyleOptionComboBox comboCopy = *cb;
            comboCopy.direction = Qt::LeftToRight;
            QWindowsStyle::drawControl(CE_ComboBoxLabel, &comboCopy, p, w);
        }
        break;
    case CE_TabBarTabLabel:
        if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(opt)) {
            if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                QStyleOptionTab myTab = *tab;
                ThemeTabDirection ttd = getTabDirection(myTab.shape);
                bool verticalTabs = ttd == kThemeTabWest || ttd == kThemeTabEast;
                myTab.rect.setHeight(myTab.rect.height() - 2);
                if (verticalTabs) {
                    p->save();
                    p->translate((ttd == kThemeTabWest) ? -2 : 0, 0);
                }
                QCommonStyle::drawControl(ce, &myTab, p, w);
                if (verticalTabs)
                    p->restore();
            } else {
                QCommonStyle::drawControl(ce, tab, p, w);
            }
        }
        break;
    default:
        if (d->useHITheme)
            d->HIThemeDrawControl(ce, opt, p, w);
        else
            d->AppManDrawControl(ce, opt, p, w);
        break;
    }
}

/*! \reimp */
QRect QMacStyle::subElementRect(SubElement sr, const QStyleOption *opt, const QWidget *w) const
{
    QRect rect;
    switch (sr) {
    default:
        if (d->useHITheme)
            rect = d->HIThemeSubElementRect(sr, opt, w);
        else
            rect = d->AppManSubElementRect(sr, opt, w);
        break;
    case SE_TabWidgetTabContents:
        rect = QWindowsStyle::subElementRect(sr, opt, w);
        if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
            if (const QStyleOptionTabWidgetFrame *twf
                   = qstyleoption_cast<const QStyleOptionTabWidgetFrame *>(opt)) {
                switch (getTabDirection(twf->shape)) {
                case kThemeTabNorth:
                    rect.adjust(0, 8, 0, 0);
                    break;
                case kThemeTabSouth:
                    rect.adjust(0, 0, 0, -8);
                    break;
                case kThemeTabWest:
                    rect.adjust(8, 0, 0, 0);
                    break;
                case kThemeTabEast:
                    rect.adjust(0, 0, -8, 0);
                    break;
                }
            }
        }
        break;
    case SE_RadioButtonContents:
    case SE_CheckBoxContents:
        {
            QRect ir = visualRect(opt->direction, opt->rect,
                                  subElementRect(sr == SE_RadioButtonContents
                                                        ? SE_RadioButtonIndicator
                                                        : SE_CheckBoxIndicator, opt, w));
            rect.setRect(ir.right() + 2, opt->rect.y(),
                         opt->rect.width() - ir.width() - 2, opt->rect.height());
            rect = visualRect(opt->direction, opt->rect, rect);
            break;
        }
        break;
    }
    return rect;
}

/*! \reimp */
void QMacStyle::drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt, QPainter *p,
                                   const QWidget *w) const
{
    switch (cc) {
    default:
        if (d->useHITheme)
            d->HIThemeDrawComplexControl(cc, opt, p, w);
        else
            d->AppManDrawComplexControl(cc, opt, p, w);
    case CC_ToolButton:
        if (w && qobject_cast<QToolBar *>(w->parentWidget())) {
            if (const QStyleOptionToolButton *tb = qstyleoption_cast<const QStyleOptionToolButton *>(opt)) {
                if (tb->subControls & QStyle::SC_ToolButtonMenu) {
                    QStyleOption arrowOpt(0);
                    arrowOpt.rect = subControlRect(cc, tb, QStyle::SC_ToolButtonMenu, w);
                    arrowOpt.rect.setY(arrowOpt.rect.y() + arrowOpt.rect.height() / 2);
                    arrowOpt.rect.setHeight(arrowOpt.rect.height() / 2);
                    arrowOpt.state = tb->state;
                    arrowOpt.palette = tb->palette;
                    drawPrimitive(QStyle::PE_IndicatorArrowDown, &arrowOpt, p, w);
                }
                if (tb->state & QStyle::State_On) {
                    QPen oldPen = p->pen();
                    p->setPen(QColor(0, 0, 0, 0x3a));
                    p->fillRect(tb->rect.adjusted(1, 1, -1, -1), QColor(0, 0, 0, 0x12));
                    p->drawLine(tb->rect.left() + 1, tb->rect.top(),
                                tb->rect.right() - 1, tb->rect.top());
                    p->drawLine(tb->rect.left() + 1, tb->rect.bottom(),
                                tb->rect.right() - 1, tb->rect.bottom());
                    p->drawLine(tb->rect.topLeft(), tb->rect.bottomLeft());
                    p->drawLine(tb->rect.topRight(), tb->rect.bottomRight());
                    p->setPen(oldPen);
                }
            }
        } else {
            if (d->useHITheme)
                d->HIThemeDrawComplexControl(cc, opt, p, w);
            else
                d->AppManDrawComplexControl(cc, opt, p, w);
        }
        drawControl(CE_ToolButtonLabel, opt, p, w);
        break;
    }
}

/*! \reimp */
QStyle::SubControl QMacStyle::hitTestComplexControl(ComplexControl cc,
                                                    const QStyleOptionComplex *opt,
                                                    const QPoint &pt, const QWidget *w) const
{
    SubControl sc = QStyle::SC_None;
    switch (cc) {
    case CC_ComboBox:
        if (const QStyleOptionComboBox *cmb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            sc = QWindowsStyle::hitTestComplexControl(cc, cmb, pt, w);
            if (!cmb->editable && sc != QStyle::SC_None)
                sc = SC_ComboBoxArrow;  // A bit of a lie, but what we want
        }
        break;
    default:
        if (d->useHITheme)
            sc = d->HIThemeHitTestComplexControl(cc, opt, pt, w);
        else
            sc = d->AppManHitTestComplexControl(cc, opt, pt, w);
        break;
    }
    return sc;
}

/*! \reimp */
QRect QMacStyle::subControlRect(ComplexControl cc, const QStyleOptionComplex *opt, SubControl sc,
                                const QWidget *w) const
{
    QRect ret;
    switch (cc) {
    default:
        if (d->useHITheme)
            ret = d->HIThemeSubControlRect(cc, opt, sc, w);
        else
            ret = d->AppManSubControlRect(cc, opt, sc, w);
    case CC_SpinBox:
        if (const QStyleOptionSpinBox *spin = qstyleoption_cast<const QStyleOptionSpinBox *>(opt)) {
            const int spinner_w = 14,
                      fw = pixelMetric(PM_SpinBoxFrameWidth, spin, w);
            switch (sc) {
            case SC_SpinBoxUp:
            case SC_SpinBoxDown:
                if (d->useHITheme)
                    ret = d->HIThemeSubControlRect(cc, opt, sc, w);
                else
                    ret = d->AppManSubControlRect(cc, opt, sc, w);
                break;
            case SC_SpinBoxEditField:
                ret.setRect(fw, fw,
                            spin->rect.width() - spinner_w - fw * 2 - macSpinBoxSep + 1,
                            spin->rect.height() - fw * 2 + 1);
                ret = visualRect(spin->direction, spin->rect, ret);
                break;
            case SC_SpinBoxFrame:
                ret.setRect(1, 1, spin->rect.width() - spinner_w - macSpinBoxSep - 1,
                            spin->rect.height() - 1);
                ret = visualRect(spin->direction, spin->rect, ret);
                break;
            default:
                ret = QWindowsStyle::subControlRect(cc, spin, sc, w);
                break;
            }
        }
        break;
    }
    return ret;
}

/*! \reimp */
QSize QMacStyle::sizeFromContents(ContentsType ct, const QStyleOption *opt,
                                  const QSize &csz, const QWidget *widget) const
{
    QSize sz(csz);
    switch (ct) {
    case QStyle::CT_SpinBox:
        sz.setWidth(sz.width() + macSpinBoxSep);
        break;
    case QStyle::CT_TabBarTab: {
            bool newStyleTabs =
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_4)
                QSysInfo::MacintoshVersion >= QSysInfo::MV_10_4 ? true :
#endif
                false;
            if (!newStyleTabs) {
                SInt32 tabh = sz.height();
                SInt32 overlap = 0;
                switch (qt_aqua_size_constrain(widget)) {
                case QAquaSizeUnknown:
                case QAquaSizeLarge:
                    GetThemeMetric(kThemeLargeTabHeight, &tabh);
                    GetThemeMetric(kThemeMetricTabFrameOverlap, &overlap);
                    break;
                case QAquaSizeMini:
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
                    if (QSysInfo::MacintoshVersion >= QSysInfo::MV_10_3) {
                        GetThemeMetric(kThemeMetricMiniTabHeight, &tabh);
                        GetThemeMetric(kThemeMetricMiniTabFrameOverlap, &overlap);
                        break;
                    }
                case QAquaSizeSmall:
                    GetThemeMetric(kThemeSmallTabHeight, &tabh);
                    GetThemeMetric(kThemeMetricSmallTabFrameOverlap, &overlap);
#endif
                default:
                     break;
                }
                tabh += overlap;
                if (sz.height() < tabh)
                    sz.rheight() = tabh;
            } else {
                QWindowsStyle::sizeFromContents(ct, opt, csz, widget);
            }
        break; }
    case QStyle::CT_PushButton:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, widget);
        sz = QSize(sz.width() + 16, sz.height()); // No idea why, but it was in the old style.
        break;
    case QStyle::CT_MenuItem:
        if (const QStyleOptionMenuItem *mi = qstyleoption_cast<const QStyleOptionMenuItem *>(opt)) {
            int maxpmw = mi->maxIconWidth;
            int w = sz.width(),
                h = sz.height();
            if (mi->menuItemType == QStyleOptionMenuItem::Separator) {
                w = 10;
                SInt16 ash;
                GetThemeMenuSeparatorHeight(&ash);
                h = ash;
            } else {
                h = qMax(h, mi->fontMetrics.height() + 2);
                if (!mi->icon.isNull())
                    h = qMax(h, mi->icon.pixmap(pixelMetric(PM_SmallIconSize), QIcon::Normal).height() + 4);
            }
            if (mi->text.contains('\t'))
                w += 12;
            if (mi->menuItemType == QStyleOptionMenuItem::SubMenu)
                w += 20;
            if (maxpmw)
                w += maxpmw + 6;
            // add space for a check. All items have place for a check too.
            w += 20;
            if (widget && qobject_cast<QComboBox*>(widget->parentWidget())
                    && widget->parentWidget()->isVisible()) {
                QStyleOptionComboBox cmb;
                cmb.init(widget->parentWidget());
                cmb.editable = false;
                cmb.subControls = QStyle::SC_ComboBoxEditField;
                cmb.activeSubControls = QStyle::SC_None;
                w = qMax(w, subControlRect(QStyle::CC_ComboBox, &cmb,
                                                   QStyle::SC_ComboBoxEditField,
                                                   widget->parentWidget()).width());
            } else {
                w += 12;
            }
            sz = QSize(w, h);
        }
        break;
    case CT_ToolButton:
        sz.rwidth() += 10;
        sz.rheight() += 10;
        return sz;
    case CT_ComboBox:
        sz.rwidth() += 37;
        break;
    default:
        sz = QWindowsStyle::sizeFromContents(ct, opt, csz, widget);
    }
    QSize macsz;
    if (qt_aqua_size_constrain(widget, ct, sz, &macsz) != QAquaSizeUnknown) {
        if (macsz.width() != -1)
            sz.setWidth(macsz.width());
        if (macsz.height() != -1)
            sz.setHeight(macsz.height());
    }
    // Adjust size to within Aqua guidelines
    if (ct == QStyle::CT_PushButton || ct == QStyle::CT_ToolButton || ct == QStyle::CT_ComboBox) {
        if (const QStyleOptionComboBox *combo = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
            if (combo->editable) {
                sz.rheight() += 1;
                return sz;
            }
        }
        if (d->useHITheme)
            d->HIThemeAdjustButtonSize(ct, sz, widget);
        else
            d->AppManAdjustButtonSize(ct, sz, widget);
    }
    return sz;
}

/*!
    \reimp
*/
void QMacStyle::drawItemText(QPainter *p, const QRect &r, int flags, const QPalette &pal,
                             bool enabled, const QString &text, QPalette::ColorRole textRole) const
{
    if(flags & Qt::TextShowMnemonic)
        flags |= Qt::TextHideMnemonic;
    QWindowsStyle::drawItemText(p, r, flags, pal, enabled, text, textRole);
}

#endif


/*!
  \reimp
*/
bool QMacStyle::event(QEvent *e)
{
    if(e->type() == QEvent::FocusIn) {
        QWidget *f = 0;
        if(QApplication::focusWidget() &&
           d->focusable(QApplication::focusWidget())) {
            f = QApplication::focusWidget();
            QWidget *top = f->parentWidget();
            while (top && !top->isWindow() && !(top->windowType() == Qt::SubWindow))
                top = top->parentWidget();
#ifndef QT_NO_MAINWINDOW
            if (qobject_cast<QMainWindow *>(top)) {
                QWidget *central = static_cast<QMainWindow *>(top)->centralWidget();
                for (const QWidget *par = f; par; par = par->parentWidget()) {
                    if (par == central) {
                        top = central;
                        break;
                    }
                    if (par->isWindow())
                        break;
                }
            }
#endif
            if (!(top && (f->width() < top->width() - 30 || f->height() < top->height() - 40)))
                f = 0;
        }
        if (f) {
#if 1
            if(!d->focusWidget)
                d->focusWidget = new QFocusFrame(QApplication::focusWidget());
            d->focusWidget->setWidget(QApplication::focusWidget());
#endif
        } else if(d->focusWidget) {
            d->focusWidget->setWidget(0);
        }
    } else if(e->type() == QEvent::FocusOut) {
        if(d->focusWidget)
            d->focusWidget->setWidget(0);
    }
    return false;
}

