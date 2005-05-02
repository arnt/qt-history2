/****************************************************************
**
** Qt tutorial 9
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QGridLayout>
#include <QLCDNumber>
#include <QPushButton>

#include "cannonfield.h"
#include "lcdrange.h"

class MyWidget : public QWidget
{
public:
    MyWidget(QWidget *parent = 0);
};

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
{
    QPushButton *quit = new QPushButton("Quit");
    quit->setFont(QFont("Times", 18, QFont::Bold));

    connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    LCDRange *angle = new LCDRange;
    angle->setRange(5, 70);

    CannonField *cannonField = new CannonField;

    connect(angle, SIGNAL(valueChanged(int)),
            cannonField, SLOT(setAngle(int)));
    connect(cannonField, SIGNAL(angleChanged(int)),
            angle, SLOT(setValue(int)));

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(quit, 0, 0);
    gridLayout->addWidget(angle, 1, 0);
    gridLayout->addWidget(cannonField, 1, 1, 2, 1);
    gridLayout->setColumnStretch(1, 10);
    setLayout(gridLayout);

    angle->setValue(60);
    angle->setFocus();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    widget.setGeometry(100, 100, 500, 355);
    widget.show();
    return app.exec();
}
