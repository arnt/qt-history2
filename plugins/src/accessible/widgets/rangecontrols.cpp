#include "rangecontrols.h"

#include <qslider.h>
#include <qdial.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qprogressbar.h>
#include <qrangecontrol.h>
#include <qstyle.h>

/*!
  \class QAccessibleRangeControl qaccessiblewidget.h
  \brief The QAccessibleRangeControl class implements the QAccessibleInterface for range controls.
*/

/*!
  Constructs a QAccessibleRangeControl object for \a o.
  \a role, \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleWidget constructor.
*/
QAccessibleRangeControl::QAccessibleRangeControl(QWidget *w, Role role, QString name,
						 QString description, QString help, QString defAction, QString accelerator)
: QAccessibleWidget(w, role, name, description, QString(), help, SetFocus, defAction, accelerator)
{
    addControllingSignal("valueChanged(int)");
}

/*! \reimp */
QString QAccessibleRangeControl::text(Text t, int child) const
{
    QString tx = QAccessibleWidget::text(t, child);
    if (!!tx)
	return stripAmp(tx);

    switch (t) {
    case Name:
	return stripAmp(buddyString(widget()));
    case Accelerator:
	tx = hotKey(buddyString(widget()));
	if (!!tx)
	    return "Alt + "+tx;
	break;
    case Value:
	{
	    int value = 0;
	    if (qt_cast<QSlider*>(object()))
		value = qt_cast<QSlider*>(object())->value();
	    else if (qt_cast<QDial*>(object()))
		value = qt_cast<QDial*>(object())->value();
	    else if (qt_cast<QScrollBar*>(object()))
		value = qt_cast<QScrollBar*>(object())->value();
	    else if (qt_cast<QSpinBox*>(object()))
		return qt_cast<QSpinBox*>(object())->text();
	    else if (qt_cast<QProgressBar*>(object()))
		value = qt_cast<QProgressBar*>(object())->progress();
	    return QString::number(value);
	}
    default:
	break;
    }
    return tx;
}

/*!
  \class QAccessibleSpinWidget qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for up/down widgets.
*/

/*!
  Constructs a QAccessibleSpinWidget object for \a o.
*/
QAccessibleSpinWidget::QAccessibleSpinWidget(QWidget *o)
: QAccessibleRangeControl(o, SpinBox)
{
}

