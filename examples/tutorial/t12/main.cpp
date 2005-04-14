/****************************************************************
**
** Qt tutorial 12
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLCDNumber>
#include <QPushButton>
#include <QVBoxLayout>

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
    QPushButton *quit = new QPushButton("&Quit", this);
    quit->setFont(QFont("Times", 18, QFont::Bold));

    connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    LCDRange *angle = new LCDRange("ANGLE", this);
    angle->setRange(5, 70);

    LCDRange *force = new LCDRange("FORCE", this);
    force->setRange(10, 50);

    CannonField *cannonField = new CannonField(this);

    connect(angle, SIGNAL(valueChanged(int)),
            cannonField, SLOT(setAngle(int)));
    connect(cannonField, SIGNAL(angleChanged(int)),
            angle, SLOT(setValue(int)));

    connect(force, SIGNAL(valueChanged(int)),
            cannonField, SLOT(setForce(int)));
    connect(cannonField, SIGNAL(forceChanged(int)),
            force, SLOT(setValue(int)));

    QPushButton *shoot = new QPushButton("&Shoot", this);
    shoot->setFont(QFont("Times", 18, QFont::Bold));

    connect(shoot, SIGNAL(clicked()), cannonField, SLOT(shoot()));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(shoot);
    topLayout->addStretch(1);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(angle);
    leftLayout->addWidget(force);

    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(quit, 0, 0);
    gridLayout->addLayout(topLayout, 0, 1);
    gridLayout->addLayout(leftLayout, 1, 0);
    gridLayout->addWidget(cannonField, 1, 1, 2, 1);
    gridLayout->setColumnStretch(1, 10);

    angle->setValue(60);
    force->setValue(25);
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
