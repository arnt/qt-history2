#include "qcolor.h"
#include "qt_mac.h"
#include <stdio.h>

int QColor::enterAllocContext()
{
  printf("QColor::enterAllocContext: %s %d\n",__FILE__,__LINE__);
  return 0;
}

void QColor::leaveAllocContext()
{
  printf("QColor::leaveAllocContext: %s %d\n",__FILE__,__LINE__);
}

void QColor::destroyAllocContext(int context)
{
  printf("QColor::destroyAllocContext: %s %d\n",__FILE__,__LINE__);
}

uint QColor::alloc()
{
  printf("QColor::alloc: %s %d\n",__FILE__,__LINE__);
  uint ret;
  RGBColor mycolour;
  mycolour.red=qRed(rgbVal)*256;
  mycolour.green=qGreen(rgbVal)*256;
  mycolour.blue=qBlue(rgbVal)*256;
  ret=Color2Index(&mycolour);
  return ret;
}

void QColor::setSystemNamedColor(const char * name)
{
  printf("QColor::setSystemNamedColor: %s %d\n",__FILE__,__LINE__);
  printf("  %s\n",name);
  setRgb(256,0,0);
}


