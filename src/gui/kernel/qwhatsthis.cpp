/****************************************************************************
**
** Implementation of QWhatsThis class.
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

#include "qwhatsthis.h"
#ifndef QT_NO_WHATSTHIS
#include "qpointer.h"
#include "qapplication.h"
#include "qdesktopwidget.h"
#include "qevent.h"
#include "qpaintdevicemetrics.h"
#include "qpixmap.h"
#include "qpainter.h"
#include "qtimer.h"
#include "qhash.h"
#include "qtoolbutton.h"
#include "qcursor.h"
#include "qbitmap.h"
#include "qtooltip.h"
#include "qsimplerichtext.h"
#include "qstylesheet.h"
#if defined(QT_ACCESSIBILITY_SUPPORT)
#include "qaccessible.h"
#endif
#if defined(Q_WS_WIN)
#include "qt_windows.h"
#ifndef SPI_GETDROPSHADOW
#define SPI_GETDROPSHADOW                   0x1024
#endif
#endif
#if defined(Q_WS_X11)
#include "qx11info_x11.h"
#include <qwidget.h>
#endif

/*!
    \class QWhatsThis qwhatsthis.h
    \brief The QWhatsThis class provides a simple description of any
    widget, i.e. answering the question "What's this?".

    \ingroup helpsystem
    \mainclass

    "What's this?" help is part of an application's online help system
    that provides users with information about functionality, usage,
    background etc., in various levels of detail from short tool tips
    to full text browsing help windows.

    QWhatsThis provides a single window with an explanatory text that
    pops up when the user asks "What's this?". The default way to do
    this is to focus the relevant widget and press Shift+F1. The help
    text appears immediately; it goes away as soon as the user does
    something else.

    (Note that if there is an accelerator for Shift+F1, this mechanism
    will not work.)

    To add "What's this?" text to a widget or an action, you simply
    call setWhatsThis() on the widget or the action.

    The text can be either rich text or plain text. If you specify a
    rich text formatted string, it will be rendered using the default
    stylesheet. This makes it possible to embed images. See
    QStyleSheet::defaultSheet() for details.

    \quotefile action/application.cpp
    \skipto fileOpenText
    \printuntil setWhatsThis

    An alternative way to enter "What's this?" mode is to use the
    ready-made tool bar tool button from
    QWhatsThis::whatsThisButton(). By invoking this context help
    button (in the picture below the first one from the right) the
    user switches into "What's this?" mode. If they now click on a
    widget the appropriate help text is shown. The mode is left when
    help is given or when the user presses Esc.

    \img whatsthis.png

    If you are using QMainWindow you can also use the
    QMainWindow::whatsThis() slot to invoke the mode from a menu item.

    If you wish to control the "What's this?" behavior of a widget
    manually see Qt::WA_CustomWhatsThis.

    It is also possible to show different help texts for different
    regions of a widget, by using a QHelpEvent of type
    QEvent::WhatsThis. Intercept the help event in your widget's
    QWidget::event() function and call QWhatsThis::showText() with the
    text you want to display for the position specified in
    QHelpEvent::pos().  If the text is rich text and the user clicks
    on a link, the widget also receives a QWhatsThisClickedEvent with
    the link's reference as QWhatsThisClickedEvent::href(). If a
    QWhatsThisClickedEvent is handled (i.e. QWidget::event() returns
    true), the help window remains visible. Call
    QWhatsThis::hideText() to hide it explicitly.

    \sa QToolTip
*/


class QWhatsThat : public QWidget
{
    Q_OBJECT
public:
    QWhatsThat(const QString& txt, QWidget* parent, QWidget *showTextFor);
    ~QWhatsThat() ;

    static QWhatsThat *instance;

protected:
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void keyPressEvent(QKeyEvent*);
    void paintEvent(QPaintEvent*);

private:
    QPointer<QWidget>widget;
    bool pressed;
    QString text;
#ifndef QT_NO_RICHTEXT
    QSimpleRichText* doc;
#endif
    QString anchor;
};

QWhatsThat *QWhatsThat::instance = 0;

// shadowWidth not const, for XP drop-shadow-fu turns it to 0
static int shadowWidth = 6;   // also used as '5' and '6' and even '8' below
static const int vMargin = 8;
static const int hMargin = 12;

