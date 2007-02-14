#include <QtGui>

#include "tabletapplication.h"

bool TabletApplication::event(QEvent *event)
{
    if (event->type() == QEvent::TabletEnterProximity ||
	event->type() == QEvent::TabletLeaveProximity) {
	myCanvas->setTabletDevice(
	    static_cast<QTabletEvent *>(event)->device());
	return true;
    }
    return QApplication::event(event);
}
