#include <QtGui>
#include <QApplication>

class MyPushButton : public QPushButton
{
public:
    MyPushButton(QWidget *parent = 0);

    void paintEvent(QPaintEvent *);
};

MyPushButton::MyPushButton(QWidget *parent)
    : QPushButton(parent)
{
}

void MyPushButton::paintEvent(QPaintEvent *)
{
    QStyleOptionButton option;
    option.initFrom(this);
    option.state = isDown() ? QStyle::State_Sunken : QStyle::State_Raised;
    if (isDefault())
        option.features |= QStyleOptionButton::DefaultButton;
    option.text = text();
    option.icon = icon();

    QPainter painter(this);
    style()->drawControl(QStyle::CE_PushButton, &option, &painter, this);
}



class MyStyle : public QStyle
{
public:
    MyStyle();

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget);
};

MyStyle::MyStyle()
{
}

void MyStyle::drawPrimitive(PrimitiveElement element,
                            const QStyleOption *option,
                            QPainter *painter,
                            const QWidget *widget)
{
    if (element == PE_FrameFocusRect) {
        const QStyleOptionFocusRect *focusRectOption =
                qstyleoption_cast<const QStyleOptionFocusRect *>(option);
        if (focusRectOption) {
            // ...
        }
    }
    // ...
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MyPushButton button;
    button.show();
    return app.exec();
}
