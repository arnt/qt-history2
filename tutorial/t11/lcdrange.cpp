/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 11
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

    setFocusProxy(slider);
}

int LCDRange::value() const
{
    return slider->value();
}

void LCDRange::setValue(int value)
{
    slider->setValue(value);
}

void LCDRange::setRange(int minValue, int maxValue)
{
    if (minValue < 0 || maxValue > 99 || minValue > maxValue) {
        qWarning("LCDRange::setRange(%d, %d)\n"
                 "\tRange must be 0..99\n"
                 "\tand minValue must not be greater than maxValue",
                 minValue, maxValue);
        return;
    }
    slider->setRange(minValue, maxValue);
}
