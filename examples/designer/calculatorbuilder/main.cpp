/****************************************************************************
**
** Copyright (C) 2005-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>

#include "calculatorform.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(calculatorbuilder);

    QApplication app(argc, argv);
    CalculatorForm *calculator = new CalculatorForm;
    calculator->show();
    return app.exec();
}
