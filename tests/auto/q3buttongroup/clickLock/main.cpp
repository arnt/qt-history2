#include <QtGui>
#include <Qt3Support>
#include <QtTest/QtTest>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    Q3ButtonGroup bg;
    bg.setColumnLayout(0, Qt::Vertical);
    QHBoxLayout* layout = new QHBoxLayout;
    QBoxLayout* oldbox = qobject_cast<QBoxLayout*>(bg.layout());
    if (oldbox)
        oldbox->addLayout(layout);

    const int buttonCount = 7;
    QPushButton* buttons[buttonCount];
    for (int i = 0; i <= buttonCount; ++i) {
        buttons[i] = new QPushButton(QString::number(i), &bg);
        layout->addWidget(buttons[i]);
    }
    bg.insert(buttons[buttonCount - 1] , buttonCount);

    bg.show();

    int clickCount = 10;
    for (int i = 0; i < clickCount; ++i) {
        QTest::mouseClick(buttons[buttonCount - 1], Qt::LeftButton);
        QApplication::processEvents();
    }
}
