/****************************************************************************
**
** Implementation of QPushButton class.
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

#include "qapplication.h"
#include "qbitmap.h"
#include "qbutton.h"
#include "qdesktopwidget.h"
#include "qdialog.h"
#include <private/qdialog_p.h>
#include "qdrawutil.h"
#include "qevent.h"
#include "qfontmetrics.h"
#include "qmenu.h"
#include "qpainter.h"
#include "qpixmap.h"
#include "qpointer.h"
#include "qpushbutton.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qtoolbar.h"

#ifndef QT_NO_ACCESSIBILITY
#include "qaccessible.h"
#endif

#include "private/qabstractbutton_p.h"


class QPushButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QPushButton)
public:
    QPushButtonPrivate():autoDefault(true), defaultButton(false), flat(false){}
    void init();
    void popupPressed();
    QStyleOptionButton getStyleOption() const;
    QPointer<QMenu> menu;
    uint autoDefault : 1;
    uint defaultButton : 1;
    uint flat : 1;
};

#define d d_func()
#define q q_func()

/*!
    \class QPushButton qpushbutton.h
    \brief The QPushButton widget provides a command button.

    \ingroup basic
    \mainclass

    The push button, or command button, is perhaps the most commonly
    used widget in any graphical user interface. Push (click) a button
    to command the computer to perform some action, or to answer a
    question. Typical buttons are OK, Apply, Cancel, Close, Yes, No
    and Help.

    A command button is rectangular and typically displays a text
    label describing its action. An underlined character in the label
    (signified by preceding it with an ampersand in the text)
    indicates a shortcut key, e.g.
    \code
        QPushButton *pb = new QPushButton("&Download", this);
    \endcode
    In this example the shortcut is \e{Alt+D}, and the label text
    will be displayed as <b><u>D</u>ownload</b>.

    Push buttons can display a textual label or a pixmap, and
    optionally a small icon. These can be set using the constructors
    and changed later using setText(), setPixmap() and setIconSet().
    If the button is disabled the appearance of the text or pixmap and
    iconset will be manipulated with respect to the GUI style to make
    the button look "disabled".

    A push button emits the signal clicked() when it is activated by
    the mouse, the Spacebar or by a keyboard shortcut. Connect to
    this signal to perform the button's action. Push buttons also
    provide less commonly used signals, for example, pressed() and
    released().

    Command buttons in dialogs are by default auto-default buttons,
    i.e. they become the default push button automatically when they
    receive the keyboard input focus. A default button is a push
    button that is activated when the user presses the Enter or Return
    key in a dialog. You can change this with setAutoDefault(). Note
    that auto-default buttons reserve a little extra space which is
    necessary to draw a default-button indicator. If you do not want
    this space around your buttons, call setAutoDefault(false).

    Being so central, the button widget has grown to accommodate a
    great many variations in the past decade. The Microsoft style
    guide now shows about ten different states of Windows push buttons
    and the text implies that there are dozens more when all the
    combinations of features are taken into consideration.

    The most important modes or states are:
    \list
    \i Available or not (grayed out, disabled).
    \i Standard push button, toggling push button or menu button.
    \i On or off (only for toggling push buttons).
    \i Default or normal. The default button in a dialog can generally
       be "clicked" using the Enter or Return key.
    \i Auto-repeat or not.
    \i Pressed down or not.
    \endlist

    As a general rule, use a push button when the application or
    dialog window performs an action when the user clicks on it (such
    as Apply, Cancel, Close and Help) \e and when the widget is
    supposed to have a wide, rectangular shape with a text label.
    Small, typically square buttons that change the state of the
    window rather than performing an action (such as the buttons in
    the top-right corner of the QFileDialog) are not command buttons,
    but tool buttons. Qt provides a special class (QToolButton) for
    these buttons.

    If you need toggle behavior (see setCheckable()) or a button
    that auto-repeats the activation signal when being pushed down
    like the arrows in a scroll bar (see setAutoRepeat()), a command
    button is probably not what you want. When in doubt, use a tool
    button.

    A variation of a command button is a menu button. These provide
    not just one command, but several, since when they are clicked
    they pop up a menu of options. Use the method setMenu() to
    associate a popup menu with a push button.

    Other classes of buttons are option buttons (see QRadioButton) and
    check boxes (see QCheckBox).

    \inlineimage qpushbt-m.png Screenshot in Motif style
    \inlineimage qpushbt-w.png Screenshot in Windows style

    In Qt, the QAbstractButton base class provides most of the modes
    and other API, and QPushButton provides GUI logic.
    See QAbstractButton for more information about the API.

    \sa QToolButton, QRadioButton QCheckBox
    \link guibooks.html#fowler GUI Design Handbook: Push Button\endlink
*/

