#include <QAbstractButton>

#include "signalsandslots.h"

void Counter::setValue(int value)
{
    if (value != m_value) {
        m_value = value;
        emit valueChanged(value);
    }
}

int main()
{
    Counter a, b;
    QObject::connect(&a, SIGNAL(valueChanged(int)),
                     &b, SLOT(setValue(int)));

    a.setValue(12);     // a.value() == 12, b.value() == 12
    b.setValue(48);     // a.value() == 12, b.value() == 48


    QWidget *widget = reinterpret_cast<QWidget *>(new QObject(0));
    if (widget->inherits("QAbstractButton")) {
        QAbstractButton *button = static_cast<QAbstractButton *>(widget);
        button->toggle();
    }

    if (QAbstractButton *button = qobject_cast<QAbstractButton *>(widget))
        button->toggle();
}
