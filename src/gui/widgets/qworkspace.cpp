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

#include "qworkspace.h"
#ifndef QT_NO_WORKSPACE
#include "qapplication.h"
#include "qbitmap.h"
#include "qcursor.h"
#include "qdatetime.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qhash.h"
#include "qicon.h"
#include "qimage.h"
#include "qlabel.h"
#include "qlayout.h"
#include "qmenubar.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpointer.h"
#include "qscrollbar.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtoolbutton.h"
#include "qtooltip.h"
#include "qvboxwidget.h"
#include "qdebug.h"
#include <private/qwidget_p.h>
#include <private/qwidgetresizehandler_p.h>

class QWorkspaceTitleBarPrivate;
class QWorkspaceTitleBar : public QWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QWorkspaceTitleBar)
    Q_PROPERTY(bool autoRaise READ autoRaise WRITE setAutoRaise)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)

public:
    QWorkspaceTitleBar (QWidget *w, QWidget *parent, Qt::WFlags f = 0);
    ~QWorkspaceTitleBar();

    bool isActive() const;
    bool usesActiveColor() const;

    bool isMovable() const;
    void setMovable(bool);

    bool autoRaise() const;
    void setAutoRaise(bool);

    QWidget *window() const;
    bool isTool() const;

    QSize sizeHint() const;
    QStyleOptionTitleBar getStyleOption() const;

public slots:
    void setActive(bool);

signals:
    void doActivate();
    void doNormal();
    void doClose();
    void doMaximize();
    void doMinimize();
    void doShade();
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);
    void doubleClicked();

protected:
    bool event(QEvent *);
    void resizeEvent(QResizeEvent *);
    void contextMenuEvent(QContextMenuEvent *);
    void changeEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *e);
    void leaveEvent(QEvent *e);
    void paintEvent(QPaintEvent *p);

    virtual void cutText();

private:
    Q_DISABLE_COPY(QWorkspaceTitleBar)
};


class QWorkspaceTitleBarPrivate : public QWidgetPrivate
{
    Q_DECLARE_PUBLIC(QWorkspaceTitleBar)
public:
    QWorkspaceTitleBarPrivate()
        : toolTip(0), act(0), window(0), movable(1), pressed(0), autoraise(0), inevent(0)
    {
    }

    Qt::WFlags flags;
    QStyle::SubControl buttonDown;
    QPoint moveOffset;
    QToolTip *toolTip;
    bool act                    :1;
    QWidget* window;
    bool movable            :1;
    bool pressed            :1;
    bool autoraise          :1;
    bool inevent : 1;

    int titleBarState() const;
    QStyleOptionTitleBar getStyleOption() const;
    void readColors();
};

inline int QWorkspaceTitleBarPrivate::titleBarState() const
{
    uint state = window ? window->windowState() : static_cast<Qt::WindowStates>(Qt::WindowNoState);
    state |= uint(act ? QStyle::State_Active : QStyle::State_None);
    return (int)state;
}

#define d d_func()
#define q q_func()


QStyleOptionTitleBar QWorkspaceTitleBarPrivate::getStyleOption() const
{
    QStyleOptionTitleBar opt;
    opt.init(q);
    opt.text = q->windowTitle();
    //################
    QIcon icon = q->windowIcon();
    QSize s = icon.actualSize(QSize(64, 64));
    opt.icon = icon.pixmap(s);
    opt.subControls = QStyle::SC_All;
    opt.activeSubControls = QStyle::SC_None;
    opt.titleBarState = titleBarState();
    opt.titleBarFlags = flags;
    return opt;
}

QWorkspaceTitleBar::QWorkspaceTitleBar(QWidget *w, QWidget *parent, Qt::WFlags f)
    : QWidget(*new QWorkspaceTitleBarPrivate, parent, Qt::FramelessWindowHint)
{
    if (f == 0 && w)
        f = w->windowFlags();
    d->flags = f;
    d->window = w;
    d->buttonDown = QStyle::SC_None;
    d->act = 0;
    if (w) {
        if (w->minimumSize() == w->maximumSize())
            d->flags &= ~Qt::WindowMaximizeButtonHint;
        setWindowTitle(w->windowTitle());
    }

    d->readColors();
    setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
    setMouseTracking(true);
    setAutoRaise(style()->styleHint(QStyle::SH_TitleBar_AutoRaise, 0, this));
}

QWorkspaceTitleBar::~QWorkspaceTitleBar()
{
}

QStyleOptionTitleBar QWorkspaceTitleBar::getStyleOption() const
{
    return d->getStyleOption();
}

#ifdef Q_WS_WIN
static inline QRgb colorref2qrgb(COLORREF col)
{
    return qRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif

void QWorkspaceTitleBarPrivate::readColors()
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

void QWorkspaceTitleBar::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::ModifiedChange)
        update();
    QWidget::changeEvent(ev);
}

