#include <QtGui>

#include "customstyle.h"

int main(int argc, char *argv[])
{
    QApplication::setStyle(new CustomStyle);
    QApplication app(argc, argv);
    QSpinBox spinBox;
    app.setMainWidget(&spinBox);
    spinBox.show();
    return app.exec();
}
