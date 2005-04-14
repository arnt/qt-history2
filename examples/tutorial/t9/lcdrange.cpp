/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 9
**
****************************************************************/

#include <QLCDNumber>
#include <QSlider>
#include <QVBoxLayout>

#include "lcdrange.h"

LCDRange::LCDRange(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QLCDNumber *lcd = new QLCDNumber(2);
    layout->addWidget(lcd);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 99);
    slider->setValue(0);
    layout->addWidget(slider);

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
