/****************************************************************
**
** Qt tutorial 7
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QGridWidget>
#include <QLCDNumber>
#include <QPushButton>
#include <QVBoxWidget>

#include "lcdrange.h"

class MyWidget : public QVBoxWidget
{
public:
    MyWidget(QWidget *parent = 0);
};

MyWidget::MyWidget(QWidget *parent)
    : QVBoxWidget(parent)
{
    QPushButton *quit = new QPushButton("Quit", this);
    quit->setFont(QFont("Times", 18, QFont::Bold));

    connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    QGridWidget *grid = new QGridWidget(4, this);
    LCDRange *previousRange = 0;

    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            LCDRange *lcdRange = new LCDRange(grid);
            if (previousRange)
                connect(lcdRange, SIGNAL(valueChanged(int)),
                        previousRange, SLOT(setValue(int)));
            previousRange = lcdRange;
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    widget.show();
    return app.exec();
}
