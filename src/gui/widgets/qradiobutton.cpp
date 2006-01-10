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
#include "qapplication.h"
#include "qbitmap.h"
#include "qbuttongroup.h"
#include "qstylepainter.h"
#include "qstyle.h"
#include "qstyleoption.h"
#include "qevent.h"

#include "private/qabstractbutton_p.h"

class QRadioButtonPrivate : public QAbstractButtonPrivate
{
    Q_DECLARE_PUBLIC(QRadioButton)
public:
    QRadioButtonPrivate():hovering(true){}
    uint hovering : 1;
};


/*!
    \class QRadioButton
    \brief The QRadioButton widget provides a radio button with a text label.

    \ingroup basic
    \mainclass

    A QRadioButton is an option button that can be switched on (checked) or
    off (unchecked). Radio buttons typically present the user with a "one
    of many" choice. In a group of radio buttons only one radio button at
    a time can be checked; if the user selects another button, the
    previously selected button is switched off.

    Radio buttons are autoExclusive by default. If auto-exclusive is
    enabled, radio buttons that belong to the same parent widget
    behave as if they were part of the same exclusive button group. If
    you need multiple exclusive button groups for radio buttons that
    belong to the same parent widget, put them into a QButtonGroup.

    Whenever a button is switched on or off it emits the toggled() signal.
    Connect to this signal if you want to trigger an action each time the
    button changes state. Otherwise, use isChecked() to see if a particular
    button is selected.

    Just like QPushButton, a radio button displays text, and
    optionally a small icon. The text can be set in the constructor or
    with setText(); the icon is set with setIcon().

    Important inherited members: text(), setText(), text(),
    setDown(), isDown(), autoRepeat(), group(), setAutoRepeat(),
    toggle(), pressed(), released(), clicked(), and toggled().

    \inlineimage macintosh-radiobutton.png Screenshot in Macintosh style
    \inlineimage windows-radiobutton.png Screenshot in Windows style

    \sa QPushButton, QToolButton, QCheckBox, {fowler}{GUI Design Handbook: Radio Button}
*/


/*
    Initializes the radio button.
*/
static void qRadioButtonInit(QRadioButton *button)
{
    button->setCheckable(true);
    button->setAutoExclusive(true);
    button->setMouseTracking(true);
}


/*!
    Constructs a radio button with the given \a parent, but with no text or
    pixmap.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QRadioButton::QRadioButton(QWidget *parent)
    : QAbstractButton(*new QRadioButtonPrivate, parent)
{
    qRadioButtonInit(this);
}

/*!
    Constructs a radio button with the given \a parent and a \a text string.

    The \a parent argument is passed on to the QAbstractButton constructor.
*/

QRadioButton::QRadioButton(const QString &text, QWidget *parent)
    : QAbstractButton(*new QRadioButtonPrivate, parent)
{
    qRadioButtonInit(this);
    setText(text);
}

static QStyleOptionButton getStyleOption(const QRadioButton *btn, bool hitButton = false)
{
    QStyleOptionButton opt;
    opt.init(btn);
    opt.text = btn->text();
    opt.icon = btn->icon();
    opt.iconSize = btn->iconSize();
    if (btn->isDown())
        opt.state |= QStyle::State_Sunken;
    opt.state |= (btn->isChecked() ? QStyle::State_On : QStyle::State_Off);
    if (btn->testAttribute(Qt::WA_Hover) && btn->underMouse()) {
        if (hitButton)
            opt.state |= QStyle::State_MouseOver;
        else
            opt.state &= ~QStyle::State_MouseOver;
    }
    return opt;
}

/*!
    \reimp
*/
QSize QRadioButton::sizeHint() const
{
    ensurePolished();
    QStyleOptionButton opt = getStyleOption(this);
    QSize sz = style()->itemTextRect(fontMetrics(), QRect(0, 0, 1, 1), Qt::TextShowMnemonic,
                                     false, text()).size();
    if (!opt.icon.isNull())
        sz = QSize(sz.width() + opt.iconSize.width() + 4, qMax(sz.height(), opt.iconSize.height()));
    return (style()->sizeFromContents(QStyle::CT_RadioButton, &opt, sz, this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    \reimp
*/
bool QRadioButton::hitButton(const QPoint &pos) const
{
    QStyleOptionButton opt = getStyleOption(this);
    return style()->subElementRect(QStyle::SE_RadioButtonClickRect, &opt, this).contains(pos);
}

/*!
    \reimp
*/
void QRadioButton::mouseMoveEvent(QMouseEvent *e)
{
    Q_D(QRadioButton);
    if (testAttribute(Qt::WA_Hover)) {
        bool hit = false;
        if (underMouse())
            hit = hitButton(e->pos());

        if (hit != d->hovering) {
            update();
            d->hovering = hit;
        }
    }

    QAbstractButton::mouseMoveEvent(e);
}

/*!\reimp
 */
void QRadioButton::paintEvent(QPaintEvent *)
{
    Q_D(QRadioButton);
    QStylePainter p(this);
    QStyleOptionButton opt = getStyleOption(this, d->hovering);
    p.drawControl(QStyle::CE_RadioButton, opt);
}

/*! \reimp */
bool QRadioButton::event(QEvent *e)
{
    return QAbstractButton::event(e);
}

#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QRadioButton::QRadioButton(QWidget *parent, const char* name)
    : QAbstractButton(*new QRadioButtonPrivate, parent)
{
    setObjectName(QString::fromAscii(name));
    qRadioButtonInit(this);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QRadioButton::QRadioButton(const QString &text, QWidget *parent, const char* name)
    : QAbstractButton(*new QRadioButtonPrivate, parent)
{
    setObjectName(QString::fromAscii(name));
    qRadioButtonInit(this);
    setText(text);
}

#endif