void QWorkspaceTitleBar::mousePressEvent(QMouseEvent *e)
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
            if (d->flags & Qt::WindowSystemMenuHint) {
                d->buttonDown = QStyle::SC_None;
                static QTime *t = 0;
                static QWorkspaceTitleBar *tc = 0;
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
            if (d->flags & Qt::WindowShadeButtonHint)
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarNormalButton:
            if (d->flags & Qt::WindowMinMaxButtonsHint)
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarMinButton:
            if (d->flags & Qt::WindowMinimizeButtonHint)
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarMaxButton:
            if (d->flags & Qt::WindowMaximizeButtonHint)
                d->buttonDown = ctrl;
            break;

        case QStyle::SC_TitleBarCloseButton:
            if (d->flags & Qt::WindowSystemMenuHint)
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

void QWorkspaceTitleBar::contextMenuEvent(QContextMenuEvent *e)
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

void QWorkspaceTitleBar::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton && d->pressed) {
        e->accept();
        QStyleOptionTitleBar opt = d->getStyleOption();
        QStyle::SubControl ctrl = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt,
                                                                 e->pos(), this);
        d->pressed = false;
        if (ctrl == d->buttonDown) {
            d->buttonDown = QStyle::SC_None;
            repaint();
            switch(ctrl) {
            case QStyle::SC_TitleBarShadeButton:
            case QStyle::SC_TitleBarUnshadeButton:
                if(d->flags & Qt::WindowShadeButtonHint)
                    emit doShade();
                break;

            case QStyle::SC_TitleBarNormalButton:
                if(d->flags & Qt::WindowMaximizeButtonHint)
                    emit doNormal();
                break;

            case QStyle::SC_TitleBarMinButton:
                if(d->flags & Qt::WindowMinimizeButtonHint) {
                    if (d->window && d->window->isMinimized())
                        emit doNormal();
                    else
                        emit doMinimize();
                }
                break;

            case QStyle::SC_TitleBarMaxButton:
                if(d->flags & Qt::WindowMaximizeButtonHint) {
                    if(d->window && d->window->isMaximized())
                        emit doNormal();
                    else
                        emit doMaximize();
                }
                break;

            case QStyle::SC_TitleBarCloseButton:
                if(d->flags & Qt::WindowSystemMenuHint) {
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

void QWorkspaceTitleBar::mouseMoveEvent(QMouseEvent *e)
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
                if(parent && parent->inherits("QWorkspaceChild")) {
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

void QWorkspaceTitleBar::resizeEvent(QResizeEvent *r)
{
    QWidget::resizeEvent(r);
    cutText();
}

bool QWorkspaceTitleBar::isTool() const
{
    return (d->flags & Qt::WindowType_Mask) == Qt::Tool;
}

void QWorkspaceTitleBar::paintEvent(QPaintEvent *)
{
    QStyleOptionTitleBar opt = d->getStyleOption();
    opt.subControls = QStyle::SC_TitleBarLabel;
    opt.activeSubControls = d->buttonDown;
    if (d->flags & Qt::WindowSystemMenuHint) {
        opt.subControls |= QStyle::SC_TitleBarSysMenu | QStyle::SC_TitleBarCloseButton;
        if (d->window && (d->flags & Qt::WindowShadeButtonHint)) {
            if (d->window->isMinimized())
                opt.subControls |= QStyle::SC_TitleBarUnshadeButton;
            else
                opt.subControls |= QStyle::SC_TitleBarShadeButton;
        }
        if (d->window && (d->flags & Qt::WindowMinMaxButtonsHint)) {
            if(d->window && d->window->isMinimized())
                opt.subControls |= QStyle::SC_TitleBarNormalButton;
            else
                opt.subControls |= QStyle::SC_TitleBarMinButton;
        }
        if (d->window && (d->flags & Qt::WindowMaximizeButtonHint) && !d->window->isMaximized())
            opt.subControls |= QStyle::SC_TitleBarMaxButton;
    }

    QStyle::SubControl under_mouse = QStyle::SC_None;
    if(autoRaise() && underMouse()) {
        under_mouse = style()->hitTestComplexControl(QStyle::CC_TitleBar, &opt,
                                                     mapFromGlobal(QCursor::pos()), this);
        opt.activeSubControls |= under_mouse;
        opt.state |= QStyle::State_MouseOver;
    }
    opt.palette.setCurrentColorGroup(usesActiveColor() ? QPalette::Active : QPalette::Inactive);

    QPainter p(this);
    style()->drawComplexControl(QStyle::CC_TitleBar, &opt, &p, this);
}

void QWorkspaceTitleBar::mouseDoubleClickEvent(QMouseEvent *e)
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
        if (d->flags & Qt::WindowSystemMenuHint)
            emit doClose();
        break;

    default:
        break;
    }
}

void QWorkspaceTitleBar::cutText()
{
    QFontMetrics fm(font());
    QStyleOptionTitleBar opt = d->getStyleOption();
    int maxw = style()->subControlRect(QStyle::CC_TitleBar, &opt, QStyle::SC_TitleBarLabel,
                                       this).width();
    if (!d->window)
        return;

    QString txt = d->window->windowTitle();
    if (style()->styleHint(QStyle::SH_TitleBarModifyNotification, 0, this) && d->window
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


void QWorkspaceTitleBar::leaveEvent(QEvent *)
{
    if(autoRaise() && !d->pressed)
        repaint();
}

void QWorkspaceTitleBar::enterEvent(QEvent *)
{
    if(autoRaise() && !d->pressed)
        repaint();
    QEvent e(QEvent::Leave);
    QApplication::sendEvent(parentWidget(), &e);
}

void QWorkspaceTitleBar::setActive(bool active)
{
    if (d->act == active)
        return ;

    d->act = active;
    update();
}

bool QWorkspaceTitleBar::isActive() const
{
    return d->act;
}

bool QWorkspaceTitleBar::usesActiveColor() const
{
    return (isActive() && isActiveWindow()) ||
        (!window() && QWidget::window()->isActiveWindow());
}

QWidget *QWorkspaceTitleBar::window() const
{
    return d->window;
}

bool QWorkspaceTitleBar::event(QEvent *e)
{
    if (d->inevent)
        return QWidget::event(e);
    d->inevent = true;
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
        update();
    } else if (e->type() == QEvent::WindowTitleChange) {
        cutText();
        update();
    }

    d->inevent = false;
    return QWidget::event(e);
}

void QWorkspaceTitleBar::setMovable(bool b)
{
    d->movable = b;
}

bool QWorkspaceTitleBar::isMovable() const
{
    return d->movable;
}

void QWorkspaceTitleBar::setAutoRaise(bool b)
{
    d->autoraise = b;
}

bool QWorkspaceTitleBar::autoRaise() const
{
    return d->autoraise;
}

QSize QWorkspaceTitleBar::sizeHint() const
{
    ensurePolished();
    QStyleOptionTitleBar opt = d->getStyleOption();
    QRect menur = style()->subControlRect(QStyle::CC_TitleBar, &opt,
                                          QStyle::SC_TitleBarSysMenu, this);
    return QSize(menur.width(), style()->pixelMetric(QStyle::PM_TitleBarHeight, &opt, this));
}

/*!
    \class QWorkspace
    \brief The QWorkspace widget provides a workspace window that be
    used in an MDI application.

    \compat

    Multiple Document Interface (MDI) applications are typically
    composed of a main window containing a menu bar, a toolbar, and
    a central QWorkspace widget. The workspace itself is used to display
    a number of child windows, each of which is a widget.

    The workspace itself is an ordinary Qt widget. It has a standard
    constructor that takes a parent widget and an object name.
    Workspaces can be placed in any layout, but are typically given
    as the central widget in a QMainWindow:

    \code
        MainWindow::MainWindow()
        {
            workspace = new QWorkspace(this);
            setCentralWidget(workspace);
            ...
        }
    \endcode

    Child windows (MDI windows) are standard Qt widgets that are
    inserted into the workspace with addWindow(). As with top-level
    widgets, you can call functions such as show(), hide(),
    showMaximized(), and setWindowTitle() on a child window to change
    its appearance within the workspace. You can also provide widget
    flags to determine the layout of the decoration or the behavior of
    the widget itself.

    To change or retrieve the geometry of a child window, you must
    operate on its parentWidget(). The parentWidget() provides
    access to the decorated frame that contains the child window
    widget. When a child window is maximised, its decorated frame
    is hidden. If the top-level widget contains a menu bar, it will display
    the maximised window's operations menu to the left of the menu
    entries, and the window's controls to the right.

    A child window becomes active when it gets the keyboard focus,
    or when setFocus() is called. The user can activate a window by moving
    focus in the usual ways, for example by clicking a window or by pressing
    Tab. The workspace emits a signal windowActivated() when the active
    window changes, and the function activeWindow() returns a pointer to the
    active child window, or 0 if no window is active.

    The convenience function windowList() returns a list of all
    child windows. This information could be used in a
    popup menu containing a list of windows, for example.
    This feature is also available as part of the
    \link http://www.trolltech.com/products/solutions/catalog/Widgets/qtwindowlistmenu/
    Window Menu \endlink Qt Solution.

    QWorkspace provides two built-in layout strategies for child
    windows: cascade() and tile(). Both are slots so you can easily
    connect menu entries to them.

    \img qworkspace-arrange.png

    If you want your users to be able to work with child windows
    larger than the visible workspace area, set the scrollBarsEnabled
    property to true.
*/

static bool inTitleChange = false;

class QWorkspaceChild : public QWidget
{
    Q_OBJECT

    friend class QWorkspacePrivate;
    friend class QWorkspace;
    friend class QWorkspaceTitleBar;

public:
    QWorkspaceChild(QWidget* window, QWorkspace* parent=0, Qt::WFlags flags = 0);
    ~QWorkspaceChild();

    void setActive(bool);
    bool isActive() const;

    void adjustToFullscreen();

    QWidget* windowWidget() const;
    QWidget* iconWidget() const;

    void doResize();
    void doMove();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QSize baseSize() const;

    int frameWidth() const;

    void show();

signals:
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);

public slots:
    void activate();
    void showMinimized();
    void showMaximized();
    void showNormal();
    void showShaded();
    void setWindowTitle(const QString&);
    void internalRaise();
    void titleBarDoubleClicked();

protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void childEvent(QChildEvent*);
    void resizeEvent(QResizeEvent *);
    void moveEvent(QMoveEvent *);
    bool eventFilter(QObject *, QEvent *);

    bool focusNextPrevChild(bool);

    void paintEvent(QPaintEvent *);
    void changeEvent(QEvent *);

private:
    Q_DISABLE_COPY(QWorkspaceChild)

    QWidget *childWidget;
    QWidget *backgroundWidget;
    QWidgetResizeHandler *widgetResizeHandler;
    QWorkspaceTitleBar *titlebar;
    QPointer<QWorkspaceTitleBar> iconw;
    QSize windowSize;
    QSize shadeRestore;
    QSize shadeRestoreMin;
    bool act                  :1;
    bool shademode            :1;
};

int QWorkspaceChild::frameWidth() const
{
    return contentsRect().left();
}



class QWorkspacePrivate : public QWidgetPrivate {
    Q_DECLARE_PUBLIC(QWorkspace)
public:
    QWorkspaceChild* active;
    QList<QWorkspaceChild *> windows;
    QList<QWorkspaceChild *> focus;
    QList<QWidget *> icons;
    QWorkspaceChild* maxWindow;
    QRect maxRestore;
    QPointer<QFrame> maxcontrols;
    QPointer<QMenuBar> maxmenubar;
    QHash<int, const char*> shortcutMap;

    int px;
    int py;
    QWidget *becomeActive;
    QPointer<QLabel> maxtools;
    QString topTitle;

    QMenu *popup, *toolPopup;
    enum WSActs { RestoreAct, MoveAct, ResizeAct, MinimizeAct, MaximizeAct, CloseAct, StaysOnTopAct, ShadeAct, NCountAct };
    QAction *actions[NCountAct];

    QScrollBar *vbar, *hbar;
    QWidget *corner;
    int yoffset, xoffset;
    QBrush background;

    void init();
    void insertIcon(QWidget* w);
    void removeIcon(QWidget* w);
    void place(QWidget*);

    QWorkspaceChild* findChild(QWidget* w);
    void showMaximizeControls();
    void hideMaximizeControls();
    void activateWindow(QWidget* w, bool change_focus = true);
    void hideChild(QWorkspaceChild *c);
    void showWindow(QWidget* w);
    void maximizeWindow(QWidget* w);
    void minimizeWindow(QWidget* w);
    void normalizeWindow(QWidget* w);

    QRect updateWorkspace();

private:
    void normalizeActiveWindow();
    void minimizeActiveWindow();
    void showOperationMenu();
    void popupOperationMenu(const QPoint&);
    void operationMenuActivated(QAction *);
    void scrollBarChanged();
    void updateActions();
};

static bool isChildOf(QWidget * child, QWidget * parent)
{
    if (!parent || !child)
        return false;
    QWidget * w = child;
    while(w && w != parent)
        w = w->parentWidget();
    return w != 0;
}

/*!
    Constructs a workspace with the given \a parent.
*/
QWorkspace::QWorkspace(QWidget *parent)
    : QWidget(*new QWorkspacePrivate, parent, 0)
{
    d->init();
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QWorkspace::QWorkspace(QWidget *parent, const char *name)
    : QWidget(*new QWorkspacePrivate, parent, 0)
{
    setObjectName(name);
    d->init();
}
#endif // QT3_SUPPORT

/*!
    \internal
*/
void
QWorkspacePrivate::init()
{
    d->maxcontrols = 0;
    d->active = 0;
    d->maxWindow = 0;
    d->maxtools = 0;
    d->px = 0;
    d->py = 0;
    d->becomeActive = 0;
    d->popup = new QMenu(q);
    d->toolPopup = new QMenu(q);
    d->popup->setObjectName("qt_internal_mdi_popup");
    d->toolPopup->setObjectName("qt_internal_mdi_tool_popup");

    d->actions[QWorkspacePrivate::RestoreAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarNormalButton)),
                                                            q->tr("&Restore"), q);
    d->actions[QWorkspacePrivate::MoveAct] = new QAction(q->tr("&Move"), q);
    d->actions[QWorkspacePrivate::ResizeAct] = new QAction(q->tr("&Size"), q);
    d->actions[QWorkspacePrivate::MinimizeAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarMinButton)),
                                                             q->tr("Mi&nimize"), q);
    d->actions[QWorkspacePrivate::MaximizeAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarMaxButton)),
                                                             q->tr("Ma&ximize"), q);
    d->actions[QWorkspacePrivate::CloseAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarCloseButton)),
                                                          q->tr("&Close")
                                                          +"\t"+(QString)QKeySequence(Qt::CTRL+Qt::Key_F4)
                                                          ,q);
    QObject::connect(d->actions[QWorkspacePrivate::CloseAct], SIGNAL(triggered()), q, SLOT(closeActiveWindow()));
    d->actions[QWorkspacePrivate::StaysOnTopAct] = new QAction(q->tr("Stay on &Top"), q);
    d->actions[QWorkspacePrivate::StaysOnTopAct]->setChecked(true);
    d->actions[QWorkspacePrivate::ShadeAct] = new QAction(QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarShadeButton)),
                                                          q->tr("Sh&ade"), q);

    QObject::connect(d->popup, SIGNAL(aboutToShow()), q, SLOT(updateActions()));
    QObject::connect(d->popup, SIGNAL(triggered(QAction*)), q, SLOT(operationMenuActivated(QAction*)));
    d->popup->addAction(d->actions[QWorkspacePrivate::RestoreAct]);
    d->popup->addAction(d->actions[QWorkspacePrivate::MoveAct]);
    d->popup->addAction(d->actions[QWorkspacePrivate::ResizeAct]);
    d->popup->addAction(d->actions[QWorkspacePrivate::MinimizeAct]);
    d->popup->addAction(d->actions[QWorkspacePrivate::MaximizeAct]);
    d->popup->addSeparator();
    d->popup->addAction(d->actions[QWorkspacePrivate::CloseAct]);

    QObject::connect(d->toolPopup, SIGNAL(aboutToShow()), q, SLOT(updateActions()));
    QObject::connect(d->toolPopup, SIGNAL(triggered(QAction*)), q, SLOT(operationMenuActivated(QAction*)));
    d->toolPopup->addAction(d->actions[QWorkspacePrivate::MoveAct]);
    d->toolPopup->addAction(d->actions[QWorkspacePrivate::ResizeAct]);
    d->toolPopup->addAction(d->actions[QWorkspacePrivate::StaysOnTopAct]);
    d->toolPopup->addSeparator();
    d->toolPopup->addAction(d->actions[QWorkspacePrivate::ShadeAct]);
    d->toolPopup->addAction(d->actions[QWorkspacePrivate::CloseAct]);

    // Set up shortcut bindings (id -> slot), most used first
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::Key_Tab), "activateNextWindow");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_Tab), "activatePreviousWindow");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::Key_F4), "closeActiveWindow");
    shortcutMap.insert(q->grabShortcut(Qt::ALT + Qt::Key_Minus), "showOperationMenu");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::Key_F6), "activateNextWindow");
    shortcutMap.insert(q->grabShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_F6), "activatePreviousWindow");
    shortcutMap.insert(q->grabShortcut(Qt::Key_Forward), "activateNextWindow");
    shortcutMap.insert(q->grabShortcut(Qt::Key_Back), "activatePreviousWindow");

    q->setAttribute(Qt::WA_NoBackground, true);
    q->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

    d->topTitle = q->window()->windowTitle();
    d->hbar = d->vbar = 0;
    d->corner = 0;
    d->xoffset = d->yoffset = 0;

    updateWorkspace();

    q->window()->installEventFilter(q);
}

