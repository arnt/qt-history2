#include <QtGui>

class Widget : public QWidget
{
public:
    Widget(QWidget *parent = 0);
};

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    QStringListModel *model = new QStringListModel();
    QStringList list;
    list << "a" << "b" << "c";
    model->setStringList(list);
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget widget;
    widget.show();
    return app.exec();
}
