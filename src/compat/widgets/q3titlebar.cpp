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

#include "qplatformdefs.h"

#ifndef QT_NO_TITLEBAR

#include "qapplication.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qevent.h"
#include "qimage.h"
#include "qpainter.h"
#include "qiodevice.h"
#include "qpixmap.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtimer.h"
#include "qtooltip.h"
#include "qdebug.h"
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#include "private/qapplication_p.h"
#include "private/qinternal_p.h"
#include "private/q3titlebar_p.h"
#include "private/qwidget_p.h"

#if 0
class Q3TitleBarTip : public QToolTip
{
public:
    Q3TitleBarTip(QWidget * parent) : QToolTip(parent) { }

    void maybeTip(const QPoint &pos)
    {
        if (!qt_cast<Q3TitleBar*>(parentWidget()))
            return;
        Q3TitleBar *t = (Q3TitleBar *)parentWidget();

        QString tipstring;
        QStyle::SubControl ctrl = t->style()->hitTestComplexControl(QStyle::CC_TitleBar, t, pos);
        QRect controlR = t->style()->subControlRect(QStyle::CC_TitleBar, t, ctrl);

        QWidget *window = t->window();
        if (window) {
            switch(ctrl) {
            case QStyle::SC_TitleBarSysMenu:
                if (t->testWFlags(Qt::WStyle_SysMenu))
                    tipstring = Q3TitleBar::tr("System Menu");
                break;

            case QStyle::SC_TitleBarShadeButton:
                if ((t->windowType() == Qt::Tool) && t->testWFlags(Qt::WStyle_MinMax))
                    tipstring = Q3TitleBar::tr("Shade");
                break;

            case QStyle::SC_TitleBarUnshadeButton:
                if ((t->windowType() == Qt::Tool) && t->testWFlags(Qt::WStyle_MinMax))
                    tipstring = Q3TitleBar::tr("Unshade");
                break;

            case QStyle::SC_TitleBarNormalButton:
            case QStyle::SC_TitleBarMinButton:
                if (!(t->windowType() == Qt::Tool) && t->testWFlags(Qt::WStyle_Minimize)) {
                    if(window->isMinimized())
                        tipstring = Q3TitleBar::tr("Normalize");
                    else
                        tipstring = Q3TitleBar::tr("Minimize");
                }
                break;

            case QStyle::SC_TitleBarMaxButton:
                if (!(t->windowType() == Qt::Tool) && t->testWFlags(Qt::WStyle_Maximize))
                    tipstring = Q3TitleBar::tr("Maximize");
                break;

            case QStyle::SC_TitleBarCloseButton:
                if (t->testWFlags(Qt::WStyle_SysMenu))
                    tipstring = Q3TitleBar::tr("Close");
                break;

            default:
                break;
            }
        }

        if (tipstring.isEmpty() && t->window()) {
            if (t->windowTitle() != t->window()->windowTitle())
                tipstring = t->window()->windowTitle();
        }
        if(!tipstring.isEmpty())
            tip(controlR, tipstring);
    }
};
#endif

class Q3TitleBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(Q3TitleBar)
public:
    Q3TitleBarPrivate()
        : toolTip(0), act(0), window(0), movable(1), pressed(0), autoraise(0)
    {
    }

    QStyle::SubControl buttonDown;
    QPoint moveOffset;
    QToolTip *toolTip;
    bool act                    :1;
    QWidget* window;
    bool movable            :1;
    bool pressed            :1;
    bool autoraise          :1;

    int titleBarState() const;
    QStyleOptionTitleBar getStyleOption() const;
    void readColors();
};

inline int Q3TitleBarPrivate::titleBarState() const
{
    uint state = window ? window->windowState() : static_cast<Qt::WindowStates>(Qt::WindowNoState);
    return (int)state;
}

#define d d_func()

