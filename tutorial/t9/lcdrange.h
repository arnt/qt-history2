/****************************************************************
**
** Definition of LCDRange class, Qt tutorial 9
**
****************************************************************/

#ifndef LCDRANGE_H
#define LCDRANGE_H

#include <QVBoxWidget>

class QSlider;

class LCDRange : public QVBoxWidget
{
    Q_OBJECT

public:
    LCDRange(QWidget *parent = 0);

    int value() const;

public slots:
    void setValue(int value);
    void setRange(int minValue, int maxValue);

signals:
    void valueChanged(int newValue);

private:
    QSlider *slider;
};

#endif // LCDRANGE_H
