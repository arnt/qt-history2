/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui>
#include "calculatorform.h"


CalculatorForm::CalculatorForm(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);
}

void CalculatorForm::on_inputSpinBox1_valueChanged(int value)
{
    outputWidget->setText(QString::number(value + inputSpinBox2->value()));
}

void CalculatorForm::on_inputSpinBox2_valueChanged(int value)
{
    outputWidget->setText(QString::number(value + inputSpinBox1->value()));
}