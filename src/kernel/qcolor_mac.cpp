#include "qcolor.h"
#include "qt_mac.h"
#include <stdio.h>

int QColor::enterAllocContext()
{
    return 0;
}

void QColor::leaveAllocContext()
{
}

void QColor::destroyAllocContext(int context)
{
}

uint QColor::alloc()
{
    uint ret;
    RGBColor mycolour;
    mycolour.red=qRed(rgbVal)*256;
    mycolour.green=qGreen(rgbVal)*256;
    mycolour.blue=qBlue(rgbVal)*256;
    ret=Color2Index(&mycolour);
    return ret;
}

void QColor::setSystemNamedColor(const QString& name)
{
    setRgb(256,0,0);
}