QWhatsThat::QWhatsThat(const QString& txt, QWidget* parent, QWidget *showTextFor)
    : QWidget(parent, WType_Popup | WDestructiveClose), widget(showTextFor),pressed(false), text(txt)
{
    delete instance;
    instance = this;
    setAttribute(WA_NoSystemBackground, true);
    setPalette(QToolTip::palette());
    setMouseTracking(true);
    setFocusPolicy(StrongFocus);
#ifndef QT_NO_CURSOR
    setCursor(ArrowCursor);
#endif


    QRect r;
#ifndef QT_NO_RICHTEXT
    doc = 0;
    if (QStyleSheet::mightBeRichText(text)) {
        QFont f = QApplication::font(this);
        doc = new QSimpleRichText(text, f);
        doc->adjustSize();
        r.setRect(0, 0, doc->width(), doc->height());
    }
    else
#endif
    {
        int sw = QApplication::desktop()->width() / 3;
        if (sw < 200)
            sw = 200;
        else if (sw > 300)
            sw = 300;

        r = fontMetrics().boundingRect(0, 0, sw, 1000,
                                        AlignAuto + AlignTop + WordBreak + ExpandTabs,
                                        text);
    }
#if defined(Q_WS_WIN)
    if ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_2000) {
        BOOL shadow;
        SystemParametersInfo(SPI_GETDROPSHADOW, 0, &shadow, 0);
        shadowWidth = shadow ? 0 : 6;
    }
#endif
    resize(r.width() + 2*hMargin + shadowWidth, r.height() + 2*vMargin + shadowWidth);
}

QWhatsThat::~QWhatsThat()
{
    instance = 0;
#ifndef QT_NO_RICHTEXT
    if (doc)
        delete doc;
#endif
}

void QWhatsThat::mousePressEvent(QMouseEvent* e)
{
    pressed = true;
    if (e->button() == LeftButton && rect().contains(e->pos())) {
#ifndef QT_NO_RICHTEXT
        if (doc)
            anchor = doc->anchorAt(e->pos() -  QPoint(hMargin, vMargin));
#endif
        return;
    }
    close();
}

void QWhatsThat::mouseReleaseEvent(QMouseEvent* e)
{
    if (!pressed)
        return;
#ifndef QT_NO_RICHTEXT
    if (widget && e->button() == LeftButton && doc && rect().contains(e->pos())) {
        QString a = doc->anchorAt(e->pos() -  QPoint(hMargin, vMargin));
        QString href;
        if (anchor == a)
            href = a;
        anchor = QString::null;
        if (!href.isEmpty()) {
            QWhatsThisClickedEvent e(href);
            if (QApplication::sendEvent(widget, &e))
                return;
        }
    }
#endif
    close();
}

void QWhatsThat::mouseMoveEvent(QMouseEvent* e)
{
#ifndef QT_NO_RICHTEXT
#ifndef QT_NO_CURSOR
    if (!doc)
        return;
    QString a = doc->anchorAt(e->pos() -  QPoint(hMargin, vMargin));
    if (!a.isEmpty())
        setCursor(PointingHandCursor);
    else
        setCursor(ArrowCursor);
#endif
#endif
}


void QWhatsThat::keyPressEvent(QKeyEvent*)
{
    close();
}



void QWhatsThat::paintEvent(QPaintEvent*)
{
    bool drawShadow = true;
#if defined(Q_WS_WIN)
    if ((QSysInfo::WindowsVersion&QSysInfo::WV_NT_based) > QSysInfo::WV_2000) {
        BOOL shadow;
        SystemParametersInfo(SPI_GETDROPSHADOW, 0, &shadow, 0);
        drawShadow = !shadow;
    }
#elif defined(Q_WS_MAC)
    drawShadow = false; //never draw it on OS X we get it for free
#endif

    QRect r = rect();
    if (drawShadow)
        r.addCoords(0, 0, -shadowWidth, -shadowWidth);
    QPainter p(this);
    p.setPen(palette().foreground());
    p.drawRect(r);
    p.setPen(palette().mid());
    p.setBrush(palette().brush(QPalette::Background));
    int w = r.width();
    int h = r.height();
    p.drawRect(1, 1, w-2, h-2);
    if (drawShadow) {
        p.setPen(palette().shadow());
        p.drawPoint(w + 5, 6);
        p.drawLine(w + 3, 6, w + 5, 8);
        p.drawLine(w + 1, 6, w + 5, 10);
        int i;
        for(i=7; i < h; i += 2)
            p.drawLine(w, i, w + 5, i + 5);
        for(i = w - i + h; i > 6; i -= 2)
            p.drawLine(i, h, i + 5, h + 5);
        for(; i > 0 ; i -= 2)
            p.drawLine(6, h + 6 - i, i + 5, h + 5);
    }
    p.setPen(palette().foreground());
    r.addCoords(hMargin, vMargin, -hMargin, -vMargin);

#ifndef QT_NO_RICHTEXT
    if (doc) {
        doc->draw(&p, r.x(), r.y(), r, palette(), 0);
    }
    else
#endif
    {
        p.drawText(r, AlignAuto + AlignTop + WordBreak + ExpandTabs, text);
    }
}

