/****************************************************************************
**
** Definition of Aqua-style guidelines functions.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qaquastyle_mac.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdockwindow.h>
#include <qevent.h>
#include <qpointer.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlist.h>
#include <qlistbox.h>
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qprogressbar.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qsize.h>
#include <qsizegrip.h>
#include <qslider.h>
#include <qtoolbutton.h>

#include <qmacstyle_mac.h>
#include <private/qt_mac_p.h>
#include <stdlib.h>
class QListViewItem;

#define QMAC_QAQUASTYLE_SIZE_CONSTRAIN

/*****************************************************************************
  QAquaStyle debug facilities
 *****************************************************************************/
//#define DEBUG_SIZE_CONSTRAINT

QAquaFocusWidget::QAquaFocusWidget(bool noerase, QWidget *w)
    : QWidget(w, "magicFocusWidget", (noerase ? (Qt::WResizeNoErase | Qt::WRepaintNoErase) : (Qt::WFlag)0)), d(0)
{
    setFocusPolicy(Qt::NoFocus);
    if(noerase)
        setAttribute(Qt::WA_NoSystemBackground, true);
}
#if 1
/* It's a real bummer I cannot use this, but you'll notice that sometimes
   the widget will scroll "offscreen" and the focus widget will remain visible
   (which looks quite bad). --Sam */
#define FOCUS_WIDGET_PARENT(x) x->topLevelWidget()
#else
#define FOCUS_WIDGET_PARENT(x) (x->isTopLevel() ? 0 : x->parentWidget())
#endif
void QAquaFocusWidget::setFocusWidget(QWidget * widget)
{
    hide();
    if(d) {
        if(d->parentWidget())
            d->parentWidget()->removeEventFilter(this);
        d->removeEventFilter(this);
    }
    d = 0;
    if(widget && widget->parentWidget()) {
        d = widget;
        setParent(FOCUS_WIDGET_PARENT(d));
        move(pos());
        d->installEventFilter(this);
        d->parentWidget()->installEventFilter(this); //we do this so we can trap the ChildAdded event
        QPoint p(widget->mapTo(parentWidget(), QPoint(0, 0)));
        setGeometry(p.x() - focusOutset(), p.y() - focusOutset(),
                    widget->width() + (focusOutset() * 2), widget->height() + (focusOutset() * 2));
        setPalette(widget->palette());
        setMask(QRegion(rect()) - focusRegion());
        stackUnder(d);
        show();
    }
}
bool QAquaFocusWidget::eventFilter(QObject * o, QEvent * e)
{
    if((e->type() == QEvent::ChildInserted || e->type() == QEvent::ChildRemoved) &&
        ((QChildEvent*)e)->child() == this) {
        if(e->type() == QEvent::ChildRemoved)
            o->removeEventFilter(this); //once we're removed, stop listening
        return true; //block child events
    } else if(o == d) {
        switch (e->type()) {
        case QEvent::PaletteChange:
            setPalette(d->palette());
            break;
        case QEvent::Hide:
            hide();
            break;
        case QEvent::Show:
            show();
            break;
        case QEvent::Move: {
            QPoint p(d->mapTo(parentWidget(), QPoint(0, 0)));
            move(p.x() - focusOutset(), p.y() - focusOutset());
            break;
        }
        case QEvent::Resize: {
            QResizeEvent *re = (QResizeEvent*)e;
            resize(re->size().width() + (focusOutset() * 2),
                    re->size().height() + (focusOutset() * 2));
            setMask(QRegion(rect()) - focusRegion());
            break;
        }
        case QEvent::Reparent: {
            QWidget *newp = FOCUS_WIDGET_PARENT(d);
            QPoint p(d->mapTo(newp, QPoint(0, 0)));
            newp->installEventFilter(this);
            setParent(newp);
            show();
            move(p);
            raise();
            break; }
        default:
            break;
        }
    }
    return false;
}

int QAquaFocusWidget::focusOutset() const
{
    SInt32 ret = 0;
    GetThemeMetric(kThemeMetricFocusRectOutset, &ret);
    return ret;
}

struct QAquaAnimatePrivate
{
    QPointer<QWidget> focus; //the focus widget
    QPointer<QPushButton> defaultButton; //default pushbuttons
    int timerID;
    QList<QPointer<QProgressBar> > progressBars; //progress bar information
    QList<const QListViewItem *> lvis;
};