/*!
    \property QPushButton::autoDefault
    \brief whether the push button is the auto default button

    If this property is set to true then the push button is the auto
    default button in a dialog.

    In some GUI styles a default button is drawn with an extra frame
    around it, up to 3 pixels or more. Qt automatically keeps this
    space free around auto-default buttons, i.e. auto-default buttons
    may have a slightly larger size hint.

    This property's default is true for buttons that have a QDialog
    parent; otherwise it defaults to false.

    See the \l default property for details of how \l default and
    auto-default interact.
*/

/*!
    \property QPushButton::autoMask
    \brief whether the button is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QPushButton::default
    \brief whether the push button is the default button

    If this property is set to true then the push button will be
    pressed if the user presses the Enter (or Return) key in a dialog.

    Regardless of focus, if the user presses Enter: If there is a
    default button the default button is pressed; otherwise, if
    there are one or more \l autoDefault buttons the first \l autoDefault
    button that is next in the tab order is pressed. If there are no
    default or \l autoDefault buttons only pressing Space on a button
    with focus, mouse clicking, or using a shortcut will press a
    button.

    In a dialog, only one push button at a time can be the default
    button. This button is then displayed with an additional frame
    (depending on the GUI style).

    The default button behavior is provided only in dialogs. Buttons
    can always be clicked from the keyboard by pressing Enter (or
    Return) or the Spacebar when the button has focus.

    This property's default is false.
*/

/*!
    \property QPushButton::flat
    \brief whether the border is disabled

    This property's default is false.
*/



/*!
    Constructs a push button with no text and a \a parent.
*/

QPushButton::QPushButton(QWidget *parent)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    d->init();
}

/*!
    Constructs a push button with the parent \a parent and the text \a
    text.
*/

QPushButton::QPushButton(const QString &text, QWidget *parent)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    d->init();
    setText(text);
}


/*!
    Constructs a push button with an \a icon and a \a text,  and a \a parent.

    Note that you can also pass a QPixmap object as an icon (thanks to
    the implicit type conversion provided by C++).

*/
QPushButton::QPushButton(const QIconSet& icon, const QString &text, QWidget *parent)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    d->init();
    setText(text);
    setIcon(icon);
}


/*!
    Destroys the push button.
*/
QPushButton::~QPushButton()
{
}