/*!
    Destroys the workspace and frees any allocated resources.
*/

QWorkspace::~QWorkspace()
{
}

/*! \reimp */
QSize QWorkspace::sizeHint() const
{
    QSize s(QApplication::desktop()->size());
    return QSize(s.width()*2/3, s.height()*2/3);
}


#ifdef QT3_SUPPORT
/*!
    Sets the background color to \a c.
    Use setBackground() instead.
*/
void QWorkspace::setPaletteBackgroundColor(const QColor & c)
{
    setBackground(c);
}

/*!
    Sets the background pixmap to \a pm.
    Use setBackground() instead.
*/
void QWorkspace::setPaletteBackgroundPixmap(const QPixmap & pm)
{
    setBackground(pm);
}
#endif // QT3_SUPPORT

/*!
    \property QWorkspace::background
    \brief the workspace's background
*/
QBrush QWorkspace::background() const
{
    if (d->background.style() == Qt::NoBrush)
        return palette().dark();
    return d->background;
}

void QWorkspace::setBackground(const QBrush &background)
{
    d->background = background;
    update();
}

/*!
  Adds widget \a w as new sub window to the workspace.  If \a flags
  are non-zero, they will override the flags set on the widget.

  Returns the window frame.

*/
QWidget * QWorkspace::addWindow(QWidget *w, Qt::WFlags flags)
{
    if (!w)
        return 0;

    bool customize =  (flags & (Qt::WindowTitleHint
            | Qt::WindowSystemMenuHint
            | Qt::WindowMinimizeButtonHint
            | Qt::WindowMaximizeButtonHint
            | Qt::WindowContextHelpButtonHint));

    uint type = (flags & Qt::WindowType_Mask);
    if (customize)
        ;
    else if (type == Qt::Dialog || type == Qt::Sheet)
        flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowContextHelpButtonHint;
    else if (type == Qt::Tool)
        flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint;
    else
        flags |= Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint;

    bool wasMaximized = w->isMaximized();
    bool wasMinimized = w->isMinimized();
    bool hasBeenHidden = w->isExplicitlyHidden();
    bool hasSize = w->testAttribute(Qt::WA_Resized);
    int x = w->x();
    int y = w->y();
    bool hasPos = w->testAttribute(Qt::WA_Moved);
    QSize s = w->size().expandedTo(w->minimumSizeHint());
    if (!hasSize && w->sizeHint().isValid())
        w->adjustSize();

    QWorkspaceChild* child = new QWorkspaceChild(w, this, flags);
    child->setObjectName("qt_workspacechild");
    child->installEventFilter(this);

    connect(child, SIGNAL(popupOperationMenu(QPoint)),
            this, SLOT(popupOperationMenu(QPoint)));
    connect(child, SIGNAL(showOperationMenu()),
            this, SLOT(showOperationMenu()));
    d->windows.append(child);
    if (child->isVisibleTo(this))
        d->focus.append(child);
    child->internalRaise();

    if (!hasPos)
        d->place(child);
    if (hasSize)
        child->resize(s + child->baseSize());
    else
        child->adjustSize();
    if (hasPos)
        child->move(x, y);

    w->setHidden(hasBeenHidden);
    return child;

    if (wasMaximized)
        w->showMaximized();
    else if (wasMinimized)
        w->showMinimized();
    else if (!hasBeenHidden)
        d->activateWindow(w);

    d->updateWorkspace();
    return child;
}

/*! \reimp */
void QWorkspace::childEvent(QChildEvent * e)
{
    if (e->removed()) {
        if (d->windows.removeAll(static_cast<QWorkspaceChild*>(e->child()))) {
            d->focus.removeAll(static_cast<QWorkspaceChild*>(e->child()));
            if (d->maxWindow == e->child())
                d->maxWindow = 0;
            d->updateWorkspace();
        }
    }
}

/*! \reimp */
#ifndef QT_NO_WHEELEVENT
void QWorkspace::wheelEvent(QWheelEvent *e)
{
    if (!scrollBarsEnabled())
        return;
    if (d->vbar && d->vbar->isVisible() && !(e->modifiers() & Qt::AltModifier))
        QApplication::sendEvent(d->vbar, e);
    else if (d->hbar && d->hbar->isVisible())
        QApplication::sendEvent(d->hbar, e);
}
#endif

void QWorkspacePrivate::activateWindow(QWidget* w, bool change_focus)
{
    if (!w) {
        d->active = 0;
        emit q->windowActivated(0);
        return;
    }
    if (!q->isVisible()) {
        d->becomeActive = w;
        return;
    }

    if (d->active && d->active->windowWidget() == w) {
        if (!isChildOf(q->focusWidget(), w)) // child window does not have focus
            d->active->setActive(true);
        return;
    }

    d->active = 0;
    // First deactivate all other workspace clients
    QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        QWorkspaceChild* c = *it;
        ++it;
        if (c->windowWidget() == w)
            d->active = c;
        else
            c->setActive(false);
    }

    if (!d->active)
        return;

    // Then activate the new one, so the focus is stored correctly
    d->active->setActive(true);

    if (!d->active)
        return;

    if (d->maxWindow && d->maxWindow != d->active && d->active->windowWidget() &&
        (d->active->windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint)) {
        d->active->showMaximized();
        if (d->maxtools) {
            QIcon icon = w->windowIcon();
            if (!icon.isNull()) {
                int iconSize = d->maxtools->size().height();
                QPixmap pm(icon.pixmap(QSize(iconSize, iconSize)));
                d->maxtools->setPixmap(pm);
            } else
            {
                QPixmap pm(14,14);
                pm.fill(Qt::color1);
                pm.setMask(pm.createHeuristicMask());
                d->maxtools->setPixmap(pm);
            }
        }
    }

    d->active->internalRaise();

    if (change_focus) {
	int from = d->focus.indexOf(d->active);
        if (from >= 0)
            d->focus.move(from, d->focus.size() - 1);
    }

    updateWorkspace();
    emit q->windowActivated(w);
}


/*!
    Returns a pointer to the widget corresponding to the active child
    window, or 0 if no window is active.

    \sa setActiveWindow()
*/
QWidget* QWorkspace::activeWindow() const
{
    return d->active?d->active->windowWidget():0;
}

/*!
    Makes the child window that contains \a w the active child window.

    \sa activeWindow()
*/
void QWorkspace::setActiveWindow(QWidget *w)
{
    d->activateWindow(w, true);
}

void QWorkspacePrivate::place(QWidget *w)
{
    QList<QWidget *> widgets;
    for (QList<QWorkspaceChild *>::Iterator it(d->windows.begin()); it != d->windows.end(); ++it)
        if (*it != w)
            widgets.append(*it);

    int overlap, minOverlap = 0;
    int possible;

    QRect r1(0, 0, 0, 0);
    QRect r2(0, 0, 0, 0);
    QRect maxRect = q->rect();
    int x = maxRect.left(), y = maxRect.top();
    QPoint wpos(maxRect.left(), maxRect.top());

    bool firstPass = true;

    do {
        if (y + w->height() > maxRect.bottom()) {
            overlap = -1;
        } else if(x + w->width() > maxRect.right()) {
            overlap = -2;
        } else {
            overlap = 0;

            r1.setRect(x, y, w->width(), w->height());

            QWidget *l;
            QList<QWidget *>::Iterator it(widgets.begin());
            while (it != widgets.end()) {
                l = *it;
                ++it;

                if (d->maxWindow == l)
                    r2 = d->maxRestore;
                else
                    r2.setRect(l->x(), l->y(), l->width(), l->height());

                if (r2.intersects(r1)) {
                    r2.setCoords(qMax(r1.left(), r2.left()),
                                 qMax(r1.top(), r2.top()),
                                 qMin(r1.right(), r2.right()),
                                 qMin(r1.bottom(), r2.bottom())
                                );

                    overlap += (r2.right() - r2.left()) *
                               (r2.bottom() - r2.top());
                }
            }
        }

        if (overlap == 0) {
            wpos = QPoint(x, y);
            break;
        }

        if (firstPass) {
            firstPass = false;
            minOverlap = overlap;
        } else if (overlap >= 0 && overlap < minOverlap) {
            minOverlap = overlap;
            wpos = QPoint(x, y);
        }

        if (overlap > 0) {
            possible = maxRect.right();
            if (possible - w->width() > x) possible -= w->width();

            QWidget *l;
            QList<QWidget *>::Iterator it(widgets.begin());
            while (it != widgets.end()) {
                l = *it;
                ++it;
                if (d->maxWindow == l)
                    r2 = d->maxRestore;
                else
                    r2.setRect(l->x(), l->y(), l->width(), l->height());

                if((y < r2.bottom()) && (r2.top() < w->height() + y)) {
                    if(r2.right() > x)
                        possible = possible < r2.right() ?
                                   possible : r2.right();

                    if(r2.left() - w->width() > x)
                        possible = possible < r2.left() - w->width() ?
                                   possible : r2.left() - w->width();
                }
            }

            x = possible;
        } else if (overlap == -2) {
            x = maxRect.left();
            possible = maxRect.bottom();

            if (possible - w->height() > y) possible -= w->height();

            QWidget *l;
            QList<QWidget *>::Iterator it(widgets.begin());
            while (it != widgets.end()) {
                l = *it;
                ++it;
                if (d->maxWindow == l)
                    r2 = d->maxRestore;
                else
                    r2.setRect(l->x(), l->y(), l->width(), l->height());

                if(r2.bottom() > y)
                    possible = possible < r2.bottom() ?
                               possible : r2.bottom();

                if(r2.top() - w->height() > y)
                    possible = possible < r2.top() - w->height() ?
                               possible : r2.top() - w->height();
            }

            y = possible;
        }
    }
    while(overlap != 0 && overlap != -1);

    w->move(wpos);
    updateWorkspace();
}


void QWorkspacePrivate::insertIcon(QWidget* w)
{
    if (!w || d->icons.contains(w))
        return;
    d->icons.append(w);
    if (w->parentWidget() != q) {
        w->setParent(q, 0);
        w->move(0,0);
    }
    QRect cr = updateWorkspace();
    int x = 0;
    int y = cr.height() - w->height();

    QList<QWidget *>::Iterator it(d->icons.begin());
    while (it != d->icons.end()) {
        QWidget* i = *it;
        ++it;
        if (x > 0 && x + i->width() > cr.width()){
            x = 0;
            y -= i->height();
        }

        if (i != w &&
            i->geometry().intersects(QRect(x, y, w->width(), w->height())))
            x += i->width();
    }
    w->move(x, y);

    if (q->isVisibleTo(q->parentWidget())) {
        w->show();
        w->lower();
    }
    updateWorkspace();
}


