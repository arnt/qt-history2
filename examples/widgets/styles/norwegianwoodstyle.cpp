#include <QtGui>

#include "norwegianwoodstyle.h"

NorwegianWoodStyle::NorwegianWoodStyle()
{
}

void NorwegianWoodStyle::polish(QPalette &palette)
{
    QWindowsStyle::polish(palette);
}

void NorwegianWoodStyle::polish(QWidget *widget)
{
    QWindowsStyle::polish(widget);
}

void NorwegianWoodStyle::unpolish(QWidget *widget)
{
    QWindowsStyle::unpolish(widget);
}