void QPushButtonPrivate::init()
{
#ifndef QT_NO_DIALOG
    d->autoDefault = (qt_cast<QDialog*>(q->topLevelWidget()) != 0);
#endif
    q->setAttribute(Qt::WA_BackgroundInherited);
    q->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

QStyleOptionButton QPushButtonPrivate::getStyleOption() const
{
    QStyleOptionButton opt(0);
    opt.init(q);
    opt.extras = QStyleOptionButton::None;
    if (flat)
        opt.extras |= QStyleOptionButton::Flat;
    if (menu)
        opt.extras |= QStyleOptionButton::HasMenu;
    if (down)
        opt.state |= QStyle::Style_Down;
    if (checked)
        opt.state |= QStyle::Style_On;
    if (!flat && !down)
        opt.state |= QStyle::Style_Raised;
    if (defaultButton)
        opt.state |= QStyle::Style_ButtonDefault;
    opt.text = text;
    opt.icon = icon;
    return opt;
}

void QPushButton::setAutoDefault(bool enable)
{
    if (d->autoDefault == enable)
        return;
    d->autoDefault = enable;
    update();
    updateGeometry();
}

bool QPushButton::autoDefault() const
{
    return d->autoDefault;
}

void QPushButton::setDefault(bool enable)
{
    if (d->defaultButton == enable)
        return;
    d->d->defaultButton = enable;
#ifndef QT_NO_DIALOG
    if (d->defaultButton) {
        QDialog *dlg = qt_cast<QDialog*>(topLevelWidget());
        if (dlg)
            dlg->d->setMainDefault(this);
    }
#endif
    update();
#ifndef QT_NO_ACCESSIBILITY
    QAccessible::updateAccessibility(this, 0, QAccessible::StateChanged);
#endif
}

bool QPushButton::isDefault() const
{
    return d->defaultButton;
}

/*!
    \reimp
*/
QSize QPushButton::sizeHint() const
{
    ensurePolished();

    int w = 0, h = 0;

    // calculate contents size...
#ifndef QT_NO_ICONSET
    if (!icon().isNull()) {
        int iw = icon().pixmap(QIconSet::Small, QIconSet::Normal).width() + 4;
        int ih = icon().pixmap(QIconSet::Small, QIconSet::Normal).height();
        w += iw;
        h = qMax(h, ih);
    }
#endif
    if (menu())
        w += style().pixelMetric(QStyle::PM_MenuButtonIndicator, this);
    QString s(text());
    bool empty = s.isEmpty();
    if (empty)
        s = QString::fromLatin1("XXXX");
    QFontMetrics fm = fontMetrics();
    QSize sz = fm.size(Qt::ShowPrefix, s);
    if(!empty || !w)
        w += sz.width();
    if(!empty || !h)
        h = qMax(h, sz.height());
    QStyleOptionButton opt = d->getStyleOption();
    return (style().sizeFromContents(QStyle::CT_PushButton, &opt, QSize(w, h), fm, this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    Draws the push button bevel on painter \a paint. Called from
    paintEvent().

    \sa drawLabel()
*/

void QPushButton::drawBevel(QPainter *paint)
{
    QStyleOptionButton opt = d->getStyleOption();
    style().drawControl(QStyle::CE_PushButton, &opt, paint, this);
}


/*!
    Draws the push button label on painter \a paint. Called from
    paintEvent().

    \sa drawBevel()
*/
void QPushButton::drawLabel(QPainter *paint)
{
    QStyleOptionButton opt = d->getStyleOption();
    opt.rect = style().subRect(QStyle::SR_PushButtonContents, &opt, this);
    style().drawControl(QStyle::CE_PushButtonLabel, &opt, paint, this);
}


/*!
    \reimp
 */
void QPushButton::updateMask()
{
    QBitmap bm(size());
    bm.fill(Qt::color0);

    QPainter p(&bm);
    QStyleOptionButton opt = d->getStyleOption();
    style().drawControlMask(QStyle::CE_PushButton, &opt, &p, this);
    p.end();

    setMask(bm);
}


/*!
    Paints the button, by first calling drawBevel() and then
    drawLabel(). If you reimplement paintEvent() just to draw a
    different label, you can call drawBevel() from your own code, for
    example:
    \code
        QPainter p(this);
        drawBevel(&p);
        // ... your label drawing code
    \endcode
*/
void QPushButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawBevel(&p);
    drawLabel(&p);
}


/*! \reimp */
void QPushButton::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (d->autoDefault || d->defaultButton) {
            click();
            break;
        }
        // fall through
    default:
        QAbstractButton::keyPressEvent(e);
    }
}

/*!
    \reimp
*/
void QPushButton::focusInEvent(QFocusEvent *e)
{
    if (d->autoDefault && !d->defaultButton) {
        d->defaultButton = true;
#ifndef QT_NO_DIALOG
        QDialog *dlg = qt_cast<QDialog*>(topLevelWidget());
        if (dlg)
            dlg->d->setDefault(this);
#endif
    }
    QAbstractButton::focusInEvent(e);
}

/*!
    \reimp
*/
void QPushButton::focusOutEvent(QFocusEvent *e)
{
    if (d->autoDefault && d->defaultButton) {
#ifndef QT_NO_DIALOG
        QDialog *dlg = qt_cast<QDialog*>(topLevelWidget());
        if (dlg)
            dlg->d->setDefault(0);
        else
#endif
            d->defaultButton = false;
    }

    QAbstractButton::focusOutEvent(e);
    if (d->menu && d->menu->isVisible())        // restore pressed status
        setDown(true);
}


/*!
    Associates the popup menu \a menu with this push button. This
    turns the button into a menu button, which in some styles will
    produce a small triangle to the right of the button's text.

    Ownership of the menu is \e not transferred to the push button.

    \sa menu()
*/
void QPushButton::setMenu(QMenu* menu)
{
    if (menu && !d->menu) {
        disconnect(this, SIGNAL(pressed()), this, SLOT(popupPressed()));
        connect(this, SIGNAL(pressed()), this, SLOT(popupPressed()));
    }
    d->menu = menu;
    update();
    updateGeometry();
}

/*!
    Returns the button's associated popup menu or 0 if no popup menu
    has been set.

    \sa setMenu()
*/
QMenu* QPushButton::menu() const
{
    return d->menu;
}

/*!
    Shows (pops up) the associated popup menu. If there is no such
    menu, this function does nothing. This function does not return
    until the popup menu has been closed by the user.
*/
void QPushButton::showMenu()
{
    if (!d || !d->menu)
        return;
    setDown(true);
    d->popupPressed();
}

void QPushButtonPrivate::popupPressed()
{
    if (!down || !menu)
        return;

    bool horizontal = true;
    bool topLeft = true;                        // ### always true
#ifndef QT_NO_TOOLBAR
    QToolBar *tb = qt_cast<QToolBar*>(q->parentWidget());
    if (tb && (tb->area() == Qt::ToolBarAreaLeft || tb->area() == Qt::ToolBarAreaRight))
        horizontal = false;
#endif
    QRect rect = q->rect();
    if (horizontal) {
        if (topLeft) {
            if (q->mapToGlobal(QPoint(0, rect.bottom())).y() + menu->sizeHint().height() <= qApp->desktop()->height())
                menu->exec(q->mapToGlobal(rect.bottomLeft()));
            else
                menu->exec(q->mapToGlobal(rect.topLeft() - QPoint(0, menu->sizeHint().height())));
        } else {
            QSize sz(menu->sizeHint());
            QPoint p = q->mapToGlobal(rect.topLeft());
            p.ry() -= sz.height();
            menu->exec(p);
        }
    }
    else {
        if (topLeft) {
            if (q->mapToGlobal(QPoint(rect.right(), 0)).x() + menu->sizeHint().width() <= qApp->desktop()->width())
                menu->exec(q->mapToGlobal(rect.topRight()));
            else
                menu->exec(q->mapToGlobal(rect.topLeft() - QPoint(menu->sizeHint().width(), 0)));
        } else {
            QSize sz(menu->sizeHint());
            QPoint p = q->mapToGlobal(rect.topLeft());
            p.rx() -= sz.width();
            menu->exec(p);
        }
    }
    q->setDown(false);
}

void QPushButton::setFlat(bool flat)
{
    if (d->flat == flat)
        return;
    d->flat = flat;
    update();
    updateGeometry();
}

bool QPushButton::isFlat() const
{
    return d->flat;
}

#ifdef QT_COMPAT
QPushButton::QPushButton(QWidget *parent, const char *name)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    setObjectName(name);
    d->init();
}

QPushButton::QPushButton(const QString &text, QWidget *parent, const char *name)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    setObjectName(name);
    d->init();
    setText(text);
}

QPushButton::QPushButton(const QIconSet& icon, const QString &text, QWidget *parent, const char *name)
    : QAbstractButton(*new QPushButtonPrivate, parent)
{
    setObjectName(name);
    d->init();
    setText(text);
    setIcon(icon);
}
#endif

#include "moc_qpushbutton.cpp"