static const char * const button_image[] = {
"16 16 3 1",
"         c None",
"o        c #000000",
"a        c #000080",
"o        aaaaa  ",
"oo      aaa aaa ",
"ooo    aaa   aaa",
"oooo   aa     aa",
"ooooo  aa     aa",
"oooooo  a    aaa",
"ooooooo     aaa ",
"oooooooo   aaa  ",
"ooooooooo aaa   ",
"ooooo     aaa   ",
"oo ooo          ",
"o  ooo    aaa   ",
"    ooo   aaa   ",
"    ooo         ",
"     ooo        ",
"     ooo        "};

class QWhatsThisPrivate : public QObject
{
 public:
    QWhatsThisPrivate();
    ~QWhatsThisPrivate();
    static QWhatsThisPrivate *instance;
    bool eventFilter(QObject *, QEvent *);
    QPointer<QToolButton> button;
    static void say(QWidget *, const QString &, int x = 0, int y = 0);
};

QWhatsThisPrivate *QWhatsThisPrivate::instance = 0;

QWhatsThisPrivate::QWhatsThisPrivate()
{
    instance = this;
    qApp->installEventFilter(this);
    QApplication::setOverrideCursor(whatsThisCursor, false);
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::ContextHelpStart);
#endif
}

QWhatsThisPrivate::~QWhatsThisPrivate()
{
    if (button)
        button->setOn(false);
    QApplication::restoreOverrideCursor();
#if defined(QT_ACCESSIBILITY_SUPPORT)
    QAccessible::updateAccessibility(this, 0, QAccessible::ContextHelpEnd);
#endif
    instance = 0;
}

bool QWhatsThisPrivate::eventFilter(QObject *o, QEvent *e)
{
    if (!o->isWidgetType())
        return false;
    QWidget * w = static_cast<QWidget *>(o);
    bool customWhatsThis = w->testAttribute(Qt::WA_CustomWhatsThis);
    switch (e->type()) {
    case QEvent::MouseButtonPress:
    {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        if (me->button() == RightButton || customWhatsThis)
            return false;
        QHelpEvent e(QEvent::WhatsThis, me->pos(), me->globalPos());
        if (!QApplication::sendEvent(w, &e))
            QWhatsThis::leaveWhatsThisMode();
    } break;

    case QEvent::MouseButtonRelease:
    case QEvent::MouseMove:
    case QEvent::MouseButtonDblClick:
        if (static_cast<QMouseEvent*>(e)->button() == RightButton || customWhatsThis)
            return false; // ignore RMB release
        break;
    case QEvent::KeyPress:
    {
        QKeyEvent* kev = (QKeyEvent*)e;

        if (kev->key() == Qt::Key_Escape) {
            QWhatsThis::leaveWhatsThisMode();
            return true;
        } else if (o->isWidgetType() && ((QWidget*)o)->testAttribute(Qt::WA_CustomWhatsThis)) {
            return false;
        } else if (kev->key() == Key_Menu ||
                    (kev->key() == Key_F10 &&
                      kev->state() == ShiftButton)) {
            // we don't react to these keys, they are used for context menus
            return false;
        } else if (kev->state() == kev->stateAfter() &&
                    kev->key() != Key_Meta) {  // not a modifier key
            QWhatsThis::leaveWhatsThisMode();
        }
    } break;
    default:
        return false;
    }
    return true;
}

class QWhatsThisButton: public QToolButton
{
    Q_OBJECT

public:
    QWhatsThisButton(QWidget * parent, const char * name);

public slots:
    void buttonToggled(bool);
};