void QWorkspacePrivate::removeIcon(QWidget* w)
{
    if (d->icons.removeAll(w))
        w->hide();
}


/*! \reimp  */
void QWorkspace::resizeEvent(QResizeEvent *)
{
    if (d->maxWindow) {
        d->maxWindow->adjustToFullscreen();
        if (d->maxWindow->windowWidget())
            d->maxWindow->windowWidget()->overrideWindowState(Qt::WindowMaximized);
    }
    d->updateWorkspace();
}

/*! \reimp */
void QWorkspace::showEvent(QShowEvent *e)
{
    if (d->maxWindow && !style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this))
        d->showMaximizeControls();
    QWidget::showEvent(e);
    if (d->becomeActive) {
        d->activateWindow(d->becomeActive);
        d->becomeActive = 0;
    } else if (d->windows.count() > 0 && !d->active) {
        d->activateWindow(d->windows.first()->windowWidget());
    }

    // force a frame repaint - this is a workaround for what seems to be a bug
    // introduced when changing the QWidget::show() implementation. Might be
    // a windows bug as well though.
    for (int i = 0; i < d->windows.count(); ++i) {
	QWorkspaceChild* c = d->windows.at(i);
        c->update(c->rect());
    }

    d->updateWorkspace();
}

/*! \reimp */
void QWorkspace::hideEvent(QHideEvent *)
{
    if (!isVisible() && !style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this))
        d->hideMaximizeControls();
}

/*! \reimp */
void QWorkspace::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QBrush bg = d->background;
    if (bg.style() == Qt::NoBrush)
        bg = q->palette().dark();

    p.fillRect(0, 0, width(), height(), bg);
}

void QWorkspacePrivate::minimizeWindow(QWidget* w)
{
    QWorkspaceChild* c = findChild(w);

    if (!w || !(w->windowFlags() & Qt::WindowMinimizeButtonHint))
        return;

    if (c) {
        bool wasMax = false;
        if (c == d->maxWindow) {
            wasMax = true;
            d->maxWindow = 0;
            inTitleChange = true;
            if (d->topTitle.size())
                q->window()->setWindowTitle(d->topTitle);
            inTitleChange = false;
            if (!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q))
                hideMaximizeControls();
            for (QList<QWorkspaceChild *>::Iterator it(d->windows.begin()); it != d->windows.end(); ++it) {
                QWorkspaceChild* c = *it;
                if (c->titlebar)
                    c->titlebar->setMovable(true);
                c->widgetResizeHandler->setActive(true);
            }
        }
        c->hide();
        if (wasMax)
            c->setGeometry(d->maxRestore);
        if (!d->focus.contains(c))
            d->focus.append(c);
        insertIcon(c->iconWidget());

        if (!d->maxWindow)
            activateWindow(w);

        updateWorkspace();

        w->overrideWindowState(Qt::WindowMinimized);
        c->overrideWindowState(Qt::WindowMinimized);
    }
}

void QWorkspacePrivate::normalizeWindow(QWidget* w)
{
    QWorkspaceChild* c = findChild(w);
    if (!w)
        return;
    if (c) {
        w->overrideWindowState(Qt::WindowNoState);
        if (!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q)
            && d->maxWindow) {
            hideMaximizeControls();
        } else {
            if (w->minimumSize() != w->maximumSize())
                c->widgetResizeHandler->setActive(true);
            if (c->titlebar)
                c->titlebar->setMovable(true);
        }
        w->overrideWindowState(Qt::WindowNoState);
        c->overrideWindowState(Qt::WindowNoState);

        if (c == d->maxWindow) {
            c->setGeometry(d->maxRestore);
            d->maxWindow = 0;
            inTitleChange = true;
            if (d->topTitle.size())
                q->window()->setWindowTitle(d->topTitle);
            inTitleChange = false;
        } else {
            if (c->iconw)
                removeIcon(c->iconw->parentWidget());
            c->show();
        }

        if (!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q))
            hideMaximizeControls();
        for (QList<QWorkspaceChild *>::Iterator it(d->windows.begin()); it != d->windows.end(); ++it) {
            QWorkspaceChild* c = *it;
            if (c->titlebar)
                c->titlebar->setMovable(true);
            if (c->childWidget && c->childWidget->minimumSize() != c->childWidget->maximumSize())
                c->widgetResizeHandler->setActive(true);
        }
        activateWindow(w, true);
        updateWorkspace();
    }
}

void QWorkspacePrivate::maximizeWindow(QWidget* w)
{
    QWorkspaceChild* c = findChild(w);

    if (!w || !(w->windowFlags() & Qt::WindowMaximizeButtonHint))
        return;

    if (!c || c == d->maxWindow)
        return;

    bool updatesEnabled = q->updatesEnabled();
    q->setUpdatesEnabled(false);

    if (c->iconw && d->icons.contains(c->iconw->parentWidget()))
        normalizeWindow(w);
    QRect r(c->geometry());
    QWorkspaceChild *oldMaxWindow = d->maxWindow;
    d->maxWindow = c;
    c->adjustToFullscreen();
    c->show();
    c->internalRaise();
    if (oldMaxWindow != c) {
        if (oldMaxWindow)
            oldMaxWindow->setGeometry(d->maxRestore);
        d->maxRestore = r;
    }

    activateWindow(w);
    if(!q->style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, q)) {
        showMaximizeControls();
    } else {
        if (!d->active && d->becomeActive) {
            d->active = (QWorkspaceChild*)d->becomeActive->parentWidget();
            d->active->setActive(true);
            d->becomeActive = 0;
            emit q->windowActivated(d->active->windowWidget());
        }
        c->widgetResizeHandler->setActive(false);
        if (c->titlebar)
            c->titlebar->setMovable(false);
    }
    inTitleChange = true;
    if (d->topTitle.size())
        q->window()->setWindowTitle(q->tr("%1 - [%2]")
                                            .arg(d->topTitle).arg(c->windowTitle()));
    inTitleChange = false;

    updateWorkspace();

    w->overrideWindowState(Qt::WindowMaximized);
    c->overrideWindowState(Qt::WindowMaximized);
    q->setUpdatesEnabled(updatesEnabled);
}

void QWorkspacePrivate::showWindow(QWidget* w)
{
    if (w->isMinimized() && (w->windowFlags() & Qt::WindowMinimizeButtonHint))
        minimizeWindow(w);
    else if ((d->maxWindow || w->isMaximized()) && w->windowFlags() & Qt::WindowMaximizeButtonHint)
        maximizeWindow(w);
    else if (w->windowFlags() & Qt::WindowMaximizeButtonHint)
        normalizeWindow(w);
    else
        w->parentWidget()->show();
    if (d->maxWindow)
        d->maxWindow->internalRaise();
    updateWorkspace();
}


QWorkspaceChild* QWorkspacePrivate::findChild(QWidget* w)
{
    QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        QWorkspaceChild* c = *it;
        ++it;
        if (c->windowWidget() == w)
            return c;
    }
    return 0;
}

/*!
    Returns a list of all child windows. If \a order is CreationOrder
    (the default), the windows are listed in the order in which they
    were inserted into the workspace. If \a order is StackingOrder,
    the windows are listed in their stacking order, with the topmost
    window as the last item in the list.
*/
QWidgetList QWorkspace::windowList(WindowOrder order) const
{
    QWidgetList windows;
    if (order == StackingOrder) {
        QObjectList cl = children();
        for (int i = 0; i < cl.size(); ++i) {
            QWorkspaceChild *c = qobject_cast<QWorkspaceChild*>(cl.at(i));
            if (c && c->windowWidget())
                windows.append(c->windowWidget());
        }
    } else {
        QList<QWorkspaceChild *>::ConstIterator it(d->windows.begin());
        while (it != d->windows.end()) {
            QWorkspaceChild* c = *it;
            ++it;
            if (c->windowWidget())
                windows.append(c->windowWidget());
        }
    }
    return windows;
}

/*! \reimp */
bool QWorkspace::eventFilter(QObject *o, QEvent * e)
{
    static QTime* t = 0;
    static QWorkspace* tc = 0;
    if (o == d->maxtools) {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            {
                QMenuBar* b = (QMenuBar*)o->parent();
                if (!t)
                    t = new QTime;
                if (tc != this || t->elapsed() > QApplication::doubleClickInterval()) {
                    if (isRightToLeft()) {
                        QPoint p = b->mapToGlobal(QPoint(b->x() + b->width(), b->y() + b->height()));
                        p.rx() -= d->popup->sizeHint().width();
                        d->popupOperationMenu(p);
                    } else {
                        d->popupOperationMenu(b->mapToGlobal(QPoint(b->x(), b->y() + b->height())));
                    }
                    t->start();
                    tc = this;
                } else {
                    tc = 0;
                    closeActiveWindow();
                }
                return true;
            }
        default:
            break;
        }
        return QWidget::eventFilter(o, e);
    }
    switch (e->type()) {
    case QEvent::HideToParent:
        break;
    case QEvent::ShowToParent:
        if (QWorkspaceChild *c = qobject_cast<QWorkspaceChild*>(o))
            if (!d->focus.contains(c))
                d->focus.append(c);
        d->updateWorkspace();
        break;
    case QEvent::WindowTitleChange:
        if (inTitleChange)
            break;

        inTitleChange = true;
        if (o == window()) {
            QWidget *tlw = (QWidget*)o;
            if (!d->maxWindow
                || tlw->windowTitle() != tr("%1 - [%2]").arg(d->topTitle).arg(d->maxWindow->windowTitle()))
                d->topTitle = tlw->windowTitle();
        }

        if (d->maxWindow && d->topTitle.size())
            window()->setWindowTitle(tr("%1 - [%2]")
                .arg(d->topTitle).arg(d->maxWindow->windowTitle()));
        inTitleChange = false;

        break;
    case QEvent::Close:
        if (o == window())
        {
            QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
            while (it != d->windows.end()) {
                QWorkspaceChild* c = *it;
                ++it;
                if (c->shademode)
                    c->showShaded();
            }
        } else if (qobject_cast<QWorkspaceChild*>(o)) {
            d->popup->hide();
        }
        d->updateWorkspace();
        break;
    case QEvent::Shortcut:
        {
            QShortcutEvent *se = static_cast<QShortcutEvent *>(e);
            const char *theSlot = d->shortcutMap.value(se->shortcutId(), 0);
            if (theSlot)
                QMetaObject::invokeMember(this, theSlot);
        }
        break;
    default:
        break;
    }
    return QWidget::eventFilter(o, e);
}

