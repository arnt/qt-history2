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
#include "qpixmap.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtimer.h"
#include "qtooltip.h"
#include "qwindowsstyle.h"
#ifndef QT_NO_WORKSPACE
#include "qworkspace.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#endif

#include "private/qapplication_p.h"
#include "private/qinternal_p.h"
#include "private/qtitlebar_p.h"
#include "private/qwidget_p.h"

#if 0
class QTitleBarTip : public QToolTip
{
public:
    QTitleBarTip(QWidget * parent) : QToolTip(parent) { }

    void maybeTip(const QPoint &pos)
    {
        if (!qt_cast<QTitleBar*>(parentWidget()))
            return;
        QTitleBar *t = (QTitleBar *)parentWidget();

        QString tipstring;
        QStyle::SubControl ctrl = t->style().querySubControl(QStyle::CC_TitleBar, t, pos);
        QRect controlR = t->style().querySubControlMetrics(QStyle::CC_TitleBar, t, ctrl);

        QWidget *window = t->window();
        if (window) {
            switch(ctrl) {
            case QStyle::SC_TitleBarSysMenu:
                if (t->testWFlags(Qt::WStyle_SysMenu))
                    tipstring = QTitleBar::tr("System Menu");
                break;

            case QStyle::SC_TitleBarShadeButton:
                if (t->testWFlags(Qt::WStyle_Tool) && t->testWFlags(Qt::WStyle_MinMax))
                    tipstring = QTitleBar::tr("Shade");
                break;

            case QStyle::SC_TitleBarUnshadeButton:
                if (t->testWFlags(Qt::WStyle_Tool) && t->testWFlags(Qt::WStyle_MinMax))
                    tipstring = QTitleBar::tr("Unshade");
                break;

            case QStyle::SC_TitleBarNormalButton:
            case QStyle::SC_TitleBarMinButton:
                if (!t->testWFlags(Qt::WStyle_Tool) && t->testWFlags(Qt::WStyle_Minimize)) {
                    if(window->isMinimized())
                        tipstring = QTitleBar::tr("Normalize");
                    else
                        tipstring = QTitleBar::tr("Minimize");
                }
                break;

            case QStyle::SC_TitleBarMaxButton:
                if (!t->testWFlags(Qt::WStyle_Tool) && t->testWFlags(Qt::WStyle_Maximize))
                    tipstring = QTitleBar::tr("Maximize");
                break;

            case QStyle::SC_TitleBarCloseButton:
                if (t->testWFlags(Qt::WStyle_SysMenu))
                    tipstring = QTitleBar::tr("Close");
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

class QTitleBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QTitleBar)
public:
    QTitleBarPrivate()
        : toolTip(0), act(0), window(0), movable(1), pressed(0), autoraise(0)
    {
    }

    QStyle::SCFlags buttonDown;
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

inline int QTitleBarPrivate::titleBarState() const
{
    uint state = window ? window->windowState() : 0;
    return (int)state;
}

#define d d_func()
#define q q_func()

QStyleOptionTitleBar QTitleBarPrivate::getStyleOption() const
{
    QStyleOptionTitleBar opt(0);
    opt.init(q);
    opt.text = q->windowTitle();
    opt.icon = q->windowIcon();
    opt.parts = QStyle::SC_All;
    opt.activeParts = QStyle::SC_None;
    opt.titleBarState = titleBarState();
    opt.titleBarFlags = q->getWFlags();
    return opt;
}

QTitleBar::QTitleBar(QWidget *w, QWidget *parent)
    : QWidget(*new QTitleBarPrivate, parent, Qt::WStyle_Customize | Qt::WStyle_NoBorder)
{
    d->window = w;
    d->buttonDown = QStyle::SC_None;
    d->act = 0;
    if (w) {
        setWFlags(static_cast<QTitleBar *>(w)->getWFlags() & ~Qt::WType_Mask);
        if (w->minimumSize() == w->maximumSize())
            clearWFlags(Qt::WStyle_Maximize);
        setWindowTitle(w->windowTitle());
    } else {
        setWFlags(Qt::WStyle_Customize);
    }

    d->readColors();
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    setMouseTracking(true);
}

QTitleBar::~QTitleBar()
{
}

#ifdef Q_WS_WIN
extern QRgb qt_colorref2qrgb(COLORREF col);
#endif

void QTitleBarPrivate::readColors()
{
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
        pal.setColor(QPalette::Active, QPalette::Highlight, qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION)));
        pal.setColor(QPalette::Inactive, QPalette::Highlight, qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION)));
        pal.setColor(QPalette::Active, QPalette::HighlightedText, qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT)));
        pal.setColor(QPalette::Inactive, QPalette::HighlightedText, qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT)));
        if (QSysInfo::WindowsVersion != QSysInfo::WV_95 && QSysInfo::WindowsVersion != QSysInfo::WV_NT) {
            colorsInitialized = true;
            BOOL gradient;
            QT_WA({
                SystemParametersInfo(SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0);
            } , {
                SystemParametersInfoA(SPI_GETGRADIENTCAPTIONS, 0, &gradient, 0);
            });
            if (gradient) {
                pal.setColor(QPalette::Active, QPalette::Base, qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTACTIVECAPTION)));
                pal.setColor(QPalette::Inactive, QPalette::Base, qt_colorref2qrgb(GetSysColor(COLOR_GRADIENTINACTIVECAPTION)));
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