QStyleOptionTitleBar Q3TitleBarPrivate::getStyleOption() const
{
    Q_Q(const Q3TitleBar);
    QStyleOptionTitleBar opt;
    opt.init(q);
    opt.text = q->windowTitle();
    opt.icon = q->windowIcon();
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_None;
    opt.titleBarState = titleBarState();
    opt.titleBarFlags = q->windowFlags();
    return opt;
}

Q3TitleBar::Q3TitleBar(QWidget *w, QWidget *parent)
    : QWidget(*new Q3TitleBarPrivate, parent, Qt::WStyle_Customize | Qt::WStyle_NoBorder)
{
    d->window = w;
    d->buttonDown = QStyle::SC_None;
    d->act = 0;
    if (w) {
        overrideWindowFlags(w->windowFlags() & ~Qt::WType_Mask);
        if (w->minimumSize() == w->maximumSize())
            overrideWindowFlags(windowFlags() & ~Qt::WStyle_Maximize);
        setWindowTitle(w->windowTitle());
    }

    d->readColors();
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    setMouseTracking(true);
    setAutoRaise(style()->styleHint(QStyle::SH_TitleBar_AutoRaise, 0, this));
}

Q3TitleBar::~Q3TitleBar()
{
}

QStyleOptionTitleBar Q3TitleBar::getStyleOption() const
{
    return d->getStyleOption();
}

#ifdef Q_WS_WIN
static inline QRgb colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif

void Q3TitleBarPrivate::readColors()
{
    Q_Q(Q3TitleBar);
    QPalette pal = q->palette();

    bool colorsInitialized = false;

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
    if (QApplication::desktopSettingsAware()) {
        pal.setColor(QPalette::Active, QPalette::Highlight, colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION)));
        pal.setColor(QPalette::Inactive, QPalette::Highlight, colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION)));
        pal.setColor(QPalette::Active, QPalette::HighlightedText, colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT)));
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText, colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT)));
        if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
            colorsInitialized = true;
            BOOL gradient;
            QT_WA({
                SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0);
            } , {
                SystemParametersInfoA(SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0);
            });
            if (gradient) {
                pal.setColor(QPalette::Active, QPalette::Base, colorref2qrgb(GetSysColor(COLOR_GRADIENTACTIVECAPTION)));
                pal.setColor(QPalette::Inactive, QPalette::Base, colorref2qrgb(GetSysColor(COLOR_GRADIENTINACTIVECAPTION)));
            } else {
                pal.setColor(QPalette::Active, QPalette::Base, pal.color(QPalette::Active, QPalette::Highlight));
                pal.setColor(QPalette::Inactive, QPalette::Base, pal.color(QPalette::Inactive, QPalette::Highlight));
            }
        }
    }
#endif // Q_WS_WIN
    if (!colorsInitialized) {
        pal.setColor(QPalette::Active, QPalette::Highlight,
                      pal.color(QPalette::Active, QPalette::Highlight));
        pal.setColor(QPalette::Active, QPalette::Base,
                      pal.color(QPalette::Active, QPalette::Highlight));
        pal.setColor(QPalette::Inactive, QPalette::Highlight,
                      pal.color(QPalette::Inactive, QPalette::Dark));
        pal.setColor(QPalette::Inactive, QPalette::Base,
                      pal.color(QPalette::Inactive, QPalette::Dark));
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText,
                      pal.color(QPalette::Inactive, QPalette::Background));
    }

    q->setPalette(pal);
    q->setActive(d->act);
}

void Q3TitleBar::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::ModifiedChange)
        update();
    QWidget::changeEvent(ev);
}

