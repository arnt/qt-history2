#include "q3complexwidgets.h"

#include <q3header.h>
#include <qapplication.h>

Q3AccessibleHeader::Q3AccessibleHeader(QWidget *w)
    : QAccessibleWidget(w)
{
    Q_ASSERT(header());
    addControllingSignal("clicked(int)");
}

/*! Returns the Q3Header. */
Q3Header *Q3AccessibleHeader::header() const
{
    return qt_cast<Q3Header*>(object());
}

/*! \reimp */
QRect Q3AccessibleHeader::rect(int child) const
{
    QPoint zero = header()->mapToGlobal(QPoint(0, 0));
    QRect sect = header()->sectionRect(child - 1);
    return QRect(sect.x() + zero.x(), sect.y() + zero.y(), sect.width(), sect.height());
}

/*! \reimp */
int Q3AccessibleHeader::childCount() const
{
    return header()->count();
}

/*! \reimp */
QString Q3AccessibleHeader::text(Text t, int child) const
{
    QString str;

    if (child <= childCount()) {
        switch (t) {
            case Name:
                str = header()->label(child - 1);
                break;
            case Description: {
                QAccessibleEvent event(QAccessibleEvent::Description, child);
                if (QApplication::sendEvent(widget(), &event))
                    str = event.value();
                break; }
            case Help: {
                QAccessibleEvent event(QAccessibleEvent::Help, child);
                if (QApplication::sendEvent(widget(), &event))
                    str = event.value();
                break; }
            default:
                break;
        }
    }
    if (str.isEmpty())
        str = QAccessibleWidget::text(t, child);;
    return str;
}

/*! \reimp */
QAccessible::Role Q3AccessibleHeader::role(int) const
{
    return (header()->orientation() == Qt::Horizontal) ? ColumnHeader : RowHeader;
}

/*! \reimp */
int Q3AccessibleHeader::state(int child) const
{
    int state = QAccessibleWidget::state(child);

    int section = child ? child - 1 : -1;
    if (!header()->isClickEnabled(section))
        state |= Unavailable;
    else
        state |= Selectable;
    if (child && section == header()->sortIndicatorSection())
        state |= Selected;
    if (header()->isResizeEnabled(section))
        state |= Sizeable;
    if (child && header()->isMovingEnabled())
        state |= Movable;
    return state;
}

