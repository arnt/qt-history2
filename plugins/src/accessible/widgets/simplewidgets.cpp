#include "simplewidgets.h"

#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qlineedit.h>

/*!
  \class QAccessibleButton qaccessible.h
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleButton object for \a w.
  \a role, \a description and \a help are propagated to the QAccessibleWidget constructor.
*/
QAccessibleButton::QAccessibleButton(QWidget *w, Role role, QString description,
				     QString help)
: QAccessibleWidget(w, role, QString(), description)
{
    Q_ASSERT(button());
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
    QString tx = QAccessibleWidget::text(t, child);
    if (!!tx)
	return tx;

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
	tx = hotKey(button()->text());
	if (!!tx) {
	    tx = "Alt + "+tx;
	} else {
	    tx = hotKey(buddyString(widget()));
	    if (!!tx)
		tx = "Alt + "+tx;
	}
	return tx;
    case Name:
	tx = button()->text();
	if (tx.isEmpty() && qt_cast<QToolButton*>(object()))
	    tx = static_cast<QToolButton*>(object())->textLabel();
	if (tx.isEmpty())
	    tx = buddyString(widget());

	return stripAmp(tx);
    default:
	break;
    }
    return tx;
}

/*! \reimp */
QAccessible::State QAccessibleButton::state(int child) const
{
    int state = QAccessibleWidget::state(child);

    QButton *b = button();
    if (b->state() == QButton::On)
	state |= Checked;
    else  if (b->state() == QButton::NoChange)
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
  Constructs a QAccessibleDisplay object for \a o.
  \a role, \a description, \a value, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleDisplay::QAccessibleDisplay(QWidget *o, Role role, QString description, QString value, QString help, QString defAction, QString accelerator)
: QAccessibleWidget(o, role, QString(), description, value, help, NoAction, defAction, accelerator)
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
    QString str = QAccessibleWidget::text(t, child);
    if (!!str)
	return str;

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
    return stripAmp(str);
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
  \class QAccessibleText qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for widgets with editable text.
*/

/*!
  Constructs a QAccessibleText object for \a o.
  \a role, \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleText::QAccessibleText(QWidget *o, Role role, QString name, QString description, QString help, QString defAction, QString accelerator)
: QAccessibleWidget(o, role, name, description, QString(), help, SetFocus, defAction, accelerator)
{
}

/*! \reimp */
QString QAccessibleText::text(Text t, int child) const
{
    QString str = QAccessibleWidget::text(t, child);
    if (!!str)
	return str;
    switch (t) {
    case Name:
	return stripAmp(buddyString(widget()));
    case Accelerator:
	str = hotKey(buddyString(widget()));
	if (!!str)
	    return "Alt + "+str;
	break;
    case Value:
	if (widget()->inherits("QLineEdit"))
	    return ((QLineEdit*)widget())->text();
	break;
    default:
	break;
    }
    return str;
}

/*! \reimp */
QAccessible::State QAccessibleText::state(int child) const
{
    int state = QAccessibleWidget::state(child);

    QLineEdit *l = qt_cast<QLineEdit*>(object());
    if (l) {
	if (l->isReadOnly())
	    state |= ReadOnly;
	if (l->echoMode() == QLineEdit::Password)
	    state |= Protected;
	state |= Selectable;
	if (l->hasSelectedText())
	    state |= Selected;
    }

    return (State)state;
}
