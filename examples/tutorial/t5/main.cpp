/****************************************************************
**
** Qt tutorial 5
**
****************************************************************/

#include <QApplication>
#include <QFont>
#include <QLCDNumber>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QWidget>

class MyWidget : public QWidget
{
public:
    MyWidget(QWidget *parent = 0);
};

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout;
    setLayout(layout);

    QPushButton *quit = new QPushButton("Quit");
    quit->setFont(QFont("Times", 18, QFont::Bold));
    layout->addWidget(quit);

    connect(quit, SIGNAL(clicked()), qApp, SLOT(quit()));

    QLCDNumber *lcd = new QLCDNumber(2);
    layout->addWidget(lcd);

    QSlider *slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 99);
    slider->setValue(0);
    layout->addWidget(slider);

    connect(slider, SIGNAL(valueChanged(int)),
            lcd, SLOT(display(int)));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    widget.show();
    return app.exec();
}