void QTitleBar::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::ModifiedChange)
        update();
    QWidget::changeEvent(ev);
}

void QTitleBar::mousePressEvent(QMouseEvent *e)
{
    if (!d->act)
        emit doActivate();
    if (e->button() == Qt::LeftButton) {
        d->pressed = true;
        QStyleOptionTitleBar opt = d->getStyleOption();
        QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, &opt, e->pos(), this);
        switch (ctrl) {
        case QStyle::SC_TitleBarSysMenu:
            if (testWFlags(Qt::WStyle_SysMenu) && !testWFlags(Qt::WStyle_Tool)) {
                d->buttonDown = QStyle::SC_None;
                static QTime *t = 0;
                static QTitleBar *tc = 0;
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
            if (testWFlags(Qt::WStyle_MinMax) && testWFlags(Qt::WStyle_Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarNormalButton:
            if (testWFlags(Qt::WStyle_Minimize) && !testWFlags(Qt::WStyle_Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarMinButton:
            if (testWFlags(Qt::WStyle_Minimize) && !testWFlags(Qt::WStyle_Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarMaxButton:
            if (testWFlags(Qt::WStyle_Maximize) && !testWFlags(Qt::WStyle_Tool))
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarCloseButton:
            if (testWFlags(Qt::WStyle_SysMenu))
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

void QTitleBar::contextMenuEvent(QContextMenuEvent *e)
{
    QStyleOptionTitleBar opt = d->getStyleOption();
    QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, &opt, e->pos(), this);
    if(ctrl == QStyle::SC_TitleBarLabel || ctrl == QStyle::SC_TitleBarSysMenu) {
        e->accept();
        emit popupOperationMenu(e->globalPos());
    } else {
        e->ignore();
    }
}

void QTitleBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && d->pressed) {
        e->accept();
        QStyleOptionTitleBar opt = d->getStyleOption();
        QStyle::SCFlags ctrl = style().querySubControl(QStyle::CC_TitleBar, &opt, e->pos(), this);

        if (ctrl == d->buttonDown) {
            switch(ctrl) {
            case QStyle::SC_TitleBarShadeButton:
            case QStyle::SC_TitleBarUnshadeButton:
                if(testWFlags(Qt::WStyle_MinMax) && testWFlags(Qt::WStyle_Tool))
                    emit doShade();
                break;

            case QStyle::SC_TitleBarNormalButton:
                if(testWFlags(Qt::WStyle_MinMax) && !testWFlags(Qt::WStyle_Tool))
                    emit doNormal();
                break;

            case QStyle::SC_TitleBarMinButton:
                if(testWFlags(Qt::WStyle_Minimize) && !testWFlags(Qt::WStyle_Tool)) {
                    if (d->window && d->window->isMinimized())
                        emit doNormal();
                    else
                        emit doMinimize();
                }
                break;

            case QStyle::SC_TitleBarMaxButton:
                if(testWFlags(Qt::WStyle_Maximize) && !testWFlags(Qt::WStyle_Tool)) {
                    if(d->window && d->window->isMaximized())
                        emit doNormal();
                    else
                        emit doMaximize();
                }
                break;

            case QStyle::SC_TitleBarCloseButton:
                if(testWFlags(Qt::WStyle_SysMenu)) {
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
        d->buttonDown = QStyle::SC_None;
        repaint();
        d->pressed = false;
    } else {
        e->ignore();
    }
}

void QTitleBar::mouseMoveEvent(QMouseEvent *e)
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
            QStyle::SCFlags last_ctrl = d->buttonDown;
            QStyleOptionTitleBar opt = d->getStyleOption();
            d->buttonDown = style().querySubControl(QStyle::CC_TitleBar, &opt, e->pos(), this);
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
#ifndef QT_NO_WORKSPACE
                if(d->window && d->window->parentWidget()->inherits("QWorkspaceChild")) {
                    QWorkspace *workspace = qt_cast<QWorkspace*>(d->window->parentWidget()->parentWidget());
                    if(workspace) {
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
                }
#endif
                QPoint pp = p - d->moveOffset;
                if (!parentWidget()->isMaximized())
                    parentWidget()->move(pp);
            }
        } else {
            QStyle::SCFlags last_ctrl = d->buttonDown;
            d->buttonDown = QStyle::SC_None;
            if(d->buttonDown != last_ctrl)
                repaint();
        }
        break;
    }
}

void QTitleBar::resizeEvent(QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    cutText();
}

void QTitleBar::paintEvent(QPaintEvent *)
{
    QStyleOptionTitleBar opt = d->getStyleOption();
    opt.parts = QStyle::SC_TitleBarLabel;
    opt.activeParts = d->buttonDown;
    if (testWFlags(Qt::WStyle_SysMenu)) {
        if (testWFlags(Qt::WStyle_Tool)) {
            opt.parts |= QStyle::SC_TitleBarCloseButton;
            if (d->window && testWFlags(Qt::WStyle_MinMax)) {
                if (d->window->isMinimized())
                    opt.parts |= QStyle::SC_TitleBarUnshadeButton;
                else
                    opt.parts |= QStyle::SC_TitleBarShadeButton;
            }
        } else {
            opt.parts |= QStyle::SC_TitleBarSysMenu | QStyle::SC_TitleBarCloseButton;
            if (d->window && testWFlags(Qt::WStyle_Minimize)) {
                if(d->window && d->window->isMinimized())
                    opt.parts |= QStyle::SC_TitleBarNormalButton;
                else
                    opt.parts |= QStyle::SC_TitleBarMinButton;
            }
            if (d->window && testWFlags(Qt::WStyle_Maximize) && !d->window->isMaximized())
                opt.parts |= QStyle::SC_TitleBarMaxButton;
        }
    }

    QStyle::SCFlags under_mouse = QStyle::SC_None;
    if(autoRaise() && underMouse()) {
        under_mouse = style().querySubControl(QStyle::CC_TitleBar, &opt,
                                              mapFromGlobal(QCursor::pos()), this);
        opt.parts ^= under_mouse;
    }
    opt.palette.setCurrentColorGroup(usesActiveColor() ? QPalette::Active : QPalette::Inactive);

    QPainter p(this);
    style().drawComplexControl(QStyle::CC_TitleBar, &opt, &p, this);
    if (under_mouse != QStyle::SC_None) {
        opt.state |= QStyle::Style_MouseOver;
        opt.parts = under_mouse;
        style().drawComplexControl(QStyle::CC_TitleBar, &opt, &p, this);
    }
}

void QTitleBar::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->button() != Qt::LeftButton) {
        e->ignore();
        return;
    }
    e->accept();
    QStyleOptionTitleBar opt = d->getStyleOption();
    switch (style().querySubControl(QStyle::CC_TitleBar, &opt, e->pos(), this)) {
    case QStyle::SC_TitleBarLabel:
        emit doubleClicked();
        break;

    case QStyle::SC_TitleBarSysMenu:
        if (testWFlags(Qt::WStyle_SysMenu))
            emit doClose();
        break;

    default:
        break;
    }
}

void QTitleBar::cutText()
{
    QFontMetrics fm(font());
    QStyleOptionTitleBar opt = d->getStyleOption();
    int maxw = style().querySubControlMetrics(QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarLabel,
                                              this).width();
    if (!d->window)
        return;

    QString txt = d->window->windowTitle();
    if (qt_cast<QWindowsStyle *>(&style()) && d->window && d->window->isWindowModified())
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


void QTitleBar::leaveEvent(QEvent *)
{
    if(autoRaise() && !d->pressed)
        repaint();
}

void QTitleBar::enterEvent(QEvent *)
{
    if(autoRaise() && !d->pressed)
        repaint();
    QEvent e(QEvent::Leave);
    QApplication::sendEvent(parentWidget(), &e);
}

void QTitleBar::setActive(bool active)
{
    if (d->act == active)
        return ;

    d->act = active;
    update();
}

bool QTitleBar::isActive() const
{
    return d->act;
}

bool QTitleBar::usesActiveColor() const
{
    return (isActive() && isActiveWindow()) ||
           (!window() && topLevelWidget()->isActiveWindow());
}

QWidget *QTitleBar::window() const
{
    return d->window;
}

bool QTitleBar::event(QEvent *e)
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
        QRect menur = style().querySubControlMetrics(QStyle::CC_TitleBar, &opt,
                                                     QStyle::SC_TitleBarSysMenu, this);
        QPixmap icon = windowIcon();
        if (icon.width() > menur.width()) {
            // try to keep something close to the same aspect
            int aspect = (icon.height() * 100) / icon.width();
            int newh = (aspect * menur.width()) / 100;
            icon.convertFromImage(icon.convertToImage().smoothScale(menur.width(),
                                                                     newh));
            QWidget::setWindowIcon(icon);
        } else if (icon.height() > menur.height()) {
            // try to keep something close to the same aspect
            int aspect = (icon.width() * 100) / icon.height();
            int neww = (aspect * menur.height()) / 100;
            icon.convertFromImage(icon.convertToImage().smoothScale(neww,
                                                                     menur.height()));
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

void QTitleBar::setMovable(bool b)
{
    d->movable = b;
}

bool QTitleBar::isMovable() const
{
    return d->movable;
}

void QTitleBar::setAutoRaise(bool b)
{
    d->autoraise = b;
}

bool QTitleBar::autoRaise() const
{
    return d->autoraise;
}

QSize QTitleBar::sizeHint() const
{
    ensurePolished();
    QStyleOptionTitleBar opt = d->getStyleOption();
    QRect menur = style().querySubControlMetrics(QStyle::CC_TitleBar, &opt,
                                                 QStyle::SC_TitleBarSysMenu, this);
    return QSize(menur.width(), style().pixelMetric(QStyle::PM_TitleBarHeight, this));
}

#endif //QT_NO_TITLEBAR
