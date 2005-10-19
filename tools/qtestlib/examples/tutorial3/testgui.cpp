#include <QtGui>
#include <QtTest>

class TestGui: public QObject
{
    Q_OBJECT

private slots:
    void testGui();

};

void TestGui::testGui()
{
    QLineEdit lineEdit;

    QtTest::keyClicks(&lineEdit, "hello world");

    COMPARE(lineEdit.text(), QString("hello world"));
}

QTTEST_MAIN(TestGui)
#include "testgui.moc"

