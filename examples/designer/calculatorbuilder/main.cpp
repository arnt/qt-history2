#include <QApplication>

#include "calculatorform.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    CalculatorForm *calculator = new CalculatorForm;
    calculator->show();
    return app.exec();
}
