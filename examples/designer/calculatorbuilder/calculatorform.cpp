#include <QtGui/QtGui>
#include <QtDesigner/QFormBuilder>

#include "calculatorform.h"

CalculatorForm::CalculatorForm(QWidget *parent)
    : QWidget(parent)
{
    QFormBuilder builder;
    QFile file(":/forms/calculatorform.ui");
    file.open(QFile::ReadOnly);
    QWidget *formWidget = builder.load(&file, this);
    file.close();

    ui_inputSpinBox1 = qFindChild<QSpinBox*>(this, "inputSpinBox1");
    ui_inputSpinBox2 = qFindChild<QSpinBox*>(this, "inputSpinBox2");
    ui_outputWidget = qFindChild<QLabel*>(this, "outputWidget");

    QMetaObject::connectSlotsByName(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(formWidget);

    setWindowTitle(tr("Calculator Builder"));
}

void CalculatorForm::on_inputSpinBox1_valueChanged(int value)
{
    ui_outputWidget->setText(QString::number(value + ui_inputSpinBox2->value()));
}

void CalculatorForm::on_inputSpinBox2_valueChanged(int value)
{
    ui_outputWidget->setText(QString::number(value + ui_inputSpinBox1->value()));
}