void QWorkspacePrivate::showMaximizeControls()
{
    Q_ASSERT(d->maxWindow);
    QMenuBar* b = 0;

    // Do a breadth-first search first on every parent,
    QWidget* w = q->parentWidget();
    QList<QMenuBar*> l;
    while (l.isEmpty() && w) {
        l = qFindChildren<QMenuBar*>(w);
        w = w->parentWidget();
    }

    // and query recursively if nothing is found.
    if (!l.size())
        l = qFindChildren<QMenuBar*>(q->window());
    if (l.size())
        b = l.at(0);

    if (!b)
        return;

    if (!d->maxcontrols) {
        d->maxmenubar = b;
        d->maxcontrols = new QFrame(q->window());
        d->maxcontrols->setObjectName("qt_maxcontrols");
        QHBoxLayout* l = new QHBoxLayout(d->maxcontrols);
        l->setMargin(d->maxcontrols->frameWidth());
        l->setSpacing(0);
        if (d->maxWindow->windowWidget() &&
            (d->maxWindow->windowWidget()->windowFlags() & Qt::WindowMinimizeButtonHint)) {
            QToolButton* iconB = new QToolButton(d->maxcontrols);
            iconB->setObjectName("iconify");
            iconB->setToolTip(q->tr("Minimize"));
            l->addWidget(iconB);
            iconB->setFocusPolicy(Qt::NoFocus);
            QPixmap pm = q->style()->standardPixmap(QStyle::SP_TitleBarMinButton);
            iconB->setIcon(pm);
            iconB->setIconSize(pm.size());
            QObject::connect(iconB, SIGNAL(clicked()),
                             q, SLOT(minimizeActiveWindow()));
        }

        QToolButton* restoreB = new QToolButton(d->maxcontrols);
        restoreB->setObjectName("restore");
        restoreB->setToolTip(q->tr("Restore Down"));
        l->addWidget(restoreB);
        restoreB->setFocusPolicy(Qt::NoFocus);
        QPixmap pm = q->style()->standardPixmap(QStyle::SP_TitleBarNormalButton);
        restoreB->setIcon(pm);
        restoreB->setIconSize(pm.size());
        QObject::connect(restoreB, SIGNAL(clicked()),
                         q, SLOT(normalizeActiveWindow()));

        l->addSpacing(2);
        QToolButton* closeB = new QToolButton(d->maxcontrols);
        closeB->setObjectName("close");
        closeB->setToolTip(q->tr("Close"));
        l->addWidget(closeB);
        closeB->setFocusPolicy(Qt::NoFocus);
        pm = q->style()->standardPixmap(QStyle::SP_TitleBarCloseButton);
        closeB->setIcon(pm);
        closeB->setIconSize(pm.size());
        QObject::connect(closeB, SIGNAL(clicked()),
                         q, SLOT(closeActiveWindow()));

        d->maxcontrols->setFixedSize(d->maxcontrols->minimumSizeHint());
    }

    b->setCornerWidget(d->maxcontrols);
    d->maxcontrols->show();
    if (!d->active && d->becomeActive) {
        d->active = (QWorkspaceChild*)d->becomeActive->parentWidget();
        d->active->setActive(true);
        d->becomeActive = 0;
        emit q->windowActivated(d->active->windowWidget());
    }
    if (d->active) {
        if (!d->maxtools) {
            d->maxtools = new QLabel(q->window());
            d->maxtools->setObjectName("qt_maxtools");
            d->maxtools->installEventFilter(q);
        }
        if (d->active->windowWidget() && !d->active->windowWidget()->windowIcon().isNull()) {
            QIcon icon = d->active->windowWidget()->windowIcon();
            int iconSize = d->maxcontrols->size().height();
            d->maxtools->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));
        } else {
            QPixmap pm = q->style()->standardPixmap(QStyle::SP_TitleBarMenuButton);
            if (pm.isNull()) {
                pm = QPixmap(14,14);
                pm.fill(Qt::black);
            }
            d->maxtools->setPixmap(pm);
        }
        b->setCornerWidget(d->maxtools, Qt::TopLeftCorner);
        d->maxtools->show();
    }
}


void QWorkspacePrivate::hideMaximizeControls()
{
    if (d->maxmenubar) {
        d->maxmenubar->setCornerWidget(0, Qt::TopLeftCorner);
        d->maxmenubar->setCornerWidget(0, Qt::TopRightCorner);
    }
    delete d->maxcontrols;
    d->maxcontrols = 0;
    delete d->maxtools;
    d->maxtools = 0;
}

/*!
    Closes the child window that is currently active.

    \sa closeAllWindows()
*/
void QWorkspace::closeActiveWindow()
{
    if (d->maxWindow && d->maxWindow->windowWidget())
        d->maxWindow->windowWidget()->close();
    else if (d->active && d->active->windowWidget())
        d->active->windowWidget()->close();
    d->updateWorkspace();
}

/*!
    Closes all child windows.

    If any child window fails to accept the close event, the remaining windows
    will remain open.

    \sa closeActiveWindow()
*/
void QWorkspace::closeAllWindows()
{
    bool did_close = true;
    QList<QWorkspaceChild *>::const_iterator it = d->windows.constBegin();
    while (it != d->windows.constEnd() && did_close) {
        QWorkspaceChild *c = *it;
        ++it;
        if (c->windowWidget() && !c->windowWidget()->isExplicitlyHidden())
            did_close = c->windowWidget()->close();
    }
}

void QWorkspacePrivate::normalizeActiveWindow()
{
    if (d->maxWindow)
        d->maxWindow->showNormal();
    else if (d->active)
        d->active->showNormal();
}

void QWorkspacePrivate::minimizeActiveWindow()
{
    if (d->maxWindow)
        d->maxWindow->showMinimized();
    else if (d->active)
        d->active->showMinimized();
}

void QWorkspacePrivate::showOperationMenu()
{
    if  (!d->active || !d->active->windowWidget())
        return;
    Q_ASSERT((d->active->windowWidget()->windowFlags() & Qt::WindowSystemMenuHint));
    QPoint p;
    QMenu *popup = d->active->titlebar->isTool() ? d->toolPopup : d->popup;
    if (q->isRightToLeft()) {
        p = QPoint(d->active->windowWidget()->mapToGlobal(QPoint(d->active->windowWidget()->width(),0)));
        p.rx() -= popup->sizeHint().width();
    } else {
        p = QPoint(d->active->windowWidget()->mapToGlobal(QPoint(0,0)));
    }
    if (!d->active->isVisible()) {
        p = d->active->iconWidget()->mapToGlobal(QPoint(0,0));
        p.ry() -= popup->sizeHint().height();
    }
    popupOperationMenu(p);
}

void QWorkspacePrivate::popupOperationMenu(const QPoint&  p)
{
    if (!d->active || !d->active->windowWidget() || !(d->active->windowWidget()->windowFlags() & Qt::WindowSystemMenuHint))
        return;
    if ((d->active->titlebar->isTool()))
        d->toolPopup->popup(p);
    else
        d->popup->popup(p);
}

void QWorkspacePrivate::updateActions()
{
    for (int i = 1; i < NCountAct-1; i++) {
        bool enable = d->active != 0;
        d->actions[i]->setEnabled(enable);
    }

    if (!d->active || !d->active->windowWidget())
        return;

    QWidget *windowWidget = d->active->windowWidget();
    bool canResize = windowWidget->maximumSize() != windowWidget->minimumSize();
    d->actions[QWorkspacePrivate::ResizeAct]->setEnabled(canResize);
    d->actions[QWorkspacePrivate::MinimizeAct]->setEnabled((windowWidget->windowFlags() & Qt::WindowMinimizeButtonHint));
    d->actions[QWorkspacePrivate::MaximizeAct]->setEnabled((windowWidget->windowFlags() & Qt::WindowMaximizeButtonHint) && canResize);

    if (d->active == d->maxWindow) {
        d->actions[QWorkspacePrivate::MoveAct]->setEnabled(false);
        d->actions[QWorkspacePrivate::ResizeAct]->setEnabled(false);
        d->actions[QWorkspacePrivate::MaximizeAct]->setEnabled(false);
    } else if (d->active->isVisible()){
        d->actions[QWorkspacePrivate::RestoreAct]->setEnabled(false);
    } else {
        d->actions[QWorkspacePrivate::MoveAct]->setEnabled(false);
        d->actions[QWorkspacePrivate::ResizeAct]->setEnabled(false);
        d->actions[QWorkspacePrivate::MinimizeAct]->setEnabled(false);
    }
    if (d->active->shademode) {
        d->actions[QWorkspacePrivate::ShadeAct]->setIcon(
            QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarUnshadeButton)));
        d->actions[QWorkspacePrivate::ShadeAct]->setText(q->tr("&Unshade"));
    } else {
        d->actions[QWorkspacePrivate::ShadeAct]->setIcon(
            QIcon(q->style()->standardPixmap(QStyle::SP_TitleBarShadeButton)));
        d->actions[QWorkspacePrivate::ShadeAct]->setText(q->tr("Sh&ade"));
    }
    d->actions[QWorkspacePrivate::StaysOnTopAct]->setEnabled(!d->active->shademode && canResize);
    d->actions[QWorkspacePrivate::StaysOnTopAct]->setChecked(
        (d->active->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint));
}

void QWorkspacePrivate::operationMenuActivated(QAction *action)
{
    if (!d->active)
        return;
    if(action == d->actions[QWorkspacePrivate::RestoreAct]) {
        d->active->showNormal();
    } else if(action == d->actions[QWorkspacePrivate::MoveAct]) {
        d->active->doMove();
    } else if(action == d->actions[QWorkspacePrivate::ResizeAct]) {
        if (d->active->shademode)
            d->active->showShaded();
        d->active->doResize();
    } else if(action == d->actions[QWorkspacePrivate::MinimizeAct]) {
        d->active->showMinimized();
    } else if(action == d->actions[QWorkspacePrivate::MaximizeAct]) {
        d->active->showMaximized();
    } else if(action == d->actions[QWorkspacePrivate::ShadeAct]) {
        d->active->showShaded();
    } else if(action == d->actions[QWorkspacePrivate::StaysOnTopAct]) {
        if(QWidget* w = d->active->windowWidget()) {
            if ((w->windowFlags() & Qt::WindowStaysOnTopHint)) {
                w->overrideWindowFlags(w->windowFlags() & ~Qt::WindowStaysOnTopHint);
            } else {
                w->overrideWindowFlags(w->windowFlags() | Qt::WindowStaysOnTopHint);
                w->parentWidget()->raise();
            }
        }
    }
}


void QWorkspacePrivate::hideChild(QWorkspaceChild *c)
{
    bool updatesEnabled = q->updatesEnabled();
    q->setUpdatesEnabled(false);
    focus.removeAll(c);
    QRect restore;
    if (d->maxWindow == c)
        restore = maxRestore;
    if (active == c) {
        q->setFocus();
        q->activatePreviousWindow();
    }
    if (active == c)
        activateWindow(0);
    if (maxWindow == c) {
        hideMaximizeControls();
        maxWindow = 0;
    }
    c->hide();
    if (!restore.isEmpty())
        c->setGeometry(restore);
    q->setUpdatesEnabled(updatesEnabled);
}

