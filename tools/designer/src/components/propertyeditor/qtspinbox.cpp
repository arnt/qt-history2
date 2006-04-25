#include "qtspinbox.h"

#include "qdebug.h"

QtSpinBox::QtSpinBox(QWidget *parent)
    : QSpinBox(parent)
{
}

void QtSpinBox::stepBy(int steps)
{
    QSpinBox::stepBy(steps);
    emit editingFinished();
}

QtDoubleSpinBox::QtDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{
}

void QtDoubleSpinBox::stepBy(int steps)
{
    QDoubleSpinBox::stepBy(steps);
    emit editingFinished();
}

void QtDoubleSpinBox::fixup(QString &input) const
{
    QDoubleSpinBox::fixup(input);
    double val = valueFromText(input);
    input = textFromValue(val);
}

