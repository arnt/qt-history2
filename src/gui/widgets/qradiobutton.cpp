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

#include "qradiobutton.h"
#ifndef QT_NO_RADIOBUTTON
#include "qapplication.h"
#include "qbitmap.h"
#include "qbuttongroup.h"
#include "qpainter.h"
#include "qstyle.h"
#include "qstyleoption.h"

/*!
    \class QRadioButton
    \brief The QRadioButton widget provides a radio button with a text or pixmap label.

    \ingroup basic
    \mainclass

    A QRadioButton is an option button that can be switched on (checked) or
    off (unchecked). Radio buttons typically present the user with a "one
    of many" choice. In a group of radio buttons only one radio button at
    a time can be checked; if the user selects another button, the
    previously selected button is switched off.

    Radio buttons are autoExclusive by default. If autoExclusive is
    enabled, radio buttons that belong to the same parent widget
    behave as if they were part of the same exclusive button group. If
    you need multiple exclusive button groups for radio buttons that
    belong to the same parent widget, put them into a QButtonGroup.

    Whenever a button is switched on or off it emits the toggled() signal.
    Connect to this signal if you want to trigger an action each time the
    button changes state. Otherwise, use isChecked() to see if a particular
    button is selected.

    Just like QPushButton, a radio button can display text or a
    pixmap. The text can be set in the constructor or with setText();
    the pixmap is set with setPixmap().

    Important inherited members: text(), setText(), text(),
    setDown(), isDown(), autoRepeat(), group(), setAutoRepeat(),
    toggle(), pressed(), released(), clicked(), and toggled().

    \inlineimage qradiobt-m.png Screenshot in Motif style
    \inlineimage qradiobt-w.png Screenshot in Windows style

    \sa QPushButton QToolButton QCheckBox
    \link guibooks.html#fowler GUI Design Handbook: Radio Button\endlink
*/

/*!
    \property QRadioButton::autoMask
    \brief whether the radio button is automatically masked

    \sa QWidget::setAutoMask()
*/

/*!
    \property QRadioButton::autoExclusive
    \brief whether the radio button is automatically exclusive

    If the radio button is exclusive, it will be deselected if another in
    the same group is enabled. Similarly, if it is selected, other exclusive
    buttons in the same group will be deselected.

    The default is true.
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
    Constructs a radio button with the given \a parent, but with no text or
    pixmap.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QRadioButton::QRadioButton(QWidget *parent)
        : QAbstractButton(parent)
{
    qRadioButtonInit(this);
}

/*!
    Constructs a radio button with the given \a parent and a \a text string.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QRadioButton::QRadioButton(const QString &text, QWidget *parent)
        : QAbstractButton(parent)
{
    qRadioButtonInit(this);
    setText(text);
}

static QStyleOptionButton getStyleOption(const QRadioButton *btn)
{
    QStyleOptionButton opt(0);
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
    QSize sz = style().itemRect(fm, QRect(0, 0, 1, 1), Qt::TextShowMnemonic, false, text()).size();
    QStyleOptionButton opt = getStyleOption(this);
    return (style().sizeFromContents(QStyle::CT_RadioButton, &opt, sz, fm, this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    \reimp
*/
bool QRadioButton::hitButton(const QPoint &pos) const
{
    QStyleOptionButton opt = getStyleOption(this);
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
    Draws the radio button bevel on painter \a p. Called from
    paintEvent().

    \sa drawLabel()
*/
void QRadioButton::drawBevel(QPainter *p)
{
    QStyleOptionButton opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonIndicator, &opt, this),
                                  this);
    style().drawControl(QStyle::CE_RadioButton, &opt, p, this);
}

/*!
    Draws the radio button label on painter \a p. Called from
    paintEvent().

    \sa drawBevel()
*/
void QRadioButton::drawLabel(QPainter *p)
{
    QStyleOptionButton opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonContents, &opt, this),
                                  this);
    style().drawControl(QStyle::CE_RadioButtonLabel, &opt, p, this);
}

/*!
    \fn void QRadioButton::paintEvent(QPaintEvent *event)

    Paints the button in response to the paint \a event, by first
    calling drawBevel() and then drawLabel(). If you reimplement
    paintEvent() just to draw a different label, you can call
    drawBevel() from your own code. For example:
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
    QStyleOptionButton opt = getStyleOption(this);
    opt.rect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonIndicator, &opt, this),
                                  this);
    QBitmap bm(width(), height());
    bm.fill(Qt::color0);
    QPainter p(&bm);
    style().drawControlMask(QStyle::CE_RadioButton, &opt, &p, this);
    if (!text().isEmpty() || !icon().isNull()) {
        QRect crect = QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonContents, &opt,
                                                         this), this);
        QRect frect =
            QStyle::visualRect(style().subRect(QStyle::SR_RadioButtonFocusRect, &opt, this), this);
        QRect label(crect.unite(frect));
        p.fillRect(label, Qt::color1);
    }
    p.end();
    setMask(bm);
}
#ifdef QT_COMPAT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QRadioButton::QRadioButton(QWidget *parent, const char* name)
    :QAbstractButton(parent)
{
    setObjectName(name);
    qRadioButtonInit(this);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QRadioButton::QRadioButton(const QString &text, QWidget *parent, const char* name)
    :QAbstractButton(parent)
{
    setObjectName(name);
    qRadioButtonInit(this);
    setText(text);
}

#endif
#endif
