/****************************************************************************
**
** Implementation of QRadioButton class.
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

#include "qradiobutton.h"
#ifndef QT_NO_RADIOBUTTON
#include "qapplication.h"
#include "qbitmap.h"
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"

/*!
    \class QRadioButton qradiobutton.h
    \brief The QRadioButton widget provides a radio button with a text or pixmap label.

    \ingroup basic
    \mainclass

    QRadioButton and QCheckBox are both option buttons. That is, they
    can be switched on (checked) or off (unchecked). The classes
    differ in how the choices for the user are restricted. Check boxes
    define "many of many" choices, whereas radio buttons provide a
    "one of many" choice. In a group of radio buttons only one radio
    button at a time can be checked; if the user selects another
    button, the previously selected button is switched off.

    The easiest way to implement a "one of many" choice is simply to
    put the radio buttons into QButtonGroup.

    Whenever a button is switched on or off it emits the signal
    toggled(). Connect to this signal if you want to trigger an action
    each time the button changes state. Otherwise, use isChecked() to
    see if a particular button is selected.

    Just like QPushButton, a radio button can display text or a
    pixmap. The text can be set in the constructor or with setText();
    the pixmap is set with setPixmap().

    <img src=qradiobt-m.png> <img src=qradiobt-w.png>

    \important text, setText, text, pixmap, setPixmap, accel, setAccel, isToggleButton, setDown, isDown, isOn, state, autoRepeat, isExclusiveToggle, group, setAutoRepeat, toggle, pressed, released, clicked, toggled, state stateChanged

    \sa QPushButton QToolButton
    \link guibooks.html#fowler GUI Design Handbook: Radio Button\endlink
*/

/*!
    \property QRadioButton::checked \brief Whether the radio button is
    checked

    This property will not effect any other radio buttons unless they
    have been placed in the same QButtonGroup. The default value is
    false (unchecked).
*/

/*!
    \property QRadioButton::autoMask \brief whether the radio button
    is automatically masked

    \sa QWidget::setAutoMask()
*/


/*
    Initializes the radio button.
*/
static void qRadioButtonInit(QRadioButton *button)
{
    button->setCheckable(true);
    button->setAutoExclusive(true);
}


/*!
    Constructs a radio button with no text.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QRadioButton::QRadioButton(QWidget *parent)
        : QAbstractButton(parent)
{
    qRadioButtonInit(this);
}

/*!
    Constructs a radio button with the text \a text.

    The \a parent and \a name arguments are sent on to the QWidget
    constructor.
*/

QRadioButton::QRadioButton(const QString &text, QWidget *parent)
        : QAbstractButton(parent)
{
    qRadioButtonInit(this);
    setText(text);
}

static Q4StyleOptionButton getStyleOption(const QRadioButton *btn)
{
    Q4StyleOptionButton opt(0);
    opt.init(btn);
    opt.text = btn->text();
    opt.icon = btn->icon();
    if (btn->isDown())
        opt.state |= QStyle::Style_Down;
    if (btn->testAttribute(Qt::WA_UnderMouse))
        opt.state |= QStyle::Style_MouseOver;
    opt.state |= (btn->isChecked() ? QStyle::Style_On : QStyle::Style_Off);
    return opt;
}

/*!
    \reimp
*/
QSize QRadioButton::sizeHint() const
{
    ensurePolished();
    QFontMetrics fm = fontMetrics();
    QSize sz = style().itemRect(fm, QRect(0, 0, 1, 1), ShowPrefix, false, text()).size();
    Q4StyleOptionButton opt = getStyleOption(this);
    return (style().sizeFromContents(QStyle::CT_RadioButton, &opt, sz, fm, this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    \reimp
*/
bool QRadioButton::hitButton(const QPoint &pos) const
{
    Q4StyleOptionButton opt = getStyleOption(this);
    QRect r =
        QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonFocusRect, &opt, this), this);
    if (qApp->reverseLayout()) {
        r.setRight(width());
    } else {
        r.setLeft(0);
    }
    return r.contains(pos);
}

/*!
    Draws the radio button bevel. Called from paintEvent().

    \sa drawLabel()
*/
void QRadioButton::drawBevel(QPainter *p)
{
    Q4StyleOptionButton opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonIndicator, &opt, this),
                                  this);
    style().drawControl(QStyle::CE_RadioButton, &opt, p, this);
}

/*!
    Draws the radio button label. Called from paintEvent().

    \sa drawBevel()
*/
void QRadioButton::drawLabel(QPainter *p)
{
    Q4StyleOptionButton opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonContents, &opt, this),
                                  this);
    style().drawControl(QStyle::CE_RadioButtonLabel, &opt, p, this);
}

/*
  Paints the button, by first calling drawBevel() and then
  drawLabel(). If you reimplement paintEvent() in order to draw a
  different label only, you can call drawBevel() from your code.

  \code
    QPainter p(this);
    drawBevel(&p);
    // ... your label drawing code
  \endcode
*/
void QRadioButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    drawBevel(&p);
    drawLabel(&p);
}


/*!
    \reimp
*/
void QRadioButton::updateMask()
{
    Q4StyleOptionButton opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonIndicator, &opt, this),
                                  this);
    QBitmap bm(width(), height());
    bm.fill(color0);
    QPainter p(&bm);
    style().drawControlMask(QStyle::CE_RadioButton, &opt, &p, this);
    if (!text().isEmpty() || !icon().isNull()) {
        QRect crect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonContents, &opt,
                                                         this), this);
        QRect frect =
            QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonFocusRect, &opt, this), this);
        QRect label(crect.unite(frect));
        p.fillRect(label, color1);
    }
    p.end();
    setMask(bm);
}
#ifdef QT_COMPAT
QRadioButton::QRadioButton(QWidget *parent, const char* name)
    :QAbstractButton(parent)
{
    setObjectName(name);
    qRadioButtonInit(this);
}

QRadioButton::QRadioButton(const QString &text, QWidget *parent, const char* name)
    :QAbstractButton(parent)
{
    setObjectName(name);
    qRadioButtonInit(this);
    setText(text);
}

#endif
#endif