/*! \reimp */
QRect QAccessibleSpinWidget::rect(int child) const
{
    QRect rect;
    switch(child) {
    case 1:
	rect = ((QSpinWidget*)widget())->upRect();
	break;
    case 2:
	rect = ((QSpinWidget*)widget())->downRect();
	break;
    default:
	rect = widget()->rect();
    }
    QPoint tl = widget()->mapToGlobal(QPoint(0, 0));
    return QRect(tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleSpinWidget::navigate(Relation rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    if (entry) switch (rel) {
    case QAccessible::Up:
	return entry < childCount() ? entry + 1 : -1;
    case QAccessible::Down:
	return entry > 1 ? entry - 1 : -1;
    case QAccessible::Left:
	return -1;
    case QAccessible::Right:
	return -1;
    default:
	break;
    }
    return QAccessibleRangeControl::navigate(rel, entry, target);
}

/*! \reimp */
int QAccessibleSpinWidget::childCount() const
{
    return 2;
}

/*! \reimp */
QString QAccessibleSpinWidget::text(Text t, int child) const
{
    switch (t) {
    case Name:
	switch (child) {
	case 1:
	    return QSpinWidget::tr("More");
	case 2:
	    return QSpinWidget::tr("Less");
	default:
	    break;
	}
	break;
/*
    case DefaultAction:
	switch(child) {
	case 1:
	case 2:
	    return QSpinWidget::tr("Press");
	default:
	    break;
	}
	break;
*/
    default:
	break;
    }
    return QAccessibleRangeControl::text(t, child);
}

/*! \reimp */
QAccessible::Role QAccessibleSpinWidget::role(int child) const
{
    switch(child) {
    case 1:
	return PushButton;
    case 2:
	return PushButton;
    default:
	break;
    }
    return QAccessibleRangeControl::role(child);
}

/*! \reimp */
QAccessible::State QAccessibleSpinWidget::state(int child) const
{
    int state = QAccessibleRangeControl::state(child);
    switch(child) {
    case 1:
	if (!((QSpinWidget*)widget())->isUpEnabled())
	    state |= Unavailable;
	return (State)state;
    case 2:
	if (!((QSpinWidget*)widget())->isDownEnabled())
	    state |= Unavailable;
	return (State)state;
    default:
	break;
    }
    return QAccessibleRangeControl::state(child);
}

/*! \reimp */
bool QAccessibleSpinWidget::doAction(int action, int child)
{
    if (action == Press) switch(child) {
    case 1:
	if (!((QSpinWidget*)widget())->isUpEnabled())
	    return FALSE;
	((QSpinWidget*)widget())->stepUp();
	return TRUE;
    case 2:
	if (!((QSpinWidget*)widget())->isDownEnabled())
	    return FALSE;
	((QSpinWidget*)widget())->stepDown();
	return TRUE;
    default:
	break;
    }
    return QAccessibleRangeControl::doAction(action, child);
}

/*!
  \class QAccessibleScrollBar qaccessiblewidget.h
  \brief The QAccessibleScrollBar class implements the QAccessibleInterface for scroll bars.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a w.
  \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleRangeControl constructor.
*/
QAccessibleScrollBar::QAccessibleScrollBar(QWidget *w, QString name,
    QString description, QString help, QString defAction, QString accelerator)
: QAccessibleRangeControl(w, ScrollBar, name, description, help, defAction, accelerator)
{
    Q_ASSERT(scrollBar());
}

/*! Returns the scroll bar. */
QScrollBar *QAccessibleScrollBar::scrollBar() const
{
    return qt_cast<QScrollBar*>(object());
}

/*! \reimp */
QRect QAccessibleScrollBar::rect(int child) const
{
    QRect rect;
    QRect srect = scrollBar()->sliderRect();
    int sz = scrollBar()->style().pixelMetric(QStyle::PM_ScrollBarExtent, scrollBar());
    switch (child) {
    case 1:
	if (scrollBar()->orientation() == Vertical)
	    rect = QRect(0, 0, sz, sz);
	else
	    rect = QRect(0, 0, sz, sz);
	break;
    case 2:
	if (scrollBar()->orientation() == Vertical)
	    rect = QRect(0, sz, sz, srect.y() - sz);
	else
	    rect = QRect(sz, 0, srect.x() - sz, sz);
	break;
    case 3:
	rect = srect;
	break;
    case 4:
	if (scrollBar()->orientation() == Vertical)
	    rect = QRect(0, srect.bottom(), sz, scrollBar()->rect().height() - srect.bottom() - sz);
	else
	    rect = QRect(srect.right(), 0, scrollBar()->rect().width() - srect.right() - sz, sz) ;
	break;
    case 5:
	if (scrollBar()->orientation() == Vertical)
	    rect = QRect(0, scrollBar()->rect().height() - sz, sz, sz);
	else
	    rect = QRect(scrollBar()->rect().width() - sz, 0, sz, sz);
	break;
    default:
	return QAccessibleRangeControl::rect(child);
    }

    QPoint tp = scrollBar()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleScrollBar::childCount() const
{
    return 5;
}

/*! \reimp */
QString	QAccessibleScrollBar::text(Text t, int child) const
{
    switch (t) {
    case Value:
	if (child && child != 3)
	    return QString();
	break;
    case Name:
	switch (child) {
	case 1:
	    return QScrollBar::tr("Line up");
	case 2:
	    return QScrollBar::tr("Page up");
	case 3:
	    return QScrollBar::tr("Position");
	case 4:
	    return QScrollBar::tr("Page down");
	case 5:
	    return QScrollBar::tr("Line down");
	}
	break;
/*
    case DefaultAction:
	if (child != 3)
	    return QScrollBar::tr("Press");
	break;
*/
    default:
	break;

    }
    return QAccessibleRangeControl::text(t, child);
}

/*! \reimp */
QAccessible::Role QAccessibleScrollBar::role(int child) const
{
    switch (child) {
    case 1:
    case 2:
    case 4:
    case 5:
	return PushButton;
    case 3:
	return Indicator;
    default:
	return ScrollBar;
    }
}

/*! \reimp */
bool QAccessibleScrollBar::doAction(int action, int child)
{
    if (action == Press) switch (child) {
    case 1:
	scrollBar()->subtractLine();
	return TRUE;
    case 2:
	scrollBar()->subtractPage();
	return TRUE;
    case 4:
	scrollBar()->addPage();
	return TRUE;
    case 5:
	scrollBar()->addLine();
	return TRUE;
    }
    return FALSE;
}

/*!
  \class QAccessibleSlider qaccessiblewidget.h
  \brief The QAccessibleScrollBar class implements the QAccessibleInterface for sliders.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a w.
  \a name, \a description, \a help, \a defAction and \a accelerator
  are propagated to the QAccessibleRangeControl constructor.
*/
QAccessibleSlider::QAccessibleSlider(QWidget *w, QString name,
    QString description, QString help, QString defAction, QString accelerator)
: QAccessibleRangeControl(w, ScrollBar, name, description, help, defAction, accelerator)
{
    Q_ASSERT(slider());
}

/*! Returns the slider. */
QSlider *QAccessibleSlider::slider() const
{
    return qt_cast<QSlider*>(object());
}

/*! \reimp */
QRect QAccessibleSlider::rect(int child) const
{
    QRect rect;
    QRect srect = slider()->sliderRect();
    switch (child) {
    case 1:
	if (slider()->orientation() == Vertical)
	    rect = QRect(0, 0, slider()->width(), srect.y());
	else
	    rect = QRect(0, 0, srect.x(), slider()->height());
	break;
    case 2:
	rect = srect;
	break;
    case 3:
	if (slider()->orientation() == Vertical)
	    rect = QRect(0, srect.y() + srect.height(), slider()->width(), slider()->height()- srect.y() - srect.height());
	else
	    rect = QRect(srect.x() + srect.width(), 0, slider()->width() - srect.x() - srect.width(), slider()->height());
	break;
    default:
	return QAccessibleRangeControl::rect(child);
    }

    QPoint tp = slider()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleSlider::relationTo(int child, const QAccessibleInterface *other, int otherChild)
{
    return QAccessibleRangeControl::relationTo(child, other, otherChild);
}

/*! \reimp */
int QAccessibleSlider::childCount() const
{
    return 3;
}

/*! \reimp */
QString	QAccessibleSlider::text(Text t, int child) const
{
    switch (t) {
    case Value:
	if (child && child != 2)
	    return QString();
	break;
    case Name:
	switch (child) {
	case 1:
	    return QSlider::tr("Page down");
	case 2:
	    return QSlider::tr("Position");
	case 3:
	    return QSlider::tr("Page up");
	}
	break;
/*
    case DefaultAction:
	if (child != 2)
	    return QSlider::tr("Press");
	break;
*/
    default:
	break;
    }
    return QAccessibleRangeControl::text(t, child);
}

/*! \reimp */
QAccessible::Role QAccessibleSlider::role(int child) const
{
    switch (child) {
    case 1:
    case 3:
	return PushButton;
    case 2:
	return Indicator;
    default:
	return Slider;
    }
}

/*! \reimp */
bool QAccessibleSlider::doAction(int action, int child)
{
    if (action == Press) switch (child) {
    case 1:
	slider()->subtractLine();
	return TRUE;
    case 3:
	slider()->addLine();
	return TRUE;
    }
    return FALSE;
}
