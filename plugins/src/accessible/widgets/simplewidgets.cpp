#include "simplewidgets.h"

#include <qaccel.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qprogressbar.h>
#include <qradiobutton.h>
#include <qtoolbutton.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qstyle.h>

QString Q_EXPORT qacc_stripAmp(const QString &text);
QString Q_EXPORT qacc_hotKey(const QString &text);

/*!
  \class QAccessibleButton qaccessible.h
  \brief The QAccessibleButton class implements the QAccessibleInterface for button type widgets.
*/

/*!
  Creates a QAccessibleButton object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
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
int QAccessibleButton::defaultAction(int child) const
{
    return Press;
}

/*! \reimp */
QString QAccessibleButton::actionText(int action, Text text, int child) const
{
    if (action == defaultAction(child) && text == Name) {
	switch(role(child)) {
	case ButtonMenu:
	    return QPushButton::tr("Open");
	case CheckBox:
	    {
		if (state(child) & Checked)
		    return QCheckBox::tr("Uncheck");
		QCheckBox *cb = qt_cast<QCheckBox*>(object());
		if (!cb || !cb->isTristate() || button()->state() == QButton::NoChange)
		    return QCheckBox::tr("Check");
		return QCheckBox::tr("Toggle");
	    }
	    break;
	case RadioButton:
	    return QRadioButton::tr("Check");
	default:
	    break;
	}
    }

    return QAccessibleWidget::actionText(action, text, child);
}

/*! \reimp */
bool QAccessibleButton::doAction(int action, int child)
{
    if (child || !widget()->isEnabled())
	return FALSE;

    Role r = role(child);
    QPushButton *pb = qt_cast<QPushButton*>(object());
    if (pb && pb->popup())
	pb->openPopup();
    else
	button()->animateClick();

    return TRUE;
}

/*! \reimp */
QString QAccessibleButton::text(Text t, int child) const
{
    QString str;

    switch (t) {
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
	break;
    default:
	break;
    }
    if (str.isEmpty())
	str = QAccessibleWidget::text(t, child);;
    return qacc_stripAmp(str);
}

/*! \reimp */
int QAccessibleButton::state(int child) const
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
    if (pb) {
	if (pb->isDefault())
	    state |= DefaultButton;
	if (pb->popup())
	    state |= HasPopup;
    }

    return state;
}


/*!
  \class QAccessibleToolButton qaccessible.h
  \brief The QAccessibleToolButton class implements the QAccessibleInterface for tool buttons.
*/

/*!
  Creates a QAccessibleToolButton object for \a w.
  \a role is propagated to the QAccessibleWidget constructor.
*/
QAccessibleToolButton::QAccessibleToolButton(QWidget *w, Role role)
: QAccessibleButton(w, role)
{
    Q_ASSERT(toolButton());
}

/*! Returns the button. */
QToolButton *QAccessibleToolButton::toolButton() const
{
    return qt_cast<QToolButton*>(object());
}

/*! 
    Returns TRUE if this tool button is a split button.
*/
bool QAccessibleToolButton::isSplitButton() const
{
    return toolButton()->popup() && !toolButton()->popupDelay();
}

/*! \reimp */
QAccessible::Role QAccessibleToolButton::role(int child) const
{
    if (isSplitButton()) switch(child) {
    case 1:
	return PushButton;
    case 2:
	return ButtonMenu;
    }
    return QAccessibleButton::role(child);
}

/*! \reimp */
int QAccessibleToolButton::state(int child) const
{
    int st = QAccessibleButton::state(child);
    if (toolButton()->autoRaise())
	st |= HotTracked;
    return st;
}

/*! \reimp */
int QAccessibleToolButton::childCount() const
{
    return isSplitButton() ? 2 : 0;
}

QRect QAccessibleToolButton::rect(int child) const
{
    if (!child)
	return QAccessibleButton::rect(child);

    QRect subrect = QStyle::visualRect( widget()->style().querySubControlMetrics(QStyle::CC_ToolButton, 
	    toolButton(), QStyle::SC_ToolButtonMenu), toolButton() );

    if (child == 1)
	subrect = QRect(0, 0, subrect.x(), widget()->height());

    QPoint ntl = widget()->mapToGlobal(subrect.topLeft());
    subrect.moveTopLeft(ntl);
    return subrect;
}

QString QAccessibleToolButton::text(Text t, int child) const
{
    QString str;

    switch (t) {
    case Name:
	str = toolButton()->text();
	if (str.isEmpty())
	    str = toolButton()->textLabel();
	break;
    default:
	break;
    }
    if (str.isEmpty())
	str = QAccessibleButton::text(t, child);;
    return qacc_stripAmp(str);
}

int QAccessibleToolButton::actionCount(int child) const
{
    if (!child)
	return toolButton()->popup() ? 1 : 0;
    return 0;
}

int QAccessibleToolButton::defaultAction(int child) const
{
    if (!child && role(child) == ButtonMenu)
	return 1;
    return QAccessibleButton::defaultAction(child);
}

QString QAccessibleToolButton::actionText(int action, Text text, int child) const
{
    if (text == Name) switch(role(child)) {
    case ButtonMenu:
	if (child == 2 || action != Press)
	    return QToolButton::tr("Open");
	break;
    case ButtonDropDown:
	switch(child) {
	case 0:
	    if (action != Press)
		return QToolButton::tr("Open");
	    break;
	case 2:
	    return QToolButton::tr("Open");
	    break;
	}
	break;
    }
    return QAccessibleButton::actionText(action, text, child);
}

bool QAccessibleToolButton::doAction(int action, int child)
{
    if (action == 1 || child == 2) {
	if(!child)
	    toolButton()->setDown(TRUE);
	toolButton()->openPopup();
	return TRUE;
    }
    return QAccessibleButton::doAction(action, 0);
}


/*!
  \class QAccessibleDisplay qaccessiblewidget.h
  \brief The QAccessibleDisplay class implements the QAccessibleInterface for widgets that display information.
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
    } else if (qt_cast<QProgressBar*>(object())) {
	return ProgressBar;
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
    case Value:
	if (qt_cast<QProgressBar*>(object()))
	    str = QString::number(qt_cast<QProgressBar*>(object())->progress());
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
    } else if (groupbox && !groupbox->title().isEmpty()) {
	if (groupbox->children().contains(o))
	    relation |= Label;
    }
    return relation;
}

/*! \reimp */
int QAccessibleDisplay::navigate(Relation rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (rel == Labelled) {
	QObject *targetObject = 0;
        QLabel *label = qt_cast<QLabel*>(object());
	QGroupBox *groupbox = qt_cast<QGroupBox*>(object());
	if (label) {
	    if (entry == 1)
		targetObject = label->buddy();
	} else if (groupbox && !groupbox->title().isEmpty()) {
	    rel = Child;
	}
	QAccessible::queryAccessibleInterface(targetObject, target);
	if (*target)
	    return 0;
    }
    return QAccessibleWidget::navigate(rel, entry, target);
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
int QAccessibleLineEdit::state(int child) const
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

    return state;
}
