#include <QLabel>
#include <QPushButton>

#include "qtcast.h"

MyWidget::MyWidget()
{
    QObject *obj = new MyWidget;

    QWidget *widget = qobject_cast<QWidget *>(obj);

    MyWidget *myWidget = qobject_cast<MyWidget *>(obj);

    QLabel *label = qobject_cast<QLabel *>(obj);
    // label is 0

    if (QLabel *label = qobject_cast<QLabel *>(obj)) {
        label->setText(tr("Ping"));
    } else if (QPushButton *button = qobject_cast<QPushButton *>(obj)) {
        button->setText(tr("Pong!"));
    }
}

int main()
{
    return 0;
}