void Q3TitleBar::mousePressEvent(QMouseEvent *e)
{
    if (!d->act)
        emit doActivate();
    if (e->button() == Qt::LeftButton) {
        d->pressed = true;
        QStyleOptionTitleBar opt = d->getStyleOption();
        QStyle::SubControl ctrl = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt,
                                                                 e->pos(), this);
        switch (ctrl) {
        case QStyle::SC_TitleBarSysMenu:
            if ((windowFlags() & Qt::WStyle_SysMenu) && !(windowType() == Qt::Tool)) {
                d->buttonDown = QStyle::SC_None;
                static QTime *t = 0;
                static Q3TitleBar *tc = 0;
                if (!t)
                    t = new QTime;
                if (tc != this || t->elapsed() > QApplication::doubleClickInterval()) {
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
            if ((windowFlags() & Qt::WStyle_MinMax) && (windowType() == Qt::Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarNormalButton:
            if ((windowFlags() & Qt::WStyle_Minimize) && !(windowType() == Qt::Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarMinButton:
            if ((windowFlags() & Qt::WStyle_Minimize) && !(windowType() == Qt::Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarMaxButton:
            if ((windowFlags() & Qt::WStyle_Maximize) && !(windowType() == Qt::Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarCloseButton:
            if ((windowFlags() & Qt::WStyle_SysMenu))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarLabel:
            d->buttonDown = ctrl;
            d->moveOffset = mapToParent(e->pos());
            break;

        default:
            break;
        }
        repaint();
    } else {
        d->pressed = false;
    }
}

void Q3TitleBar::contextMenuEvent(QContextMenuEvent *e)
{
    QStyleOptionTitleBar opt = d->getStyleOption();
    QStyle::SubControl ctrl = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt, e->pos(),
                                                             this);
    if(ctrl == QStyle::SC_TitleBarLabel || ctrl == QStyle::SC_TitleBarSysMenu) {
        e->accept();
        emit popupOperationMenu(e->globalPos());
    } else {
        e->ignore();
    }
}

void Q3TitleBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && d->pressed) {
        e->accept();
        QStyleOptionTitleBar opt = d->getStyleOption();
        QStyle::SubControl ctrl = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt,
                                                                 e->pos(), this);
        if (ctrl == d->buttonDown) {
            d->buttonDown = QStyle::SC_None;
            repaint();
            d->pressed = false;
            switch(ctrl) {
            case QStyle::SC_TitleBarShadeButton:
            case QStyle::SC_TitleBarUnshadeButton:
                if((windowFlags() & Qt::WStyle_MinMax) && (windowType() == Qt::Tool))
                    emit doShade();
                break;

            case QStyle::SC_TitleBarNormalButton:
                if((windowFlags() & Qt::WStyle_MinMax) && !(windowType() == Qt::Tool))
                    emit doNormal();
                break;

            case QStyle::SC_TitleBarMinButton:
                if((windowFlags() & Qt::WStyle_Minimize) && !(windowType() == Qt::Tool)) {
                    if (d->window && d->window->isMinimized())
                        emit doNormal();
                    else
                        emit doMinimize();
                }
                break;

            case QStyle::SC_TitleBarMaxButton:
                if((windowFlags() & Qt::WStyle_Maximize) && !(windowType() == Qt::Tool)) {
                    if(d->window && d->window->isMaximized())
                        emit doNormal();
                    else
                        emit doMaximize();
                }
                break;

            case QStyle::SC_TitleBarCloseButton:
                if((windowFlags() & Qt::WStyle_SysMenu)) {
                    d->buttonDown = QStyle::SC_None;
                    repaint();
                    emit doClose();
                    return;
                }
                break;

            default:
                break;
            }
        }
    } else {
        e->ignore();
    }
}

void Q3TitleBar::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
    switch (d->buttonDown) {
    case QStyle::SC_None:
        if(autoRaise())
            repaint();
        break;
    case QStyle::SC_TitleBarSysMenu:
        break;
    case QStyle::SC_TitleBarShadeButton:
    case QStyle::SC_TitleBarUnshadeButton:
    case QStyle::SC_TitleBarNormalButton:
    case QStyle::SC_TitleBarMinButton:
    case QStyle::SC_TitleBarMaxButton:
    case QStyle::SC_TitleBarCloseButton:
        {
            QStyle::SubControl last_ctrl = d->buttonDown;
            QStyleOptionTitleBar opt = d->getStyleOption();
            d->buttonDown = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt, e->pos(), this);
            if (d->buttonDown != last_ctrl)
                d->buttonDown = QStyle::SC_None;
            repaint();
            d->buttonDown = last_ctrl;
        }
        break;

    case QStyle::SC_TitleBarLabel:
        if (d->buttonDown == QStyle::SC_TitleBarLabel && d->movable && d->pressed) {
            if ((d->moveOffset - mapToParent(e->pos())).manhattanLength() >= 4) {
                QPoint p = mapFromGlobal(e->globalPos());

                QWidget *parent = d->window ? d->window->parentWidget() : 0;
                if(parent && parent->inherits("Q3WorkspaceChild")) {
                    QWidget *workspace = parent->parentWidget();
                    p = workspace->mapFromGlobal(e->globalPos());
                    if (!workspace->rect().contains(p)) {
                        if (p.x() < 0)
                            p.rx() = 0;
                        if (p.y() < 0)
                            p.ry() = 0;
                        if (p.x() > workspace->width())
                            p.rx() = workspace->width();
                        if (p.y() > workspace->height())
                            p.ry() = workspace->height();
                    }
                }

                QPoint pp = p - d->moveOffset;
                if (!parentWidget()->isMaximized())
                    parentWidget()->move(pp);
            }
        } else {
            QStyle::SubControl last_ctrl = d->buttonDown;
            d->buttonDown = QStyle::SC_None;
            if(d->buttonDown != last_ctrl)
                repaint();
        }
        break;
    }
}

void Q3TitleBar::resizeEvent(QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    cutText();
}

void Q3TitleBar::paintEvent(QPaintEvent *)
{
    QStyleOptionTitleBar opt = d->getStyleOption();
    opt.subControls = QStyle::SC_TitleBarLabel;
    opt.activeSubControls = d->buttonDown;
    if ((windowFlags() & Qt::WStyle_SysMenu)) {
        if ((windowType() == Qt::Tool)) {
            opt.subControls |= QStyle::SC_TitleBarCloseButton;
            if (d->window && (windowFlags() & Qt::WStyle_MinMax)) {
                if (d->window->isMinimized())
                    opt.subControls |= QStyle::SC_TitleBarUnshadeButton;
                else
                    opt.subControls |= QStyle::SC_TitleBarShadeButton;
            }
        } else {
            opt.subControls |= QStyle::SC_TitleBarSysMenu | QStyle::SC_TitleBarCloseButton;
            if (d->window && (windowFlags() & Qt::WStyle_Minimize)) {
                if(d->window && d->window->isMinimized())
                    opt.subControls |= QStyle::SC_TitleBarNormalButton;
                else
                    opt.subControls |= QStyle::SC_TitleBarMinButton;
            }
            if (d->window && (windowFlags() & Qt::WStyle_Maximize) && !d->window->isMaximized())
                opt.subControls |= QStyle::SC_TitleBarMaxButton;
        }
    }

    QStyle::SubControl under_mouse = QStyle::SC_None;
    if(autoRaise() && underMouse()) {
        under_mouse = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt,
                                                     mapFromGlobal(QCursor::pos()), this);
        opt.subControls ^= under_mouse;
    }
    opt.palette.setCurrentColorGroup(usesActiveColor() ? QPalette::Active : QPalette::Inactive);

    QPainter p(this);
    style()->drawComplexControl(QStyle::CC_TitleBar, &opt, &p, this);
    if (under_mouse != QStyle::SC_None) {
        opt.state |= QStyle::State_MouseOver;
        opt.subControls = under_mouse;
        style()->drawComplexControl(QStyle::CC_TitleBar, &opt, &p, this);
    }
}

void Q3TitleBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    e->accept();
    QStyleOptionTitleBar opt = d->getStyleOption();
    switch (style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt, e->pos(), this)) {
    case QStyle::SC_TitleBarLabel:
        emit doubleClicked();
        break;

    case QStyle::SC_TitleBarSysMenu:
        if ((windowFlags() & Qt::WStyle_SysMenu))
            emit doClose();
        break;

    default:
        break;
    }
}

