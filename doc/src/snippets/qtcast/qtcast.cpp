#include <QLabel>
#include <QPushButton>

#include "qtcast.h"

MyWidget::MyWidget()
{
    QObject *obj = new MyWidget;

    QWidget *widget = qt_cast<QWidget *>(obj);

    MyWidget *myWidget = qt_cast<MyWidget *>(obj);

    QLabel *label = qt_cast<QLabel *>(obj);
    // label is 0

    if (QLabel *label = qt_cast<QLabel *>(obj)) {
        label->setText(tr("Ping"));
    } else if (QPushButton *button = qt_cast<QPushButton *>(obj)) {
        button->setText(tr("Pong!"));
    }
}

int main()
{
    return 0;
}
