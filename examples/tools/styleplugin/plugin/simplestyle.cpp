#include <QtGui>

#include "simplestyle.h"

void SimpleStyle::polish(QPalette &palette)
{
    palette.setBrush(QPalette::Button, Qt::red);
}