void Q3TitleBar::cutText()
{
    QFontMetrics fm(font());
    QStyleOptionTitleBar opt = d->getStyleOption();
    int maxw = style()->subControlRect(QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarLabel,
                                       this).width();
    if (!d->window)
        return;

    QString txt = d->window->windowTitle();
    if (style()->styleHint(QStyle::SH_TitlebarModifyNotification, 0, this) && d->window
        && d->window->isWindowModified())
        txt += " *";

    QString cuttext = txt;
    if (fm.width(txt + "m") > maxw) {
        int i = txt.length();
        int dotlength = fm.width("...");
        while (i>0 && fm.width(txt.left(i)) + dotlength > maxw)
            i--;
        if(i != (int)txt.length())
            cuttext = txt.left(i) + "...";
    }

    setWindowTitle(cuttext);
}


void Q3TitleBar::leaveEvent(QEvent *)
{
    if(autoRaise() && !d->pressed)
        repaint();
}

void Q3TitleBar::enterEvent(QEvent *)
{
    if(autoRaise() && !d->pressed)
        repaint();
    QEvent e(QEvent::Leave);
    QApplication::sendEvent(parentWidget(), &e);
}

void Q3TitleBar::setActive(bool active)
{
    if (d->act == active)
        return ;

    d->act = active;
    update();
}

