/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 7
**
****************************************************************/

#include <QLCDNumber>
#include <QSlider>

#include "lcdrange.h"

LCDRange::LCDRange(QWidget *parent)
    : QVBoxWidget(parent)
{
    QLCDNumber *lcd = new QLCDNumber(2, this);
    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 99);
    slider->setValue(0);
    connect(slider, SIGNAL(valueChanged(int)),
            lcd, SLOT(display(int)));
    connect(slider, SIGNAL(valueChanged(int)),
            this, SIGNAL(valueChanged(int)));
}

int LCDRange::value() const
{
    return slider->value();
}

void LCDRange::setValue(int value)
{
    slider->setValue(value);
}
