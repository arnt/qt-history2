/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 7
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
}

int LCDRange::value() const
{
    return slider->value();
}

void LCDRange::setValue(int value)
{
    slider->setValue(value);
}