QAquaAnimate::QAquaAnimate()
{
    d = new QAquaAnimatePrivate;
    d->timerID = -1;
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
    //already knew of it
    if(static_cast<QPushButton*>(w) == d->defaultButton || d->progressBars.contains(static_cast<QProgressBar*>(w)))
        return false;

    if(QPushButton *btn = ::qt_cast<QPushButton *>(w)) {
        btn->installEventFilter(this);
        if(btn->isDefault() || (btn->autoDefault() && btn->hasFocus()))
            startAnimate(AquaPushButton, btn);
        return true;
    } else if(QProgressBar *pb = ::qt_cast<QProgressBar *>(w)) {
        pb->installEventFilter(this);
        startAnimate(AquaProgressBar, pb);
        return true;
    }
    return false;
}
void QAquaAnimate::removeWidget(QWidget *w)
{
    if(focusWidget() == w)
        setFocusWidget(0);
    QPushButton *btn = ::qt_cast<QPushButton *>(w);
    if(btn && btn == d->defaultButton) {
        stopAnimate(AquaPushButton, btn);
    } else if(::qt_cast<QProgressBar *>(w)) {
        stopAnimate(AquaProgressBar, static_cast<QProgressBar *>(w));
    }
}

void QAquaAnimate::objDestroyed(QObject *o)
{
    if(o == d->focus)
        setFocusWidget(0);
}

bool QAquaAnimate::animatable(QAquaAnimate::Animates as, const QWidget *w)
{
    if(as == AquaPushButton) {
        if(static_cast<const QPushButton *>(w) == d->defaultButton)
            return true;
    } else if(as == AquaProgressBar) {
        if(d->progressBars.contains(static_cast<QProgressBar *>(const_cast<QWidget *>(w))))
            return true;
    }
    return false;
}

void QAquaAnimate::stopAnimate(QAquaAnimate::Animates as, QWidget *w)
{
    if(as == AquaPushButton && d->defaultButton) {
        QPushButton *tmp = d->defaultButton;
        d->defaultButton = 0;
        if(tmp->isVisible())
            tmp->update();
    } else if(as == AquaProgressBar) {
        d->progressBars.removeAll(static_cast<QProgressBar *>(w));
    }
}

void QAquaAnimate::startAnimate(QAquaAnimate::Animates as, QWidget *w)
{
    if(as == AquaPushButton)
        d->defaultButton = static_cast<QPushButton *>(w);
    else if(as == AquaProgressBar)
        d->progressBars.append(static_cast<QProgressBar *>(w));

    if((d->defaultButton || !d->progressBars.isEmpty()) && d->timerID <= -1)
        d->timerID = startTimer(animateSpeed(AquaListViewItemOpen));
}


void QAquaAnimate::timerEvent(QTimerEvent *)
{
    int animated = 0;
    if(d->defaultButton && d->defaultButton->isEnabled() && d->defaultButton->isVisibleTo(0)
       && (d->defaultButton->isDefault() || (d->defaultButton->autoDefault() && d->defaultButton->hasFocus()))
       && doAnimate(AquaPushButton)) {
        ++animated;
        d->defaultButton->update();
    }
    if(!d->progressBars.isEmpty()) {
        int i = 0;
        while (i < d->progressBars.size()) {
            QProgressBar *pb = d->progressBars.at(i);
            if(!pb) {
                d->progressBars.removeAt(i);
            } else {
                if(pb->totalSteps() == 0 || pb->progress() > 0
                   && pb->progress() < pb->totalSteps()) {
                    if(doAnimate(AquaProgressBar))
                        pb->update();
                }
                ++i;
            }
        }
        animated += i;
    }
    if(!d->lvis.isEmpty()) {
#if 0
        if(doAnimate(AquaListViewItemOpen)) {
            for (int i = 0; i < d->lvis.count(); ++i) {
                ++animated;
                d->lvis.at(i)->repaint();
            }
        }
#endif
    }

    if(!animated) {
        killTimer(d->timerID);
        d->timerID = -1;
    }
}

