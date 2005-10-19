#include <QtGui>
#include <QtTest>

class TestGui: public QObject
{
    Q_OBJECT

private slots:
    void testGui_data(QtTestTable &t);
    void testGui();
};

void TestGui::testGui_data(QtTestTable &t)
{
    t.defineElement("QtTestEventList", "events");
    t.defineElement("QString", "expected");

    QtTestEventList list1;
    list1.addKeyClick('a');
    *t.newData("char") << list1 << "a";

    QtTestEventList list2;
    list2.addKeyClick('a');
    list2.addKeyClick(Qt::Key_Backspace);
    *t.newData("there and back again") << list2 << "";
}

void TestGui::testGui()
{
    FETCH(QtTestEventList, events);
    FETCH(QString, expected);

    QLineEdit lineEdit;

    events.simulate(&lineEdit);

    COMPARE(lineEdit.text(), expected);
}

QTTEST_MAIN(TestGui)
#include "testgui.moc"

