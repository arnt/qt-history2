#include "qdragobject.h"

#ifdef QT_FEATURE_DRAGANDDROP

#include <stdio.h>

bool QDragManager::eventFilter(QObject * o,QEvent * e)
{
    return false;
}

void QDragManager::updateMode(ButtonState newstate)
{
}

void QDragManager::updateCursor()
{
}

void QDragManager::cancel(bool)
{
}

bool QDragManager::drag(QDragObject * o,QDragObject::DragMode mode)
{
    return false;
}

void QDragManager::updatePixmap()
{
}

void QDragManager::timerEvent ( QTimerEvent * e )
{
}

#endif // QT_FEATURE_DRAGANDDROP