bool QAquaAnimate::eventFilter(QObject * o, QEvent * e)
{
    //focus
    if(o->isWidgetType() && focusWidget() && focusable((QWidget *)o) &&
       ((e->type() == QEvent::FocusOut && focusWidget() == o) ||
        (e->type() == QEvent::FocusIn && focusWidget() != o)))  { //restore it
        if(((QFocusEvent *)e)->reason() != QFocusEvent::Popup)
            setFocusWidget(0);
    }
    if(o && o->isWidgetType() && e->type() == QEvent::FocusIn) {
        QWidget *w = (QWidget *)o;
        if(focusable(w))
            setFocusWidget(w);
    }
    //animate
    if(QProgressBar *pb = ::qt_cast<QProgressBar *>(o)) {
        switch (e->type()) {
        default:
            break;
        case QEvent::Show:
            if(!d->progressBars.contains(pb))
                startAnimate(AquaProgressBar, pb);
            break;
        case QEvent::Hide:
            d->progressBars.removeAll(pb);
        }
    } else if(QPushButton *btn = ::qt_cast<QPushButton *>(o)) {
        switch (e->type()) {
        default:
            break;
        case QEvent::FocusIn:
            if(btn->autoDefault())
                startAnimate(AquaPushButton, btn);
            break;
        case QEvent::Hide:
            if(btn == d->defaultButton)
                stopAnimate(AquaPushButton, btn);
            break;
        case QEvent::MouseButtonPress:
            // It is very confusing to keep the button pulsing, so just stop the animation.
            stopAnimate(AquaPushButton, btn);
            break;
        case QEvent::Paint:
            if(btn->isDefault() && btn->topLevelWidget()->isActiveWindow())
                startAnimate(AquaPushButton, btn);
            break;
        case QEvent::MouseButtonRelease:
        case QEvent::FocusOut:
        case QEvent::Show: {
            QObjectList list = btn->topLevelWidget()->queryList("QPushButton");
            for (int i = 0; i < list.size(); ++i) {
                QPushButton *pBtn = static_cast<QPushButton *>(list.at(i));
                if ((e->type() == QEvent::FocusOut
                     && (pBtn->isDefault() || (pBtn->autoDefault() && pBtn->hasFocus()))
                     && pBtn != btn)
                    || ((e->type() == QEvent::Show || e->type() == QEvent::MouseButtonRelease)
                        && pBtn->isDefault())) {
                    if (pBtn->topLevelWidget()->isActiveWindow())
                        startAnimate(AquaPushButton, pBtn);
                    break;
                }
            }
            break; }
        }
    }
    return false;
}

const QWidget *QAquaAnimate::focusWidget() const
{
    return d->focus;
}

