#include "simplewidgets.h"

#include <qaccel.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qlineedit.h>

QString Q_EXPORT qacc_stripAmp(const QString &text);
QString Q_EXPORT qacc_hotKey(const QString &text);

/*!
  \class QAccessibleButton qaccessible.h
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleButton object for \a w.
  \a role, \a description and \a help are propagated to the QAccessibleWidget constructor.
*/
QAccessibleButton::QAccessibleButton(QWidget *w, Role role)
: QAccessibleWidget(w, role)
{
    Q_ASSERT(button());
    if (button()->isToggleButton())
	addControllingSignal("toggled(bool)");
    else
	addControllingSignal("clicked()");
}

/*! Returns the button. */
QButton *QAccessibleButton::button() const
{
    return qt_cast<QButton*>(object());
}

/*! \reimp */
bool QAccessibleButton::doAction(int action, int child)
{
    if (!widget()->isEnabled())
	return FALSE;

    Role r = role(child);
    QToolButton *tb = qt_cast<QToolButton*>(object());
    if (tb && tb->popup())
	tb->openPopup();
    else
	button()->animateClick();

    return TRUE;
}

/*! \reimp */
QString QAccessibleButton::text(Text t, int child) const
{
    QString str;

    switch (t) {
/*
    case DefaultAction:
	switch(role(child)) {
	case PushButton:
	    return QButton::tr("Press");
	case CheckBox:
	    if (state(child) & Checked)
		return QButton::tr("UnCheck");
	    return QButton::tr("Check");
	case RadioButton:
	    return QButton::tr("Check");
	default:
	    return QButton::tr("Press");
	}
*/
    case Accelerator:
	{
	    QPushButton *pb = qt_cast<QPushButton*>(object());
	    if (pb && pb->isDefault())
		str = QAccel::keyToString(Key_Enter);
	    if (str.isEmpty())
		str = qacc_hotKey(button()->text());
	}
	break;
    case Name:
	str = button()->text();
	if (str.isEmpty() && qt_cast<QToolButton*>(object()))
	    str = static_cast<QToolButton*>(object())->textLabel();
	break;
    default:
	break;
    }
    if (str.isEmpty())
	str = QAccessibleWidget::text(t, child);;
    return qacc_stripAmp(str);
}

/*! \reimp */
QAccessible::State QAccessibleButton::state(int child) const
{
    int state = QAccessibleWidget::state(child);

    QButton *b = button();
    if (b->state() == QButton::On)
	state |= Checked;
    else if (b->state() == QButton::NoChange)
	    state |= Mixed;
    if (b->isDown())
	state |= Pressed;
    QPushButton *pb = qt_cast<QPushButton*>(b);
    if (pb && pb->isDefault())
	state |= DefaultButton;

    return (State)state;
}



/*!
  \class QAccessibleDisplay qaccessiblewidget.h
  \brief The QAccessibleDisplay class implements the QAccessibleInterface for widgets that display static information.
*/

/*!
  Constructs a QAccessibleDisplay object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleDisplay::QAccessibleDisplay(QWidget *w, Role role)
: QAccessibleWidget(w, role)
{
}

/*! \reimp */
QAccessible::Role QAccessibleDisplay::role(int child) const
{
    QLabel *l = qt_cast<QLabel*>(object());
    if (l) {
	if (l->pixmap() || l->picture())
	    return Graphic;
#ifndef QT_NO_PICTURE
	if (l->picture())
	    return Graphic;
#endif
#ifndef QT_NO_MOVIE
	if (l->movie())
	    return Animation;
#endif
    }
    return QAccessibleWidget::role(child);
}

/*! \reimp */
QString QAccessibleDisplay::text(Text t, int child) const
{
    QString str;
    switch (t) {
    case Name:
	if (qt_cast<QLabel*>(object())) {
	    str = qt_cast<QLabel*>(object())->text();
	} else if (qt_cast<QGroupBox*>(object())) {
	    str = qt_cast<QGroupBox*>(object())->title();
	} else if (qt_cast<QLCDNumber*>(object())) {
	    QLCDNumber *l = qt_cast<QLCDNumber*>(object());
	    if (l->numDigits())
		str = QString::number(l->value());
	    else
		str = QString::number(l->intValue());
	}
	break;
    default:
	break;
    }
    if (str.isEmpty())
	str = QAccessibleWidget::text(t, child);;
    return qacc_stripAmp(str);
}

/*! \reimp */
int QAccessibleDisplay::relationTo(int child, const QAccessibleInterface *other, int otherChild) const
{
    int relation = QAccessibleWidget::relationTo(child, other, otherChild);
    if (child || otherChild)
	return relation;

    QObject *o = other->object();
    QLabel *label = qt_cast<QLabel*>(object());
    QGroupBox *groupbox = qt_cast<QGroupBox*>(object());
    if (label) {
	if (o == label->buddy())
	    relation |= Label;
    } else if (groupbox) {
	if (groupbox->children().contains(o))
	    relation |= Label;
    }
    return relation;
}

/*!
  \class QAccessibleLineEdit qaccessiblewidget.h
  \brief The QAccessibleLineEdit class implements the QAccessibleInterface for widgets with editable text.
*/

/*!
  Constructs a QAccessibleLineEdit object for \a w.
  \a name is propagated to the QAccessibleWidget constructor.
*/
QAccessibleLineEdit::QAccessibleLineEdit(QWidget *w, const QString &name)
: QAccessibleWidget(w, EditableText, name)
{
    addControllingSignal("textChanged(const QString&)");
    addControllingSignal("returnPressed()");
}

/*! Returns the line edit. */
QLineEdit *QAccessibleLineEdit::lineEdit() const
{
    return qt_cast<QLineEdit*>(object());
}

/*! \reimp */
QString QAccessibleLineEdit::text(Text t, int child) const
{
    QString str;
    switch (t) {
    case Value:
	str = lineEdit()->text();
	break;
    default:
	break;
    }
    if (str.isEmpty())
	str = QAccessibleWidget::text(t, child);;
    return qacc_stripAmp(str);
}

/*! \reimp */
QAccessible::State QAccessibleLineEdit::state(int child) const
{
    int state = QAccessibleWidget::state(child);

    QLineEdit *l = lineEdit();
    if (l->isReadOnly())
	state |= ReadOnly;
    if (l->echoMode() == QLineEdit::Password)
	state |= Protected;
    state |= Selectable;
    if (l->hasSelectedText())
	state |= Selected;

    return (State)state;
}
