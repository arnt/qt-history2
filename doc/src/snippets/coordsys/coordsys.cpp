#include <QtGui>

class MyWidget : public QWidget
{
public:
    MyWidget();

protected:
    void paintEvent(QPaintEvent *);
};

MyWidget::MyWidget()
{
    QPalette palette(MyWidget::palette());
    palette.setColor(backgroundRole(), Qt::white);
    setPalette(palette);
}

void MyWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::darkGray);
    painter.drawRect(1, 2, 4, 3);
    painter.setPen(Qt::lightGray);
    painter.drawLine(9, 2, 7, 7);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyWidget widget;
    app.setMainWidget(&widget);
    widget.show();
    return app.exec();
}
