#include "assistantapplication.h"
#include <qevent.h>

AssistantApplication::AssistantApplication(int argc, char **argv, bool GUIEnabled)
    : QApplication(argc, argv, GUIEnabled), shiftKeyPressed(FALSE)
{
}

bool AssistantApplication::notify(QObject *reciever, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
	QKeyEvent *ke = (QKeyEvent *)event;
	if (ke->key() == Qt::Key_Shift)
	    shiftKeyPressed = (event->type() == QEvent::KeyPress);
    }
    return QApplication::notify(reciever, event);
}

bool AssistantApplication::isShiftKeyPressed()
{
    return shiftKeyPressed;
}