/*!
    Gives the input focus to the next window in the list of child
    windows.

    \sa activatePreviousWindow()
*/
void QWorkspace::activateNextWindow()
{
    if (d->focus.isEmpty())
        return;
    if (!d->active) {
        if (d->focus.first())
            d->activateWindow(d->focus.first()->windowWidget(), false);
        return;
    }

    int a = d->focus.indexOf(d->active) + 1;

    a = a % d->focus.count();

    if (d->focus.at(a))
        d->activateWindow(d->focus.at(a)->windowWidget(), false);
    else
        d->activateWindow(0);
}

/*!
    Gives the input focus to the previous window in the list of child
    windows.

    \sa activateNextWindow()
*/
void QWorkspace::activatePreviousWindow()
{
    if (d->focus.isEmpty())
        return;
    if (!d->active) {
        if (d->focus.last())
            d->activateWindow(d->focus.first()->windowWidget(), false);
        else
            d->activateWindow(0);
        return;
    }

    int a = d->focus.indexOf(d->active) - 1;
    if (a < 0)
        a = d->focus.count()-1;

    if (d->focus.at(a))
        d->activateWindow(d->focus.at(a)->windowWidget(), false);
    else
        d->activateWindow(0);
}


/*!
    \fn void QWorkspace::windowActivated(QWidget* w)

    This signal is emitted when the child window \a w becomes active.
    Note that \a w can be 0, and that more than one signal may be
    emitted for a single activation event.

    \sa activeWindow(), windowList()
*/

/*!
    Arranges all the child windows in a cascade pattern.

    \sa tile()
*/
void QWorkspace::cascade()
{
    blockSignals(true);
    if  (d->maxWindow)
        d->maxWindow->showNormal();

    if (d->vbar) {
        d->vbar->blockSignals(true);
        d->vbar->setValue(0);
        d->vbar->blockSignals(false);
        d->hbar->blockSignals(true);
        d->hbar->setValue(0);
        d->hbar->blockSignals(false);
        d->scrollBarChanged();
    }

    const int xoffset = 13;
    const int yoffset = 20;

    // make a list of all relevant mdi clients
    QList<QWorkspaceChild *> widgets;
    QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
    QWorkspaceChild* wc = 0;

    for (it = d->focus.begin(); it != d->focus.end(); ++it) {
        wc = *it;
        if (wc->windowWidget()->isVisibleTo(this) && !(wc->titlebar->isTool()))
            widgets.append(wc);
    }

    int x = 0;
    int y = 0;

    it = widgets.begin();
    while (it != widgets.end()) {
        QWorkspaceChild *child = *it;
        ++it;

        QSize prefSize = child->windowWidget()->sizeHint().expandedTo(child->windowWidget()->minimumSizeHint());
        if (!prefSize.isValid())
            prefSize = child->windowWidget()->size();
        prefSize = prefSize.expandedTo(child->windowWidget()->minimumSize()).boundedTo(child->windowWidget()->maximumSize());
        if (prefSize.isValid())
            prefSize += QSize(child->baseSize().width(), child->baseSize().height());

        int w = prefSize.width();
        int h = prefSize.height();

        child->showNormal();
        if (y + h > height())
            y = 0;
        if (x + w > width())
            x = 0;
        child->setGeometry(x, y, w, h);
        x += xoffset;
        y += yoffset;
        child->internalRaise();
    }
    d->updateWorkspace();
    blockSignals(false);
}

/*!
    Arranges all child windows in a tile pattern.

    \sa cascade()
*/
void QWorkspace::tile()
{
    blockSignals(true);
    QWidget *oldActive = d->active ? d->active->windowWidget() : 0;
    if  (d->maxWindow)
        d->maxWindow->showNormal();

    if (d->vbar) {
        d->vbar->blockSignals(true);
        d->vbar->setValue(0);
        d->vbar->blockSignals(false);
        d->hbar->blockSignals(true);
        d->hbar->setValue(0);
        d->hbar->blockSignals(false);
        d->scrollBarChanged();
    }

    int rows = 1;
    int cols = 1;
    int n = 0;
    QWorkspaceChild* c;

    QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        c = *it;
        ++it;
        if (!c->windowWidget()->isExplicitlyHidden()
            && !(c->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)
            && !c->iconw)
            n++;
    }

    while (rows * cols < n) {
        if (cols <= rows)
            cols++;
        else
            rows++;
    }
    int add = cols * rows - n;
    bool* used = new bool[cols*rows];
    for (int i = 0; i < rows*cols; i++)
        used[i] = false;

    int row = 0;
    int col = 0;
    int w = width() / cols;
    int h = height() / rows;

    it = d->windows.begin();
    while (it != d->windows.end()) {
        c = *it;
        ++it;
        if (c->iconw || c->windowWidget()->isExplicitlyHidden() || (c->titlebar->isTool()))
            continue;
        if (!row && !col) {
            w -= c->baseSize().width();
            h -= c->baseSize().height();
        }
        if ((c->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)) {
            QPoint p = c->pos();
            if (p.x()+c->width() < 0)
                p.setX(0);
            if (p.x() > width())
                p.setX(width() - c->width());
            if (p.y() + 10 < 0)
                p.setY(0);
            if (p.y() > height())
                p.setY(height() - c->height());

            if (p != c->pos())
                c->QWidget::move(p);
        } else {
            c->showNormal();
            used[row*cols+col] = true;
            QSize sz(w, h);
            QSize bsize(c->baseSize());
            sz = sz.expandedTo(c->windowWidget()->minimumSize()).boundedTo(c->windowWidget()->maximumSize());
            sz += bsize;

	    if ( add ) {
                if (sz.height() == h + bsize.height()) // no relevant constrains
                    sz.rheight() *= 2;
		used[(row+1)*cols+col] = true;
		add--;
	    }

            c->setGeometry(col*w + col*bsize.width(), row*h + row*bsize.height(), sz.width(), sz.height());

            while(row < rows && col < cols && used[row*cols+col]) {
                col++;
                if (col == cols) {
                    col = 0;
                    row++;
                }
            }
        }
    }
    delete [] used;

    d->activateWindow(oldActive);
    d->updateWorkspace();
    blockSignals(false);
}

QWorkspaceChild::QWorkspaceChild(QWidget* window, QWorkspace *parent, Qt::WFlags flags)
    : QWidget(parent,
             Qt::FramelessWindowHint | Qt::SubWindow)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setAttribute(Qt::WA_NoMousePropagation);
    setMouseTracking(true);
    act = false;
    iconw = 0;
    shademode = false;
    titlebar = 0;

    backgroundWidget = new QWidget(this);
    backgroundWidget->setAttribute(Qt::WA_NoSystemBackground);
    if (window) {
        if (flags)
            window->setParent(this, flags & ~Qt::WindowType_Mask);
        else
            window->setParent(this);
        switch (window->focusPolicy()) {
        case Qt::NoFocus:
            window->setFocusPolicy(Qt::ClickFocus);
            break;
        case Qt::TabFocus:
            window->setFocusPolicy(Qt::StrongFocus);
            break;
        default:
            break;
        }
    }

    if (window && (flags & Qt::WindowTitleHint)) {
        titlebar = new QWorkspaceTitleBar(window, this, flags);
        connect(titlebar, SIGNAL(doActivate()),
                 this, SLOT(activate()));
        connect(titlebar, SIGNAL(doClose()),
                 window, SLOT(close()));
        connect(titlebar, SIGNAL(doMinimize()),
                 this, SLOT(showMinimized()));
        connect(titlebar, SIGNAL(doNormal()),
                 this, SLOT(showNormal()));
        connect(titlebar, SIGNAL(doMaximize()),
                 this, SLOT(showMaximized()));
        connect(titlebar, SIGNAL(popupOperationMenu(QPoint)),
                 this, SIGNAL(popupOperationMenu(QPoint)));
        connect(titlebar, SIGNAL(showOperationMenu()),
                 this, SIGNAL(showOperationMenu()));
        connect(titlebar, SIGNAL(doShade()),
                 this, SLOT(showShaded()));
        connect(titlebar, SIGNAL(doubleClicked()),
                 this, SLOT(titleBarDoubleClicked()));
    }

    setMinimumSize(128, 0);
    int fw =  style()->pixelMetric(QStyle::PM_MDIFrameWidth, 0, this);
    setContentsMargins(fw, fw, fw, fw);

    childWidget = window;
    if (!childWidget)
        return;

    setWindowTitle(childWidget->windowTitle());

    QPoint p;
    QSize s;
    QSize cs;

    bool hasBeenResized = childWidget->testAttribute(Qt::WA_Resized);

    if (!hasBeenResized)
        cs = childWidget->sizeHint().expandedTo(childWidget->minimumSizeHint());
    else
        cs = childWidget->size();

    int th = titlebar ? titlebar->sizeHint().height() : 0;
    if (titlebar) {
        if (!childWidget->windowIcon().isNull())
            titlebar->setWindowIcon(childWidget->windowIcon());
        if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            th += frameWidth();
        else
            th -= contentsRect().y();

        p = QPoint(contentsRect().x(),
                    th + contentsRect().y());
        s = QSize(cs.width() + 2*frameWidth(),
                   cs.height() + 2*frameWidth() + th);
    } else {
        p = QPoint(contentsRect().x(), contentsRect().y());
        s = QSize(cs.width() + 2*frameWidth(),
                   cs.height() + 2*frameWidth());
    }

    childWidget->move(p);
    resize(s);

    childWidget->installEventFilter(this);

    widgetResizeHandler = new QWidgetResizeHandler(this, window);
    widgetResizeHandler->setSizeProtection(!parent->scrollBarsEnabled());
    widgetResizeHandler->setFrameWidth(frameWidth());
    connect(widgetResizeHandler, SIGNAL(activate()),
             this, SLOT(activate()));
    if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
        widgetResizeHandler->setExtraHeight(th + contentsRect().y() - 2*frameWidth());
    else
        widgetResizeHandler->setExtraHeight(th + contentsRect().y() - frameWidth());
    if (childWidget->minimumSize() == childWidget->maximumSize())
        widgetResizeHandler->setActive(QWidgetResizeHandler::Resize, false);
    setBaseSize(baseSize());
}

QWorkspaceChild::~QWorkspaceChild()
{
    if (iconw)
        delete iconw->parentWidget();

    QWorkspace *workspace = qobject_cast<QWorkspace*>(parentWidget());
    if (workspace) {
        workspace->d->focus.removeAll(this);
        if (workspace->d->active == this)
            workspace->activatePreviousWindow();
        if (workspace->d->active == this)
            workspace->d->activateWindow(0);
        if (workspace->d->maxWindow == this) {
            workspace->d->hideMaximizeControls();
            workspace->d->maxWindow = 0;
        }
    }
}

void QWorkspaceChild::moveEvent(QMoveEvent *)
{
    ((QWorkspace*)parentWidget())->d->updateWorkspace();
}

