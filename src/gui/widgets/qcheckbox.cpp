/****************************************************************************
**
** Implementation of QCheckBox class.
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

#include "qcheckbox.h"
#ifndef QT_NO_CHECKBOX
#include "qpainter.h"
#include "qbitmap.h"
#include "qapplication.h"
#include "qstyle.h"

/*!
    \class QCheckBox qcheckbox.h
    \brief The QCheckBox widget provides a checkbox with a text label.

    \ingroup basic
    \mainclass

    QCheckBox and QRadioButton are both option buttons. That is, they
    can be switched on (checked) or off (unchecked). The classes
    differ in how the choices for the user are restricted. Radio
    buttons define a "one of many" choice, whereas checkboxes provide
    "many of many" choices.

    A QButtonGroup can be used to group check buttons visually.

    Whenever a checkbox is checked or cleared it emits the signal
    toggled(). Connect to this signal if you want to trigger an action
    each time the checkbox changes state. You can use isChecked() to
    query whether or not a checkbox is checked.

    \warning The toggled() signal can not be trusted for tristate
    checkboxes.

    In addition to the usual checked and unchecked states, QCheckBox
    optionally provides a third state to indicate "no change". This
    is useful whenever you need to give the user the option of neither
    checking nor unchecking a checkbox. If you need this third state,
    enable it with setTristate() and use state() to query the current
    toggle state. When a tristate checkbox changes state, it emits the
    stateChanged() signal.

    Just like QPushButton, a checkbox can display text or a pixmap.
    The text can be set in the constructor or with setText(); the
    pixmap is set with setPixmap().

    \important text(), setText(), text(), pixmap(), setPixmap(),
    accel(), setAccel(), isToggleButton(), setDown(), isDown(),
    isOn(), state(), autoRepeat(), isExclusiveToggle(), group(),
    setAutoRepeat(), toggle(), pressed(), released(), clicked(),
    toggled(), state() stateChanged()

    <img src=qchkbox-m.png> <img src=qchkbox-w.png>

    \sa QButton QRadioButton
    \link guibooks.html#fowler Fowler: Check Box \endlink
*/

/*!
    \property QCheckBox::checked
    \brief whether the checkbox is checked

    The default is unchecked, i.e. false.
*/

/*!
    \property QCheckBox::autoMask
    \brief whether the checkbox is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QCheckBox::tristate
    \brief whether the checkbox is a tri-state checkbox

    The default is two-state, i.e. tri-state is false.
*/

static QPixmap *qt_checkbox_painter_pix = 0;

/*!
    Constructs a checkbox with no text.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/

QCheckBox::QCheckBox(QWidget *parent, const char *name)
        : QButton(parent, name, WMouseNoMask)
{
    setToggleButton(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
}

/*!
    Constructs a checkbox with text \a text.

    The \a parent and \a name arguments are sent to the QWidget
    constructor.
*/

QCheckBox::QCheckBox(const QString &text, QWidget *parent, const char *name)
        : QButton(parent, name, WMouseNoMask)
{
    setText(text);
    setToggleButton(true);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
}

/*!
    Sets the checkbox to the "no change" state.

    \sa setTristate()
*/
void QCheckBox::setNoChange()
{
    setTristate(true);
    setState(NoChange);
}

void QCheckBox::setTristate(bool y)
{
    setToggleType(y ? Tristate : Toggle);
}

bool QCheckBox::isTristate() const
{
    return toggleType() == Tristate;
}


/*!\reimp
*/
QSize QCheckBox::sizeHint() const
{
    // NB: QRadioButton::sizeHint() is similar
    ensurePolished();


    if(!qt_checkbox_painter_pix)
        qt_checkbox_painter_pix = new QPixmap(1, 1);
    QPainter p(qt_checkbox_painter_pix, this);
    QSize sz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, false,
                                pixmap(), text()).size();

    return (style().sizeFromContents(QStyle::CT_CheckBox, this, sz).
            expandedTo(QApplication::globalStrut()));
}


/*!\reimp
*/

void QCheckBox::drawButton(QPainter *paint)
{
    QPainter *p = paint;
    QRect irect = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxIndicator, this), this);
    const QPalette &pal = palette();

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (hasFocus())
        flags |= QStyle::Style_HasFocus;
    if (isDown())
        flags |= QStyle::Style_Down;
    if (testAttribute(WA_UnderMouse))
        flags |= QStyle::Style_MouseOver;
    if (state() == QButton::On)
        flags |= QStyle::Style_On;
    else if (state() == QButton::Off)
        flags |= QStyle::Style_Off;
    else if (state() == QButton::NoChange)
        flags |= QStyle::Style_NoChange;

    style().drawControl(QStyle::CE_CheckBox, p, this, irect, pal, flags);

    drawButtonLabel(paint);
}


/*!\reimp
*/
void QCheckBox::drawButtonLabel(QPainter *p)
{
    QRect r =
        QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxContents, this), this);

    QStyle::SFlags flags = QStyle::Style_Default;
    if (isEnabled())
        flags |= QStyle::Style_Enabled;
    if (hasFocus())
        flags |= QStyle::Style_HasFocus;
    if (isDown())
        flags |= QStyle::Style_Down;
    if (state() == QButton::On)
        flags |= QStyle::Style_On;
    else if (state() == QButton::Off)
        flags |= QStyle::Style_Off;
    else if (state() == QButton::NoChange)
        flags |= QStyle::Style_NoChange;

    style().drawControl(QStyle::CE_CheckBoxLabel, p, this, r, palette(), flags);
}

/*!
  \reimp
*/
void QCheckBox::resizeEvent(QResizeEvent *e)
{
    QButton::resizeEvent(e);
    if (isVisible()) {
    if(!qt_checkbox_painter_pix)
        qt_checkbox_painter_pix = new QPixmap(1, 1);
    QPainter p(qt_checkbox_painter_pix, this);
    QSize isz = style().itemRect(&p, QRect(0, 0, 1, 1), ShowPrefix, false,
                                 pixmap(), text()).size();
    QSize wsz = (style().sizeFromContents(QStyle::CT_CheckBox, this, isz).
                 expandedTo(QApplication::globalStrut()));

    update(wsz.width(), isz.width(), 0, wsz.height());
    }
    if (autoMask())
        updateMask();
}

/*!
  \reimp
*/
void QCheckBox::updateMask()
{
    QRect irect =
        QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxIndicator, this), this);

    QBitmap bm(width(), height());
    bm.fill(color0);

    QPainter p(&bm, this);
    style().drawControlMask(QStyle::CE_CheckBox, &p, this, irect);
    if (! text().isNull() || (pixmap() && ! pixmap()->isNull())) {
        QRect crect =
            QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxContents,
                                                 this), this);
        QRect frect =
            QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxFocusRect,
                                                 this), this);
        QRect label(crect.unite(frect));
        p.fillRect(label, color1);
    }
    p.end();

    setMask(bm);
}

/*!\reimp*/
bool QCheckBox::hitButton(const QPoint &pos) const
{
    QRect r = QStyle::visualRect(style().subRect(QStyle::SR_CheckBoxFocusRect, this), this);
    if (qApp->reverseLayout()) {
        r.setRight(width());
    } else {
        r.setLeft(0);
    }
    return r.contains(pos);
}

#endif
