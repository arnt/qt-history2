#include <QStyleOption>
#include <QStylePainter>
#include <QWidget>

class MyWidget : public QWidget
{
protected:
    void paintEvent(QPaintEvent *event);
    void paintEvent2(QPaintEvent *event);

};

void MyWidget::paintEvent(QPaintEvent * /* event */)
{
    QPainter painter(this);

    QStyleOptionFocusRect option;
    option.init(this);
    option.backgroundColor = palette().color(QPalette::Background);

    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter, this);
}

void MyWidget::paintEvent2(QPaintEvent * /* event */)
{
    QStylePainter painter(this);

    QStyleOptionFocusRect option;
    option.init(this);
    option.backgroundColor = palette().color(QPalette::Background);

    painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
}

int main()
{
    return 0;
}