bool Q3TitleBar::isActive() const
{
    return d->act;
}

bool Q3TitleBar::usesActiveColor() const
{
    return (isActive() && isActiveWindow()) ||
           (!window() && window()->isActiveWindow());
}

QWidget *Q3TitleBar::window() const
{
    return d->window;
}

bool Q3TitleBar::event(QEvent *e)
{
    if (e->type() == QEvent::ApplicationPaletteChange) {
        d->readColors();
        return true;
    } else if (e->type() == QEvent::WindowActivate) {
        setActive(d->act);
    } else if (e->type() == QEvent::WindowDeactivate) {
        bool wasActive = d->act;
        setActive(false);
        d->act = wasActive;
    } else if (e->type() == QEvent::WindowIconChange) {
#ifndef QT_NO_IMAGE_SMOOTHSCALE
        QStyleOptionTitleBar opt = d->getStyleOption();
        QRect menur = style()->subControlRect(QStyle::CC_TitleBar, &opt,
                                              QStyle::SC_TitleBarSysMenu, this);
        QPixmap icon = windowIcon();
        if (icon.width() > menur.width()) {
            // try to keep something close to the same aspect
            int aspect = (icon.height() * 100) / icon.width();
            int newh = (aspect * menur.width()) / 100;
            icon.fromImage(icon.toImage().scale(menur.width(), newh));
            QWidget::setWindowIcon(icon);
        } else if (icon.height() > menur.height()) {
            // try to keep something close to the same aspect
            int aspect = (icon.width() * 100) / icon.height();
            int neww = (aspect * menur.height()) / 100;
            icon.fromImage(icon.toImage().scale(neww, menur.height()));
            QWidget::setWindowIcon(icon);
        }

#endif
        update();
    } else if (e->type() == QEvent::WindowTitleChange) {
        cutText();
        update();
    }

    return QWidget::event(e);
}

void Q3TitleBar::setMovable(bool b)
{
    d->movable = b;
}

bool Q3TitleBar::isMovable() const
{
    return d->movable;
}

void Q3TitleBar::setAutoRaise(bool b)
{
    d->autoraise = b;
}

bool Q3TitleBar::autoRaise() const
{
    return d->autoraise;
}

QSize Q3TitleBar::sizeHint() const
{
    ensurePolished();
    QStyleOptionTitleBar opt = d->getStyleOption();
    QRect menur = style()->subControlRect(QStyle::CC_TitleBar, &opt,
                                          QStyle::SC_TitleBarSysMenu, this);
    return QSize(menur.width(), style()->pixelMetric(QStyle::PM_TitleBarHeight, &opt, this));
}

#endif //QT_NO_TITLEBAR
