/****************************************************************
**
** Qt tutorial 6
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QGridWidget>
#include <QLCDNumber>
#include <QPushButton>
#include <QSlider>
#include <QVBoxWidget>

class LCDRange : public QVBoxWidget
{
public:
    LCDRange(QWidget *parent = 0);
};

LCDRange::LCDRange(QWidget *parent)
    : QVBoxWidget(parent)
{
    QLCDNumber *lcd = new QLCDNumber(2, this);
    QSlider *slider = new QSlider(Qt::Horizontal, this);
    slider->setRange(0, 99);
    slider->setValue(0);
    connect(slider, SIGNAL(valueChanged(int)),
            lcd, SLOT(display(int)));
}

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
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            (void) new LCDRange(grid);
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    app.setMainWidget(&widget);
    widget.show();
    return app.exec();
}
