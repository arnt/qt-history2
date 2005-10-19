#include <QtTest>

class TestQString: public QObject
{
    Q_OBJECT
private slots:
    void toUpper();
};

void TestQString::toUpper()
{
    QString str = "Hello";
    COMPARE(str.toUpper(), QString("HELLO"));
}

QTTEST_MAIN(TestQString)
#include "testqstring.moc"

