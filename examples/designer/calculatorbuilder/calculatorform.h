#ifndef CALCULATORFORM_H
#define CALCULATORFORM_H

#include <QVBoxWidget>

class QLabel;
class QSpinBox;

class CalculatorForm : public QWidget
{
    Q_OBJECT

public:
    CalculatorForm(QWidget *parent = 0);

private slots:
    void on_inputSpinBox1_valueChanged(int value);
    void on_inputSpinBox2_valueChanged(int value);

private:
    QSpinBox *ui_inputSpinBox1;
    QSpinBox *ui_inputSpinBox2;
    QLabel *ui_outputWidget;
};

#endif
