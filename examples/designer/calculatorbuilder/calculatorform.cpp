#include <QtGui>
#include <formbuilder.h>

#include "calculatorform.h"

CalculatorForm::CalculatorForm(QWidget *parent)
    : QVBoxWidget(parent)
{
    FormBuilder builder;
    QFile file(":/forms/calculatorform.ui");
    file.open(QFile::ReadOnly);
    QWidget *widget = builder.load(&file, this);
    file.close();

    ui_inputSpinBox1 = qFindChild<QSpinBox*>(this, "inputSpinBox1");
    ui_inputSpinBox2 = qFindChild<QSpinBox*>(this, "inputSpinBox2");
    ui_resultLabel = qFindChild<QLabel*>(this, "resultLabel");

    QMetaObject::connectSlotsByName(this);
    setWindowTitle(widget->windowTitle());
}

void CalculatorForm::on_inputSpinBox1_valueChanged(int value)
{
    ui_resultLabel->setText(QString::number(value + ui_inputSpinBox2->value()));
}

void CalculatorForm::on_inputSpinBox2_valueChanged(int value)
{
    ui_resultLabel->setText(QString::number(value + ui_inputSpinBox1->value()));
}
