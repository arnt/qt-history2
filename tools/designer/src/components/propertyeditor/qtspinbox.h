#ifndef QTSPINBOX_H
#define QTSPINBOX_H

#include <QSpinBox>

class QtSpinBox : public QSpinBox
{
    Q_OBJECT
public:
    QtSpinBox(QWidget *parent = 0);

    void stepBy(int steps);
};

class QtDoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    QtDoubleSpinBox(QWidget *parent = 0);
    void fixup(QString &input) const;

    void stepBy(int steps);
};

#endif