QWhatsThisButton::QWhatsThisButton(QWidget * parent, const char * name)
    : QToolButton(parent, name)
{
    QPixmap p((const char**)button_image);
    setIcon(p);
    setToggleButton(true);
    setAutoRaise(true);
    setFocusPolicy(NoFocus);
    setTextLabel(tr("What's this?"));
    connect(this, SIGNAL(toggled(bool)), this, SLOT(buttonToggled(bool)));
}

void QWhatsThisButton::buttonToggled(bool on)
{
    if (on) {
        QWhatsThis::enterWhatsThisMode();
        QWhatsThisPrivate::instance->button = this;
    }
}

QWhatsThis::QWhatsThis()
{}

/*!\obsolete Use QWidget::setWhatsThis() instead.*/
void QWhatsThis::add(QWidget *w, const QString &s)
{
    w->setWhatsThis(s);
}

/*!\obsolete Use QWidget::setWhatsThis() instead.*/
void QWhatsThis::remove(QWidget *w)
{
    w->setWhatsThis(QString::null);
}

QToolButton * QWhatsThis::whatsThisButton(QWidget * parent)
{
    return new QWhatsThisButton(parent, "automatic what's this? button");
}

void QWhatsThis::enterWhatsThisMode()
{
    if (QWhatsThisPrivate::instance)
        return;
    (void) new QWhatsThisPrivate;
}
bool QWhatsThis::inWhatsThisMode()
{
    return (QWhatsThisPrivate::instance != 0);
}
void QWhatsThis::leaveWhatsThisMode()
{
    delete QWhatsThisPrivate::instance;
}

void QWhatsThisPrivate::say(QWidget * widget, const QString &text, int x, int y)
{
    if (text.size() == 0)
        return;
    // make a fresh widget, and set it up
    QWhatsThat *whatsThat = new QWhatsThat(
        text,
#if defined(Q_WS_X11)
        QApplication::desktop()->screen(widget ?
                                         widget->x11Info()->screen() :
                                        QCursor::x11Screen()),
#else
        0,
#endif
        widget
       );


    // okay, now to find a suitable location

    int scr = (widget ?
                QApplication::desktop()->screenNumber(widget) :
#if defined(Q_WS_X11)
                QCursor::x11Screen()
#else
                QApplication::desktop()->screenNumber(QPoint(x,y))
#endif // Q_WS_X11
               );
    QRect screen = QApplication::desktop()->screenGeometry(scr);

    int w = whatsThat->width();
    int h = whatsThat->height();
    int sx = screen.x();
    int sy = screen.y();

    // first try locating the widget immediately above/below,
    // with nice alignment if possible.
    QPoint pos;
    if (widget)
        pos = widget->mapToGlobal(QPoint(0,0));

    if (widget && w > widget->width() + 16)
        x = pos.x() + widget->width()/2 - w/2;
    else
        x = x - w/2;

        // squeeze it in if that would result in part of what's this
        // being only partially visible
    if (x + w  + shadowWidth > sx+screen.width())
        x = (widget? (qMin(screen.width(),
                           pos.x() + widget->width())
                     ) : screen.width())
            - w;

    if (x < sx)
        x = sx;

    if (widget && h > widget->height() + 16) {
        y = pos.y() + widget->height() + 2; // below, two pixels spacing
        // what's this is above or below, wherever there's most space
        if (y + h + 10 > sy+screen.height())
            y = pos.y() + 2 - shadowWidth - h; // above, overlap
    }
    y = y + 2;

        // squeeze it in if that would result in part of what's this
        // being only partially visible
    if (y + h + shadowWidth > sy+screen.height())
        y = (widget ? (qMin(screen.height(),
                             pos.y() + widget->height())
                       ) : screen.height())
            - h;
    if (y < sy)
        y = sy;

    whatsThat->move(x, y);
    whatsThat->show();
    whatsThat->grabKeyboard();
}


void QWhatsThis::showText(const QPoint &pos, const QString &text, QWidget *w)
{
    leaveWhatsThisMode();
    QWhatsThisPrivate::say(w, text, pos.x(), pos.y());
}


void QWhatsThis::hideText()
{
    delete QWhatsThat::instance;
}

#include "qwhatsthis.moc"
#endif
