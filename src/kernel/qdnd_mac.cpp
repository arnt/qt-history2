#include "qdragobject.h"
#include <stdio.h>

bool QDragManager::eventFilter(QObject * o,QEvent * e)
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

void QDragManager::updateMode(ButtonState newstate)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QDragManager::updateCursor()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QDragManager::cancel()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

bool QDragManager::drag(QDragObject * o,QDragObject::DragMode mode)
{
  printf("%s %d\n",__FILE__,__LINE__);
  return false;
}

void QDragManager::updatePixmap()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QDragManager::timerEvent ( QTimerEvent * e )
{
}


