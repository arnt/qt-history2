/****************************************************************
**
** Implementation of LCDRange class, Qt tutorial 13
**
****************************************************************/

#include <QLCDNumber>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

#include "lcdrange.h"

LCDRange::LCDRange(QWidget *parent)
    : QWidget(parent)
{
    init();
}

LCDRange::LCDRange(const QString &text, QWidget *parent)
    : QWidget(parent)
{
    init();
    setText(text);
}

void LCDRange::init()
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QLCDNumber *lcd = new QLCDNumber(2);
    layout->addWidget(lcd);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 99);
    slider->setValue(0);
    layout->addWidget(slider);

    label = new QLabel(this);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    layout->addWidget(label);

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

QString LCDRange::text() const
{
    return label->text();
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

void LCDRange::setText(const QString &text)
{
    label->setText(text);
}