void QWorkspaceChild::resizeEvent(QResizeEvent *)
{
    bool wasMax = isMaximized();
    QRect r = contentsRect();
    QRect cr;

    if (titlebar) {
        int th = titlebar->sizeHint().height();
        QRect tbrect(0, 0, width(), th);
        if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            tbrect = QRect(r.x(), r.y(), r.width(), th);
        titlebar->setGeometry(tbrect);

        if (style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            th -= frameWidth();
        cr = QRect(r.x(), r.y() + th + (shademode ? (frameWidth() * 3) : 0),
                    r.width(), r.height() - th);
    } else {
        cr = r;
    }

    if (!childWidget)
        return;

    windowSize = cr.size();
    backgroundWidget->setGeometry(cr);
    childWidget->move(cr.topLeft());
    if (!childWidget->testAttribute(Qt::WA_PendingResizeEvent))
        childWidget->resize(cr.size());
    ((QWorkspace*)parentWidget())->d->updateWorkspace();

    if (wasMax) {
        overrideWindowState(Qt::WindowMaximized);
        childWidget->overrideWindowState(Qt::WindowMaximized);
    }
}

QSize QWorkspaceChild::baseSize() const
{
    int th = titlebar ? titlebar->sizeHint().height() : 0;
    if (style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
        th -= frameWidth();
    return QSize(2*frameWidth(), 2*frameWidth() + th);
}

QSize QWorkspaceChild::sizeHint() const
{
    if (!childWidget)
        return QWidget::sizeHint() + baseSize();

    QSize prefSize = windowWidget()->sizeHint().expandedTo(windowWidget()->minimumSizeHint());
    prefSize = prefSize.expandedTo(windowWidget()->minimumSize()).boundedTo(windowWidget()->maximumSize());
    prefSize += baseSize();

    return prefSize;
}

QSize QWorkspaceChild::minimumSizeHint() const
{
    if (!childWidget)
        return QWidget::minimumSizeHint() + baseSize();
    QSize s = childWidget->minimumSize();
    if (s.isEmpty())
        s = childWidget->minimumSizeHint();
    return s + baseSize();
}

void QWorkspaceChild::activate()
{
    ((QWorkspace*)parentWidget())->d->activateWindow(windowWidget());
}

bool QWorkspaceChild::eventFilter(QObject * o, QEvent * e)
{
    if (!isActive() && (e->type() == QEvent::MouseButtonPress ||
        e->type() == QEvent::FocusIn)) {
        if (iconw) {
            ((QWorkspace*)parentWidget())->d->normalizeWindow(windowWidget());
            if (iconw) {
                ((QWorkspace*)parentWidget())->d->removeIcon(iconw->parentWidget());
                delete iconw->parentWidget();
                iconw = 0;
            }
        }
        activate();
    }

    // for all widgets except the window, that's the only thing we
    // process, and if we have no childWidget we skip totally
    if (o != childWidget || childWidget == 0)
        return false;

    switch (e->type()) {
    case QEvent::ShowToParent:
        if (((QWorkspace*)parentWidget())->d->focus.indexOf(this) < 0)
            ((QWorkspace*)parentWidget())->d->focus.append(this);

        if (windowWidget() && (windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)) {
            internalRaise();
            show();
            backgroundWidget->lower();
        }
        ((QWorkspace*)parentWidget())->d->showWindow(windowWidget());
        break;
    case QEvent::WindowStateChange: {
        Qt::WindowStates state = windowWidget()->windowState();

        if (state & Qt::WindowMinimized) {
            ((QWorkspace*)parentWidget())->d->minimizeWindow(windowWidget());
        } else if (state & Qt::WindowMaximized) {
            if (windowWidget()->maximumSize().isValid() &&
                (windowWidget()->maximumWidth() < parentWidget()->width() ||
                 windowWidget()->maximumHeight() < parentWidget()->height())) {
                windowWidget()->resize(windowWidget()->maximumSize());
                windowWidget()->overrideWindowState(Qt::WindowNoState);
                if (titlebar)
                    titlebar->repaint();
                break;
            }
            if ((windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint))
                ((QWorkspace*)parentWidget())->d->maximizeWindow(windowWidget());
            else
                ((QWorkspace*)parentWidget())->d->normalizeWindow(windowWidget());
        } else {
            ((QWorkspace*)parentWidget())->d->normalizeWindow(windowWidget());
            if (iconw) {
                ((QWorkspace*)parentWidget())->d->removeIcon(iconw->parentWidget());
                delete iconw->parentWidget();
            }
        }
    } break;
    case QEvent::HideToParent:
    {
        QWidget * w = iconw;
        if (w && (w = w->parentWidget())) {
            ((QWorkspace*)parentWidget())->d->removeIcon(w);
            delete w;
        }
        ((QWorkspace*)parentWidget())->d->hideChild(this);
    } break;
    case QEvent::WindowTitleChange:
        setWindowTitle(childWidget->windowTitle());
        if (iconw)
            iconw->setWindowTitle(childWidget->windowTitle());
        break;
    case QEvent::WindowIconChange:
        {
            QWorkspace* ws = (QWorkspace*)parentWidget();
            if (!titlebar)
                break;
            int iconSize = titlebar->size().height();
            QIcon icon = childWidget->windowIcon();

            titlebar->setWindowIcon(icon);
            if (iconw)
                iconw->setWindowIcon(icon);

            if (ws->d->maxWindow != this)
                break;

            if (ws->d->maxtools)
                ws->d->maxtools->setPixmap(icon.pixmap(QSize(iconSize, iconSize)));
        }
        break;
    case QEvent::Resize:
        {
            QResizeEvent* re = (QResizeEvent*)e;
            if (re->size() != windowSize && !shademode)
                resize(re->size() + baseSize());
        }
        break;

    case QEvent::WindowDeactivate:
        if (titlebar)
            titlebar->setActive(false);
        repaint();
        break;

    case QEvent::WindowActivate:
        if (titlebar)
            titlebar->setActive(act);
        repaint();
        break;

    default:
        break;
    }

    return QWidget::eventFilter(o, e);
}

bool QWorkspaceChild::focusNextPrevChild(bool next)
{
    extern Q_GUI_EXPORT bool qt_tab_all_widgets;
    uint focus_flag = qt_tab_all_widgets ? Qt::TabFocus : Qt::StrongFocus;

    QWidget *f = focusWidget();
    if (!f)
        f = this;

    QWidget *w = f;
    QWidget *test = f->nextInFocusChain();
    while (test != f) {
        if ((test->focusPolicy() & focus_flag) == focus_flag
            && !(test->focusProxy()) && test->isVisibleTo(this)
            && test->isEnabled() && isAncestorOf(w)) {
            w = test;
            if (next)
                break;
        }
        test = test->nextInFocusChain();
    }
    if (w == f)
        return false;
    w->setFocus();
    return true;
}

void QWorkspaceChild::childEvent(QChildEvent* e)
{
    if (e->type() == QEvent::ChildRemoved && e->child() == childWidget) {
        childWidget = 0;
        if (iconw) {
            ((QWorkspace*)parentWidget())->d->removeIcon(iconw->parentWidget());
            delete iconw->parentWidget();
        }
        close();
    }
}


void QWorkspaceChild::doResize()
{
    widgetResizeHandler->doResize();
}

void QWorkspaceChild::doMove()
{
    widgetResizeHandler->doMove();
}

void QWorkspaceChild::enterEvent(QEvent *)
{
}

void QWorkspaceChild::leaveEvent(QEvent *)
{
#ifndef QT_NO_CURSOR
    if (!widgetResizeHandler->isButtonDown())
        setCursor(Qt::ArrowCursor);
#endif
}

void QWorkspaceChild::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    QStyleOptionFrame opt;
    opt.rect = rect();
    opt.palette = palette();
    opt.state = QStyle::State_None;
    opt.lineWidth = style()->pixelMetric(QStyle::PM_MDIFrameWidth, 0, this);
    opt.midLineWidth = 1;

    if (titlebar && titlebar->isActive())
        opt.state |= QStyle::State_Active;

    style()->drawPrimitive(QStyle::PE_FrameWindow, &opt, &p, this);
}

void QWorkspaceChild::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        resizeEvent(0);
        if (iconw) {
            QVBoxWidget *vbox = qobject_cast<QVBoxWidget*>(iconw->parentWidget());
            Q_ASSERT(vbox);
            if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar)) {
                vbox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
                vbox->resize(196+2*vbox->frameWidth(), 20 + 2*vbox->frameWidth());
            } else {
                vbox->resize(196, 20);
            }
        }
    }
    QWidget::changeEvent(ev);
}

