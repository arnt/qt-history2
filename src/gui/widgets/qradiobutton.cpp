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
#include "qstylepainter.h"
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

    \sa QPushButton QToolButton QCheckBox
    \link guibooks.html#fowler GUI Design Handbook: Radio Button\endlink
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
    QStyleOptionButton opt;
    opt.init(btn);
    opt.text = btn->text();
    opt.icon = btn->icon();
    if (btn->isDown())
        opt.state |= QStyle::State_Sunken;
    opt.state |= (btn->isChecked() ? QStyle::State_On : QStyle::State_Off);
    return opt;
}

/*!
    \reimp
*/
QSize QRadioButton::sizeHint() const
{
    ensurePolished();
    QSize sz = style()->itemTextRect(fontMetrics(), QRect(0, 0, 1, 1), Qt::TextShowMnemonic,
                                     false, text()).size();
    QStyleOptionButton opt = getStyleOption(this);
    return (style()->sizeFromContents(QStyle::CT_RadioButton, &opt, sz, this).
            expandedTo(QApplication::globalStrut()));
}

/*!
    \reimp
*/
bool QRadioButton::hitButton(const QPoint &pos) const
{
    QStyleOptionButton opt = getStyleOption(this);
    QRect r = QStyle::visualRect(opt.direction, opt.rect, style()->subElementRect(QStyle::SE_RadioButtonClickRect, &opt, this));
    if (isRightToLeft()) {
        r.setRight(width());
    } else {
        r.setLeft(0);
    }
    return r.contains(pos);
}

/*!\reimp
 */
void QRadioButton::paintEvent(QPaintEvent *)
{
    QStylePainter p(this);
    QStyleOptionButton opt = getStyleOption(this);
    p.drawControl(QStyle::CE_RadioButton, opt);
}

#ifdef QT3_SUPPORT
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