void QAquaAnimate::setFocusWidget(QWidget *w)
{
    if(w) {
        QWidget *top = w->parentWidget();
        while (!top->isTopLevel() && !top->testWFlags(Qt::WSubWindow))
            top = top->parentWidget();
#ifndef QT_NO_MAINWINDOW
        if(::qt_cast<QMainWindow *>(top)) {
            QWidget *central = static_cast<QMainWindow *>(top)->centerWidget();
            for (const QWidget *par = w; par; par = par->parentWidget()) {
                if(par == central) {
                    top = central;
                    break;
                }
                if(par->isTopLevel())
                    break;
            }
        }
#endif
        if(top && (w->width() < top->width() - 30 || w->height() < top->height() - 40)) {
            if(QComboBox *cmb = ::qt_cast<QComboBox *>(w)) {
                if(cmb->isEditable())
                    w = cmb->lineEdit();
            }
        } else {
            w = 0;
        }
    }
    if(w == d->focus)
        return;
    doFocus(const_cast<QWidget*>(w));
    if(d->focus)
        QObject::disconnect(d->focus, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
    if((d->focus = w))
        QObject::connect(w, SIGNAL(destroyed(QObject*)), this, SLOT(objDestroyed(QObject*)));
}

bool QAquaAnimate::focusable(const QWidget *w) const
{
    QMacStyle *macStyle = ::qt_cast<QMacStyle *>(&w->style());
    QMacStyle::FocusRectPolicy fp = macStyle ? macStyle->focusRectPolicy(w)
                                   : QMacStyle::FocusDefault;
    if(fp == QMacStyle::FocusEnabled)
        return true;
    else if(fp == QMacStyle::FocusDisabled)
        return false;
    return (w && !w->isTopLevel() && w->parentWidget() &&
            (/*::qt_cast<QSpinWidget *>(w) || ::qt_cast<QDateTimeEdit *>(w)
            ||*/ ::qt_cast<QComboBox *>(w)|| ::qt_cast<QListBox *>(w) || w->inherits("QListView")             || (::qt_cast<QLineEdit *>(w) /*&& ::qt_cast<QSpinWidget *>(w->parentWidget())*/)
//           || (w->inherits("QTextEdit") && wstatic_cast<const QTextEdit *>(w)->isReadOnly())
             || (::qt_cast<QFrame *>(w) && ::qt_cast<QLineEdit *>(w)
                 && (static_cast<const QFrame *>(w)->frameStyle() != QFrame::NoFrame
                     || ::qt_cast<QComboBox *>(w->parentWidget())))));
}

#if defined(QMAC_QAQUASTYLE_SIZE_CONSTRAIN) || defined(DEBUG_SIZE_CONSTRAINT)
static QAquaWidgetSize qt_aqua_guess_size(const QWidget *widg, QSize large, QSize small, QSize mini)
{
    if(large == QSize(-1, -1)) {
        if(small != QSize(-1, -1))
            return QAquaSizeSmall;
        if(mini != QSize(-1, -1))
            return QAquaSizeMini;
        return QAquaSizeUnknown;
    } else if(small == QSize(-1, -1)) {
        if(mini != QSize(-1, -1))
            return QAquaSizeMini;
        return QAquaSizeLarge;
    } else if(mini == QSize(-1, -1)) {
        return QAquaSizeLarge;
    }

#ifndef QT_NO_MAINWINDOW
    if(::qt_cast<QDockWindow *>(widg->topLevelWidget()) || getenv("QWIDGET_ALL_SMALL")) {
        //if(small.width() != -1 || small.height() != -1)
            return QAquaSizeSmall;
    } else if(getenv("QWIDGET_ALL_MINI")) {
        return QAquaSizeMini;
    }
#endif

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
    int mini_delta=0;
    if(mini.width() != -1) {
        int delta = mini.width() - widg->width();
        mini_delta += delta * delta;
    }
    if(mini.height() != -1) {
        int delta = mini.height() - widg->height();
        mini_delta += delta * delta;
    }
    if(mini_delta < small_delta && mini_delta < large_delta)
        return QAquaSizeMini;
    else if(small_delta < large_delta)
        return QAquaSizeSmall;
#endif
    return QAquaSizeLarge;
}

ThemeDrawState QAquaAnimate::getDrawState(QStyle::SFlags flags, const QPalette &pal)
{
    ThemeDrawState tds = kThemeStateActive;
    if (flags & QStyle::Style_Down) {
        tds = kThemeStatePressed;
    } else if (qAquaActive(pal)) {
        if (!(flags & QStyle::Style_Enabled))
            tds = kThemeStateUnavailable;
    } else {
        if (flags & QStyle::Style_Enabled)
            tds = kThemeStateInactive;
        else
            tds = kThemeStateUnavailableInactive;
    }
    return tds;
}

static int qt_mac_aqua_get_metric(ThemeMetric met)
{
    SInt32 ret;
    GetThemeMetric(met, &ret);
    return ret;
}
static QSize qt_aqua_get_known_size(QStyle::ContentsType ct, const QWidget *widg, QSize szHint, QAquaWidgetSize sz)
{
    QSize ret(-1, -1);
    if(QSysInfo::MacintoshVersion < QSysInfo::MV_PANTHER && sz == QAquaSizeMini)
        return ret;

    if(sz != QAquaSizeSmall && sz != QAquaSizeLarge && sz != QAquaSizeMini) {
        qDebug("Not sure how to return this..");
        return ret;
    }
    if(widg && widg->testAttribute(Qt::WA_SetFont)) //if you're using a custom font, no constraints for you!
        return ret;

    if(ct == QStyle::CT_CustomBase && widg) {
        if(::qt_cast<QPushButton *>(widg))
            ct = QStyle::CT_PushButton;
        else if(::qt_cast<QRadioButton *>(widg))
            ct = QStyle::CT_RadioButton;
        else if(::qt_cast<QCheckBox *>(widg))
            ct = QStyle::CT_CheckBox;
        else if(::qt_cast<QComboBox *>(widg))
            ct = QStyle::CT_ComboBox;
        else if(::qt_cast<QComboBox *>(widg))
            ct = QStyle::CT_ToolButton;
        else if(::qt_cast<QSlider *>(widg))
            ct = QStyle::CT_Slider;
        else if(::qt_cast<QProgressBar *>(widg))
            ct = QStyle::CT_ProgressBar;
        else if(::qt_cast<QLineEdit *>(widg))
            ct = QStyle::CT_LineEdit;
        else if(widg->inherits("QHeader"))
            ct = QStyle::CT_Header;
        else if(::qt_cast<QMenuBar *>(widg) || widg->inherits("Q3MenuBar"))
            ct = QStyle::CT_MenuBar;
        else if(::qt_cast<QSizeGrip *>(widg))
            ct = QStyle::CT_SizeGrip;
        else
            return ret;
    }

    if(ct == QStyle::CT_PushButton) {
        const QPushButton *psh = static_cast<const QPushButton *>(widg);
        int minw = -1;
        // Aqua Style guidelines restrict the size of OK and Cancel buttons to 68 pixels.
        // However, this doesn't work for German, therefore only do it for English,
        // I suppose it would be better to do some sort of lookups for languages
        // that like to have really long words.
        if(psh->text() == "OK" || psh->text() == "Cancel")
            minw = 69;
        if(sz == QAquaSizeLarge)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricPushButtonHeight));
        else if(sz == QAquaSizeSmall)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricSmallPushButtonHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(minw, qt_mac_aqua_get_metric(kThemeMetricMiniPushButtonHeight));
#endif
#if 0 //Not sure we are applying the rules correctly for RadioButtons/CheckBoxes --Sam
    } else if(ct == QStyle::CT_RadioButton) {
        QRadioButton *rdo = static_cast<QRadioButton *>(widg);
        // Exception for case where multiline radiobutton text requires no size constrainment
        if(rdo->text().find('\n') != -1)
            return ret;
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricRadioButtonHeight));
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallRadioButtonHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniRadioButtonHeight));
#endif
    } else if(ct == QStyle::CT_CheckBox) {
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricCheckBoxHeight));
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallCheckBoxHeight));
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniCheckBoxHeight));
#endif
#endif
    } else if(ct == QStyle::CT_SizeGrip) {
        if(sz == QAquaSizeLarge || sz == QAquaSizeSmall) {
            Rect r;
            Point p = { 0, 0 };
            ThemeGrowDirection dir = kThemeGrowRight | kThemeGrowDown;
            if(QApplication::reverseLayout())
                dir = kThemeGrowLeft | kThemeGrowDown;
            if(GetThemeStandaloneGrowBoxBounds(p, dir, sz != QAquaSizeSmall, &r) == noErr)
                ret = QSize(r.right - r.left, r.bottom - r.top);
        }
    } else if(ct == QStyle::CT_ComboBox) {
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricPopupButtonHeight)+1);
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricSmallPopupButtonHeight)+1);
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricMiniPopupButtonHeight));
#endif
    } else if(ct == QStyle::CT_ToolButton && sz == QAquaSizeSmall) {
        int width = 0, height = 0;
        if(szHint == QSize(-1, -1)) { //just 'guess'..
            const QToolButton *bt = static_cast<const QToolButton *>(widg);
            if(!bt->icon().isNull()) {
                QIconSet::Size sz = QIconSet::Small;
                if(bt->usesBigPixmap())
                    sz = QIconSet::Large;
                QSize iconSize = QIconSet::iconSize(sz);
                QPixmap pm = bt->icon().pixmap(sz, QIconSet::Normal);
                width = qMax(width, qMax(iconSize.width(), pm.width()));
                height = qMax(height, qMax(iconSize.height(), pm.height()));
            }
            if(!bt->text().isNull() && bt->usesTextLabel()) {
                int text_width = bt->fontMetrics().width(bt->text()),
                   text_height = bt->fontMetrics().height();
                if(bt->textPosition() == QToolButton::Under) {
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
    } else if(ct == QStyle::CT_Slider) {
        int w = -1;
        const QSlider *sld = static_cast<const QSlider *>(widg);
        if(sz == QAquaSizeLarge) {
            if(sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricHSliderHeight);
                if(sld->tickmarks() != QSlider::NoMarks)
                    w += qt_mac_aqua_get_metric(kThemeMetricHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricVSliderWidth);
                if(sld->tickmarks() != QSlider::NoMarks)
                    w += qt_mac_aqua_get_metric(kThemeMetricVSliderTickWidth);
            }
        } else if(sz == QAquaSizeSmall) {
            if(sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricSmallHSliderHeight);
                if(sld->tickmarks() != QSlider::NoMarks)
                    w += qt_mac_aqua_get_metric(kThemeMetricSmallHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricSmallVSliderWidth);
                if(sld->tickmarks() != QSlider::NoMarks)
                    w += qt_mac_aqua_get_metric(kThemeMetricSmallVSliderTickWidth);
            }
        }
#if (MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3)
        else if(sz == QAquaSizeMini) {
            if(sld->orientation() == Qt::Horizontal) {
                w = qt_mac_aqua_get_metric(kThemeMetricMiniHSliderHeight);
                if(sld->tickmarks() != QSlider::NoMarks)
                    w += qt_mac_aqua_get_metric(kThemeMetricMiniHSliderTickHeight);
            } else {
                w = qt_mac_aqua_get_metric(kThemeMetricMiniVSliderWidth);
                if(sld->tickmarks() != QSlider::NoMarks)
                    w += qt_mac_aqua_get_metric(kThemeMetricMiniVSliderTickWidth);
            }
        }
#endif
        if(sld->orientation() == Qt::Horizontal)
            ret.setHeight(w);
        else
            ret.setWidth(w);
    } else if(ct == QStyle::CT_ProgressBar) {
        if(sz == QAquaSizeLarge)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricLargeProgressBarThickness));
        else if(sz == QAquaSizeSmall)
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricNormalProgressBarThickness));
    } else if(ct == QStyle::CT_LineEdit) {
        if(!widg || !widg->parentWidget() || !widg->parentWidget()->inherits("QComboBox")) {
            //should I take into account the font dimentions of the lineedit? -Sam
            if(sz == QAquaSizeLarge)
                ret = QSize(-1, 22);
            else
                ret = QSize(-1, 19);
        }
    } else if(ct == QStyle::CT_Header) {
	if(sz == QAquaSizeLarge && (widg && !widg->parentWidget()->inherits("QTable")))
            ret = QSize(-1, qt_mac_aqua_get_metric(kThemeMetricListHeaderHeight));
    } else if(ct == QStyle::CT_MenuBar) {
        if(sz == QAquaSizeLarge) {
            SInt16 size;
            if(!GetThemeMenuBarHeight(&size))
                ret = QSize(-1, size);
        }
    }
    return ret;
}
#endif
QAquaWidgetSize qt_aqua_size_constrain(const QWidget *widg, QStyle::ContentsType ct,
                                       QSize szHint, QSize *insz)
{
#if defined(QMAC_QAQUASTYLE_SIZE_CONSTRAIN) || defined(DEBUG_SIZE_CONSTRAINT)
    if(!widg) {
        if(insz)
            *insz = QSize();
        if(getenv("QWIDGET_ALL_SMALL"))
            return QAquaSizeSmall;
        if(getenv("QWIDGET_ALL_MINI"))
            return QAquaSizeMini;
        return QAquaSizeUnknown;
    }
    QSize large = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeLarge),
          small = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeSmall),
          mini  = qt_aqua_get_known_size(ct, widg, szHint, QAquaSizeMini);
    bool guess_size = false;
    QAquaWidgetSize ret = QAquaSizeUnknown;
    QMacStyle *macStyle = ::qt_cast<QMacStyle *>(&widg->style());
    if(macStyle) {
        QMacStyle::WidgetSizePolicy wsp = macStyle->widgetSizePolicy(widg);
        if(wsp == QMacStyle::SizeDefault)
            guess_size = true;
        else if(wsp == QMacStyle::SizeMini)
            ret = QAquaSizeMini;
        else if(wsp == QMacStyle::SizeSmall)
            ret = QAquaSizeSmall;
        else if(wsp == QMacStyle::SizeLarge)
            ret = QAquaSizeLarge;
    }
    if(guess_size)
        ret = qt_aqua_guess_size(widg, large, small, mini);

    QSize *sz = 0;
    if(ret == QAquaSizeSmall)
        sz = &small;
    else if(ret == QAquaSizeLarge)
        sz = &large;
    else if(ret == QAquaSizeMini)
        sz = &mini;
    if(insz)
        *insz = sz ? *sz : QSize(-1, -1);
#ifdef DEBUG_SIZE_CONSTRAINT
    if(sz) {
        const char *size_desc = "Unknown";
        if(sz == &small)
            size_desc = "Small";
        else if(sz == &large)
            size_desc = "Large";
        else if(sz == &mini)
            size_desc = "Mini";
        qDebug("%s - %s: %s taken (%d, %d) [%d, %d]", widg ? widg->name() : "*Unknown*",
               widg ? widg->className() : "*Unknown*", size_desc, widg->width(), widg->height(),
               sz->width(), sz->height());
    }
#endif
    return ret;
#else
    if(insz)
        *insz = QSize();
    Q_UNUSED(widg);
    Q_UNUSED(ct);
    Q_UNUSED(szHint);
    return QAquaSizeUnknown;
#endif
}