void QWorkspaceChild::setActive(bool b)
{
    if (!childWidget)
        return;

    bool hasFocus = isChildOf(window()->focusWidget(), this);
    if (act == b && hasFocus)
        return;

    act = b;

    if (titlebar)
        titlebar->setActive(act);
    if (iconw)
        iconw->setActive(act);
    update();

    QList<QWidget*> wl = qFindChildren<QWidget*>(childWidget);
    if (act) {
        for (int i = 0; i < wl.size(); ++i) {
            QWidget *w = wl.at(i);
            w->removeEventFilter(this);
        }
        if (!hasFocus) {
            QWidget *lastfocusw = childWidget->focusWidget();
            if (lastfocusw && lastfocusw->focusPolicy() != Qt::NoFocus) {
                lastfocusw->setFocus();
            } else if (childWidget->focusPolicy() != Qt::NoFocus) {
                childWidget->setFocus();
            } else {
                // find something, anything, that accepts focus, and use that.
                for (int i = 0; i < wl.size(); ++i) {
                    QWidget *w = wl.at(i);
                    if(w->focusPolicy() != Qt::NoFocus) {
                        w->setFocus();
                        break;
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < wl.size(); ++i) {
            QWidget *w = wl.at(i);
            w->removeEventFilter(this);
            w->installEventFilter(this);
        }
    }
}

bool QWorkspaceChild::isActive() const
{
    return act;
}

QWidget* QWorkspaceChild::windowWidget() const
{
    return childWidget;
}


QWidget* QWorkspaceChild::iconWidget() const
{
    if (!iconw) {
        QWorkspaceChild* that = (QWorkspaceChild*) this;

        QVBoxWidget* vbox = new QVBoxWidget(that, Qt::Window);
        vbox->setObjectName("qt_vbox");
        QWorkspaceTitleBar *tb = new QWorkspaceTitleBar(windowWidget(), vbox);
        tb->setObjectName("_workspacechild_icon_");
        QStyleOptionTitleBar opt = tb->getStyleOption();
        int th = style()->pixelMetric(QStyle::PM_TitleBarHeight, &opt, tb);
        int iconSize = style()->pixelMetric(QStyle::PM_MDIMinimizedWidth, 0, this);
        if (!style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar)) {
            vbox->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
            vbox->resize(iconSize+2*vbox->frameWidth(), th+2*vbox->frameWidth());
        } else {
            vbox->resize(iconSize, th);
        }
        that->iconw = tb;
        iconw->setActive(isActive());

        connect(iconw, SIGNAL(doActivate()),
                 this, SLOT(activate()));
        connect(iconw, SIGNAL(doClose()),
                 windowWidget(), SLOT(close()));
        connect(iconw, SIGNAL(doNormal()),
                 this, SLOT(showNormal()));
        connect(iconw, SIGNAL(doMaximize()),
                 this, SLOT(showMaximized()));
        connect(iconw, SIGNAL(popupOperationMenu(QPoint)),
                 this, SIGNAL(popupOperationMenu(QPoint)));
        connect(iconw, SIGNAL(showOperationMenu()),
                 this, SIGNAL(showOperationMenu()));
        connect(iconw, SIGNAL(doubleClicked()),
                 this, SLOT(titleBarDoubleClicked()));
    }
    if (windowWidget()) {
        iconw->setWindowTitle(windowWidget()->windowTitle());
    }
    return iconw->parentWidget();
}

void QWorkspaceChild::showMinimized()
{
    windowWidget()->setWindowState(Qt::WindowMinimized | (windowWidget()->windowState() & ~Qt::WindowMaximized));
}

void QWorkspaceChild::showMaximized()
{
    windowWidget()->setWindowState(Qt::WindowMaximized | (windowWidget()->windowState() & ~Qt::WindowMinimized));
}

void QWorkspaceChild::showNormal()
{
    windowWidget()->setWindowState(windowWidget()->windowState() & ~(Qt::WindowMinimized|Qt::WindowMaximized));
}

void QWorkspaceChild::showShaded()
{
    if (!titlebar)
        return;
    ((QWorkspace*)parentWidget())->d->activateWindow(windowWidget());
    QWidget* w = windowWidget();
    if (shademode) {
        w->overrideWindowState(Qt::WindowNoState);
        overrideWindowState(Qt::WindowNoState);

        shademode = false;
        resize(shadeRestore.expandedTo(minimumSizeHint()));
        setMinimumSize(shadeRestoreMin);
        style()->polish(this);
    } else {
        shadeRestore = size();
        shadeRestoreMin = minimumSize();
        setMinimumHeight(0);
        shademode = true;
        w->overrideWindowState(Qt::WindowMinimized);
        overrideWindowState(Qt::WindowMinimized);

        if (style()->styleHint(QStyle::SH_TitleBar_NoBorder, 0, titlebar))
            resize(width(), titlebar->height());
        else
            resize(width(), titlebar->height() + 2*frameWidth() + 1);
        style()->polish(this);
    }
    titlebar->update();
}

void QWorkspaceChild::titleBarDoubleClicked()
{
    if (!windowWidget())
        return;
    if (iconw)
        showNormal();
    else if (windowWidget()->windowFlags() & Qt::WindowShadeButtonHint)
            showShaded();
    else if (windowWidget()->windowFlags() & Qt::WindowMaximizeButtonHint)
        showMaximized();
}

void QWorkspaceChild::adjustToFullscreen()
{
    if (!childWidget)
        return;

    if(style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this)) {
        setGeometry(parentWidget()->rect());
    } else {
        int fw =  style()->pixelMetric(QStyle::PM_MDIFrameWidth, 0, this);
        int th = titlebar ? titlebar->sizeHint().height() : 0;
        int w = parentWidget()->width() + 2*fw;
        int h = parentWidget()->height() + 2*fw + th;
        w = qMax(w, childWidget->minimumWidth());
        h = qMax(h, childWidget->minimumHeight());
        setGeometry(-fw, -fw - th, w, h);
    }
    childWidget->overrideWindowState(Qt::WindowMaximized);
    overrideWindowState(Qt::WindowMaximized);
}

void QWorkspaceChild::setWindowTitle(const QString& cap)
{
    if (titlebar)
        titlebar->setWindowTitle(cap);
    QWidget::setWindowTitle(cap);
}

void QWorkspaceChild::internalRaise()
{

    QWidget *stackUnderWidget = 0;
    if (!windowWidget() || (windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint) == 0) {

        QList<QWorkspaceChild *>::Iterator it(((QWorkspace*)parent())->d->windows.begin());
        while (it != ((QWorkspace*)parent())->d->windows.end()) {
            QWorkspaceChild* c = *it;
            ++it;
            if (c->windowWidget() &&
                !c->windowWidget()->isExplicitlyHidden() &&
                (c->windowWidget()->windowFlags() & Qt::WindowStaysOnTopHint)) {
                if (stackUnderWidget)
                    c->stackUnder(stackUnderWidget);
                else
                    c->raise();
                stackUnderWidget = c;
            }
        }
    }

    if (stackUnderWidget) {
        if (iconw)
            iconw->parentWidget()->stackUnder(stackUnderWidget);
        stackUnder(stackUnderWidget);
    } else {
        if (iconw)
            iconw->parentWidget()->raise();
        raise();
    }

}

void QWorkspaceChild::show()
{
    if (childWidget && childWidget->isExplicitlyHidden())
        childWidget->show();
    QWidget::show();
}

bool QWorkspace::scrollBarsEnabled() const
{
    return d->vbar != 0;
}

/*!
    \property QWorkspace::scrollBarsEnabled
    \brief whether the workspace provides scrollbars

    If this property is true, the workspace will provide scrollbars if any
    of the child windows extend beyond the edges of the visible
    workspace. The workspace area will automatically increase to
    contain child windows if they are resized beyond the right or
    bottom edges of the visible area.

    If this property is false (the default), resizing child windows
    out of the visible area of the workspace is not permitted, although
    it is still possible to position them partially outside the visible area.
*/
void QWorkspace::setScrollBarsEnabled(bool enable)
{
    if ((d->vbar != 0) == enable)
        return;

    d->xoffset = d->yoffset = 0;
    if (enable) {
        d->vbar = new QScrollBar(Qt::Vertical, this);
        d->vbar->setObjectName("vertical scrollbar");
        connect(d->vbar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarChanged()));
        d->hbar = new QScrollBar(Qt::Horizontal, this);
        d->hbar->setObjectName("horizontal scrollbar");
        connect(d->hbar, SIGNAL(valueChanged(int)), this, SLOT(scrollBarChanged()));
        d->corner = new QWidget(this);
        d->corner->setObjectName("qt_corner");
        d->updateWorkspace();
    } else {
        delete d->vbar;
        delete d->hbar;
        delete d->corner;
        d->vbar = d->hbar = 0;
        d->corner = 0;
    }

    QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        QWorkspaceChild *child = *it;
        ++it;
        child->widgetResizeHandler->setSizeProtection(!enable);
    }
}

QRect QWorkspacePrivate::updateWorkspace()
{
    QRect cr(q->rect());

    if (q->scrollBarsEnabled() && !d->maxWindow) {
        d->corner->raise();
        d->vbar->raise();
        d->hbar->raise();
        if (d->maxWindow)
            d->maxWindow->internalRaise();

        QRect r(0, 0, 0, 0);
        QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
        while (it != d->windows.end()) {
            QWorkspaceChild *child = *it;
            ++it;
            if (!child->isExplicitlyHidden())
                r = r.unite(child->geometry());
        }
        d->vbar->blockSignals(true);
        d->hbar->blockSignals(true);

        int hsbExt = d->hbar->sizeHint().height();
        int vsbExt = d->vbar->sizeHint().width();


        bool showv = d->yoffset || d->yoffset + r.bottom() - q->height() + 1 > 0 || d->yoffset + r.top() < 0;
        bool showh = d->xoffset || d->xoffset + r.right() - q->width() + 1 > 0 || d->xoffset + r.left() < 0;

        if (showh && !showv)
            showv = d->yoffset + r.bottom() - q->height() + hsbExt + 1 > 0;
        if (showv && !showh)
            showh = d->xoffset + r.right() - q->width() + vsbExt  + 1 > 0;

        if (!showh)
            hsbExt = 0;
        if (!showv)
            vsbExt = 0;

        if (showv) {
            d->vbar->setSingleStep(qMax(q->height() / 12, 30));
            d->vbar->setPageStep(q->height() - hsbExt);
            d->vbar->setMinimum(qMin(0, d->yoffset + qMin(0, r.top())));
            d->vbar->setMaximum(qMax(0, d->yoffset + qMax(0, r.bottom() - q->height() + hsbExt + 1)));
            d->vbar->setGeometry(q->width() - vsbExt, 0, vsbExt, q->height() - hsbExt);
            d->vbar->setValue(d->yoffset);
            d->vbar->show();
        } else {
            d->vbar->hide();
        }

        if (showh) {
            d->hbar->setSingleStep(qMax(q->width() / 12, 30));
            d->hbar->setPageStep(q->width() - vsbExt);
            d->hbar->setMinimum(qMin(0, d->xoffset + qMin(0, r.left())));
            d->hbar->setMaximum(qMax(0, d->xoffset + qMax(0, r.right() - q->width() + vsbExt  + 1)));
            d->hbar->setGeometry(0, q->height() - hsbExt, q->width() - vsbExt, hsbExt);
            d->hbar->setValue(d->xoffset);
            d->hbar->show();
        } else {
            d->hbar->hide();
        }

        if (showh && showv) {
            d->corner->setGeometry(q->width() - vsbExt, q->height() - hsbExt, vsbExt, hsbExt);
            d->corner->show();
        } else {
            d->corner->hide();
        }

        d->vbar->blockSignals(false);
        d->hbar->blockSignals(false);

        cr.setRect(0, 0, q->width() - vsbExt, q->height() - hsbExt);
    }

    QList<QWidget *>::Iterator ii(d->icons.begin());
    while (ii != d->icons.end()) {
        QWidget* w = *ii;
        ++ii;
        int x = w->x();
        int y = w->y();
        bool m = false;
        if (x+w->width() > cr.width()) {
            m = true;
            x =  cr.width() - w->width();
        }
        if (y+w->height() >  cr.height()) {
            y =  cr.height() - w->height();
            m = true;
        }
        if (m) {
            if (QWorkspaceChild *child = qobject_cast<QWorkspaceChild*>(w))
                child->move(x, y);
            else
                w->move(x, y);
        }
    }

    return cr;

}

void QWorkspacePrivate::scrollBarChanged()
{
    int ver = d->yoffset - d->vbar->value();
    int hor = d->xoffset - d->hbar->value();
    d->yoffset = d->vbar->value();
    d->xoffset = d->hbar->value();

    QList<QWorkspaceChild *>::Iterator it(d->windows.begin());
    while (it != d->windows.end()) {
        QWorkspaceChild *child = *it;
        ++it;
        // we do not use move() due to the reimplementation in QWorkspaceChild
        child->setGeometry(child->x() + hor, child->y() + ver, child->width(), child->height());
    }
    d->updateWorkspace();
}

/*!
    \enum QWorkspace::WindowOrder

    Specifies the order in which child windows are returned from windowList().

    \value CreationOrder The windows are returned in the order of their creation
    \value StackingOrder The windows are returned in the order of their stacking
*/

/*!\reimp */
void QWorkspace::changeEvent(QEvent *ev)
{
    if(ev->type() == QEvent::StyleChange) {
        if (isVisible() && d->maxWindow) {
            if(style()->styleHint(QStyle::SH_Workspace_FillSpaceOnMaximize, 0, this))
                d->hideMaximizeControls();
            else
                d->showMaximizeControls();
        }
    }
    QWidget::changeEvent(ev);
}

#include "moc_qworkspace.cpp"
#include "qworkspace.moc"

#endif // QT_NO_WORKSPACE
