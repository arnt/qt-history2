#include <QtTest>

class TestQString: public QObject
{
    Q_OBJECT

private slots:
    void toUpper_data(QtTestTable &t);
    void toUpper();
};

void TestQString::toUpper_data(QtTestTable &t)
{
    t.defineElement("QString", "string");
    t.defineElement("QString", "result");

    *t.newData("all lower") << "hello" << "HELLO";
    *t.newData("mixed")     << "Hello" << "HELLO";
    *t.newData("all upper") << "HELLO" << "HELLO";
}

void TestQString::toUpper()
{
    FETCH(QString, string);
    FETCH(QString, result);

    COMPARE(string.toUpper(), result);
}

QTTEST_MAIN(TestQString)
#include "testqstring.moc"

