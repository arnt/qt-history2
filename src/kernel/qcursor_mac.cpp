#include "qcursor.h"
#include <stdio.h>

static const int cursors = 14;
static QCursor cursorTable[cursors];

QT_STATIC_CONST_IMPL QCursor & Qt::arrowCursor = cursorTable[0];
QT_STATIC_CONST_IMPL QCursor & Qt::upArrowCursor = cursorTable[1];
QT_STATIC_CONST_IMPL QCursor & Qt::crossCursor = cursorTable[2];
QT_STATIC_CONST_IMPL QCursor & Qt::waitCursor = cursorTable[3];
QT_STATIC_CONST_IMPL QCursor & Qt::ibeamCursor = cursorTable[4];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeVerCursor = cursorTable[5];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeHorCursor = cursorTable[6];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeBDiagCursor = cursorTable[7];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeFDiagCursor = cursorTable[8];
QT_STATIC_CONST_IMPL QCursor & Qt::sizeAllCursor = cursorTable[9];
QT_STATIC_CONST_IMPL QCursor & Qt::blankCursor = cursorTable[10];
QT_STATIC_CONST_IMPL QCursor & Qt::splitHCursor = cursorTable[11];
QT_STATIC_CONST_IMPL QCursor & Qt::splitVCursor = cursorTable[12];
QT_STATIC_CONST_IMPL QCursor & Qt::pointingHandCursor = cursorTable[13];

QCursor::QCursor()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QCursor::QCursor(int shape)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QCursor::QCursor(const QCursor &c)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QCursor::~QCursor()
{
  printf("%s %d\n",__FILE__,__LINE__);
}

void QCursor::setBitmap(const QBitmap &bitmap,const QBitmap &mask,
                        int hotX,int hotY)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QCursor &QCursor::operator=(const QCursor &c)
{
  printf("%s %d\n",__FILE__,__LINE__);
  return QCursor();
}

int QCursor::shape() const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

void QCursor::setShape(int shape)
{
  printf("%s %d\n",__FILE__,__LINE__);
}

QPoint QCursor::hotSpot() const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return QPoint();
}

const QBitmap * QCursor::mask() const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

const QBitmap * QCursor::bitmap() const
{
  printf("%s %d\n",__FILE__,__LINE__);
  return 0;
}

QPoint QCursor::pos()
{
  printf("%s %d\n",__FILE__,__LINE__);
  return QPoint();
}

void * QCursor::handle() const
{
  return 0;
}
