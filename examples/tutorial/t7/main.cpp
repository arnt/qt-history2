/****************************************************************
**
** Qt tutorial 7
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QGridLayout>
#include <QLCDNumber>
#include <QPushButton>
#include <QWidget>

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

    QGridLayout *grid = new QGridLayout;
    LCDRange *previousRange = 0;

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            LCDRange *lcdRange = new LCDRange;
            grid->addWidget(lcdRange, row, column);
            if (previousRange)
                connect(lcdRange, SIGNAL(valueChanged(int)),
                        previousRange, SLOT(setValue(int)));
            previousRange = lcdRange;
        }
    }
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(quit);
    layout->addLayout(grid);
    setLayout(layout);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    widget.show();
    return app.exec();
}
