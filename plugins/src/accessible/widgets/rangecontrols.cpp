#include "rangecontrols.h"

#include <qslider.h>
#include <qdial.h>
#include <qspinbox.h>
#include <qscrollbar.h>
#include <qslider.h>
#include <qstyle.h>

QString Q_EXPORT qacc_stripAmp(const QString &text);

/*!
  \class QAccessibleSpinBox qaccessiblewidget.h
  \brief The QAccessibleText class implements the QAccessibleInterface for spinbox widgets.
*/

/*!
  Constructs a QAccessibleSpinWidget object for \a w.
*/
QAccessibleSpinBox::QAccessibleSpinBox(QWidget *w)
: QAccessibleWidget(w, SpinBox)
{
    Q_ASSERT(spinBox());
    addControllingSignal("valueChanged(int)");
    addControllingSignal("valueChanged(QString)");
}

/*! \reimp */
QSpinBox *QAccessibleSpinBox::spinBox() const
{
    return qt_cast<QSpinBox*>(object());
}

/*! \reimp */
int QAccessibleSpinBox::childCount() const
{
    return 3;
}

/*! \reimp */
QRect QAccessibleSpinBox::rect(int child) const
{
    QRect rect;
    switch(child) {
    case 1:
	rect = widget()->rect();
	rect.setRight(spinBox()->upRect().left());
	break;
    case 2:
	rect = spinBox()->upRect();
	break;
    case 3:
	rect = spinBox()->downRect();
	break;
    default:
	rect = widget()->rect();
	break;
    }
    QPoint tl = widget()->mapToGlobal(QPoint(0, 0));
    return QRect(tl.x() + rect.x(), tl.y() + rect.y(), rect.width(), rect.height());
}

/*! \reimp */
int QAccessibleSpinBox::navigate(Relation rel, int entry, QAccessibleInterface **target) const
{
    *target = 0;
    
    if (entry) switch (rel) {
    case Child:
	return entry <= childCount() ? entry : -1;
    case QAccessible::Left:
	return (entry == 2 || entry == 3) ? 1 : -1;
    case QAccessible::Right:
	return entry == 1 ? 2 : -1;
    case QAccessible::Up:
	return entry == 3 ? 2 : -1;
    case QAccessible::Down:
	return entry == 2 ? 3 : -1;
    }
    return QAccessibleWidget::navigate(rel, entry, target);
}

/*! \reimp */
QString QAccessibleSpinBox::text(Text t, int child) const
{
    switch (t) {
    case Name:
	switch (child) {
	case 2:
	    return QSpinWidget::tr("More");
	case 3:
	    return QSpinWidget::tr("Less");
	}
	break;
    case Value:
	if (child < 2)
	    return spinBox()->text();
	break;
    }
    return QAccessibleWidget::text(t, 0);
}

/*! \reimp */
QAccessible::Role QAccessibleSpinBox::role(int child) const
{
    switch(child) {
    case 1:
	return EditableText;
    case 2:
	return PushButton;
    case 3:
	return PushButton;
    default:
	break;
    }
    return QAccessibleWidget::role(child);
}

/*! \reimp */
QAccessible::State QAccessibleSpinBox::state(int child) const
{
    int state = QAccessibleWidget::state(child);
    switch(child) {
    case 2:
	if (spinBox()->value() >= spinBox()->maxValue())
	    state |= Unavailable;
	return (State)state;
    case 3:
	if (spinBox()->value() <= spinBox()->minValue())
	    state |= Unavailable;
	return (State)state;
    default:
	break;
    }
    return (State)state;
}

/*! \reimp */
bool QAccessibleSpinBox::doAction(int action, int child)
{
    if (action == Press) switch(child) {
    case 2:
	if (spinBox()->value() >= spinBox()->maxValue())
	    return FALSE;
	spinBox()->stepUp();
	return TRUE;
    case 3:
	if (spinBox()->value() <= spinBox()->minValue())
	    return FALSE;
	spinBox()->stepDown();
	return TRUE;
    default:
	break;
    }
    return QAccessibleWidget::doAction(action, 0);
}

/*!
  \class QAccessibleScrollBar qaccessiblewidget.h
  \brief The QAccessibleScrollBar class implements the QAccessibleInterface for scroll bars.
*/

/*!
  Constructs a QAccessibleScrollBar object for \a w.
  \a name is propagated to the QAccessibleWidget constructor.
*/
QAccessibleScrollBar::QAccessibleScrollBar(QWidget *w, const QString &name)
: QAccessibleWidget(w, ScrollBar, name)
{
    Q_ASSERT(scrollBar());
    addControllingSignal("valueChanged(int)");
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
	return QAccessibleWidget::rect(child);
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
	if (!child || child == 3)
	    return QString::number(scrollBar()->value());
	return QString();
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
    default:
	break;

    }
    return QAccessibleWidget::text(t, child);
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
  \a name is propagated to the QAccessibleWidget constructor.
*/
QAccessibleSlider::QAccessibleSlider(QWidget *w, const QString &name)
: QAccessibleWidget(w, Slider, name)
{
    Q_ASSERT(slider());
    addControllingSignal("valueChanged(int)");
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
	return QAccessibleWidget::rect(child);
    }

    QPoint tp = slider()->mapToGlobal(QPoint(0,0));
    return QRect(tp.x() + rect.x(), tp.y() + rect.y(), rect.width(), rect.height());
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
	if (!child || child == 2)
	    return QString::number(slider()->value());
	return QString();
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
    default:
	break;
    }
    return QAccessibleWidget::text(t, child);
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
